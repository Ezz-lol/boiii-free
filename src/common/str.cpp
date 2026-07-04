/*	$OpenBSD: strlcpy.c,v 1.16 2019/01/25 00:19:25 millert Exp $	*/

/*
 * Copyright (c) 1998, 2015 Todd C. Miller <millert@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "str.hpp"
#include <cerrno>
#include <climits>

size_t strlcpy(char *dst, const char *src, size_t dsize) {
  const char *osrc = src;
  size_t nleft = dsize;

  /* Copy as many bytes as will fit. */
  if (nleft != 0) {
    while (--nleft != 0) {
      if ((*dst++ = *src++) == '\0')
        break;
    }
  }

  /* Not enough room in dst, add NUL and traverse rest of src. */
  if (nleft == 0) {
    if (dsize != 0)
      *dst = '\0'; /* NUL-terminate dst */
    while (*src++)
      ;
  }

  return (src - osrc - 1); /* count does not include NUL */
}

// SPDX-License-Identifier: GPL-2.0
/*
 *  linux/lib/string.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */
size_t sized_strscpy(char *dest, const char *src, size_t count) {
  long res = 0;

  if (count == 0 || count > INT_MAX)
    return -E2BIG;

  while (count > 1) {
    char c;

    c = src[res];
    dest[res] = c;
    if (!c)
      return res;
    res++;
    count--;
  }

  /* Force NUL-termination. */
  dest[res] = '\0';

  /* Return E2BIG if the source didn't stop */
  return src[res] ? -E2BIG : res;
}