#pragma once

#include <type_traits>

namespace detail {

// Helper to check if T is in List...
template <typename T, typename... List>
inline constexpr bool is_in_v = (std::is_same_v<T, List> || ...);

// Helper to check for duplicate types in a pack
template <typename... TPack> struct has_duplicates;

template <> struct has_duplicates<> : std::false_type {};

template <typename T, typename... Rest>
struct has_duplicates<T, Rest...>
    : std::bool_constant<(std::is_same_v<T, Rest> || ...) ||
                         has_duplicates<Rest...>::value> {};

template <typename... TPack>
inline constexpr bool has_duplicates_v = has_duplicates<TPack...>::value;

// Helper to check if all types in a pack are identical
template <typename... TPack> struct all_same;

template <> struct all_same<> : std::true_type {};

template <typename T> struct all_same<T> : std::true_type {};

template <typename T1, typename T2, typename... Rest>
struct all_same<T1, T2, Rest...>
    : std::bool_constant<std::is_same_v<T1, T2> &&
                         all_same<T2, Rest...>::value> {};

template <typename... TPack>
inline constexpr bool all_same_v = all_same<TPack...>::value;

} // namespace detail

template <typename... Ts> struct AnyOf {
  // Any of the left-hand types in the right-hand list
  template <typename... List>
  static constexpr bool In = (... || detail::is_in_v<Ts, List...>);

  // None of the left-hand types in the right-hand list
  template <typename... List> static constexpr bool NotIn = !In<List...>;

  // Set properties
  static constexpr bool Equal = detail::has_duplicates_v<Ts...>;
  static constexpr bool Inequal = !detail::all_same_v<Ts...>;
  static constexpr bool Unique = !Equal;

  // Checks if any type in Ts... satisfies a trait/concept template.
  template <template <typename...> class Pred, typename... Extra>
  static constexpr bool Satisfying = (... || Pred<Ts, Extra...>::value);
};

template <typename... Ts> struct AllOf {
  template <typename... List>
  static constexpr bool In = (... && detail::is_in_v<Ts, List...>);

  template <typename... List>
  static constexpr bool NotIn = (... && !detail::is_in_v<Ts, List...>);

  static constexpr bool Equal = detail::has_duplicates_v<Ts...>;
  static constexpr bool Inequal = !detail::all_same_v<Ts...>;
  static constexpr bool Unique = !Equal;
  static constexpr bool Homogeneous = detail::all_same_v<Ts...>;

  template <template <typename...> class Pred, typename... Extra>
  static constexpr bool Satisfying = (... && Pred<Ts, Extra...>::value);
};

// ============================================================================
// NoneOf: Negative quantification (No types match)
// ============================================================================
template <typename... Ts> struct NoneOf {
  template <typename... List>
  static constexpr bool In = (... && !detail::is_in_v<Ts, List...>);

  static constexpr bool Equal = detail::has_duplicates_v<Ts...>;
  static constexpr bool Inequal = !detail::all_same_v<Ts...>;
  static constexpr bool Unique = !Equal;

  template <template <typename...> class Pred, typename... Extra>
  static constexpr bool Satisfying = (... && !Pred<Ts, Extra...>::value);
};

static_assert(AnyOf<int, short>::In<float, double, int, char>,
              "AnyOf: At least one type must be present");

static_assert(AllOf<int, char>::In<float, double, int, char>,
              "AllOf: All left-hand types must be present");

static_assert(NoneOf<void, bool>::In<float, double, int, char>,
              "NoneOf: None of the left-hand types are in the list");
static_assert(!NoneOf<int, bool>::In<float, double, int, char>,
              "NoneOf: 'int' is present, so this correctly fails");

static_assert(NoneOf<float, double>::Satisfying<std::is_integral>,
              "NoneOf: None of the specified types are integral");
static_assert(!NoneOf<float, int>::Satisfying<std::is_integral>,
              "NoneOf: 'int' is integral, so this correctly fails");
