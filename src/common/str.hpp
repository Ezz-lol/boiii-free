#pragma once

#include <climits>
#include <cstddef>
#include <cstring>

#include <string_view>

constexpr size_t strlcpy(char *dst, const char *src, size_t size);

/* All declarations and definitions below are essentially
   the C++-equivalent of the macro-expanded, platform-independent source for the
   equivalent functions in the Linux kernel. Thus, the appropriate license is
   provided below.
*/
// SPDX-License-Identifier: GPL-2.0
/*
 *  linux/lib/string.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */
inline constexpr int64_t sized_strscpy(char *dest, const char *src,
                                       size_t count) {
  int64_t res = 0;

  if (count == 0 || count > INT_MAX) {
    return -E2BIG;
  }

  while (count > 1) {
    char c = src[res];
    dest[res] = c;
    if (c == NULL) {
      return res;
    }
    res++;
    count--;
  }

  /* Force NUL-termination. */
  dest[res] = NULL;

  /* Return E2BIG if the source didn't stop */
  return src[res] == NULL ? res : -E2BIG;
}

/**
 * sized_strscpy_pad - Core string copy loop with trailing zero-padding
 */
inline int64_t sized_strscpy_pad(char *dest, const char *src, size_t count) {
  int64_t wrote = sized_strscpy(dest, src, count);

  if (wrote >= 0 && std::cmp_less(wrote, count)) {
    std::memset(dest + wrote + 1, 0, count - wrote - 1);
  }

  return wrote;
}

inline constexpr int64_t strscpy(char *dst, const char *src, size_t size) {
  return sized_strscpy(dst, src, size);
}

inline constexpr int64_t strscpy(volatile char *dst, const char *src,
                                 size_t size) {
  return sized_strscpy(const_cast<char *>(dst), src, size);
}

inline constexpr int64_t strscpy_pad(char *dst, const char *src, size_t size) {
  return sized_strscpy_pad(dst, src, size);
}

template <size_t N>
inline constexpr int64_t strscpy(char (&dst)[N], const char *src) {
  return sized_strscpy(dst, src, N);
}

template <size_t N>
inline constexpr int64_t strscpy(volatile char (&dst)[N], const char *src) {
  return sized_strscpy(const_cast<char *>(dst), src, N);
}

template <size_t N>
inline constexpr int64_t strscpy_pad(char (&dst)[N], const char *src) {
  return sized_strscpy_pad(dst, src, N);
}

template <size_t N>
inline constexpr int64_t strscpy(char (&dst)[N], const std::string_view &src) {
  return sized_strscpy(dst, src.data(), N);
}

template <size_t N>
inline constexpr int64_t strscpy(volatile char (&dst)[N],
                                 const std::string_view &src) {
  return sized_strscpy(const_cast<char *>(dst), src.data(), N);
}

template <size_t N>
inline constexpr int64_t strscpy_pad(char (&dst)[N],
                                     const std::string_view &src) {
  return sized_strscpy_pad(dst, src.data(), N);
}
