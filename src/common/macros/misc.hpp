#pragma once

#ifndef WITH_DIAG_DISABLED

// Helper macro to properly evaluate and stringify arguments for standard
// _Pragma
#define DETAIL_DIAG_PRAGMA(x) _Pragma(#x)

#if defined(__clang__)
#define WITH_DIAG_DISABLED(clang_diag, gnu_diag, msc_diag, ...)                \
  _Pragma("clang diagnostic push")                                             \
      DETAIL_DIAG_PRAGMA(clang diagnostic ignored clang_diag)                  \
          __VA_ARGS__ _Pragma("clang diagnostic pop")

#elif defined(__GNUC__)
#define WITH_DIAG_DISABLED(clang_diag, gnu_diag, msc_diag, ...)                \
  _Pragma("GCC diagnostic push")                                               \
      DETAIL_DIAG_PRAGMA(GCC diagnostic ignored gnu_diag)                      \
          __VA_ARGS__ _Pragma("GCC diagnostic pop")

#elif defined(_MSC_VER)
#define WITH_DIAG_DISABLED(clang_diag, gnu_diag, msc_diag, ...)                \
  __pragma(warning(push)) __pragma(warning(disable : msc_diag)) __VA_ARGS__    \
  __pragma(warning(pop))

#else
#pragma message(                                                               \
    "WITH_DIAG_DISABLED: Unsupported compiler. Diagnostics will not be disabled.")
#define WITH_DIAG_DISABLED(clang_diag, gnu_diag, msc_diag, ...) __VA_ARGS__

#endif
#endif // WITH_DIAG_DISABLED
