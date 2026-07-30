#pragma once

#include <cstdint>

#include "../macros.hpp"
#include <macros.hpp>
#include <structs/func.hpp>

namespace game {

struct qboolean {
  int32_t value;

  inline constexpr operator bool() noexcept { return value != 0; }
  template <IntegralLike<int32_t> T> inline constexpr operator T() noexcept {
    return static_cast<T>(value);
  }
  inline constexpr bool operator!() noexcept { return value == 0; }

  inline constexpr operator bool() const noexcept { return value != 0; }
  template <IntegralLike<int32_t> T>
  inline constexpr operator T() const noexcept {
    return static_cast<T>(value);
  }
  inline constexpr bool operator!() const noexcept { return value == 0; }

  inline constexpr operator bool() volatile noexcept { return value != 0; }
  template <IntegralLike<int32_t> T>
  inline constexpr operator T() volatile noexcept {
    return static_cast<T>(value);
  }
  inline constexpr bool operator!() volatile noexcept { return value == 0; }

  template <IntegralLike<int32_t> T>
  static inline constexpr qboolean from(T val) noexcept {
    return qboolean{.value = static_cast<int32_t>(val)};
  }
};

ASSERT_SIZE(qboolean, sizeof(int32_t));
ASSERT_CPP03_POD(qboolean);
constexpr qboolean qtrue = qboolean::from(true);
constexpr qboolean qfalse = qboolean::from(false);

// Type used by engine
typedef cdeclPtr_t<int32_t(const void *, const void *)>
    _CoreCrtNonSecureSearchSortCompareFunction;

// Can be used by boiii for more strict, specific argument typing
template <typename T>
using CoreCrtNonSecureSearchSortCompareFunction =
    cdeclPtr_t<int32_t(const T *, const T *)>;

// Type used by engine
typedef cdeclPtr_t<int32_t(void *, const void *, const void *)>
    _CoreCrtSecureSearchSortCompareFunction;

// Can be used by boiii for more strict, specific argument typing
template <typename T>
using CoreCrtSecureSearchSortCompareFunction =
    cdeclPtr_t<int32_t(T *, const T *, const T *)>;
} // namespace game

using game::qboolean;
using game::qfalse;
using game::qtrue;