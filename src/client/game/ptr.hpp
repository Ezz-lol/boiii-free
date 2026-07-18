#pragma once

#include "base.hpp"

#include <cstdint>
#include <intrin.h>

namespace game {

// Without ASLR
constexpr uintptr_t ENGINE_MODULE_BASE = 0x140000000;
constexpr uintptr_t ENGINE_ADDRESS_SPACE_SIZE = 0x030000000;

inline uintptr_t relocate(const uintptr_t ptr) {
  if (ptr) {
    const uintptr_t base = get_base();
    return base + (ptr - ENGINE_MODULE_BASE);
  }

  return reinterpret_cast<uintptr_t>(nullptr);
}

inline uintptr_t derelocate(const uintptr_t ptr) {
  if (ptr) {
    const uintptr_t base = get_base();
    return (ptr - base) + ENGINE_MODULE_BASE;
  }

  return reinterpret_cast<uintptr_t>(nullptr);
}

template <typename T> inline T *derelocate(T *ptr) {
  return reinterpret_cast<T *>(derelocate(reinterpret_cast<uintptr_t>(ptr)));
}

template <typename T> inline const T *derelocate(const T *ptr) {
  return reinterpret_cast<const T *>(
      derelocate(reinterpret_cast<uintptr_t>(ptr)));
}

inline uintptr_t select(const uintptr_t client_val,
                        const uintptr_t server_val) {
  return relocate(is_server() ? server_val : client_val);
}

template <typename T>
inline const T *select(const T *client_val, const T *server_val) {
  return reinterpret_cast<const T *>(
      select(reinterpret_cast<uintptr_t>(client_val),
             reinterpret_cast<uintptr_t>(server_val)));
}

template <typename T> inline T *select(T *client_val, T *server_val) {
  return reinterpret_cast<T *>(select(reinterpret_cast<uintptr_t>(client_val),
                                      reinterpret_cast<uintptr_t>(server_val)));
}

/// @brief Checks if a pointer resides within any of the process's allocated
/// heap blocks.
/// @warning Walking the heap is extremely slow and blocks other threads. Do not
/// use in a hot loop!
bool valid_heap_ptr(uintptr_t ptr);

/// @brief Checks if a pointer resides within any of the process's allocated
/// heap blocks.
/// @warning Walking the heap is extremely slow and blocks other threads. Do not
/// use in a hot loop!
template <typename T> inline bool valid_heap_ptr(const T *ptr) {
  return valid_heap_ptr(reinterpret_cast<uintptr_t>(ptr));
}

/// @brief Checks if a pointer resides within the current thread's stack
/// boundaries.
inline bool valid_stack_ptr(uintptr_t ptr) {
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

template <typename T> inline bool valid_stack_ptr(const T *ptr) {
  return valid_stack_ptr(reinterpret_cast<uintptr_t>(ptr));
}

inline bool valid_module_ptr(uintptr_t ptr) {
  return ptr >= get_base() && ptr < (get_base() + ENGINE_ADDRESS_SPACE_SIZE);
}

template <typename T> inline bool valid_module_ptr(const T *ptr) {
  return valid_module_ptr(reinterpret_cast<uintptr_t>(ptr));
}

/// @brief Checks if a pointer's address points to the engine module address
/// space or a location on the current thread's stack.
inline bool valid_engine_ptr(uintptr_t ptr) {
  return ptr && (valid_module_ptr(ptr) || valid_stack_ptr(ptr));
}

template <typename T> inline bool valid_engine_ptr(const T *ptr) {
  return valid_engine_ptr(reinterpret_cast<uintptr_t>(ptr));
}
} // namespace game

inline size_t operator""_g(const size_t val) { return game::relocate(val); }