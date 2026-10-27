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
