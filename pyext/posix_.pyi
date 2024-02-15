# Copied from
# /home/andy/.local/lib/python3.5/site-packages/mypy/typeshed/stdlib/2/posix.pyi

from mycpp import mylib
from typing import Dict, List, Mapping, Tuple, Union, Sequence, IO, Optional, TypeVar

error = OSError

confstr_names = ...  # type: Dict[str, int]
environ = ...  # type: Dict[str, str]
pathconf_names = ...  # type: Dict[str, int]
sysconf_names = ...  # type: Dict[str, int]

EX_CANTCREAT = ...  # type: int
EX_CONFIG = ...  # type: int
EX_DATAERR = ...  # type: int
EX_IOERR = ...  # type: int
EX_NOHOST = ...  # type: int
EX_NOINPUT = ...  # type: int
EX_NOPERM = ...  # type: int
EX_NOUSER = ...  # type: int
EX_OK = ...  # type: int
EX_OSERR = ...  # type: int
EX_OSFILE = ...  # type: int
EX_PROTOCOL = ...  # type: int
EX_SOFTWARE = ...  # type: int
EX_TEMPFAIL = ...  # type: int
EX_UNAVAILABLE = ...  # type: int
EX_USAGE = ...  # type: int
F_OK = ...  # type: int
NGROUPS_MAX = ...  # type: int
O_APPEND = ...  # type: int
O_ASYNC = ...  # type: int
O_CREAT = ...  # type: int
O_DIRECT = ...  # type: int
O_DIRECTORY = ...  # type: int
O_DSYNC = ...  # type: int
O_EXCL = ...  # type: int
O_LARGEFILE = ...  # type: int
O_NDELAY = ...  # type: int
O_NOATIME = ...  # type: int
O_NOCTTY = ...  # type: int
O_NOFOLLOW = ...  # type: int
O_NONBLOCK = ...  # type: int
O_RDONLY = ...  # type: int
O_RDWR = ...  # type: int
O_RSYNC = ...  # type: int
O_SYNC = ...  # type: int
O_TRUNC = ...  # type: int
O_WRONLY = ...  # type: int
R_OK = ...  # type: int
TMP_MAX = ...  # type: int
WCONTINUED = ...  # type: int
WNOHANG = ...  # type: int
WUNTRACED = ...  # type: int
W_OK = ...  # type: int
X_OK = ...  # type: int

def WCOREDUMP(status: int) -> int: ...
def WEXITSTATUS(status: int) -> int: ...
def WIFCONTINUED(status: int) -> bool: ...
def WIFEXITED(status: int) -> bool: ...
def WIFSIGNALED(status: int) -> bool: ...
def WIFSTOPPED(status: int) -> bool: ...
def WSTOPSIG(status: int) -> int: ...
def WTERMSIG(status: int) -> int: ...

class stat_result(object):
    n_fields = ...  # type: int
    n_sequence_fields = ...  # type: int
    n_unnamed_fields = ...  # type: int
    st_mode = ...  # type: int
    st_ino = ...  # type: int
    st_dev = ...  # type: int
    st_nlink = ...  # type: int
    st_uid = ...  # type: int
    st_gid = ...  # type: int
    st_size = ...  # type: int
    st_atime = ...  # type: int
    st_mtime = ...  # type: int
    st_ctime = ...  # type: int

class statvfs_result(object):
    n_fields = ...  # type: int
    n_sequence_fields = ...  # type: int
    n_unnamed_fields = ...  # type: int
    f_bsize = ...  # type: int
    f_frsize = ...  # type: int
    f_blocks = ...  # type: int
    f_bfree = ...  # type: int
    f_bavail = ...  # type: int
    f_files = ...  # type: int
    f_ffree = ...  # type: int
    f_favail = ...  # type: int
    f_flag = ...  # type: int
    f_namemax = ...  # type: int

def _exit(status: int) -> None: ...
def abort() -> None: ...
def access(path: unicode, mode: int) -> bool: ...
def chdir(path: unicode) -> None: ...
def chmod(path: unicode, mode: int) -> None: ...
def chown(path: unicode, uid: int, gid: int) -> None: ...
def chroot(path: unicode) -> None: ...
def close(fd: int) -> None: ...
def closerange(fd_low: int, fd_high: int) -> None: ...
def confstr(name: Union[str, int]) -> str: ...
def ctermid() -> str: ...
def dup(fd: int) -> int: ...
def dup2(fd: int, fd2: int) -> None: ...
def execv(path: str, args: Sequence[str], env: Mapping[str, str]) -> None: ...
def execve(path: str, args: Sequence[str], env: Mapping[str, str]) -> None: ...
def fchdir(fd: int) -> None: ...
def fchmod(fd: int, mode: int) -> None: ...
def fchown(fd: int, uid: int, gid: int) -> None: ...
def fdatasync(fd: int) -> None: ...

# Oil patch: IO[str] -> mylib.LineReader
#def fdopen(fd: int, mode: str = ..., bufsize: int = ...) -> mylib.LineReader: ...
def fdopen(fd: int, mode: str = ..., bufsize: int = ...) -> IO[str]: ...
def fork() -> int:
    raise OSError()
def forkpty() -> Tuple[int, int]:
    raise OSError()
def fpathconf(fd: int, name: str) -> None: ...
def fstat(fd: int) -> stat_result: ...
def fstatvfs(fd: int) -> statvfs_result: ...
def fsync(fd: int) -> None: ...
def ftruncate(fd: int, length: int) -> None: ...
def getcwd() -> str: ...
def getcwdu() -> unicode: ...
def getegid() -> int: ...
def geteuid() -> int: ...
def getgid() -> int: ...
def getgroups() -> List[int]: ...
def getloadavg() -> Tuple[float, float, float]:
    raise OSError()
def getlogin() -> str: ...
def getpgid(pid: int) -> int: ...
def getpgrp() -> int: ...
def getpid() -> int: ...
def getppid() -> int: ...
def getresgid() -> Tuple[int, int, int]: ...
def getresuid() -> Tuple[int, int, int]: ...
def getsid(pid: int) -> int: ...
def getuid() -> int: ...
def initgroups(username: str, gid: int) -> None: ...
def isatty(fd: int) -> bool: ...
def kill(pid: int, sig: int) -> None: ...
def killpg(pgid: int, sig: int) -> None: ...
def lchown(path: unicode, uid: int, gid: int) -> None: ...
def link(source: unicode, link_name: str) -> None: ...
_T = TypeVar("_T")
def listdir(path: _T) -> List[_T]: ...
def lseek(fd: int, pos: int, how: int) -> None: ...
def lstat(path: unicode) -> stat_result: ...
def major(device: int) -> int: ...
def makedev(major: int, minor: int) -> int: ...
def minor(device: int) -> int: ...
def mkdir(path: unicode, mode: int = ...) -> None: ...
def mkfifo(path: unicode, mode: int = ...) -> None: ...
def mknod(filename: unicode, mode: int = ..., device: int = ...) -> None: ...
def nice(increment: int) -> int: ...
def open(file: unicode, flags: int, mode: int = ...) -> int: ...
def openpty() -> Tuple[int, int]: ...
def pathconf(path: unicode, name: str) -> str: ...
def pipe() -> Tuple[int, int]: ...
def popen(command: str, mode: str = ..., bufsize: int = ...) -> IO[str]: ...
def putenv(varname: str, value: str) -> None: ...
def read(fd: int, n: int) -> str: ...
def readlink(path: _T) -> _T: ...
def remove(path: unicode) -> None: ...
def rename(src: unicode, dst: unicode) -> None: ...
def rmdir(path: unicode) -> None: ...
def setegid(egid: int) -> None: ...
def seteuid(euid: int) -> None: ...
def setgid(gid: int) -> None: ...
def setgroups(groups: Sequence[int]) -> None: ...
def setpgid(pid: int, pgrp: int) -> None: ...
def setpgrp() -> None: ...
def setregid(rgid: int, egid: int) -> None: ...
def setresgid(rgid: int, egid: int, sgid: int) -> None: ...
def setresuid(ruid: int, euid: int, suid: int) -> None: ...
def setreuid(ruid: int, euid: int) -> None: ...
def setsid() -> None: ...
def setuid(pid: int) -> None: ...
def stat(path: unicode) -> stat_result: ...
def statvfs(path: unicode) -> statvfs_result: ...
def stat_float_times(fd: int) -> None: ...
def strerror(code: int) -> str: ...
def symlink(source: unicode, link_name: unicode) -> None: ...
def sysconf(name: Union[str, int]) -> int: ...
def system(command: unicode) -> int: ...
def tcgetpgrp(fd: int) -> int: ...
def tcsetpgrp(fd: int, pg: int) -> None: ...
def times() -> Tuple[float, float, float, float, float]: ...
def tmpfile() -> IO[str]: ...
def ttyname(fd: int) -> str: ...
def umask(mask: int) -> int: ...
def uname() -> Tuple[str, str, str, str, str]: ...
def unlink(path: unicode) -> None: ...
def unsetenv(varname: str) -> None: ...
def urandom(n: int) -> str: ...
def utime(path: unicode, times: Optional[Tuple[int, int]]) -> None:
    raise OSError
def wait() -> int: ...
_r = Tuple[float, float, int, int, int, int, int, int, int, int, int, int, int, int, int, int]
def wait3(options: int) -> Tuple[int, int, _r]: ...
def wait4(pid: int, options: int) -> Tuple[int, int, _r]: ...
def waitpid(pid: int, options: int) -> Tuple[int, int]:
    raise OSError()
def write(fd: int, str: str) -> int: ...
