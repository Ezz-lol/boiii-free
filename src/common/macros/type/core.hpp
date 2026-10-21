#pragma once

template <typename T, size_t N>
concept SizeEqualTo = sizeof(T) == N;

template <typename T, size_t N>
concept SizeGreaterThan = sizeof(T) > N;

template <typename T, size_t N>
concept SizeGreaterOrEqualTo = SizeEqualTo<T, N> || SizeGreaterThan<T, N>;

template <typename T, size_t N>
concept SizeLessThan = sizeof(T) < N;

template <typename T, size_t N>
concept SizeLessThanOrEqualTo = SizeEqualTo<T, N> || SizeLessThan<T, N>;
