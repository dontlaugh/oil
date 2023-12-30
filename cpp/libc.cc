// libc.cc: Replacement for pyext/libc.c

#include "cpp/libc.h"

#include <errno.h>
#include <fnmatch.h>
#include <glob.h>
#include <locale.h>
#include <stdlib.h>  // getenv()
#include <sys/ioctl.h>
#include <unistd.h>  // gethostname()
#include <wchar.h>

#include <algorithm>

namespace libc {

RegexCache::CacheEntry::CacheEntry(BigStr* pat, int cflags) : pat_() {
  int status = ::regcomp(&compiled_, pat->data_, cflags);
  if (status != 0) {
    char error_desc[50];
    regerror(status, &compiled_, error_desc, 50);

    char error_message[80];
    snprintf(error_message, 80, "Invalid regex %s (%s)", pat->data_,
             error_desc);

    throw Alloc<ValueError>(StrFromC(error_message));
  }

  pat_ = static_cast<char*>(malloc(len(pat) + 1));
  memcpy(pat_, pat->data_, len(pat) + 1);
  pat_hash_ = hash(pat);
}

RegexCache::CacheEntry::~CacheEntry() {
  DCHECK(pat_ != nullptr);
  free(pat_);
  regfree(&compiled_);
}

RegexCache::RegexCache(int capacity) : capacity_(capacity), access_list_() {
  // Override if env var is set.
  char* e = getenv("OILS_REGEX_CACHE_SIZE");
  if (e) {
    int result;
    if (StringToInteger(e, strlen(e), 10, &result)) {
      capacity_ = result;
    }
  }
}

RegexCache::~RegexCache() {
  for (auto& it : access_list_) {
    delete it;
  }
}

regex_t* RegexCache::regcomp(BigStr* pat, int cflags) {
  RegexCache::CacheEntry* entry = TakeEntry(pat);
  if (entry == nullptr) {
    // Dealing with a new entry. Make space and compile.
    MaybeEvict();
    entry = new RegexCache::CacheEntry(pat, cflags);
  }

  SetMostRecent(entry);

  return &entry->compiled_;
}

RegexCache::CacheEntry* RegexCache::TakeEntry(BigStr* pat) {
  auto it = std::find_if(access_list_.begin(), access_list_.end(),
                         [pat](RegexCache::CacheEntry* entry) {
                           return hash(pat) == entry->pat_hash_ &&
                                  strcmp(pat->data_, entry->pat_) == 0;
                         });
  if (it == access_list_.end()) {
    return nullptr;
  }

  RegexCache::CacheEntry* ret = *it;
  access_list_.erase(it);
  return ret;
}

void RegexCache::MaybeEvict() {
  if (access_list_.size() < capacity_) {
    return;
  }

  // Evict the least recently used entry.
  if (access_list_.size()) {
    delete *access_list_.begin();
    access_list_.erase(access_list_.begin());
  }
}

void RegexCache::SetMostRecent(RegexCache::CacheEntry* entry) {
  access_list_.push_back(entry);
}

RegexCache gRegexCache(RegexCache::kDefaultSize);

BigStr* gethostname() {
  // Note: Fixed issue #1656 - OS X and FreeBSD don't have HOST_NAME_MAX
  // https://reviews.freebsd.org/D30062
  BigStr* result = OverAllocatedStr(_POSIX_HOST_NAME_MAX);
  int status = ::gethostname(result->data_, _POSIX_HOST_NAME_MAX);
  if (status != 0) {
    throw Alloc<OSError>(errno);
  }
  // Important: set the length of the string!
  result->MaybeShrink(strlen(result->data_));
  return result;
}

BigStr* realpath(BigStr* path) {
  BigStr* result = OverAllocatedStr(PATH_MAX);
  char* p = ::realpath(path->data_, result->data_);
  if (p == nullptr) {
    throw Alloc<OSError>(errno);
  }
  result->MaybeShrink(strlen(result->data_));
  return result;
}

int fnmatch(BigStr* pat, BigStr* str, int flags) {
#ifdef FNM_EXTMATCH
  flags |= FNM_EXTMATCH;
#else
  // TODO: We should detect this at ./configure time, and then maybe flag these
  // at parse time, not runtime
#endif

  int result = ::fnmatch(pat->data_, str->data_, flags);
  switch (result) {
  case 0:
    return 1;
  case FNM_NOMATCH:
    return 0;
  default:
    // Other error
    return -1;
  }
}

List<BigStr*>* glob(BigStr* pat) {
  glob_t results;
  // Hm, it's weird that the first one can't be called with GLOB_APPEND.  You
  // get a segfault.
  int flags = 0;
  // int flags = GLOB_APPEND;
  // flags |= GLOB_NOMAGIC;
  int ret = glob(pat->data_, flags, NULL, &results);

  const char* err_str = NULL;
  switch (ret) {
  case 0:  // no error
    break;
  case GLOB_ABORTED:
    err_str = "read error";
    break;
  case GLOB_NOMATCH:
    // No error, because not matching isn't necessarily a problem.
    // NOTE: This can be turned on to log overaggressive calls to glob().
    // err_str = "nothing matched";
    break;
  case GLOB_NOSPACE:
    err_str = "no dynamic memory";
    break;
  default:
    err_str = "unknown problem";
    break;
  }
  if (err_str) {
    throw Alloc<RuntimeError>(StrFromC(err_str));
  }

  // http://stackoverflow.com/questions/3512414/does-this-pylist-appendlist-py-buildvalue-leak
  size_t n = results.gl_pathc;
  auto matches = NewList<BigStr*>();

  // Print array of results
  size_t i;
  for (i = 0; i < n; i++) {
    const char* m = results.gl_pathv[i];
    matches->append(StrFromC(m));
  }
  globfree(&results);

  return matches;
}

// Raises RuntimeError if the pattern is invalid.  TODO: Use a different
// exception?
List<int>* regex_search(BigStr* pattern, int cflags, BigStr* str, int eflags,
                        int pos) {
  cflags |= REG_EXTENDED;
  regex_t* compiled = gRegexCache.regcomp(pattern, cflags);
  DCHECK(compiled != nullptr);

  int num_groups = compiled->re_nsub + 1;  // number of captures

  List<int>* indices = NewList<int>();
  indices->reserve(num_groups * 2);

  const char* s = str->data_;
  regmatch_t* pmatch =
      static_cast<regmatch_t*>(malloc(sizeof(regmatch_t) * num_groups));
  bool match = regexec(compiled, s + pos, num_groups, pmatch, eflags) == 0;
  if (match) {
    int i;
    for (i = 0; i < num_groups; i++) {
      int start = pmatch[i].rm_so;
      if (start != -1) {
        start += pos;
      }
      indices->append(start);

      int end = pmatch[i].rm_eo;
      if (end != -1) {
        end += pos;
      }
      indices->append(end);
    }
  }

  free(pmatch);

  if (!match) {
    return nullptr;
  }

  return indices;
}

// For ${//}, the number of groups is always 1, so we want 2 match position
// results -- the whole regex (which we ignore), and then first group.
//
// For [[ =~ ]], do we need to count how many matches the user gave?

const int NMATCH = 2;

// Odd: This a Tuple2* not Tuple2 because it's Optional[Tuple2]!
Tuple2<int, int>* regex_first_group_match(BigStr* pattern, BigStr* str,
                                          int pos) {
  regmatch_t m[NMATCH];

  // Could have been checked by regex_parse for [[ =~ ]], but not for glob
  // patterns like ${foo/x*/y}.

  regex_t* compiled = gRegexCache.regcomp(pattern, REG_EXTENDED);
  DCHECK(compiled != nullptr);

  // Match at offset 'pos'
  int result = regexec(compiled, str->data_ + pos, NMATCH, m, 0 /*flags*/);

  if (result != 0) {
    return nullptr;
  }

  // Assume there is a match
  regoff_t start = m[1].rm_so;
  regoff_t end = m[1].rm_eo;
  Tuple2<int, int>* tup = Alloc<Tuple2<int, int>>(pos + start, pos + end);

  return tup;
}

// TODO: SHARE with pyext
int wcswidth(BigStr* s) {
  // Behavior of mbstowcs() depends on LC_CTYPE

  // Calculate length first
  int num_wide_chars = mbstowcs(NULL, s->data_, 0);
  if (num_wide_chars == -1) {
    throw Alloc<UnicodeError>(StrFromC("mbstowcs() 1"));
  }

  // Allocate buffer
  int buf_size = (num_wide_chars + 1) * sizeof(wchar_t);
  wchar_t* wide_chars = static_cast<wchar_t*>(malloc(buf_size));
  assert(wide_chars != nullptr);

  // Convert to wide chars
  num_wide_chars = mbstowcs(wide_chars, s->data_, num_wide_chars);
  if (num_wide_chars == -1) {
    throw Alloc<UnicodeError>(StrFromC("mbstowcs() 2"));
  }

  // Find number of columns
  int width = ::wcswidth(wide_chars, num_wide_chars);
  if (width == -1) {
    // unprintable chars
    throw Alloc<UnicodeError>(StrFromC("wcswidth()"));
  }
  free(wide_chars);

  return width;
}

int get_terminal_width() {
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
    throw Alloc<IOError>(errno);
  }
  return w.ws_col;
}

}  // namespace libc
