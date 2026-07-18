#pragma once

#include <cstdint>

// std::tolower is not a constexpr
inline constexpr char tolower(char c) noexcept {
  return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

inline constexpr unsigned char tolower(unsigned char c) noexcept {
  return static_cast<unsigned char>(tolower(static_cast<char>(c)));
}

typedef uint32_t hash32_t;
typedef uint64_t hash64_t;

typedef hash32_t fnv1aHash_t;

template <const fnv1aHash_t IV = 0x811c9dc5,
          const fnv1aHash_t PRIME = 0x01000193>
inline constexpr fnv1aHash_t fnv1a(const char *str) noexcept {
  fnv1aHash_t hash = IV;
  for (const char *c = str; *c; ++c) {
    hash = PRIME *
           (static_cast<fnv1aHash_t>(tolower(static_cast<unsigned char>(*c))) ^
            hash);
  }
  return hash;
}

typedef hash32_t fnv1aHashNull_t;

// Same as fnv1a, but also hashes null terminator.
// Used in e.g. `Dvar_GenerateHash`.
template <const fnv1aHashNull_t IV = 0x811c9dc5,
          const fnv1aHashNull_t PRIME = 0x01000193>
inline constexpr fnv1aHashNull_t fnv1a_null(const char *str) noexcept {
  return PRIME * fnv1a<IV, PRIME>(str);
}

typedef hash64_t fnv1aHash64_t;

template <const fnv1aHash64_t IV = 0x811c9dc5,
          const fnv1aHash64_t PRIME = 0x01000193>
inline constexpr fnv1aHash64_t fnv1a64(const char *str) noexcept {
  fnv1aHash64_t hash = IV;
  for (const char *c = str; *c; ++c) {
    hash =
        PRIME *
        (static_cast<fnv1aHash64_t>(tolower(static_cast<unsigned char>(*c))) ^
         hash);
  }
  return hash;
}

typedef hash32_t djb2Hash_t;
template <const djb2Hash_t INITIAL_SEED, const djb2Hash_t CONSTANT>
inline constexpr djb2Hash_t djb2(const char *str) noexcept {
  djb2Hash_t hash = INITIAL_SEED;
  for (const char *c = str; *c; ++c) {
    hash = static_cast<djb2Hash_t>(tolower(static_cast<unsigned char>(*c))) +
           hash * CONSTANT;
  }
  return hash;
}