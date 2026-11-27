#pragma once

#include <exception>
#include <filesystem>
#include <type_traits>

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

template <typename T>
concept Function = std::is_function_v<T>;

template <typename T>
concept NonFunction = !Function<T>;

template <typename Derived, typename Base>
concept BasedOn = std::is_base_of_v<Base, Derived>;

template <typename T>
concept IsFilesystemPath =
    std::is_same_v<std::remove_cvref_t<T>, std::filesystem::path>;

template <typename T>
concept IsEnum = std::is_enum_v<std::remove_cvref_t<T>>;

template <typename T>
concept IsByte = std::is_same_v<std::remove_cvref_t<T>, std::byte>;

template <typename T>
concept IsException = std::is_base_of_v<std::exception, std::remove_cvref_t<T>>;

template <typename T>
concept IsString = std::is_same_v<std::remove_cvref_t<T>, std::string>;
