#pragma once

#ifndef inline_def
#if defined(__clang__) || defined(__GNUC__)
#define inline_def __attribute__((always_inline))
#elif defined(_MSC_VER)
#define inline_def __forceinline
#define TEMPLATE_INVALID_HANDLE_VALUE -1
#else
#error "Unsupported compiler. Only MSVC, Clang and GCC are supported."
#endif
#endif