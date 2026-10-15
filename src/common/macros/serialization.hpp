#pragma once

#include <concepts>
#include <format>
#include <string>
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

// Same as std::remove_pointer_t, but also removes CV qualifiers from both data
// and pointer
template <typename T>
using strip_pointer_t =
    std::remove_cv_t<std::remove_pointer_t<std::remove_cv_t<T>>>;

template <typename T, typename = std::void_t<>>
struct has_serialize : std::false_type {};

template <typename T>
struct has_serialize<T, std::void_t<decltype(std::declval<T &>().serialize())>>
    : std::true_type {};

template <typename T> constexpr bool has_serialize_v = has_serialize<T>::value;

template <typename T>
concept Serializable = requires(T t) { t.serialize(); };

template <typename T>
concept SerializablePtr = !std::is_reference_v<T> && std::is_pointer_v<T> &&
                          Serializable<strip_pointer_t<T>>;

// Require bool argument in template rather than in argument type to ensure this
// is an _explicit_ boolean argument, and not a value implicitly casted to
// boolean
template <typename T>
  requires(std::same_as<T, bool>)
inline constexpr const char *serialize(const T val) {
  return val ? "true" : "false";
}

inline constexpr std::string serialize(const char *val) {
  return val ? std::format("\"{}\"", val) : "NULL";
}

template <SerializablePtr T> inline std::string serialize(const T &ptr) {
  return ptr ? ptr->serialize() : "NULL";
}

template <Serializable T> inline std::string serialize(const T &ptr) {
  return ptr.serialize();
}

template <typename T>
  requires(std::is_integral_v<T> && !std::is_same_v<T, char> &&
           !std::is_same_v<T, bool>)
inline std::string serialize(const T *const(&val)) {
  return val ? std::format("0x{:X}", *val) : "NULL";
}

template <typename T>
  requires(std::floating_point<T> && !std::is_same_v<T, char> &&
           !std::is_same_v<T, bool>)
inline std::string serialize(const T *const(&val)) {
  return val ? std::to_string(*val) : "NULL";
}

template <typename T>
  requires(std::is_integral_v<T> && !std::is_enum_v<T> &&
           !std::is_same_v<T, bool> && !std::is_same_v<T, char>)
inline std::string serialize(const T *arr, const size_t N) {
  std::string result = "[";
  if (N && arr) {
    result += std::format("0x{:X}", arr[0]);

    for (size_t i = 1; i < N; ++i) {
      result += std::format(", 0x{:X}", arr[i]);
    }
  }
  result += "]";
  return result;
}

template <typename T>
  requires(std::is_integral_v<T> && !std::is_enum_v<T> &&
           !std::is_same_v<T, bool> && !std::is_same_v<T, char>)
inline std::string serialize(const T *const *const arr, const size_t N) {
  std::string result = "[";
  if (N && arr && arr[0]) {
    result += std::format("0x{:X}", *arr[0]);

    for (size_t i = 1; i < N; ++i) {
      if (arr[i]) {
        result += std::format(", 0x{:X}", *arr[i]);
      }
    }
  }
  result += "]";
  return result;
}

template <typename T>
  requires(std::is_floating_point_v<T>)
inline std::string serialize(const T *const *const arr, const size_t N) {
  std::string result = "[";
  if (N && arr && arr[0]) {
    result += std::format("{}", *arr[0]);

    for (size_t i = 1; i < N; ++i) {
      if (arr[i]) {
        result += std::format(", {}", *arr[i]);
      }
    }
  }
  result += "]";
  return result;
}

template <typename T>
  requires(std::is_floating_point_v<T>)
inline std::string serialize(const T *arr, const size_t N) {
  std::string result = "[";
  if (N && arr) {
    result += std::format("{}", arr[0]);

    for (size_t i = 1; i < N; ++i) {
      result += std::format(", {}", arr[i]);
    }
  }
  result += "]";
  return result;
}

template <typename T>
  requires(std::is_enum_v<T> || std::is_same_v<T, bool>)
inline std::string serialize(const T *const arr, const size_t N) {
  std::string result = "[";
  if (N && arr) {
    result += std::format(", {}", serialize(arr[0]));

    for (size_t i = 1; i < N; ++i) {
      result += std::format(", {}", serialize(arr[i]));
    }
  }
  result += "]";
  return result;
}

template <typename T>
  requires(std::is_enum_v<T> || std::is_same_v<T, bool> ||
           std::is_same_v<T, char>)
inline std::string serialize(const T *const *const arr, const size_t N) {
  std::string result = "[";
  if (N && arr && arr[0]) {
    if constexpr (std::is_same_v<T, char>) {
      result += std::format(", {}", serialize(arr[0]));
    } else {
      result += std::format(", {}", serialize(*arr[0]));
    }

    for (size_t i = 1; i < N; ++i) {
      if (arr[i]) {
        if constexpr (std::is_same_v<T, char>) {
          result += std::format(", {}", serialize(arr[i]));
        } else {
          result += std::format(", {}", serialize(*arr[i]));
        }
      }
    }
  }
  result += "]";
  return result;
}

template <SerializablePtr T>
inline std::string serialize(const T arr, const size_t N) {
  std::string result = "[";
  if (N && arr) {
    // Delegate to global ::serialize to allow recursive requirement resolution
    result += std::format("{}", ::serialize(arr[0]));

    for (size_t i = 1; i < N; ++i) {
      result += std::format(", {}", ::serialize(arr[i]));
    }
  }
  result += "]";
  return result;
}

template <SerializablePtr T>
inline std::string serialize(const T *arr, const size_t N) {
  std::string result = "[";
  if (N && arr && arr[0]) {
    // Delegate to global ::serialize to allow recursive requirement resolution
    result += std::format("{}", ::serialize(arr[0]));

    for (size_t i = 1; i < N; ++i) {
      if (arr[i]) {
        result += std::format(", {}", ::serialize(arr[i]));
      }
    }
  }
  result += "]";
  return result;
}

template <typename T, const size_t N>
  requires(std::is_integral_v<T> && !std::is_enum_v<T> &&
           !std::is_same_v<T, bool> && !std::is_same_v<T, char>)
inline std::string serialize(const T (&arr)[N]) {
  return serialize(&arr[0], N);
}

template <typename T, const size_t N>
  requires(std::is_floating_point_v<T>)
inline std::string serialize(const T (&arr)[N]) {
  return serialize(&arr[0], N);
}

template <typename T, const size_t N>
  requires(std::is_enum_v<T> || std::is_same_v<T, bool> ||
           std::is_same_v<T, char>)
inline std::string serialize(const T (&arr)[N]) {
  return serialize(&arr[0], N);
}

template <Serializable T, const size_t N>
inline std::string serialize(const T (&arr)[N]) {
  return serialize(&arr[0], N);
}

template <typename T, const size_t N>
  requires(std::is_integral_v<T> && !std::is_enum_v<T> &&
           !std::is_same_v<T, bool> && !std::is_same_v<T, char>)
inline std::string serialize(const T *const (&arr)[N]) {
  return serialize(&arr[0], N);
}

template <typename T, const size_t N>
  requires(std::is_floating_point_v<T>)
inline std::string serialize(const T *const (&arr)[N]) {
  return serialize(&arr[0], N);
}

template <typename T, const size_t N>
  requires(std::is_enum_v<T> || std::is_same_v<T, bool> ||
           std::is_same_v<T, char>)
inline std::string serialize(const T *const (&arr)[N]) {
  return serialize(&arr[0], N);
}

template <SerializablePtr T, const size_t N>
inline std::string serialize(const T (&arr)[N]) {
  return serialize(&arr[0], N);
}
