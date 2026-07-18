#pragma once

#include <cstdint>
#include <type_traits>

#include "../macros.hpp"
#include "../func.hpp"

namespace game {

struct qboolean {
  int32_t value;

  constexpr operator bool() const noexcept { return value != 0; }
  template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
  constexpr operator T() const noexcept {
    return static_cast<T>(value);
  }
  constexpr bool operator!() const noexcept { return value == 0; }

  template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
  static constexpr qboolean from(T val) noexcept {
    return qboolean{.value = static_cast<int32_t>(val)};
  }
};

ASSERT_SIZE(qboolean, sizeof(int32_t));
ASSERT_POD(qboolean);
constexpr qboolean qtrue = qboolean::from(true);
constexpr qboolean qfalse = qboolean::from(false);

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

using game::qboolean;
using game::qfalse;
using game::qtrue;