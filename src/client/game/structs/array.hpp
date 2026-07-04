#pragma once

#include <atomic>
#include <cstdint>

template <typename T, const auto N> using array = T[N];
template <const auto N> using bytearray = array<uint8_t, N>;
template <typename T, const auto N>
using atomicarray = array<std::atomic<T>, N>;
template <typename T, const auto X, const auto Y>
using matrix2d = array<array<T, Y>, X>;