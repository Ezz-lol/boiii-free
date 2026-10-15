#pragma once

#include <cstdint>
#include <type_traits>

inline constexpr uint64_t byteswap(uint64_t val) {
#if defined(__GNUC__) || defined(__clang__)
  return __builtin_bswap64(val);
#elif defined(_MSC_VER)
  return _byteswap_uint64(val);
#else
  // Fallback if compiler is unknown
  return ((val & 0xFF00000000000000ULL) >> 56) |
         ((val & 0x00FF000000000000ULL) >> 40) |
         ((val & 0x0000FF0000000000ULL) >> 24) |
         ((val & 0x000000FF00000000ULL) >> 8) |
         ((val & 0x00000000FF000000ULL) << 8) |
         ((val & 0x0000000000FF0000ULL) << 24) |
         ((val & 0x000000000000FF00ULL) << 40) |
         ((val & 0x00000000000000FFULL) << 56);
#endif
}

template <typename T>
  requires(sizeof(T) == sizeof(uint64_t) && !std::is_same_v<T, uint64_t> &&
           std::is_convertible_v<T, uint64_t>)
inline constexpr T byteswap(T val) {
  return static_cast<T>(byteswap(static_cast<uint64_t>(val)));
}

inline constexpr uint32_t byteswap(uint32_t val) {
#if defined(__GNUC__) || defined(__clang__)
  return __builtin_bswap32(val);
#elif defined(_MSC_VER)
  return _byteswap_ulong(val);
#else
  // Fallback if compiler is unknown
  return ((val & 0xFF000000U) >> 24) | ((val & 0x00FF0000U) >> 8) |
         ((val & 0x0000FF00U) << 8) | ((val & 0x000000FFU) << 24);
#endif
}

template <typename T>
  requires(sizeof(T) == sizeof(uint32_t) && !std::is_same_v<T, uint32_t> &&
           std::is_convertible_v<T, uint32_t>)
inline constexpr T byteswap(T val) {
  return static_cast<T>(byteswap(static_cast<uint32_t>(val)));
}

inline constexpr uint16_t byteswap(uint16_t val) {
#if defined(__GNUC__) || defined(__clang__)
  return __builtin_bswap16(val);
#elif defined(_MSC_VER)
  return _byteswap_ushort(val);
#else
  // Fallback if compiler is unknown
  return static_cast<uint16_t>(((val & 0xFF00U) >> 8) | ((val & 0x00FFU) << 8));
#endif
}

template <typename T>
  requires(sizeof(T) == sizeof(uint16_t) && !std::is_same_v<T, uint16_t>,
           std::is_convertible_v<T, uint16_t>)
inline constexpr T byteswap(T val) {
  return static_cast<T>(byteswap(static_cast<uint16_t>(val)));
}
