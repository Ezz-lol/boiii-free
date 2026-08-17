#pragma once

#include "base.hpp"
#include <macros.hpp>

#include <cstdint>
#include <intrin.h>
#include <span>

namespace game {

#if defined(_WIN64)
#include <intrin.h>
inline uintptr_t PEB() {
  // PEB pointer location on x64 Windows TEB (GS:[0x60])
  return std::bit_cast<uintptr_t>(__readgsqword(0x60));
}
#elif defined(_WIN32)
#include <intrin.h>
inline uintptr_t PEB() {
  // PEB pointer location on x86 Windows TEB (FS:[0x30])
  return static_cast<uintptr_t>(__readfsdword(0x30));
}
#else
inline uintptr_t PEB() {
  return 0; // Fallback
}
#endif

inline uint32_t PEB32() { return static_cast<uint32_t>(PEB()); }

// Without ASLR
constexpr uintptr_t ENGINE_MODULE_BASE = 0x140000000;
constexpr uintptr_t ENGINE_ADDRESS_SPACE_SIZE = 0x030000000;

inline uintptr_t relocate(const uintptr_t ptr) {
  if (ptr) {
    const uintptr_t base = get_engine_base();
    return base + (ptr - ENGINE_MODULE_BASE);
  }

  return reinterpret_cast<uintptr_t>(nullptr);
}

inline uintptr_t derelocate(const uintptr_t ptr) {
  if (ptr) {
    const uintptr_t base = get_engine_base();
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

template <typename T>
inline volatile T *select(volatile T *client_val, volatile T *server_val) {
  return reinterpret_cast<volatile T *>(
      select(reinterpret_cast<uintptr_t>(client_val),
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

inline bool valid_engine_module_ptr(uintptr_t ptr) {
  return ptr >= get_engine_base() &&
         ptr < (get_engine_base() + ENGINE_ADDRESS_SPACE_SIZE);
}

template <typename T> inline bool valid_engine_module_ptr(const T *ptr) {
  return valid_engine_module_ptr(reinterpret_cast<uintptr_t>(ptr));
}

inline bool valid_current_module_ptr(uintptr_t ptr) {
  return ptr >= get_base() && ptr < (get_base() + current_module_size());
}

template <typename T> inline bool valid_current_module_ptr(const T *ptr) {
  return valid_current_module_ptr(reinterpret_cast<uintptr_t>(ptr));
}

inline bool valid_module_ptr(uintptr_t ptr) {
  return ptr && (valid_engine_module_ptr(ptr) || valid_current_module_ptr(ptr));
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

/// NOTE: DO NOT USE ON WINDOWS. Wine performs this check very quickly, but
/// using this on Windows _throttles_ performance.
/// @brief Rapidly checks if a memory address is committed and readable.
/// Safe to use in high-frequency loops.
bool readable_ptr(uintptr_t ptr);

template <typename T> inline bool readable_ptr(const T *ptr) {
  return readable_ptr(reinterpret_cast<uintptr_t>(ptr));
}

inline constexpr bool nonnull(uintptr_t ptr) {
  return ptr >= 0x00000000'00010000 && ptr <= 0x00007FFF'FFFFFFFF;
}

template <typename T> inline constexpr bool nonnull(const T *ptr) {
  return nonnull(reinterpret_cast<uintptr_t>(ptr));
}

template <IntegralLike<size_t> S>
inline constexpr bool contains(const uintptr_t base, const S size,
                               const uintptr_t cmp) {
  return cmp >= base && cmp < base + static_cast<size_t>(size);
}

template <typename A, typename B, IntegralLike<size_t> S>
inline constexpr bool contains(const A *base, const S size, const B *cmp) {
  return contains<S>(reinterpret_cast<uintptr_t>(base), size,
                     reinterpret_cast<uintptr_t>(cmp));
}

template <typename A, typename B>
inline constexpr bool contains(const std::span<const A> base, const B *cmp) {
  return contains<uint64_t>(reinterpret_cast<uintptr_t>(base.data()),
                            base.size(), reinterpret_cast<uintptr_t>(cmp));
}

template <typename T, IntegralLike<T> Align>
  requires(!std::is_pointer_v<T>)
T align(T val, Align alignment) {
  return (val + static_cast<T>(alignment) - 1) &
         ~(static_cast<T>(alignment) - 1);
}

template <typename T, IntegralLike<uintptr_t> Align>
const T *align(const T *val, Align alignment) {
  return reinterpret_cast<const T *>(
      align<uintptr_t, Align>(reinterpret_cast<uintptr_t>(val), alignment));
}

template <typename T, IntegralLike<uintptr_t> Align>
T *align(T *val, Align alignment) {
  return reinterpret_cast<T *>(
      align<uintptr_t, Align>(reinterpret_cast<uintptr_t>(val), alignment));
}
} // namespace game

inline size_t operator""_g(const size_t val) { return game::relocate(val); }