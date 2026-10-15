#pragma once

#include <array>

#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif

#ifndef ARRAYSIZE
template <typename Element, const size_t Count>
inline constexpr auto ARRAYSIZE(const Element (&)[Count]) {
  return Count;
}

template <typename Element, const size_t Count>
inline constexpr auto ARRAYSIZE(const std::array<Element, Count> &arr) {
  return arr.size();
}
#endif

template <typename T> using element_of = std::remove_all_extents_t<T>;
