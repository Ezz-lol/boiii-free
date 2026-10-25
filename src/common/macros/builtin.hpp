#pragma once

#ifndef GNUC_OR_CLANG
#if defined(__GNUC__) || defined(__clang__)
#define GNUC_OR_CLANG 1
#else
#define GNUC_OR_CLANG 0
#endif
#endif

#if !GNUC_OR_CLANG
#include <atomic>
#endif

#ifndef __inline_def
#if GNUC_OR_CLANG
#define __inline_def __attribute__((always_inline))
#elif defined(_MSC_VER)
#define __inline_def __forceinline
#define TEMPLATE_INVALID_HANDLE_VALUE -1
#else
#error "Unsupported compiler. Only MSVC, Clang and GCC are supported."
#endif
#endif

#ifndef __optimize
#if GNUC_OR_CLANG
#define __optimize __attribute__((hot))
#elif defined(_MSC_VER)
// No equivalent that I can find. Add if found or known otherwise.
#define __optimize
#else
#error "Unsupported compiler. Only MSVC, Clang and GCC are supported."
#endif
#endif

#ifndef __no_inline
#define __no_inline __attribute__((noinline))
#endif

#ifndef __no_optimize
#define __no_optimize __attribute__((optnone))
#endif

#ifndef unreachable
#ifdef NDEBUG
#if GNUC_OR_CLANG
#define unreachable() __builtin_unreachable()
#elif defined(_MSC_VER)
#define unreachable() __assume(false)
#else
#error "Unsupported compiler. Only MSVC, Clang and GCC are supported."
#endif
#else
#define unreachable() assert(false && "This code should be unreachable.")
#endif
#endif

#ifndef __trivial_abi
#if defined(__clang__) && __has_cpp_attribute(clang::trivial_abi)
#define __trivial_abi [[clang::trivial_abi]]
#elif defined(__GNUC__) && __has_cpp_attribute(gnu::trivial_abi)
#define __trivial_abi [[gnu::trivial_abi]]
#else
// An equivalent does not exist in MSVC. This is a new language extension,
// added in 2017 in clang and GCC.
#define __trival_abi
#endif
#endif

#ifndef __export
#if defined(_WIN32) || defined(_WIN64)
#if defined(__clang__) && __has_cpp_attribute(clang::lto_visibility_public)
#define __export                                                               \
  [[clang::lto_visibility_public]] __attribute__((visibility("default")))      \
  __declspec(dllexport)
#else
#define __export __attribute__((visibility("default"))) __declspec(dllexport)
#endif
#else
#if defined(__clang__) && __has_cpp_attribute(clang::lto_visibility_public)
#define __export                                                               \
  [[clang::lto_visibility_public]] __attribute__((visibility("default")))
#else
#define __export __attribute__((visibility("default")))
#endif
#endif
#endif

#ifndef __external
#if GNUC_OR_CLANG
#define __external                                                             \
  __attribute__((visibility("default"))) __attribute__((__may_alias__))
#else
// MSVC does not use strict aliasing
#define __external
#endif
#endif

#ifndef __always_inline
#if defined(_MSC_VER)
#define __always_inline __forceinline
#elif GNUC_OR_CLANG
#define __always_inline __attribute__((always_inline))
#else
#define __always_inline
#endif
#endif

#ifndef alwaysinline
#define alwaysinline inline __always_inline
#endif

alwaysinline void ir_memory_barrier() {
  // Ideal implementation: signal to LLVM pipeline that _any_ memory could be
  // read or written here, so it cannot make any caching assumptions.
#if GNUC_OR_CLANG
  __asm__ __volatile__("" ::: "memory");
/*
   Concerning, incomplete implementation: only disallows caching values in
   registers across this particular fence, or optimizing away or reordering
   loads or stores across the fence, on this thread.
*/
#else
  std::atomic_signal_fence(std::memory_order_seq_cst);
#endif
}

#ifndef PUSH_DISABLE_OPTIMIZATION
#if defined(__clang__)
#define PUSH_DISABLE_OPTIMIZATION() _Pragma("clang optimize off")
#elif defined(__GNUC__)
#define PUSH_DISABLE_OPTIMIZATION()                                            \
  _Pragma("GCC push_options") _Pragma("GCC optimize(\"O0\")")
#elif defined(_MSC_VER)
#define PUSH_DISABLE_OPTIMIZATION() _Pragma("optimize(\"\", off)")
#endif
#endif

#ifndef POP_DISABLE_OPTIMIZATION
#if defined(__clang__)
#define POP_DISABLE_OPTIMIZATION() _Pragma("clang optimize on")
#elif defined(__GNUC__)
#define POP_DISABLE_OPTIMIZATION() _Pragma("GCC pop_options")
#elif defined(_MSC_VER)
#define POP_DISABLE_OPTIMIZATION() _Pragma("optimize(\"\", on)")
#endif
#endif

#define UNOPTIMIZED(...)                                                       \
  PUSH_DISABLE_OPTIMIZATION()                                                  \
  __VA_ARGS__                                                                  \
  POP_DISABLE_OPTIMIZATION()
