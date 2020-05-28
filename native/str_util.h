#ifndef STR_UTIL_H
#define STR_UTIL_H

#include <stdio.h>
#include <string>
#include <algorithm>

size_t strlcpy(char *dst, const char *src, size_t siz);
size_t strlcat(char *d, char const *s, size_t n);

size_t wstrlcpy(wchar_t *dst, const wchar_t *src, size_t siz);
size_t wstrlcat(wchar_t *d, wchar_t const *s, size_t n);

char *u8strdup(const wchar_t *buffer);
char *u8strdup(const wchar_t *buffer, int len);

#ifdef _WIN32
std::string Utf16ToUtf8(const wchar_t *buffer, size_t len);
std::string Utf16ToUtf8(const std::wstring &str);
std::wstring Utf8ToUtf16(const std::string &str);
#endif

namespace util
{

inline std::string tolower(const std::string s)
{
  std::string r;
  r.reserve(s.length());
  std::transform(s.begin(), s.end(), r.begin(), ::tolower);
  return r;
};

inline bool startsWith(const std::string &s, const std::string &sub)
{
  if (s.find(sub) == 0)
    return true;
  else
    return false;
}

inline bool startsWithCaseInsensitive(const std::string s,
                                      const std::string toMatch)
{
  // Convert to lower case
  if (tolower(s).find(tolower(toMatch)) == 0)
    return true;
  else
    return false;
}

inline bool endsWith(const std::string &s, const std::string &sub)
{
  size_t n1 = s.size();
  size_t n2 = sub.size();

  if (n2 > n1) {
    return false;
  }

  const char *p = s.data() + n1 - n2;
  return memcmp(p, sub.data(), n2) == 0;
}

}  // namespace util
#endif  // STR_UTIL_H
