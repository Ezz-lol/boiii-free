#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <type_traits>

#include "macros/optimization.hpp"
#include "macros/type/integral.hpp"

#ifndef INTRINSIC_MEMSET
#if defined(__clang__) || defined(__GNUC__)
#define INTRINSIC_MEMSET(buf, val, count) __builtin_memset(buf, val, count)
#elif defined(_MSC_VER)
#pragma intrinsic(memset)
#define INTRINSIC_MEMSET(buf, val, count) memset(buf, val, count)
#else
#error "Unsupported compiler. Only MSVC, Clang and GCC are supported."
#endif
#endif

#ifndef INTRINSIC_MEMCPY
#if defined(__clang__) || defined(__GNUC__)
#define INTRINSIC_MEMCPY(buf, src, count) __builtin_memcpy(buf, src, count)
#elif defined(_MSC_VER)
#pragma intrinsic(memcpy)
#define INTRINSIC_MEMCPY(buf, src, count) memcpy(buf, src, count)
#else
#error "Unsupported compiler. Only MSVC, Clang and GCC are supported."
#endif
#endif

#ifndef INTRINSIC_MEMCMP
#if defined(__clang__) || defined(__GNUC__)
#define INTRINSIC_MEMCMP(lhs, rhs, count) __builtin_memcmp(lhs, rhs, count)
#elif defined(_MSC_VER)
#pragma intrinsic(memcmp)
#define INTRINSIC_MEMCMP(lhs, rhs, count) memcmp(lhs, rhs, count)
#else
#error "Unsupported compiler. Only MSVC, Clang and GCC are supported."
#endif
#endif

/**
 * @brief Casts between pointer types using `std::bit_cast` if executed at
 * compile-time or `reinterpret_cast` if executed at runtime.
 */
template <typename Out, typename In>
  requires(std::is_pointer_v<In> && std::is_pointer_v<Out>)
inline constexpr Out constexpr_ptr_cast(const In in) {
  if (std::is_constant_evaluated()) {
    return std::bit_cast<Out>(in);
  } else {
    return reinterpret_cast<Out>(in);
  }
}

/**
 * @brief Fills a block of memory with a specified byte value, supporting both
 * compile-time and runtime evaluation.
 */
template <typename T, IntegralLike<uint8_t> Val, IntegralLike<size_t> Size>
inline constexpr T *constexpr_memset(T *dst_arg, Val val_arg,
                                     Size count) noexcept {
  const uint8_t val = static_cast<uint8_t>(val_arg);
  const size_t n = static_cast<size_t>(count);
  uint8_t *dst = constexpr_ptr_cast<uint8_t *>(dst_arg);

  if (std::is_constant_evaluated()) {
    UNROLL_NEXT_LOOP
    for (size_t i = 0; i < n; ++i) {
      dst[i] = val;
    }
  } else {
    INTRINSIC_MEMSET(dst, val, n);
  }

  return dst_arg;
}

/**
 * @brief Fills a std::span with a specified byte value, supporting both
 * compile-time and runtime evaluation.
 */
template <typename T, size_t Extent, IntegralLike<uint8_t> Val>
inline constexpr std::span<T, Extent> constexpr_memset(std::span<T, Extent> dst,
                                                       Val val_arg) noexcept {
  constexpr_memset(dst.data(), val_arg, dst.size_bytes());
  return dst;
}

/**
 * @brief Fills a C-style array with a specified byte value.
 */
template <typename T, const size_t Extent, IntegralLike<uint8_t> Val>
inline constexpr T (&constexpr_memset(T (&dst)[Extent], Val val_arg)) [Extent] {
  constexpr_memset(std::span<T, Extent>(dst, Extent), val_arg);
  return dst;
}

/**
 * @brief Fills an std::array object with a specified byte value.
 */
template <typename T, const size_t Extent, IntegralLike<uint8_t> Val>
inline constexpr std::array<T, Extent> &constexpr_memset(
    std::array<T, Extent> &dst, Val val_arg) noexcept {
  constexpr_memset(std::span<T, Extent>(dst.data(), Extent), val_arg);
  return dst;
}

/**
 * @brief Copies a block of memory from source to destination, supporting both
 * compile-time and runtime evaluation.
 */
template <typename D, typename S, IntegralLike<size_t> Size>
inline constexpr D *constexpr_memcpy(D *dst_arg, const S *src_arg,
                                     Size count) noexcept {
  const size_t n = static_cast<size_t>(count);
  uint8_t *dst = constexpr_ptr_cast<uint8_t *>(dst_arg);
  const uint8_t *src = constexpr_ptr_cast<const uint8_t *>(src_arg);

  if (std::is_constant_evaluated()) {
    UNROLL_NEXT_LOOP
    for (size_t i = 0; i < n; ++i) {
      dst[i] = src[i];
    }
  } else {
    INTRINSIC_MEMCPY(reinterpret_cast<void *>(dst_arg),
                     reinterpret_cast<const void *>(src_arg), n);
  }

  return dst_arg;
}

/**
 * @brief Copies data between two std::span objects, with support for both
 * static and dynamic extents.
 */
template <typename T, size_t ExtentDst, size_t ExtentSrc>
inline constexpr std::span<T, ExtentDst>
constexpr_memcpy(std::span<T, ExtentDst> dst,
                 std::span<const T, ExtentSrc> src) noexcept {
  if (std::is_constant_evaluated()) {
    constexpr size_t Count = std::min<size_t>(ExtentDst, ExtentSrc);
    UNROLL_NEXT_LOOP
    for (size_t i = 0; i < Count; ++i) {
      dst[i] = src[i];
    }
  } else {
    const size_t count = std::min<size_t>(dst.size_bytes(), src.size_bytes());
    INTRINSIC_MEMCPY(reinterpret_cast<void *>(dst.data()),
                     reinterpret_cast<const void *>(src.data()), count);
  }

  return dst;
}

/**
 * @brief Copies data between two C-style arrays.
 */
template <typename T, const size_t ExtentDst, const size_t ExtentSrc>
inline constexpr T (&constexpr_memcpy(T (&dst)[ExtentDst],
                                      const T (&src)[ExtentSrc])) [ExtentDst] {
  constexpr_memcpy(std::span<T, ExtentDst>(dst, ExtentDst),
                   std::span<const T, ExtentSrc>(src, ExtentSrc));
  return dst;
}

/**
 * @brief Copies data between two std::array objects.
 */
template <typename T, const size_t ExtentDst, const size_t ExtentSrc>
inline constexpr std::array<T, ExtentDst> &constexpr_memcpy(
    std::array<T, ExtentDst> &dst,
    const std::array<T, ExtentSrc> &src) noexcept {
  constexpr_memcpy(std::span<T, ExtentDst>(dst.data(), ExtentDst),
                   std::span<const T, ExtentSrc>(src.data(), ExtentSrc));
  return dst;
}

/**
 * @brief Compares two blocks of memory, supporting both
 * compile-time and runtime evaluation.
 */
template <typename L, typename R, IntegralLike<size_t> Size>
inline constexpr int32_t constexpr_memcmp(const L *lhs_arg, const R *rhs_arg,
                                          Size count) noexcept {
  const size_t n = static_cast<size_t>(count);
  const uint8_t *lhs = constexpr_ptr_cast<const uint8_t *>(lhs_arg);
  const uint8_t *rhs = constexpr_ptr_cast<const uint8_t *>(rhs_arg);

  if (std::is_constant_evaluated()) {
    UNROLL_NEXT_LOOP
    for (size_t i = 0; i < n; ++i) {
      if (lhs[i] != rhs[i]) {
        return static_cast<int32_t>(lhs[i]) - static_cast<int32_t>(rhs[i]);
      }
    }
  } else {
    return INTRINSIC_MEMCMP(reinterpret_cast<const void *>(lhs_arg),
                            reinterpret_cast<const void *>(rhs_arg), n);
  }

  return 0;
}

/**
 * @brief Compares data between two std::span objects, with support for both
 * static and dynamic extents.
 */
template <typename T, size_t ExtentLhs, size_t ExtentRhs>
inline constexpr int32_t
constexpr_memcmp(std::span<const T, ExtentLhs> lhs,
                 std::span<const T, ExtentRhs> rhs) noexcept {
  if (std::is_constant_evaluated()) {
    constexpr size_t Count = std::min<size_t>(ExtentLhs, ExtentRhs);
    UNROLL_NEXT_LOOP
    for (size_t i = 0; i < Count; ++i) {
      if (lhs[i] != rhs[i]) {
        return static_cast<int32_t>(lhs[i]) - static_cast<int32_t>(rhs[i]);
      }
    }
  } else {
    const size_t count = std::min<size_t>(lhs.size_bytes(), rhs.size_bytes());
    return INTRINSIC_MEMCMP(reinterpret_cast<const void *>(lhs.data()),
                            reinterpret_cast<const void *>(rhs.data()), count);
  }

  return 0;
}

/**
 * @brief Compares data between two C-style arrays.
 */
template <typename T, const size_t ExtentLhs, const size_t ExtentRhs>
inline constexpr int constexpr_memcmp(const T (&lhs)[ExtentLhs],
                                      const T (&rhs)[ExtentRhs]) noexcept {
  return constexpr_memcmp(std::span<const T, ExtentLhs>(lhs, ExtentLhs),
                          std::span<const T, ExtentRhs>(rhs, ExtentRhs));
}

/**
 * @brief Compares data between two std::array objects.
 */
template <typename T, const size_t ExtentLhs, const size_t ExtentRhs>
inline constexpr int
constexpr_memcmp(const std::array<T, ExtentLhs> &lhs,
                 const std::array<T, ExtentRhs> &rhs) noexcept {
  return constexpr_memcmp(std::span<const T, ExtentLhs>(lhs.data(), ExtentLhs),
                          std::span<const T, ExtentRhs>(rhs.data(), ExtentRhs));
}

/**
 * @brief Computes the total byte length of concatenated arguments at compile
 * time based purely on their types.
 */
template <typename B, typename... Args>
constexpr size_t concatenated_bytes_len_v = []() {
  return ((std::is_array_v<std::remove_cvref_t<Args>>
               ? std::extent_v<std::remove_cvref_t<Args>>
               : sizeof(B)) +
          ...);
}();

/**
 * @brief Concatenates any combination and order of scalar byte and byte array
 * arguments into one, flattened byte array.
 */
template <typename B = uint8_t, typename... Args>
  requires(sizeof(B) == sizeof(uint8_t))
constexpr std::array<B, concatenated_bytes_len_v<B, Args...>>
concat_bytes(Args &&...args) {
  using returnType_t =
      std::invoke_result_t<decltype(concat_bytes<B, Args...>), Args...>;
  returnType_t result{};
  size_t offset = 0;

  // Unpack array and append contents or append scalar byte
  auto append = [&result, &offset]<typename T>(T &&arg) {
    using Decayed = std::remove_cvref_t<T>;
    if constexpr (std::is_array_v<Decayed>) {
      constexpr size_t extent = std::extent_v<Decayed>;
      [&result, &offset, &arg]<size_t... Is>(std::index_sequence<Is...>) {
        ((result[offset++] = static_cast<B>(arg[Is])), ...);
      }(std::make_index_sequence<extent>{});
    } else {
      result[offset++] = static_cast<B>(arg);
    }
  };

  // Fold expression, processing all arguments in order
  (append(std::forward<Args>(args)), ...);

  return result;
}
