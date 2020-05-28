#include "fs_util.h"

#include <stdio.h>
#include <fstream>

#if defined(_WIN32)
#include <Windows.h>
#include <sys/stat.h>
#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode)&S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode) (((mode)&S_IFMT) == S_IFREG)
#endif
#else
#include <errno.h>
#include <sys/stat.h>

#define _POSIX_SOURCE
#include <unistd.h>
#endif

#include <algorithm>
#include <vector>

#include "str_util.h"

namespace fs_util {

int _mkdir(const char* p) {
#if defined(_WIN32)
  if (CreateDirectoryA(p, NULL)) {
    return 0;
  } else {
    if (GetLastError() == ERROR_ALREADY_EXISTS) return 0;
  }
#else
  if (mkdir(p, S_IRWXU) == 0) {
    return 0;
  } else {
    if (errno == EEXIST) {
      return 0;
    }
  }
#endif
  return -1;
}

int _mkdirs(char* path, size_t len) {
  if (_mkdir(path) == 0) {
    return 0;
  }

  char* p = NULL;
#ifdef _WIN32
  const char sep = '\\';
#else
  const char sep = '/';
#endif

  for (p = path + len - 1; p != path; --p) {
    if (*p == sep) {
      *p = 0;
      int r = _mkdirs(path, p - path);
      *p = '\\';
      if (r == 0) return _mkdir(path);
      return -1;
    }
  }
  return -1;
}

#if defined(_WIN32)
int _mkdirsW(wchar_t* path, size_t len) {
  wchar_t* p = NULL;

  if (CreateDirectoryW(path, NULL)) {
    return 0;
  } else {
    if (GetLastError() == ERROR_ALREADY_EXISTS) return 0;
  }

  p = wcsrchr(path, L'\\');
  if (p) {
    *p = 0;
    int r = _mkdirsW(path, len);
    *p = L'\\';

    if (r == 0) {
      if (CreateDirectoryW(path, NULL)) {
        return 0;
      } else {
        return -1;
      }
    }
  }
  return -1;
}
#endif

bool make_dirs(const std::string& dir) {
#ifdef _WIN32
  auto wdir = Utf8ToUtf16(dir);
  return _mkdirsW(const_cast<wchar_t*>(wdir.c_str()), wdir.size()) == 0;
#else
  return _mkdirs(const_cast<char*>(dir.c_str()), dir.size()) == 0;
#endif
}

bool file_exits(const std::string& filepath) {
#ifdef _WIN32
  struct _stat st;
  int ret = _wstat(Utf8ToUtf16(filepath).c_str(), &st);
  return (ret == 0) && !S_ISDIR(st.st_mode);
#else
  struct stat st;
  int ret = stat(filepath.c_str(), &st);
  return (ret == 0) && !S_ISDIR(st.st_mode);
#endif
}

bool directory_exists(const std::string& filepath) {
#ifdef _WIN32
  struct _stat st;
  int ret = _wstat(Utf8ToUtf16(filepath).c_str(), &st);
  return (ret == 0) && S_ISDIR(st.st_mode);
#else
  struct stat st;
  int ret = stat(filepath.c_str(), &st);
  return (ret == 0) && S_ISDIR(st.st_mode);
#endif
}

std::string dirname(const std::string& p) {
  auto it = p.find_last_of("/\\");
  if (it == p.npos) {
    return p;
  }
  return p.substr(0, it);
}

std::string basename(const std::string& p) {
  auto it = p.find_last_of("/\\");
  if (it == p.npos) {
    return p;
  }
  return p.substr(it + 1);
}

inline bool isAbs(const std::string& p) {
#ifdef _WIN32
  return p.find_first_of(':') != std::string::npos;
#else
  return !p.empty() && p[0] == '/';
#endif
}

#ifdef _WIN32
const char path_sep = '\\';
#else
const char path_sep = '/';
#endif

std::string path_join(const std::string& p1, const std::string& p2) {
  if (p1.empty() || isAbs(p2)) return p2;
  if (!p1.empty() && p1[p1.length() - 1] == path_sep) {
    return p1 + p2;
  }
  return p1 + path_sep + p2;
}

void copy_file(const std::string& from, const std::string& to) {
  std::ifstream src(from, std::ios::binary);
  std::ofstream dst(to, std::ios::binary);
  dst << src.rdbuf();
}

}  // namespace fs_util
