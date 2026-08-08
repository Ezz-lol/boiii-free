#pragma once

#include <bit>
#include <cstdint>
#include <cassert>
#include <macros.hpp>

template <typename T, const IntegralLike auto N>
using array = T[static_cast<size_t>(N)];
template <const IntegralLike auto N> using bytearray = array<uint8_t, N>;
template <typename T, const IntegralLike auto X, const IntegralLike auto Y>
using matrix2d = array<array<T, Y>, X>;

template <size_t SubArrayLen, size_t StartIdx, typename T, size_t TotalLen>
inline constexpr array<T, SubArrayLen> &slice(array<T, TotalLen> &arr) {
  static_assert(StartIdx + SubArrayLen <= TotalLen,
                "Slice goes out of bounds!");

  // Cast the address of the start index to a reference of the new array size
  return reinterpret_cast<T(&)[SubArrayLen]>(arr[StartIdx]);
}

inline constexpr size_t alignment_size(size_t size, size_t align) {
  return (align - (size % align)) % align;
}

template <typename T, const IntegralLike auto N, const IntegralLike auto Align>
struct aligned_array {
  alignas(Align) array<T, N> items;

  inline constexpr void assert_range(size_t index) const {
    assert(index < N && "index to aligned_array must be < array length");
  }
  template <IntegralLike Index>
  inline constexpr const T &get(Index index_arg) const noexcept {
    const size_t index = static_cast<size_t>(index_arg);
    assert_range(index);
    return items[index];
  }
  template <IntegralLike Index>
  inline constexpr T &get(Index index_arg) noexcept {
    const size_t index = static_cast<size_t>(index_arg);
    assert_range(index);
    return items[index];
  }

  template <IntegralLike Index>
  inline constexpr const T &operator[](Index index) const noexcept {
    return get(index);
  }
  template <IntegralLike Index>
  inline constexpr T &operator[](Index index) noexcept {
    return get(index);
  }

  inline operator const T *() const noexcept { return items; }
  inline operator T *() noexcept { return items; }

  inline operator const void *() const noexcept { return items; }
  inline operator void *() noexcept { return items; }

  inline operator const array<T, N> &() const noexcept { return items; }
  inline operator array<T, N> &() noexcept { return items; }

  inline constexpr const void *allocation() const noexcept {
    return reintepret_cast<const void *>(
        static_cast<uintptr_t>(this) -
        /* padding */ std::max(alignment_size(sizeof(T) * N, Align), 0x8) -
        /* original allocation pointer */ sizeof(void *));
  }
  inline constexpr void *allocation() noexcept {
    return reinterpret_cast<void *>(
        static_cast<uintptr_t>(this) -
        /* padding */ std::max(alignment_size(sizeof(T) * N, Align), 0x8) -
        /* original allocation pointer */ sizeof(void *));
  }
};
static_assert(
    sizeof(aligned_array<uint8_t, 0x500000, 0x10>) == 0x500000,
    "sizeof(aligned_array<uint8_t, 0x500000, 0x10>) should be 0x500000");

template <typename T, const IntegralLike auto N, const IntegralLike auto Align>
struct aligned_array_ref {
  aligned_array<T, N, Align> *ref;

  inline constexpr void assert_range(size_t index) const {
    assert(index < N && "index to aligned_array must be < array length");
  }
  template <IntegralLike Index>
  inline constexpr const T &get(Index index_arg) const noexcept {
    const size_t index = static_cast<size_t>(index_arg);
    assert_range(index);
    return ref->get(index);
  }
  template <IntegralLike Index>
  inline constexpr T &get(Index index_arg) noexcept {
    const size_t index = static_cast<size_t>(index_arg);
    assert_range(index);
    return ref->get(index);
  }

  template <IntegralLike Index>
  inline constexpr const T &operator[](Index index) const noexcept {
    return get(index);
  }
  template <IntegralLike Index>
  inline constexpr T &operator[](Index index) noexcept {
    return get(index);
  }

  inline operator const T *() const noexcept { return *ref; }
  inline operator T *() noexcept { return *ref; }

  inline operator const void *() const noexcept { return *ref; }
  inline operator void *() noexcept { return *ref; }

  inline operator const array<T, N> &() const noexcept { return *ref; }
  inline operator array<T, N> &() noexcept { return *ref; }

  inline constexpr const void *allocation() const noexcept {
    return ref->allocation();
  }
  inline constexpr void *allocation() noexcept { return ref->allocation(); }
};

inline constexpr std::array<char, sizeof(uint64_t)>
byteswap(std::array<char, sizeof(uint64_t)> val) {
  return std::bit_cast<std::array<char, sizeof(uint64_t)>>(
      byteswap(std::bit_cast<uint64_t>(val)));
}

inline constexpr std::array<char, sizeof(uint32_t)>
byteswap(std::array<char, sizeof(uint32_t)> val) {
  return std::bit_cast<std::array<char, sizeof(uint32_t)>>(
      byteswap(std::bit_cast<uint32_t>(val)));
}

inline constexpr std::array<char, sizeof(uint16_t)>
byteswap(std::array<char, sizeof(uint16_t)> val) {
  return std::bit_cast<std::array<char, sizeof(uint16_t)>>(
      byteswap(std::bit_cast<uint16_t>(val)));
}