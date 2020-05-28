#include "str_util.h"

#include <vector>

#ifdef _WIN32
#define  WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#if defined(WIN32)
size_t strlcpy(char *dst, const char *src, size_t siz) {
  register char *d = dst;
  register const char *s = src;
  register size_t n = siz;

  /* Copy as many bytes as will fit */
  if (n != 0 && --n != 0) {
    do {
      if ((*d++ = *s++) == 0) break;
    } while (--n != 0);
  }

  /* Not enough room in dst, add NUL and traverse rest of src */
  if (n == 0) {
    if (siz != 0) *d = '\0'; /* NUL-terminate dst */
    while (*s++)
      ;
  }

  return (s - src - 1); /* count does not include NUL */
}

size_t wstrlcpy(wchar_t *dst, const wchar_t *src, size_t siz) {
  register wchar_t *d = dst;
  register const wchar_t *s = src;
  register size_t n = siz;

  /* Copy as many bytes as will fit */
  if (n != 0 && --n != 0) {
    do {
      if ((*d++ = *s++) == 0) break;
    } while (--n != 0);
  }

  /* Not enough room in dst, add NUL and traverse rest of src */
  if (n == 0) {
    if (siz != 0) *d = 0; /* NUL-terminate dst */
    while (*s++)
      ;
  }

  return (s - src - 1); /* count does not include NUL */
}

size_t strlcat(char *dst, const char *src, size_t siz) {
  char *d = dst;
  const char *s = src;
  size_t n = siz;
  size_t dlen;

  /* Find the end of dst and adjust bytes left but don't go past end */
  while (n-- != 0 && *d != '\0') d++;
  dlen = d - dst;
  n = siz - dlen;

  if (n == 0) return (dlen + strlen(s));

  while (*s != '\0') {
    if (n != 1) {
      *d++ = *s;
      n--;
    }
    s++;
  }
  *d = '\0';

  return (dlen + (s - src)); /* count does not include NUL */
}

size_t wstrlcat(wchar_t *dst, const wchar_t *src, size_t siz) {
  wchar_t *d = dst;
  const wchar_t *s = src;
  size_t n = siz;
  size_t dlen;

  /* Find the end of dst and adjust bytes left but don't go past end */
  while (n-- != 0 && *d != '\0') d++;
  dlen = d - dst;
  n = siz - dlen;

  if (n == 0) return (dlen + wcslen(s));

  while (*s != '\0') {
    if (n != 1) {
      *d++ = *s;
      n--;
    }
    s++;
  }
  *d = '\0';

  return (dlen + (s - src)); /* count does not include NUL */
}

#endif

#ifdef _WIN32
char *u8strdup(const wchar_t *buffer) {
  return u8strdup(buffer, wcslen(buffer));
}

char *u8strdup(const wchar_t *buffer, int len) {
  int nChars =
      ::WideCharToMultiByte(CP_UTF8, 0, buffer, len, NULL, 0, NULL, NULL);
  if (nChars == 0) return NULL;

  char *p = (char *)malloc(nChars + 1);
  nChars =
      ::WideCharToMultiByte(CP_UTF8, 0, buffer, len, p, nChars, NULL, NULL);
  p[nChars] = '\0';

  return p;
}

std::string Utf16ToUtf8(const wchar_t *buffer, size_t len) {
  int nChars =
      ::WideCharToMultiByte(CP_UTF8, 0, buffer, len, NULL, 0, NULL, NULL);
  if (nChars == 0) return "";

  std::string newbuffer;
  newbuffer.resize(nChars);
  ::WideCharToMultiByte(CP_UTF8, 0, buffer, len,
                        const_cast<char *>(newbuffer.c_str()), nChars, NULL,
                        NULL);

  return newbuffer;
}

std::string Utf16ToUtf8(const std::wstring &str) {
  return Utf16ToUtf8(str.c_str(), str.size());
}

std::wstring Utf8ToUtf16(const std::string &str) {
  std::vector<wchar_t> buf;
  int nChars =
      ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, NULL);
  if (nChars == 0) return L"";

  buf.resize(nChars);
  ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), (LPWSTR)&buf[0],
                        nChars);

  return std::wstring(buf.begin(), buf.end());
}

#endif

