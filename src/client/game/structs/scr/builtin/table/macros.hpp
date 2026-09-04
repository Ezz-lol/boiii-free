#pragma once

#include <ranges>                 // IWYU pragma: export
#include <array>                  // IWYU pragma: export
#include <cstdint>                // IWYU pragma: export
#include <frozen/unordered_map.h> // IWYU pragma: export

#include <macros.hpp> // IWYU pragma: export

#include "hash.hpp" // IWYU pragma: export

template <typename T>
struct Enumerate : public std::ranges::view_interface<Enumerate<T>> {
  T *iterable_ptr;

  constexpr explicit Enumerate(T &iterable) : iterable_ptr(&iterable) {}

  struct Iterator {
    using IterType = decltype(std::begin(std::declval<T &>()));
    using difference_type = std::ptrdiff_t;
    using value_type = std::pair<size_t, std::iter_value_t<IterType>>;
    using iterator_concept = std::random_access_iterator_tag;
    using iterator_category = std::input_iterator_tag;

    IterType iter;
    size_t index;

    constexpr bool operator==(const Iterator &other) const = default;
    constexpr auto operator<=>(const Iterator &other) const = default;

    constexpr Iterator &operator++() {
      ++iter;
      ++index;
      return *this;
    }
    constexpr Iterator operator++(int) {
      Iterator tmp = *this;
      ++*this;
      return tmp;
    }
    constexpr Iterator &operator--() {
      --iter;
      --index;
      return *this;
    }
    constexpr Iterator operator--(int) {
      Iterator tmp = *this;
      --*this;
      return tmp;
    }
    constexpr Iterator &operator+=(difference_type n) {
      iter += n;
      index += n;
      return *this;
    }
    constexpr Iterator &operator-=(difference_type n) {
      iter -= n;
      index -= n;
      return *this;
    }
    friend constexpr Iterator operator+(Iterator i, difference_type n) {
      i += n;
      return i;
    }
    friend constexpr Iterator operator+(difference_type n, Iterator i) {
      i += n;
      return i;
    }
    friend constexpr Iterator operator-(Iterator i, difference_type n) {
      i -= n;
      return i;
    }
    friend constexpr difference_type operator-(const Iterator &a,
                                               const Iterator &b) {
      return a.iter - b.iter;
    }

    constexpr auto operator[](difference_type n) const {
      return std::make_pair(index + n, *(iter + n));
    }

    constexpr auto operator*() const { return std::make_pair(index, *iter); }
  };

  constexpr Iterator begin() const {
    return Iterator{std::begin(*iterable_ptr), 0};
  }
  constexpr Iterator end() const {
    return Iterator{std::end(*iterable_ptr),
                    static_cast<size_t>(std::size(*iterable_ptr))};
  }
};

template <typename T> Enumerate(T &) -> Enumerate<T>;

template <typename T>
inline constexpr bool std::ranges::enable_view<Enumerate<T>> = true;

#ifndef PROTECTED
#define PROTECTED(entry) __protected__##entry
#endif

#ifndef STR
#define STR(s) #s
#endif

#ifndef PROTECTED_STR
#define PROTECTED_STR(entry) STR(PROTECTED(entry))
#endif

// Helper function to unpack indices and populate the array
template <typename View, std::size_t... Is>
inline constexpr auto to_array_expand_internal(View &&view,
                                               std::index_sequence<Is...>) {
  // Note: This relies on the view providing random access (like an array/vector
  // source)
  return std::array{view[Is]...};
}

// Main converter function
template <const IntegralLike auto N, typename View>
inline constexpr auto to_array(View &&view) {
  return to_array_expand_internal(
      std::forward<View>(view),
      std::make_index_sequence<static_cast<size_t>(N)>());
}

struct NameIdxPair {
  const char *name;
  size_t idx;
};

#ifndef NAME_MAP
#define NAME_MAP(names)                                                        \
  frozen::make_unordered_map(to_array<names.size()>(                           \
      std::views::transform(Enumerate{names}, [](auto &&pair) {                \
        auto [idx, name] = pair;                                               \
        const fnv1aHashNull_t hash = fnv1a(name);                              \
        return std::make_pair(hash, NameIdxPair{name, idx});                   \
      })));
#endif

#ifndef DEFINE_NAME_MAP
#define DEFINE_NAME_MAP(names, map)                                            \
  static inline constexpr frozen::unordered_map<fnv1aHashNull_t, NameIdxPair,  \
                                                names.size()>                  \
      map = NAME_MAP(names);
#endif

#ifndef IMPL_TABLE_OPERATORS
#define IMPL_TABLE_OPERATORS(table)                                            \
  template <IntegralLike Index>                                                \
  inline constexpr element_of<decltype(table)> &operator[](                    \
      Index index) noexcept {                                                  \
    return table[static_cast<size_t>(index)];                                  \
  }                                                                            \
  template <IntegralLike Index>                                                \
  inline constexpr const element_of<decltype(table)> &operator[](Index index)  \
      const noexcept {                                                         \
    return table[static_cast<size_t>(index)];                                  \
  }                                                                            \
                                                                               \
  inline constexpr const element_of<decltype(table)> *get(                     \
      ScrVarCanonicalName_t hash) const noexcept {                             \
    if (hashes.contains(hash)) {                                               \
      return &table[hashes.at(hash).idx];                                      \
    }                                                                          \
    return nullptr;                                                            \
  }                                                                            \
                                                                               \
  inline constexpr const element_of<decltype(table)> *get(const char *name)    \
      const noexcept {                                                         \
    const ScrVarCanonicalName_t hash = fnv1a(name);                            \
    return get(hash);                                                          \
  }                                                                            \
                                                                               \
  inline constexpr const element_of<decltype(table)> *operator[](              \
      const char *name) const noexcept {                                       \
    return get(name);                                                          \
  }                                                                            \
  inline constexpr element_of<decltype(table)> *get(                           \
      ScrVarCanonicalName_t hash) noexcept {                                   \
    if (hashes.contains(hash)) {                                               \
      return &table[hashes.at(hash).idx];                                      \
    }                                                                          \
    return nullptr;                                                            \
  }                                                                            \
                                                                               \
  inline constexpr element_of<decltype(table)> *get(char *name) noexcept {     \
    const ScrVarCanonicalName_t hash = fnv1a(name);                            \
    return get(hash);                                                          \
  }                                                                            \
                                                                               \
  inline constexpr element_of<decltype(table)> *operator[](                    \
      char *name) noexcept {                                                   \
    return get(name);                                                          \
  }
#endif