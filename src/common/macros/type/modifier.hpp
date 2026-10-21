#pragma once

#include <type_traits>

template <typename T> struct remove_pointer_recursive {
  using type = std::remove_cv_t<T>;
};

template <typename T> struct remove_pointer_recursive<T *> {
  using type = typename remove_pointer_recursive<T>::type;
};

template <typename T> struct remove_pointer_recursive<T *const> {
  using type = typename remove_pointer_recursive<T>::type;
};

template <typename T> struct remove_pointer_recursive<T *volatile> {
  using type = typename remove_pointer_recursive<T>::type;
};

template <typename T> struct remove_pointer_recursive<T *const volatile> {
  using type = typename remove_pointer_recursive<T>::type;
};

template <typename T>
using remove_pointer_recursive_t = typename remove_pointer_recursive<T>::type;

template <typename T> struct remove_reference_recursive {
  using type = std::remove_cv_t<T>;
};

template <typename T> struct remove_reference_recursive<T &> {
  using type = typename remove_reference_recursive<T>::type;
};

template <typename T>
using remove_reference_recursive_t =
    typename remove_reference_recursive<T>::type;

// Same as std::remove_pointer_t, but also removes CV qualifiers from both data
// and pointer
template <typename T>
using strip_pointer_t =
    std::remove_cv_t<std::remove_pointer_t<std::remove_cv_t<T>>>;

template <typename T>
using strip_reference_t = std::remove_cv_t<std::remove_reference_t<T>>;
