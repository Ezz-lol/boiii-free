#pragma once

#include "macros/builtin.hpp"
#include <array>

#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif

#ifndef ARRAYSIZE
/**
 * @brief Returns the compile-time size of a C-style or std::array.
 */
template <typename Element, const size_t Count, typename Array>
  requires(
      std::is_same_v<std::remove_cvref_t<Array>, Element[Count]> ||
      std::is_same_v<std::remove_cvref_t<Array>, std::array<Element, Count>>)
alwaysinline constexpr size_t
ARRAYSIZE([[maybe_unused]] const Array & /*unused*/) {
  return Count;
}
#endif

/**
 * @brief Traits helper for array types providing element type and size.
 */
template <typename T> struct array_traits;

template <typename T, std::size_t N> struct array_traits<T[N]> {
  using element_type = T;
  // Lower snakecase is standard for type utilities
  // NOLINTNEXTLINE(readability-identifier-naming)
  static constexpr std::size_t size = N;
};

template <typename T, std::size_t N> struct array_traits<std::array<T, N>> {
  using element_type = T;
  // Lower snakecase is standard for type utilities
  // NOLINTNEXTLINE(readability-identifier-naming)
  static constexpr std::size_t size = N;
};

/**
 * @brief Extracts the base element type of an array type.
 */
template <typename T>
using element_of = array_traits<std::remove_cvref_t<T>>::element_type;

/**
 * @brief Helper variable template to retrieve the size of an array type at
 * compile time.
 */
template <typename T>
// Lower snakecase is standard for type utilities
// NOLINTNEXTLINE(readability-identifier-naming)
inline constexpr std::size_t array_size_v =
    array_traits<std::remove_cvref_t<T>>::size;

/**
 * @brief Templated type alias computing the resulting mapped std::array type.
 */
template <typename Array, typename Func>
using mapped_array_v =
    std::array<std::decay_t<std::invoke_result_t<Func, element_of<Array>>>,
               array_size_v<Array>>;

/**
 * @brief Implementation helper that applies a function to each element of an
 * array using an index sequence, returning a strongly-typed mapped array.
 */
template <typename Array, typename Func, std::size_t... I>
constexpr mapped_array_v<Array, Func>
map_array_impl(const Array &arr, Func &&func,
               [[maybe_unused]] std::index_sequence<I...> /*unused*/) {
  return mapped_array_v<Array, Func>{func(arr[I])...};
}

/**
 * @brief Applies a given function to each element of a C-style array or
 * std::array, returning a new strongly-typed mapped array containing the
 * results.
 */
template <typename Array, typename Func>
constexpr mapped_array_v<Array, Func> map_array(const Array &arr, Func &&func) {
  return map_array_impl(arr, std::forward<Func>(func),
                        std::make_index_sequence<array_size_v<Array>>{});
}
