#ifndef FS_UTIL_H
#define FS_UTIL_H

#include <string>

namespace fs_util {

bool make_dirs(const std::string& dir);
bool file_exits(const std::string& filepath);
bool directory_exists(const std::string& filepath);

void copy_file(const std::string& from, const std::string& to);

std::string dirname(const std::string& p);
std::string basename(const std::string& p);

std::string path_join(const std::string& p1, const std::string& p2);

template <class T>
std::string join(T p) {
  return p;
}

template <class T, class... Args>
std::string join(T p, Args... parts) {
  return path_join(p, join(parts...));
}

}  // namespace fs_util
#endif  // FS_UTIL_H
