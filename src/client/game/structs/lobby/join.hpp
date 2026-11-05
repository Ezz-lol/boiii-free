#pragma once

#include <game/structs/lobby/core.hpp>
#include <game/structs/macros.hpp>

#include <cstdint>

namespace game {
namespace lobby {
namespace join {

enum class JoinType : int32_t {
  NORMAL = 0x0,
  PLAYLIST = 0x1,
  FRIEND = 0x2,
  INVITE = 0x3,
  PARTY = 0x4,
  GROUPS = 0x5,
  COUNT = 0x6,
};
IMPL_ENUM_OPERATORS(JoinType);

constexpr bool validate(const JoinType joinType) {
  return joinType >= JoinType::NORMAL && joinType < JoinType::COUNT;
}

enum class JoinSourceState : uint32_t {
  IDLE = 0x0,
  CONNECT_TO_NEXT_HOST = 0x1,
  ASSOCIATING = 0x2,
  HANDSHAKING = 0x3,
  WAITING_FOR_AGREEMENT = 0x4,
  CONNECTION_FAILED = 0x5,
  CONNECTION_SUCCESS = 0x6,
  ENDING_HOST = 0x7,
  CLEANUP = 0x8,
  COUNT = 0x9,
};
IMPL_ENUM_OPERATORS(JoinSourceState);

// Verified
enum class JoinResult : uint32_t {
  INVALID = 0x0,
  SUCCESS = 0x1,
  CONNECT_TO_HOST_FAILURE = 0x2,
  PROBE_SEND_FAILURE = 0x3,
  PROBE_TIMEOUT = 0x4,
  PROBE_INVALID_LOBBY = 0x5,
  PROBE_INVALID_INFO = 0x6,
  PROBE_RESULT_INVALID = 0x7,
  INVALID_LOBBY = 0x8,
  SEND_AGREEMENT_REQUEST_FAILED = 0x9,
  HANDSHAKE_WINDOW_EXPIRED = 0xA,
  AGREEMENT_WINDOW_EXPIRED = 0xB,
  JOIN_DISABLED = 0xC,
  JOIN_ALREADY_IN_PROGRESS = 0xD,
  NOT_JOINABLE_OFFLINE_MODE = 0xE,
  NOT_JOINABLE_NOT_HOSTING = 0xF,
  NOT_JOINABLE_NOT_IDLE = 0x10,
  NOT_JOINABLE_CLOSED = 0x11,
  NOT_JOINABLE_INVITE_ONLY = 0x12,
  NOT_JOINABLE_FRIENDS_ONLY = 0x13,
  NOT_JOINABLE_SOLO_MODE = 0x14,
  NO_JOIN_IN_PROGRESS = 0x15,
  GAME_PAUSED = 0x16,
  CHUNK_MP_REQUIRED = 0x17,
  CHUNK_ZM_REQUIRED = 0x18,
  CHUNK_CP_REQUIRED = 0x19,
  CHUNK_MP_REQUIRED_HOST = 0x1A,
  CHUNK_ZM_REQUIRED_HOST = 0x1B,
  CHUNK_CP_REQUIRED_HOST = 0x1C,
  SPLITSCREEN_NOT_ALLOWED = 0x1D,
  IN_PUBLIC_PROBATION = 0x1E,
  IN_ARENA_PROBATION = 0x1F,
  OVER_MAX_PARTY_LIMIT = 0x20,
  NO_PARTIES = 0x21,
  LOBBY_FULL = 0x22,
  NETWORK_MODE_MISMATCH = 0x23,
  MISMATCH_PLAYLISTID = 0x24,
  MISMATCH_PLAYLIST_VERSION_TO_NEW = 0x25,
  MISMATCH_PLAYLIST_VERSION_TO_OLD = 0x26,
  MISMATCH_PROTOCOL_VERSION = 0x27,
  MISMATCH_NETFIELD_CHECKSUM = 0x28,
  MISMATCH_FFOTD_VERSION_TO_NEW = 0x29,
  MISMATCH_FFOTD_VERSION_TO_OLD = 0x2A,
  MIGRATE_IN_PROGRESS = 0x2B,
  COULD_NOT_RESERVE = 0x2C,
  BAD_DLC_BITS = 0x2D,
  BAD_MPHD_BITS = 0x2E,
  VM_FAILURE_1 = 0x2F,
  VM_FAILURE_2 = 0x30,
  VM_FAILURE_3 = 0x31,
  VM_FAILURE_4 = 0x32,
  STARTER_PACK_RESTRICT = 0x33,
  INVALID_PASSWORD = 0x34,
  BANNED = 0x35,
  BANNED_MEMBER = 0x36,
  UNDER_MIN_PARTY_LIMIT = 0x37,
  COUNT = 0x38,
};
IMPL_ENUM_OPERATORS(JoinResult);

enum class JoinClientState : int32_t {
  IDLE = 0x0,
  CONNECT_TO_NEXT_HOST = 0x1,
  ASSOCIATING = 0x2,

  HANDSHAKING = 0x3,
  WAITING_FOR_AGREEMENT = 0x4,
  CONNECTED_FAILED = 0x5,
  CONNECTION_SUCCESS = 0x6,
  ENDING_HOST = 0x7,
  CLEANUP = 0x8,
  COUNT = 0x9,
};
IMPL_ENUM_OPERATORS(JoinClientState);

// Verified
struct JoinPartyMember {
  XUID xuid = 0;
  LobbyID lobbyID = 0;
  float skillRating = 0.0f;
  float skillVariance = 0.0f;
  union {
    uint32_t probationEndTime[2];
    struct {
      // Serialized as "pprobation" in lobby message packet
      uint32_t publicProbationEndTime = 0;
      // Serialized as "aprobation" in lobby message packet
      uint32_t arenaProbationEndTime = 0;
    };
  };
};
ASSERT_SIZE(JoinPartyMember, 0x20);

typedef fastcallPtr_t<void(int32_t, JoinResult result)> joinCompleteCallback;

struct Join {
  JoinSourceState state;
  int32_t actionId;
  int32_t startTime;
  int32_t duration;
  ControllerIndex_t controllerIndex;
  LobbyType sourceLobbyType;
  LobbyType targetLobbyType;
  joinCompleteCallback joinComplete;
  JoinHost hostList[50];
  int32_t hostCount;
  int32_t processedCount;
  bool isFinalized;
  JoinHost potentialHost;
  Agreement agreement;
  Agreement debugAgreement;
  JoinType joinType;
  XUID probedXuid;
  JoinResult joinResult;

  IMPL_SLICE_METHODS(hostList, hostCount, HostList);
};
ASSERT_OFFSET(Join, joinResult, 0x2838);

struct JoinClient {
  JoinClientState state;
  uint8_t _padding04[4];
  JoinHost potentialHost;
  LobbyType sourceLobbyType;
  ControllerIndex_t controllerIndex;
};
// Verified
ASSERT_SIZE(JoinClient, 0xB0);

template <typename T> union JoinSourceStatePool {
  typedef uint8_t index_t;

  T pool[+JoinSourceState::COUNT];

  struct {
    T idle;
    T connectToNextHost;
    T associating;
    T handshaking;
    T waitingForAgreement;
    T connectionFailed;
    T connectionSuccess;
    T endingHost;
    T cleanup;
  };

  constexpr void assert_range([[maybe_unused]] index_t index) const {
    assert(index < +JoinSourceState::COUNT &&
           "index to JoinSourceStatePool must satisfy range "
           "JoinSourceState::IDLE <= "
           "index < JoinSourceState::COUNT");
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

  operator const array<T, +JoinSourceState::COUNT> &() const noexcept {
    return pool;
  }

  operator array<T, +JoinSourceState::COUNT> &() noexcept { return pool; }

  operator std::span<const T, +JoinSourceState::COUNT>() const noexcept {
    return std::span<const T, +JoinSourceState::COUNT>(pool);
  }

  operator std::span<T, +JoinSourceState::COUNT>() noexcept {
    return std::span<T, +JoinSourceState::COUNT>(pool);
  }
};

} // namespace join
} // namespace lobby
} // namespace game
