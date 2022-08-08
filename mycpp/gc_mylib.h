// gc_mylib.h
//
// Rewrites of mylib.py in C++.
// TODO: Remove mylib.{h,cc}, and make this the main copy.

#ifndef GC_MYLIB_H
#define GC_MYLIB_H

#include "mycpp/gc_builtins.h"  // Tuple2
#include "mycpp/gc_containers.h"







namespace mylib {

Tuple2<Str*, Str*> split_once(Str* s, Str* delim);

template <typename V>
void dict_remove(Dict<Str*, V>* haystack, Str* needle) {
  int pos = haystack->position_of_key(needle);
  if (pos == -1) {
    return;
  }
  haystack->entry_->items_[pos] = kDeletedEntry;
  // Zero out for GC.  These could be nullptr or 0
  haystack->keys_->items_[pos] = 0;
  haystack->values_->items_[pos] = 0;
  haystack->len_--;
}

class LineReader : Obj {
 public:
  // Abstract type with no fields: unknown size
  LineReader(uint16_t field_mask, int obj_len)
      : Obj(Tag::FixedSize, field_mask, obj_len) {
  }
  virtual Str* readline() = 0;
  virtual bool isatty() {
    return false;
  }
  virtual int fileno() {
    NotImplemented();  // Uncalled
  }
};

class BufLineReader : public LineReader {
 public:
  explicit BufLineReader(Str* s);
  virtual Str* readline();

  Str* s_;
  int pos_;

  DISALLOW_COPY_AND_ASSIGN(BufLineReader)
};

constexpr uint16_t maskof_BufLineReader() {
  return maskbit_v(offsetof(BufLineReader, s_));
}

inline BufLineReader::BufLineReader(Str* s)
    : LineReader(maskof_BufLineReader(), sizeof(BufLineReader)),
      s_(s),
      pos_(0) {
}

// Wrap a FILE*
class CFileLineReader : public LineReader {
 public:
  explicit CFileLineReader(FILE* f);
  virtual Str* readline();
  virtual int fileno() {
    return ::fileno(f_);
  }

 private:
  FILE* f_;

  DISALLOW_COPY_AND_ASSIGN(CFileLineReader)
};

inline CFileLineReader::CFileLineReader(FILE* f)
    : LineReader(kZeroMask, sizeof(CFileLineReader)), f_(f) {
}

extern LineReader* gStdin;

inline LineReader* Stdin() {
  if (gStdin == nullptr) {
    gStdin = new CFileLineReader(stdin);
  }
  return gStdin;
}

inline LineReader* open(Str* path) {
  StackRoots _roots({&path});

  FILE* f = fopen(path->data_, "r");

  // TODO: Better error checking.  IOError?
  if (!f) {
    throw new AssertionError("file not found");
  }
  return Alloc<CFileLineReader>(f);
}

class Writer : public Obj {
 public:
  Writer(uint8_t heap_tag, uint16_t field_mask, int obj_len)
      : Obj(heap_tag, field_mask, obj_len) {
  }
  virtual void write(Str* s) = 0;
  virtual void flush() = 0;
  virtual bool isatty() = 0;
};

class BufWriter : public Writer {
 public:
  BufWriter()
      : Writer(Tag::FixedSize, kZeroMask, sizeof(BufWriter)),
        data_(nullptr),
        len_(0) {
  }
  virtual void write(Str* s) override;
  virtual void flush() override {
  }
  virtual bool isatty() override {
    return false;
  }
  // For cStringIO API
  Str* getvalue() {
    if (data_) {
      Str* ret = StrFromC(data_, len_);
      reset();  // Invalidate this instance
      return ret;
    } else {
      // log('') translates to this
      // Strings are immutable so we can do this.
      return kEmptyString;
    }
  }

  // Methods to compile printf format strings to

  // To reuse the global gBuf instance
  // problem: '%r' % obj will recursively call asdl/format.py, which has its
  // own % operations
  void reset() {
    data_ = nullptr;  // make sure we get a new buffer next time
    len_ = 0;
  }

  // Note: we do NOT need to instantiate a Str() to append
  void write_const(const char* s, int len);

  // strategy: snprintf() based on sizeof(int)
  void format_d(int i);
  void format_s(Str* s);
  void format_r(Str* s);  // formats with quotes

  // looks at arbitrary type tags?  Is this possible
  // Passes "this" to functions generated by ASDL?
  void format_r(void* s);

 private:
  // Just like a string, except it's mutable
  char* data_;
  int len_;
};

// Wrap a FILE*
class CFileWriter : public Writer {
 public:
  explicit CFileWriter(FILE* f)
      : Writer(Tag::FixedSize, kZeroMask, sizeof(BufWriter)), f_(f) {
  }
  virtual void write(Str* s) override;
  virtual void flush() override;
  virtual bool isatty() override;

 private:
  FILE* f_;

  DISALLOW_COPY_AND_ASSIGN(CFileWriter)
};

extern Writer* gStdout;

inline Writer* Stdout() {
  if (gStdout == nullptr) {
    gStdout = new CFileWriter(stdout);
  }
  return gStdout;
}

extern Writer* gStderr;

inline Writer* Stderr() {
  if (gStderr == nullptr) {
    gStderr = new CFileWriter(stderr);
  }
  return gStderr;
}

}  // namespace mylib

// Global formatter
extern mylib::BufWriter gBuf;

#endif  // GC_MYLIB_H
