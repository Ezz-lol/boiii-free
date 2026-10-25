#pragma once

#include "macros/type/integral.hpp"
#include <cstdint>
#include <span>

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
