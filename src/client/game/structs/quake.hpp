#pragma once

#include <cstdint>
#include <type_traits>

#include "macros.hpp"
#include "func.hpp"

namespace game {

struct qboolean {
  int32_t value;

  constexpr qboolean() noexcept = default;
  constexpr qboolean(const qboolean &) noexcept = default;
  constexpr qboolean(qboolean &&) noexcept = default;

  constexpr qboolean(bool b) noexcept : value(b ? 1 : 0) {}

  // Implicit conversion from all standard integer types
  template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
  constexpr qboolean(T val) noexcept : value(static_cast<int32_t>(val)) {}

  constexpr operator bool() const noexcept { return value != 0; }

  template <typename T, typename = std::enable_if_t<std::is_integral_v<T> &&
                                                    !std::is_same_v<T, bool>>>
  constexpr operator T() const noexcept {
    return static_cast<T>(value);
  }

  constexpr qboolean &operator=(const qboolean &) noexcept = default;
  constexpr qboolean &operator=(qboolean &&) noexcept = default;

  constexpr bool operator!() const noexcept { return value == 0; }
};

ASSERT_SIZE(qboolean, sizeof(int32_t));
static_assert(std::is_standard_layout_v<qboolean>,
              "qboolean must be standard layout!");
static_assert(std::is_trivially_copyable_v<qboolean>,
              "qboolean must be trivially copyable!");

constexpr qboolean qtrue = true;
constexpr qboolean qfalse = false;

// Type used by engine
typedef cdecl_t<int32_t, const void *, const void *>
    _CoreCrtNonSecureSearchSortCompareFunction;

// Can be used by boiii for more strict, specific argument typing
template <typename T>
using CoreCrtNonSecureSearchSortCompareFunction =
    cdecl_t<int32_t, const T *, const T *>;

// Type used by engine
typedef cdecl_t<int32_t, void *, const void *, const void *>
    _CoreCrtSecureSearchSortCompareFunction;

// Can be used by boiii for more strict, specific argument typing
template <typename T>
using CoreCrtSecureSearchSortCompareFunction =
    cdecl_t<int32_t, T *, const T *, const T *>;
} // namespace game