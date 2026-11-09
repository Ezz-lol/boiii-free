#pragma once

#include <game/structs/lobby/core.hpp>
#include <game/structs/macros.hpp>

#include <cstdint>

namespace game {
namespace lobby {
namespace spinner {

// Verified
enum class SpinnerSystem : int32_t {
  INVALID = -1,
  JOIN_RESULT = 0x0,
  JOIN_PROGRESS = 0x1,
  LOBBY_SEARCH = 0x2,
  LOBBY_CREATE = 0x3,
  LOBBY_LEAVE = 0x4,
  LOBBY_MOVE = 0x5,
  COUNT = 0x6,
};
IMPL_ENUM_OPERATORS(SpinnerSystem);

[[nodiscard]] inline constexpr const char *
serialize(const SpinnerSystem system) {
  switch (system) {
  case SpinnerSystem::JOIN_RESULT:
    return "SPINNER_SYSTEM_JOIN_RESULT";
  case SpinnerSystem::JOIN_PROGRESS:
    return "SPINNER_SYSTEM_JOIN_PROGRESS";
  case SpinnerSystem::LOBBY_SEARCH:
    return "SPINNER_SYSTEM_LOBBY_SEARCH";
  case SpinnerSystem::LOBBY_CREATE:
    return "SPINNER_SYSTEM_LOBBY_CREATE";
  case SpinnerSystem::LOBBY_LEAVE:
    return "SPINNER_SYSTEM_LOBBY_LEAVE";
  case SpinnerSystem::LOBBY_MOVE:
    return "SPINNER_SYSTEM_LOBBY_MOVE";
  case SpinnerSystem::COUNT:
    return "SPINNER_SYSTEM_COUNT";
  default:
    return "SPINNER_SYSTEM_INVALID";
  }
}

PACKED(struct Spinner {
  SpinnerSystem system = SpinnerSystem::INVALID;
  uint8_t _padding04[4] = {0};
  const char *name = nullptr;

  constexpr Spinner(SpinnerSystem sys, const char *name)
      : system(sys), name(name) {}
});

template <typename T> union SpinnerPool {
  static inline constexpr auto SIZE = +SpinnerSystem::COUNT;
  typedef uint8_t index_t;

  T pool[SIZE];

  struct {
    T join_result = {};
    T join_progress = {};
    T lobby_search = {};
    T lobby_create = {};
    T lobby_leave = {};
    T lobby_move = {};
  };

  constexpr void assert_range([[maybe_unused]] index_t index) const {
    assert(index < SIZE && "index to SpinnerPool must satisfy range "
                           "SpinnerSystem::JOIN_RESULT <= "
                           "index < SpinnerSystem::::COUNT");
  }
  template <IntegralLike<index_t> Index>
  constexpr const T &get(Index index_arg) const noexcept {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return pool[index];
  }
  template <IntegralLike<index_t> Index>
  constexpr T &get(Index index_arg) noexcept {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return pool[index];
  }

  template <IntegralLike<index_t> Index>
  constexpr const T &operator[](Index index) const noexcept {
    return get(index);
  }
  template <IntegralLike<index_t> Index>
  constexpr T &operator[](Index index) noexcept {
    return get(index);
  }

  operator const T *() const noexcept { return pool; }
  operator T *() noexcept { return pool; }

  operator const void *() const noexcept { return pool; }
  operator void *() noexcept { return pool; }

  operator const array<T, SIZE> &() const noexcept { return pool; }

  operator array<T, SIZE> &() noexcept { return pool; }

  operator std::span<const T, SIZE>() const noexcept {
    return std::span<const T, SIZE>(pool);
  }

  operator std::span<T, SIZE>() noexcept { return std::span<T, SIZE>(pool); }

  static inline constexpr auto size() noexcept { return SIZE; }
};

} // namespace spinner
} // namespace lobby
} // namespace game
