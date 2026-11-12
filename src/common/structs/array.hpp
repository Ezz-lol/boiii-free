#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <span>

#include "macros/byte.hpp"          // IWYU pragma: keep
#include "macros/type/integral.hpp" // IWYU pragma: keep

template <typename T, const IntegralLike auto N>
using array = T[static_cast<size_t>(N)];
template <const IntegralLike auto N> using bytearray = array<uint8_t, N>;
template <typename T, const IntegralLike auto X, const IntegralLike auto Y>
using matrix2d = array<array<T, Y>, X>;

template <const IntegralLike<size_t> auto SubArrayLen,
          const IntegralLike<size_t> auto StartIdx = 0, typename T,
          const IntegralLike<size_t> auto TotalLen>
inline constexpr array<T, SubArrayLen> &slice(T (&arr)[TotalLen] /* C-Style array length cannot
                                                                    automatically be deduced in
                                                                    type defintion - cannot use
                                                                    `array<>` here */) {
  static_assert(static_cast<size_t>(StartIdx) +
                        static_cast<size_t>(SubArrayLen) <=
                    static_cast<size_t>(TotalLen),
                "Slice goes out of bounds!");

  // Cast the address of the start index to a reference of the new array size
  return reinterpret_cast<array<T, static_cast<size_t>(SubArrayLen)> &>(
      arr[static_cast<size_t>(StartIdx)]);
}

inline constexpr size_t alignment_size(size_t size, size_t align) {
  return (align - (size % align)) % align;
}

template <typename T, const IntegralLike<size_t> auto N,
          const IntegralLike<size_t> auto Align>
struct aligned_array {
  alignas(Align) array<T, N> items;

  typedef size_t index_t;

  inline constexpr void assert_range([[maybe_unused]] index_t index) const {
    assert(index < N && "index to aligned_array must be < array length");
  }
  template <IntegralLike<index_t> Index>
  inline constexpr const T &get(Index index_arg) const noexcept {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return items[index];
  }
  template <IntegralLike<index_t> Index>
  inline constexpr T &get(Index index_arg) noexcept {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return items[index];
  }

  template <IntegralLike<index_t> Index>
  inline constexpr const T &operator[](Index index) const noexcept {
    return get(index);
  }
  template <IntegralLike<index_t> Index>
  inline constexpr T &operator[](Index index) noexcept {
    return get(index);
  }

  inline operator const T *() const noexcept { return items; }
  inline operator T *() noexcept { return items; }

  inline operator const void *() const noexcept { return items; }
  inline operator void *() noexcept { return items; }

  inline operator const array<T, N> &() const noexcept { return items; }
  inline operator array<T, N> &() noexcept { return items; }

  [[nodiscard]] inline constexpr const void *allocation() const noexcept {
    return reintepret_cast<const void *>(
        static_cast<uintptr_t>(this) -
        /* padding */
        std::max<size_t>(alignment_size(sizeof(T) * N, Align),
                         sizeof(uintptr_t)) -
        /* original allocation pointer */ sizeof(void *));
  }
  [[nodiscard]] inline constexpr void *allocation() noexcept {
    return reinterpret_cast<void *>(
        static_cast<uintptr_t>(this) -
        /* padding */
        std::max<size_t>(alignment_size(sizeof(T) * N, Align),
                         sizeof(uintptr_t)) -
        /* original allocation pointer */ sizeof(void *));
  }
};
static_assert(
    sizeof(aligned_array<uint8_t, 0x500000, 0x10>) == 0x500000,
    "sizeof(aligned_array<uint8_t, 0x500000, 0x10>) should be 0x500000");

template <typename T, const IntegralLike auto N, const IntegralLike auto Align>
struct aligned_array_ref {
  aligned_array<T, N, Align> *ref;

  inline constexpr void assert_range([[maybe_unused]] size_t index) const {
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

  [[nodiscard]] inline constexpr const void *allocation() const noexcept {
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

template <typename T, const IntegralLike<size_t> auto SIZE,
          typename index_t = size_t>
struct Pool {

  T pool[SIZE];

  constexpr void assert_range([[maybe_unused]] index_t index) const {
    assert(index < SIZE &&
           "index to Pool must satisfy range 0 <= index < SIZE");
  }

  template <IntegralLike<index_t> Index>
  constexpr const T &get(Index index_arg) const noexcept {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return pool[index];
  }

  template <IntegralLike<index_t> Index>
  constexpr T &get(Index index_arg) noexcept {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return pool[index];
  }

  template <IntegralLike<index_t> Index>
  constexpr const T &operator[](Index index) const noexcept {
    return get(index);
  }

  template <IntegralLike<index_t> Index>
  constexpr T &operator[](Index index) noexcept {
    return get(index);
  }

  operator const T *() const noexcept { return pool; }
  operator T *() noexcept { return pool; }

  operator const void *() const noexcept { return pool; }
  operator void *() noexcept { return pool; }

  operator const std::array<T, SIZE> &() const noexcept { return pool; }
  operator std::array<T, SIZE> &() noexcept { return pool; }

  operator std::span<const T, SIZE>() const noexcept {
    return std::span<const T, SIZE>(pool);
  }

  operator std::span<T, SIZE>() noexcept { return std::span<T, SIZE>(pool); }

  static inline constexpr auto size() noexcept { return SIZE; }

  [[nodiscard]] static constexpr bool empty() noexcept { return SIZE == 0; }

  constexpr auto ssize() const noexcept {
    return static_cast<std::ptrdiff_t>(SIZE);
  }

  constexpr T *data() noexcept { return pool; }
  constexpr const T *data() const noexcept { return pool; }

  constexpr T *begin() noexcept { return pool; }
  constexpr const T *begin() const noexcept { return pool; }
  constexpr const T *cbegin() const noexcept { return pool; }

  constexpr T *end() noexcept { return pool + SIZE; }
  constexpr const T *end() const noexcept { return pool + SIZE; }
  constexpr const T *cend() const noexcept { return pool + SIZE; }

  constexpr std::reverse_iterator<T *> rbegin() noexcept {
    return std::reverse_iterator<T *>(end());
  }
  constexpr std::reverse_iterator<const T *> rbegin() const noexcept {
    return std::reverse_iterator<const T *>(end());
  }
  constexpr std::reverse_iterator<const T *> crbegin() const noexcept {
    return std::reverse_iterator<const T *>(end());
  }

  constexpr std::reverse_iterator<T *> rend() noexcept {
    return std::reverse_iterator<T *>(begin());
  }
  constexpr std::reverse_iterator<const T *> rend() const noexcept {
    return std::reverse_iterator<const T *>(begin());
  }
  constexpr std::reverse_iterator<const T *> crend() const noexcept {
    return std::reverse_iterator<const T *>(begin());
  }
};
