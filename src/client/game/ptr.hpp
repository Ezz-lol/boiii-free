#pragma once

#include "base.hpp"

#include <cstdint>
#include <intrin.h>

namespace game {

inline size_t relocate(const size_t val) {
  if (!val)
    return 0;

  const size_t base = get_base();
  return base + (val - 0x140000000);
}

inline size_t derelocate(const size_t val) {
  if (!val)
    return 0;

  const size_t base = get_base();
  return (val - base) + 0x140000000;
}

inline size_t derelocate(const void *val) {
  return derelocate(reinterpret_cast<size_t>(val));
}

inline size_t select(const size_t client_val, const size_t server_val) {
  return relocate(is_server() ? server_val : client_val);
}

inline size_t select(const void *client_val, const void *server_val) {
  return select(reinterpret_cast<size_t>(client_val),
                reinterpret_cast<size_t>(server_val));
}

/// @brief Checks if a pointer resides within the current thread's stack
/// boundaries.
inline bool valid_stack_ptr(uintptr_t ptr) {
  if (!ptr) {
    return false;
  }

  // In x64 Windows, GS:[0x08] holds the Stack Base (highest address)
  // and GS:[0x10] holds the Stack Limit (lowest address).
  // The stack grows downwards from the Base to the Limit.
  uintptr_t stack_base = __readgsqword(0x08);
  uintptr_t stack_limit = __readgsqword(0x10);

  return (ptr >= stack_limit) && (ptr < stack_base);
}

template <typename T> inline bool valid_stack_ptr(T *ptr) {
  return valid_stack_ptr(reinterpret_cast<uintptr_t>(ptr));
}

/// @brief Checks if a pointer's address points to the engine module address
/// space or a location on the current thread's stack.
inline bool valid_engine_ptr(uintptr_t ptr) {
  if (!ptr) {
    return false;
  }

  bool in_module = (ptr >= get_base()) && (ptr < (get_base() + 0x030000000));
  bool on_stack = valid_stack_ptr(ptr);

  return in_module || on_stack;
}

template <typename T> inline bool valid_engine_ptr(T *ptr) {
  return valid_engine_ptr(reinterpret_cast<uintptr_t>(ptr));
}
} // namespace game