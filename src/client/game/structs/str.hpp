#pragma once

#include "array.hpp"

template <const auto N> using str = array<char, N>;

typedef str<8> str8_t;
typedef str<16> str16_t;
typedef str<24> str24_t;
typedef str<32> str32_t;
typedef str<64> str64_t;
typedef str<128> str128_t;
typedef str<256> str256_t;
typedef str<512> str512_t;