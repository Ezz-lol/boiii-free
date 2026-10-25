#pragma once

#include <type_traits>

#include "core.hpp"

template <typename T>
concept ScopedEnum =
    std::is_enum_v<T> && !std::is_convertible_v<T, std::underlying_type_t<T>>;
template <typename T, typename V>
concept ScopedUnderlying =
    ScopedEnum<T> && std::is_same_v<std::underlying_type_t<T>, V>;

template <ScopedEnum T>
inline constexpr std::underlying_type_t<T> to_underlying(T e) noexcept {
  return static_cast<std::underlying_type_t<T>>(e);
}

template <typename From, typename To = uint64_t>
concept ScopedIntegralLike =
    ScopedEnum<From> &&
    (ScopedUnderlying<From, int8_t> || ScopedUnderlying<From, uint8_t> ||
     ScopedUnderlying<From, int16_t> || ScopedUnderlying<From, uint16_t> ||
     ScopedUnderlying<From, int32_t> || ScopedUnderlying<From, uint32_t> ||
     ScopedUnderlying<From, int64_t> || ScopedUnderlying<From, uint64_t> ||
     std::is_integral_v<std::underlying_type_t<From>> ||
     std::is_convertible_v<std::underlying_type_t<From>, To>);

template <typename From, typename To = uint64_t>
concept IntegralLike =
    std::is_integral_v<To> &&
    (std::is_convertible_v<From, To> || ScopedIntegralLike<From, To>);

template <typename From, typename To>
concept SizedIntegralLike =
    (std::is_convertible_v<From, To> || ScopedIntegralLike<From, To>) &&
    !SizeGreaterThan<From, sizeof(To)>;

template <typename T>
concept Function = std::is_function_v<T>;

template <typename T>
concept NonFunction = !Function<T>;
