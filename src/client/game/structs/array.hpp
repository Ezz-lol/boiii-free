#pragma once

#include <atomic>
#include <cstdint>
#include <macros.hpp>

template <typename T, const IntegralLike auto N>
using array = T[static_cast<size_t>(N)];
template <const IntegralLike auto N> using bytearray = array<uint8_t, N>;
template <typename T, const IntegralLike auto N>
using atomicarray = array<std::atomic<T>, N>;
template <typename T, const IntegralLike auto X, const IntegralLike auto Y>
using matrix2d = array<array<T, Y>, X>;

template <size_t SubArrayLen, size_t StartIdx, typename T, size_t TotalLen>
inline constexpr array<T, SubArrayLen> &slice(array<T, TotalLen> &arr) {
  static_assert(StartIdx + SubArrayLen <= TotalLen,
                "Slice goes out of bounds!");

  // Cast the address of the start index to a reference of the new array size
  return reinterpret_cast<T(&)[SubArrayLen]>(arr[StartIdx]);
}