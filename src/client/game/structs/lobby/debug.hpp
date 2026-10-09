#pragma once

#include <game/structs/dvar.hpp>
#include <game/structs/quake/vec.hpp>

#include <structs/str.hpp>

namespace game {
namespace lobby {
namespace debug {

enum class DebugSystem : uint32_t {
  JOIN = 0x0,
  MSG = 0x1,
  RESERVATION = 0x2,
  RESERVATION_COUNT = 0x3,
  AGREEMENT = 0x4,
  AGREEMENT_COUNT = 0x5,
  PRIVATE_CLIENT = 0x6,
  PRIVATE_HOST = 0x7,
  GAME_CLIENT = 0x8,
  GAME_HOST = 0x9,
  DW_SESSION = 0xA,
  LOBBY_TASKS = 0xB,
  COUNT = 0xC,
};
IMPL_ENUM_OPERATORS(DebugSystem);

inline constexpr const char *serialize(const DebugSystem system) {
  switch (system) {
  case DebugSystem::JOIN:
    return "DEBUG_SYSTEM_JOIN";
  case DebugSystem::MSG:
    return "DEBUG_SYSTEM_MSG";
  case DebugSystem::RESERVATION:
    return "DEBUG_SYSTEM_RESERVATION";
  case DebugSystem::RESERVATION_COUNT:
    return "DEBUG_SYSTEM_RESERVATION_COUNT";
  case DebugSystem::AGREEMENT:
    return "DEBUG_SYSTEM_AGREEMENT";
  case DebugSystem::AGREEMENT_COUNT:
    return "DEBUG_SYSTEM_AGREEMENT_COUNT";
  case DebugSystem::PRIVATE_CLIENT:
    return "DEBUG_SYSTEM_PRIVATE_CLIENT";
  case DebugSystem::PRIVATE_HOST:
    return "DEBUG_SYSTEM_PRIVATE_HOST";
  case DebugSystem::GAME_CLIENT:
    return "DEBUG_SYSTEM_GAME_CLIENT";
  case DebugSystem::GAME_HOST:
    return "DEBUG_SYSTEM_GAME_HOST";
  case DebugSystem::DW_SESSION:
    return "DEBUG_SYSTEM_DW_SESSION";
  case DebugSystem::LOBBY_TASKS:
    return "DEBUG_SYSTEM_LOBBY_TASKS";
  default:
    return "DEBUG_SYSTEM_INVALID";
  }
}

// Verified
PACKED(struct DebugHistoryRow {
  int32_t time;
  str128_t str;
  uint8_t _padding84[4];
  const vec4_t *color;
});
ASSERT_SIZE(DebugHistoryRow, 0x90);

// Verified
PACKED(struct DebugHistory {
  DebugHistoryRow row[20];
  int32_t head;
  int32_t touchTime;
  bool canTimeout;
  bool rowsPresent;
  uint8_t _paddingB4A[6];
  const vec4_t *color;
  str16_t name;
  int32_t maxRows;
  int32_t displayDuration;
  const dvar_t *locDvar;
  int32_t visLevel;
  uint8_t _paddingB7C[4];
});
ASSERT_SIZE(DebugHistory, 0xB80);

template <const auto COUNT> union DebugHistoryPool {
  DebugHistory history[COUNT];

  static inline constexpr auto size() noexcept { return COUNT; }

  inline constexpr void assert_range([[maybe_unused]] size_t index) const {
    assert(index < COUNT && "index to aligned_array must be < array length");
  }
  template <IntegralLike<size_t> Index>
  inline constexpr const DebugHistory &get(Index index_arg) const noexcept {
    const size_t index = static_cast<size_t>(index_arg);
    assert_range(index);
    return history[index];
  }
  template <IntegralLike<size_t> Index>
  inline constexpr DebugHistory &get(Index index_arg) noexcept {
    const size_t index = static_cast<size_t>(index_arg);
    assert_range(index);
    return history[index];
  }

  template <IntegralLike<size_t> Index>
  inline constexpr const DebugHistory &operator[](Index index) const noexcept {
    return get(index);
  }
  template <IntegralLike<size_t> Index>
  inline constexpr DebugHistory &operator[](Index index) noexcept {
    return get(index);
  }

  inline operator const DebugHistory *() const noexcept { return history; }
  inline operator DebugHistory *() noexcept { return history; }

  inline operator const void *() const noexcept { return history; }
  inline operator void *() noexcept { return history; }

  inline operator const array<DebugHistory, COUNT> &() const noexcept {
    return history;
  }
  inline operator array<DebugHistory, COUNT> &() noexcept { return history; }
  inline operator const std::span<const DebugHistory, COUNT>() const noexcept {
    return std::span<const DebugHistory, COUNT>(history);
  }
  inline operator std::span<DebugHistory, COUNT>() noexcept {
    return std::span<DebugHistory, COUNT>(history);
  }

  inline bool contains(const DebugHistory *item) const noexcept {
    return game::contains(history, sizeof(history), item);
  }
};

typedef DebugHistoryPool<23> DebugHistoryPool_cl;
typedef DebugHistoryPool<15> DebugHistoryPool_sv;

union EngineDependentDebugHistoryPool {
  DebugHistoryPool_sv *sv;
  DebugHistoryPool_cl *cl;

  inline const DebugHistory *get(size_t index) const {
    return is_server() ? &sv->history[index] : &cl->history[index];
  }
  inline DebugHistory *get(size_t index) {
    return is_server() ? &sv->history[index] : &cl->history[index];
  }

  inline const DebugHistory *operator[](size_t index) const {
    return get(index);
  }
  inline DebugHistory *operator[](size_t index) { return get(index); }

  inline bool contains(const DebugHistory *item) const noexcept {
    return is_server() ? sv->contains(item) : cl->contains(item);
  }

  inline auto size() const noexcept {
    return is_server() ? sv->size() : cl->size();
  }

  inline constexpr bool null() const noexcept { return sv == nullptr; }
  inline constexpr bool nonnull() const noexcept { return sv != nullptr; }

  inline constexpr EngineDependentDebugHistoryPool() noexcept = default;
  inline constexpr EngineDependentDebugHistoryPool(
      DebugHistoryPool_sv *pool) noexcept
      : sv(pool) {}
  inline constexpr EngineDependentDebugHistoryPool(
      DebugHistoryPool_cl *pool) noexcept
      : cl(pool) {}
  inline constexpr EngineDependentDebugHistoryPool(std::nullptr_t pool) noexcept
      : cl(pool) {}

  inline constexpr bool operator!() const noexcept { return null(); }
  inline constexpr operator bool() const noexcept { return nonnull(); }
  template <typename T>
  friend inline constexpr bool
  operator==(const EngineDependentDebugHistoryPool &lhs, const T *&rhs) {
    return lhs.cl == rhs;
  }
  template <PtrLike T>
  friend inline constexpr bool
  operator==(const EngineDependentDebugHistoryPool &lhs, const T &rhs) {
    return reinterpret_cast<uintptr_t>(lhs.cl) == static_cast<uintptr_t>(rhs);
  }

  friend inline constexpr bool
  operator==(const EngineDependentDebugHistoryPool &lhs,
             const std::nullptr_t &rhs) {
    return lhs.sv == rhs;
  }
};

} // namespace debug
} // namespace lobby
} // namespace game
