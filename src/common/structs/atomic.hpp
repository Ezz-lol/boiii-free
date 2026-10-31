#pragma once

#include "macros/type/integral.hpp" // IWYU pragma: keep
#include "structs/array.hpp"        // IWYU pragma: keep

#include <atomic>
#include <optional>

template <typename T, const IntegralLike auto N>
using atomicarray = array<std::atomic<T>, N>;

template <typename T> using atomic_optional = std::atomic<std::optional<T>>;

template <typename T, const IntegralLike auto N>
using atomic_optional_array = array<atomic_optional<T>, N>;
