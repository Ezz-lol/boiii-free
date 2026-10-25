#pragma once

#include "base.hpp"
#include <macros/macros.hpp>

#include <cstdint>
#include <intrin.h>

namespace game {
#if defined(_WIN64)
#include <intrin.h>
alwaysinline uintptr_t PEB() {
  // PEB pointer location on x64 Windows TEB (GS:[0x60])
  return std::bit_cast<uintptr_t>(__readgsqword(0x60));
}
#elif defined(_WIN32)
#include <intrin.h>
alwaysinline uintptr_t PEB() {
  // PEB pointer location on x86 Windows TEB (FS:[0x30])
  return static_cast<uintptr_t>(__readfsdword(0x30));
}
#else
alwaysinline uintptr_t PEB() {
  return 0; // Fallback
}
#endif

alwaysinline uint32_t PEB32() { return static_cast<uint32_t>(PEB()); }

// Without ASLR
constexpr uintptr_t ENGINE_MODULE_BASE = 0x140000000;
constexpr uintptr_t ENGINE_ADDRESS_SPACE_SIZE = 0x030000000;

alwaysinline uintptr_t relocate(const uintptr_t ptr) {
  if (ptr) {
    const uintptr_t base = get_engine_base();
    return base + (ptr - ENGINE_MODULE_BASE);
  }

  return reinterpret_cast<uintptr_t>(nullptr);
}

alwaysinline uintptr_t derelocate(const uintptr_t ptr) {
  if (ptr) {
    const uintptr_t base = get_engine_base();
    return (ptr - base) + ENGINE_MODULE_BASE;
  }

  return reinterpret_cast<uintptr_t>(nullptr);
}

template <typename T> alwaysinline T *derelocate(T *ptr) {
  return reinterpret_cast<T *>(derelocate(reinterpret_cast<uintptr_t>(ptr)));
}

template <typename T> alwaysinline const T *derelocate(const T *ptr) {
  return reinterpret_cast<const T *>(
      derelocate(reinterpret_cast<uintptr_t>(ptr)));
}

template <typename T>
alwaysinline T select_value(T client, T legacy_client, T server) {
  if (is_server()) {
    return server;
  }
  if (is_new_client()) {
    return client;
  }
  return legacy_client;
}

template <typename T>
  requires(!std::is_pointer_v<T> && !std::is_same_v<T, uintptr_t> &&
           !std::is_same_v<T, intptr_t>)
alwaysinline T select(T client, T legacy_client, T server) {
  return select_value(client, legacy_client, server);
}

alwaysinline uintptr_t select(const uintptr_t client_val,
                              const uintptr_t legacy_client_val,
                              const uintptr_t server_val) {
  return relocate(select_value(client_val, legacy_client_val, server_val));
}

template <typename T>
alwaysinline const T *select(const T *client_val, const T *legacy_client_val,
                             const T *server_val) {
  return reinterpret_cast<const T *>(
      select(reinterpret_cast<uintptr_t>(client_val),
             reinterpret_cast<uintptr_t>(legacy_client_val),
             reinterpret_cast<uintptr_t>(server_val)));
}

template <typename T>
alwaysinline T *select(T *client_val, T *legacy_client_val, T *server_val) {
  return reinterpret_cast<T *>(
      select(reinterpret_cast<uintptr_t>(client_val),
             reinterpret_cast<uintptr_t>(legacy_client_val),
             reinterpret_cast<uintptr_t>(server_val)));
}

template <typename T>
alwaysinline volatile T *select(volatile T *client_val,
                                volatile T *legacy_client_val,
                                volatile T *server_val) {
  return reinterpret_cast<volatile T *>(
      select(reinterpret_cast<uintptr_t>(client_val),
             reinterpret_cast<uintptr_t>(legacy_client_val),
             reinterpret_cast<uintptr_t>(server_val)));
}

/// @brief Checks if a pointer resides within the current thread's stack
/// boundaries.
alwaysinline bool valid_stack_ptr(uintptr_t ptr) {
  if (ptr) {
    // In x64 Windows, GS:[0x08] holds the Stack Base (highest address)
    // and GS:[0x10] holds the Stack Limit (lowest address).
    // The stack grows downwards from the Base to the Limit.
    uintptr_t stack_base = __readgsqword(0x08);
    uintptr_t stack_limit = __readgsqword(0x10);

    return (ptr >= stack_limit) && (ptr < stack_base);
  }

  return false;
}

template <typename T> alwaysinline bool valid_stack_ptr(const T *ptr) {
  return valid_stack_ptr(reinterpret_cast<uintptr_t>(ptr));
}

alwaysinline bool valid_engine_module_ptr(uintptr_t ptr) {
  return ptr >= get_engine_base() &&
         ptr < (get_engine_base() + ENGINE_ADDRESS_SPACE_SIZE);
}

template <typename T> alwaysinline bool valid_engine_module_ptr(const T *ptr) {
  return valid_engine_module_ptr(reinterpret_cast<uintptr_t>(ptr));
}

alwaysinline bool valid_current_module_ptr(uintptr_t ptr) {
  return ptr >= get_base() && ptr < (get_base() + current_module_size());
}

template <typename T> alwaysinline bool valid_current_module_ptr(const T *ptr) {
  return valid_current_module_ptr(reinterpret_cast<uintptr_t>(ptr));
}

alwaysinline bool valid_module_ptr(uintptr_t ptr) {
  return ptr && (valid_engine_module_ptr(ptr) || valid_current_module_ptr(ptr));
}

template <typename T> alwaysinline bool valid_module_ptr(const T *ptr) {
  return valid_module_ptr(reinterpret_cast<uintptr_t>(ptr));
}

/// @brief Checks if a pointer's address points to the engine module address
/// space or a location on the current thread's stack.
alwaysinline bool valid_engine_ptr(uintptr_t ptr) {
  return ptr && (valid_module_ptr(ptr) || valid_stack_ptr(ptr));
}

template <typename T> alwaysinline bool valid_engine_ptr(const T *ptr) {
  return valid_engine_ptr(reinterpret_cast<uintptr_t>(ptr));
}

alwaysinline bool readable_ptr(uintptr_t ptr) {
  return ptr && (valid_engine_ptr(ptr) || ::readable_ptr(ptr));
}

template <typename T> alwaysinline bool readable_ptr(const T *ptr) {
  return readable_ptr(reinterpret_cast<uintptr_t>(ptr));
}

} // namespace game

alwaysinline size_t operator""_g(const size_t val) {
  return game::relocate(val);
}
