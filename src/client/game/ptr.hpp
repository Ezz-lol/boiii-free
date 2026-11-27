#pragma once

#include "base.hpp"
#include <macros/macros.hpp>

#include <cstdint>
#include <intrin.h>

namespace game {
#if defined(_WIN64) || defined(_WIN32)
#include <intrin.h>
#include <winternl.h>
[[nodiscard]] alwaysinline uintptr_t UINT_PPEB() {
  const PTEB teb = reinterpret_cast<const PTEB>(NtCurrentTeb());
  return reinterpret_cast<uintptr_t>(teb->ProcessEnvironmentBlock);
}
#else
alwaysinline uintptr_t UINT_PPEB() {
  return 0; // Fallback
}
#endif

alwaysinline uint32_t UINT_PPEB32() {
  return static_cast<uint32_t>(UINT_PPEB());
}

// Without ASLR
constexpr uintptr_t ENGINE_MODULE_BASE = 0x140000000;
constexpr uintptr_t ENGINE_ADDRESS_SPACE_SIZE = 0x020000000;
constexpr uintptr_t ENGINE_MODULE_ADDRESS_SPACE_END =
    ENGINE_MODULE_BASE + ENGINE_ADDRESS_SPACE_SIZE;

alwaysinline bool valid_default_base_engine_module_ptr(uintptr_t ptr) {
  return ptr >= ENGINE_MODULE_BASE && ptr < ENGINE_MODULE_ADDRESS_SPACE_END;
}

template <typename T>
alwaysinline bool valid_default_base_engine_module_ptr(const T *ptr) {
  return valid_default_base_engine_module_ptr(reinterpret_cast<uintptr_t>(ptr));
}

alwaysinline bool valid_engine_module_ptr(uintptr_t ptr) {
  return ptr >= get_engine_base() &&
         ptr < (get_engine_base() + ENGINE_ADDRESS_SPACE_SIZE);
}

template <typename T> alwaysinline bool valid_engine_module_ptr(const T *ptr) {
  return valid_engine_module_ptr(reinterpret_cast<uintptr_t>(ptr));
}

alwaysinline uintptr_t relocate(const uintptr_t ptr) {
  if (valid_default_base_engine_module_ptr(ptr)) {
    return get_engine_base() + (ptr - ENGINE_MODULE_BASE);
  }

  return ptr;
}

alwaysinline uintptr_t derelocate(const uintptr_t ptr) {
  if (valid_engine_module_ptr(ptr)) {
    return (ptr - get_engine_base()) + ENGINE_MODULE_BASE;
  }

  return ptr;
}

template <typename T> alwaysinline T *derelocate(T *ptr) {
  return reinterpret_cast<T *>(derelocate(reinterpret_cast<uintptr_t>(ptr)));
}

template <typename T> alwaysinline const T *derelocate(const T *ptr) {
  return reinterpret_cast<const T *>(
      derelocate(reinterpret_cast<uintptr_t>(ptr)));
}

template <typename T> alwaysinline T select_value(T client, T server) {
  if (is_server()) {
    return server;
  }
  return client;
}

template <typename T>
  requires(!std::is_pointer_v<T> && !std::is_same_v<T, uintptr_t> &&
           !std::is_same_v<T, intptr_t>)
alwaysinline T select(T client, T server) {
  return select_value(client, server);
}

alwaysinline uintptr_t select(const uintptr_t client_val,
                              const uintptr_t server_val) {
  return relocate(select_value(client_val, server_val));
}

template <typename T>
alwaysinline const T *select(const T *client_val, const T *server_val) {
  return reinterpret_cast<const T *>(
      select(reinterpret_cast<uintptr_t>(client_val),
             reinterpret_cast<uintptr_t>(server_val)));
}

template <typename T> alwaysinline T *select(T *client_val, T *server_val) {
  return reinterpret_cast<T *>(select(reinterpret_cast<uintptr_t>(client_val),
                                      reinterpret_cast<uintptr_t>(server_val)));
}

template <typename T>
alwaysinline volatile T *select(volatile T *client_val,

                                volatile T *server_val) {
  return reinterpret_cast<volatile T *>(
      select(reinterpret_cast<uintptr_t>(client_val),
             reinterpret_cast<uintptr_t>(server_val)));
}

/// @brief Checks if a pointer resides within the current thread's stack
/// boundaries.
alwaysinline bool valid_stack_ptr(uintptr_t ptr) {
  if (ptr == UINTPTR_NULLPTR) {
    return false;
  }
  const NT_TIB *tib = reinterpret_cast<const NT_TIB *>(NtCurrentTeb());
  const uintptr_t stack_base = reinterpret_cast<uintptr_t>(tib->StackBase);
  const uintptr_t stack_limit = reinterpret_cast<uintptr_t>(tib->StackLimit);

  return (ptr >= stack_limit) && (ptr < stack_base);
}

template <typename T> alwaysinline bool valid_stack_ptr(const T *ptr) {
  return valid_stack_ptr(reinterpret_cast<uintptr_t>(ptr));
}

alwaysinline bool valid_current_module_ptr(uintptr_t ptr) {
  return ptr >= get_base() && ptr < (get_base() + current_module_size());
}

template <typename T> alwaysinline bool valid_current_module_ptr(const T *ptr) {
  return valid_current_module_ptr(reinterpret_cast<uintptr_t>(ptr));
}

alwaysinline bool valid_module_ptr(uintptr_t ptr) {
  return ptr != UINTPTR_NULLPTR &&
         (valid_engine_module_ptr(ptr) || valid_current_module_ptr(ptr));
}

template <typename T> alwaysinline bool valid_module_ptr(const T *ptr) {
  return valid_module_ptr(reinterpret_cast<uintptr_t>(ptr));
}

/// @brief Checks if a pointer's address points to the engine module address
/// space or a location on the current thread's stack.
alwaysinline bool valid_engine_ptr(uintptr_t ptr) {
  return ptr != UINTPTR_NULLPTR &&
         (valid_module_ptr(ptr) || valid_stack_ptr(ptr));
}

template <typename T> alwaysinline bool valid_engine_ptr(const T *ptr) {
  return valid_engine_ptr(reinterpret_cast<uintptr_t>(ptr));
}

alwaysinline bool readable_ptr(uintptr_t ptr) {
  return ptr != UINTPTR_NULLPTR &&
         (valid_engine_ptr(ptr) || ::readable_ptr(ptr));
}

template <typename T> alwaysinline bool readable_ptr(const T *ptr) {
  return readable_ptr(reinterpret_cast<uintptr_t>(ptr));
}

} // namespace game

alwaysinline size_t operator""_g(const size_t val) {
  return game::relocate(val);
}
