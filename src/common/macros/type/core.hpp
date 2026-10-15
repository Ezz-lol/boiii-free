#pragma once

template <typename T, size_t N>
concept SizeGreaterThan = sizeof(T) > N;
