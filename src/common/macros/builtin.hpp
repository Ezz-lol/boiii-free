#pragma once

#ifndef __inline_def
#if defined(__clang__) || defined(__GNUC__)
#define __inline_def __attribute__((always_inline))
#elif defined(_MSC_VER)
#define __inline_def __forceinline
#define TEMPLATE_INVALID_HANDLE_VALUE -1
#else
#error "Unsupported compiler. Only MSVC, Clang and GCC are supported."
#endif
#endif

#ifndef __optimize
#if defined(__clang__) || defined(__GNUC__)
#define __optimize __attribute__((hot))
#elif defined(_MSC_VER)
// No equivalent that I can find. Add if found or known otherwise.
#define __optimize
#else
#error "Unsupported compiler. Only MSVC, Clang and GCC are supported."
#endif
#endif

#ifndef unreachable
#ifdef NDEBUG
#if defined(__GNUC__) || defined(__clang__)
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
