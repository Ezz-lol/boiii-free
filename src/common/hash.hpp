#pragma once

#include <array>
#include <cstdint>

#include <structs/array.hpp>

#include <macros/macros.hpp>

// std::tolower is not a constexpr
alwaysinline constexpr char tolower(char c) noexcept {
  return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

inline constexpr uint8_t tolower(uint8_t c) noexcept {
  return static_cast<uint8_t>(tolower(static_cast<char>(c)));
}

typedef uint32_t hash32_t;
typedef uint64_t hash64_t;

template <const auto PRIME, typename T = decltype(PRIME)>
alwaysinline constexpr T fnv1a_update(const T hash, const char c) {
  return PRIME * (static_cast<T>(tolower(c)) ^ hash);
}

typedef hash32_t fnv1aHash_t;
inline constexpr fnv1aHash_t DEFAULT_FNV1A_IV = 0x811c9dc5;
inline constexpr fnv1aHash_t DEFAULT_FNV1A_PRIME = 0x01000193;

template <const fnv1aHash_t IV = DEFAULT_FNV1A_IV,
          const fnv1aHash_t PRIME = DEFAULT_FNV1A_PRIME>
inline constexpr fnv1aHash_t fnv1a(const char *str) noexcept {
  fnv1aHash_t hash = IV;
  for (const char *c = str; *c; ++c) {
    hash = fnv1a_update<PRIME>(hash, *c);
  }
  return hash;
}

template <const fnv1aHash_t IV = DEFAULT_FNV1A_IV,
          const fnv1aHash_t PRIME = DEFAULT_FNV1A_PRIME>
inline constexpr fnv1aHash_t fnv1a(const std::string_view &str) noexcept {
  fnv1aHash_t hash = IV;
  for (const char c : str) {
    hash = fnv1a_update<PRIME>(hash, c);
  }
  return hash;
}

typedef hash32_t fnv1aHashNull_t;

// Same as fnv1a, but also hashes null terminator.
// Used in e.g. `Dvar_GenerateHash`.
template <const fnv1aHashNull_t IV = DEFAULT_FNV1A_IV,
          const fnv1aHashNull_t PRIME = DEFAULT_FNV1A_PRIME>
alwaysinline constexpr fnv1aHashNull_t fnv1a_null(const char *str) noexcept {
  return PRIME * fnv1a<IV, PRIME>(str);
}

template <const fnv1aHashNull_t IV = DEFAULT_FNV1A_IV,
          const fnv1aHashNull_t PRIME = DEFAULT_FNV1A_PRIME>
alwaysinline constexpr fnv1aHashNull_t
fnv1a_null(const std::string_view &str) noexcept {
  return PRIME * fnv1a<IV, PRIME>(str);
}

template <const fnv1aHashNull_t IV = DEFAULT_FNV1A_IV,
          const fnv1aHashNull_t PRIME = DEFAULT_FNV1A_PRIME,
          IntegralLike<size_t> auto ArraySize>
inline constexpr std::array<fnv1aHashNull_t, ArraySize>
fnv1a_null(const array<const char *, ArraySize> &strings) noexcept {
  std::array<fnv1aHashNull_t, ArraySize> result = {0};
  for (size_t i = 0; i < static_cast<size_t>(ArraySize); ++i) {
    result[i] = fnv1a_null(strings[i]);
  }
  return result;
}

template <const fnv1aHashNull_t IV = DEFAULT_FNV1A_IV,
          const fnv1aHashNull_t PRIME = DEFAULT_FNV1A_PRIME,
          IntegralLike<size_t> auto ArraySize>
inline constexpr std::array<fnv1aHashNull_t, ArraySize>
fnv1a_null(const std::array<const char *, ArraySize> strings) noexcept {
  std::array<fnv1aHashNull_t, ArraySize> result = {0};
  for (size_t i = 0; i < static_cast<size_t>(ArraySize); ++i) {
    result[i] = fnv1a_null(strings[i]);
  }
  return result;
}

template <const fnv1aHashNull_t IV = DEFAULT_FNV1A_IV,
          const fnv1aHashNull_t PRIME = DEFAULT_FNV1A_PRIME>
inline constexpr std::pair<const fnv1aHashNull_t, const char *>
fnv1a_null_pair(const char *str) noexcept {
  return {PRIME * fnv1a<IV, PRIME>(str), str};
}

template <const fnv1aHashNull_t IV = DEFAULT_FNV1A_IV,
          const fnv1aHashNull_t PRIME = DEFAULT_FNV1A_PRIME,
          IntegralLike<size_t> auto ArraySize>
inline constexpr std::array<std::pair<fnv1aHashNull_t, const char *>, ArraySize>
fnv1a_null_pair(const array<const char *, ArraySize> &strings) noexcept {
  std::array<std::pair<fnv1aHashNull_t, const char *>, ArraySize> result = {};
  for (size_t i = 0; i < static_cast<size_t>(ArraySize); ++i) {
    result[i] = fnv1a_null_pair(strings[i]);
  }
  return result;
}

template <const fnv1aHashNull_t IV = DEFAULT_FNV1A_IV,
          const fnv1aHashNull_t PRIME = DEFAULT_FNV1A_PRIME,
          IntegralLike<size_t> auto ArraySize>
inline constexpr std::array<std::pair<fnv1aHashNull_t, const char *>, ArraySize>
fnv1a_null_pair(const std::array<const char *, ArraySize> strings) noexcept {
  std::array<std::pair<fnv1aHashNull_t, const char *>, ArraySize> result = {};
  for (size_t i = 0; i < static_cast<size_t>(ArraySize); ++i) {
    result[i] = fnv1a_null_pair(strings[i]);
  }
  return result;
}

typedef hash64_t fnv1aHash64_t;
inline constexpr fnv1aHash64_t DEFAULT_FNV1A64_IV = 0xcbf29ce484222325;
inline constexpr fnv1aHash64_t DEFAULT_FNV1A64_PRIME = 0x100000001b3;

template <const fnv1aHash64_t IV = DEFAULT_FNV1A64_IV,
          const fnv1aHash64_t PRIME = DEFAULT_FNV1A64_PRIME>
inline constexpr fnv1aHash64_t fnv1a64(const char *str) noexcept {
  fnv1aHash64_t hash = IV;
  for (const char *c = str; *c; ++c) {
    hash = fnv1a_update<PRIME>(hash, *c);
  }
  return hash;
}

template <const fnv1aHash64_t PRIME = DEFAULT_FNV1A64_PRIME>
inline constexpr fnv1aHash64_t fnv1a64(fnv1aHash64_t iv,
                                       const char *str) noexcept {
  fnv1aHash64_t hash = iv;
  for (const char *c = str; *c; ++c) {
    hash = fnv1a_update<PRIME>(hash, *c);
  }
  return hash;
}

template <const fnv1aHash64_t IV = DEFAULT_FNV1A64_IV,
          const fnv1aHash64_t PRIME = DEFAULT_FNV1A64_PRIME>
inline constexpr fnv1aHash64_t fnv1a64(const uint8_t *buf,
                                       size_t len) noexcept {
  fnv1aHash64_t hash = IV;
  for (size_t idx = 0; idx < len; ++idx) {
    hash = fnv1a64<PRIME>(hash, static_cast<char>(buf[idx]));
  }
  return hash;
}

template <const fnv1aHash64_t PRIME = DEFAULT_FNV1A64_PRIME>
inline constexpr fnv1aHash64_t fnv1a64(fnv1aHash64_t iv, const uint8_t *buf,
                                       size_t len) noexcept {
  fnv1aHash64_t hash = iv;
  for (size_t idx = 0; idx < len; ++idx) {
    hash = fnv1a_update<PRIME>(hash, static_cast<char>(buf[idx]));
  }
  return hash;
}

template <const fnv1aHash64_t PRIME = DEFAULT_FNV1A64_PRIME>
inline constexpr fnv1aHash64_t fnv1a64(fnv1aHash64_t iv,
                                       const std::string_view &str) noexcept {
  fnv1aHash64_t hash = iv;
  for (const char c : str) {
    hash = fnv1a_update<PRIME>(hash, c);
  }
  return hash;
}

template <const fnv1aHash64_t IV = DEFAULT_FNV1A64_IV,
          const fnv1aHash64_t PRIME = DEFAULT_FNV1A64_PRIME>
inline constexpr fnv1aHash64_t fnv1a64(const std::string_view &str) noexcept {
  fnv1aHash64_t hash = IV;
  for (const char c : str) {
    hash = fnv1a_update<PRIME>(hash, c);
  }
  return hash;
}

inline constexpr auto DEFAULT_DJB2_INITIAL_SEED = 0x1505;
inline constexpr auto DEFAULT_DJB2_CONSTANT = 0x21;

template <const auto CONSTANT, typename T = decltype(CONSTANT)>
alwaysinline constexpr T djb2_update(const T hash, const char c) {
  return static_cast<T>(tolower(c)) + hash * CONSTANT;
}

typedef hash32_t djb2Hash_t;
template <const djb2Hash_t INITIAL_SEED = DEFAULT_DJB2_INITIAL_SEED,
          const djb2Hash_t CONSTANT = DEFAULT_DJB2_CONSTANT>
inline constexpr djb2Hash_t djb2(const char *str) noexcept {
  djb2Hash_t hash = INITIAL_SEED;
  for (const char *c = str; *c; ++c) {
    hash = djb2_update<CONSTANT>(hash, *c);
  }
  return hash;
}

typedef hash32_t djb2Hash_t;
template <const djb2Hash_t INITIAL_SEED = DEFAULT_DJB2_INITIAL_SEED,
          const djb2Hash_t CONSTANT = DEFAULT_DJB2_CONSTANT>
inline constexpr djb2Hash_t djb2(const std::string_view &str) noexcept {
  djb2Hash_t hash = INITIAL_SEED;
  for (const char c : str) {
    hash = djb2_update<CONSTANT>(hash, c);
  }
  return hash;
}

typedef hash64_t djb2Hash64_t;
template <const djb2Hash64_t INITIAL_SEED = DEFAULT_DJB2_INITIAL_SEED,
          const djb2Hash_t CONSTANT = DEFAULT_DJB2_CONSTANT>
inline constexpr djb2Hash64_t djb264(const char *str) noexcept {
  djb2Hash64_t hash = INITIAL_SEED;
  for (const char *c = str; *c; ++c) {
    hash = djb2_update<CONSTANT>(hash, *c);
  }
  return hash;
}

typedef hash32_t djb2Hash_t;
template <const djb2Hash_t INITIAL_SEED = DEFAULT_DJB2_INITIAL_SEED,
          const djb2Hash_t CONSTANT = DEFAULT_DJB2_CONSTANT>
alwaysinline constexpr djb2Hash_t djb264(const std::string_view &str) noexcept {
  djb2Hash64_t hash = INITIAL_SEED;
  for (const char c : str) {
    hash = djb2_update<CONSTANT>(hash, c);
  }
  return hash;
}

inline constexpr auto DEFAULT_SDBM_INITIAL_SEED = 0;
inline constexpr auto DEFAULT_SDBM_CONSTANT = 0x1003f;

template <const auto CONSTANT, typename T = decltype(CONSTANT)>
alwaysinline constexpr T sdbm_update(const T hash, const char c) {
  return static_cast<T>(tolower(c)) + hash * CONSTANT;
}

template <const auto CONSTANT, typename T = decltype(CONSTANT)>
alwaysinline constexpr T sdbm_shift_update(const T hash, const char c) {
  return (hash << 16) + (hash << 6) + static_cast<T>(tolower(c)) - hash;
}

typedef hash32_t sdbmHash_t;
template <const sdbmHash_t INITIAL_SEED = DEFAULT_SDBM_INITIAL_SEED,
          const sdbmHash_t CONSTANT = DEFAULT_SDBM_CONSTANT>
inline constexpr sdbmHash_t sdbm(const char *str) noexcept {
  sdbmHash_t hash = INITIAL_SEED;
  for (const char *c = str; *c; ++c) {
    hash = sdbm_update<CONSTANT>(hash, *c);
  }
  return hash;
}

template <const sdbmHash_t INITIAL_SEED = DEFAULT_SDBM_INITIAL_SEED,
          const sdbmHash_t CONSTANT = DEFAULT_SDBM_CONSTANT>
inline constexpr sdbmHash_t sdbm(const std::string_view &str) noexcept {
  sdbmHash_t hash = INITIAL_SEED;
  for (const char c : str) {
    hash = sdbm_update<CONSTANT>(hash, c);
  }
  return hash;
}

typedef hash64_t sdbmHash64_t;
template <const sdbmHash64_t INITIAL_SEED = DEFAULT_SDBM_INITIAL_SEED,
          const sdbmHash64_t CONSTANT = DEFAULT_SDBM_CONSTANT>
inline constexpr sdbmHash64_t sdbm64(const char *str) noexcept {
  sdbmHash64_t hash = INITIAL_SEED;
  for (const char *c = str; *c; ++c) {
    hash = sdbm_update<CONSTANT>(hash, *c);
  }
  return hash;
}

template <const sdbmHash64_t INITIAL_SEED = DEFAULT_SDBM_INITIAL_SEED,
          const sdbmHash64_t CONSTANT = DEFAULT_SDBM_CONSTANT>
inline constexpr sdbmHash64_t sdbm64(const std::string_view &str) noexcept {
  sdbmHash64_t hash = INITIAL_SEED;
  for (const char c : str) {
    hash = sdbm_update<CONSTANT>(hash, c);
  }
  return hash;
}

// Used in engine when hashing paths, with part separator normalized
template <const sdbmHash_t INITIAL_SEED = DEFAULT_SDBM_INITIAL_SEED,
          const sdbmHash_t CONSTANT = DEFAULT_SDBM_CONSTANT>
inline constexpr sdbmHash_t sdbm_path(const char *str) noexcept {
  sdbmHash_t hash = INITIAL_SEED;
  for (const char *c = str; *c; ++c) {
    hash = sdbm_update<CONSTANT>(hash, *c == '\\' ? '/' : *c);
  }
  return hash;
}

template <const sdbmHash_t INITIAL_SEED = DEFAULT_SDBM_INITIAL_SEED,
          const sdbmHash_t CONSTANT = DEFAULT_SDBM_CONSTANT>
inline constexpr sdbmHash_t sdbm_path(const std::string_view &str) noexcept {
  sdbmHash_t hash = INITIAL_SEED;
  for (const char c : str) {
    hash = sdbm_update<CONSTANT>(hash, c == '\\' ? '/' : c);
  }
  return hash;
}
