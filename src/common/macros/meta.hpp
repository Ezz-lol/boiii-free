#pragma once

// IWYU pragma: always_keep

#ifndef GNUC_OR_CLANG
#if defined(__GNUC__) || defined(__clang__)
#define GNUC_OR_CLANG 1
#else
#define GNUC_OR_CLANG 0
#endif
#endif

#ifndef MSC
#if defined(_MSC_VER)
#define MSC 1
#else
#define MSC 0
#endif
#endif

#ifndef CONCAT_IMPL
#define CONCAT_IMPL(x, y) x##y
#endif

#ifndef CONCAT
#define CONCAT(x, y) CONCAT_IMPL(x, y)
#endif

#ifndef UNIQUE_VAR
#define UNIQUE_VAR(prefix) CONCAT(prefix, __COUNTER__)
#endif
