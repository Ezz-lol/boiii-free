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

inline constexpr const char *serialize(const JoinType joinType) {
  switch (joinType) {
  case JoinType::NORMAL:
    return "JOIN_TYPE_NORMAL";
  case JoinType::PLAYLIST:
    return "JOIN_TYPE_PLAYLIST";
  case JoinType::FRIEND:
    return "JOIN_TYPE_FRIEND";
  case JoinType::INVITE:
    return "JOIN_TYPE_INVITE";
  case JoinType::PARTY:
    return "JOIN_TYPE_PARTY";
  case JoinType::GROUPS:
    return "JOIN_TYPE_GROUPS";
  default:
    return "JOIN_TYPE_INVALID";
  }
}

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

inline constexpr const char *serialize(const JoinSourceState state) {
  switch (state) {
  case JoinSourceState::IDLE:
    return "JOIN_SOURCE_STATE_IDLE";
  case JoinSourceState::CONNECT_TO_NEXT_HOST:
    return "JOIN_SOURCE_STATE_CONNECT_TO_NEXT_HOST";
  case JoinSourceState::ASSOCIATING:
    return "JOIN_SOURCE_STATE_ASSOCIATING";
  case JoinSourceState::HANDSHAKING:
    return "JOIN_SOURCE_STATE_HANDSHAKING";
  case JoinSourceState::WAITING_FOR_AGREEMENT:
    return "JOIN_SOURCE_STATE_WAITING_FOR_AGREEMENT";
  case JoinSourceState::CONNECTION_FAILED:
    return "JOIN_SOURCE_STATE_CONNECTION_FAILED";
  case JoinSourceState::CONNECTION_SUCCESS:
    return "JOIN_SOURCE_STATE_CONNECTION_SUCCESS";
  case JoinSourceState::ENDING_HOST:
    return "JOIN_SOURCE_STATE_ENDING_HOST";
  case JoinSourceState::CLEANUP:
    return "JOIN_SOURCE_STATE_CLEANUP";
  default:
    return "JOIN_SOURCE_STATE_INVALID";
  }
}

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

inline constexpr const char *serialize(const JoinResult result) {
  switch (result) {
  case JoinResult::INVALID:
    return "JOIN_RESULT_INVALID";
  case JoinResult::SUCCESS:
    return "JOIN_RESULT_SUCCESS";
  case JoinResult::CONNECT_TO_HOST_FAILURE:
    return "JOIN_RESULT_CONNECT_TO_HOST_FAILURE";
  case JoinResult::PROBE_SEND_FAILURE:
    return "JOIN_RESULT_PROBE_SEND_FAILURE";
  case JoinResult::PROBE_TIMEOUT:
    return "JOIN_RESULT_PROBE_TIMEOUT";
  case JoinResult::PROBE_INVALID_LOBBY:
    return "JOIN_RESULT_PROBE_INVALID_LOBBY";
  case JoinResult::PROBE_INVALID_INFO:
    return "JOIN_RESULT_PROBE_INVALID_INFO";
  case JoinResult::PROBE_RESULT_INVALID:
    return "JOIN_RESULT_PROBE_RESULT_INVALID";
  case JoinResult::INVALID_LOBBY:
    return "JOIN_RESULT_INVALID_LOBBY";
  case JoinResult::SEND_AGREEMENT_REQUEST_FAILED:
    return "JOIN_RESULT_SEND_AGREEMENT_REQUEST_FAILED";
  case JoinResult::HANDSHAKE_WINDOW_EXPIRED:
    return "JOIN_RESULT_HANDSHAKE_WINDOW_EXPIRED";
  case JoinResult::AGREEMENT_WINDOW_EXPIRED:
    return "JOIN_RESULT_AGREEMENT_WINDOW_EXPIRED";
  case JoinResult::JOIN_DISABLED:
    return "JOIN_RESULT_JOIN_DISABLED";
  case JoinResult::JOIN_ALREADY_IN_PROGRESS:
    return "JOIN_RESULT_JOIN_ALREADY_IN_PROGRESS";
  case JoinResult::NOT_JOINABLE_OFFLINE_MODE:
    return "JOIN_RESULT_NOT_JOINABLE_OFFLINE_MODE";
  case JoinResult::NOT_JOINABLE_NOT_HOSTING:
    return "JOIN_RESULT_NOT_JOINABLE_NOT_HOSTING";
  case JoinResult::NOT_JOINABLE_NOT_IDLE:
    return "JOIN_RESULT_NOT_JOINABLE_NOT_IDLE";
  case JoinResult::NOT_JOINABLE_CLOSED:
    return "JOIN_RESULT_NOT_JOINABLE_CLOSED";
  case JoinResult::NOT_JOINABLE_INVITE_ONLY:
    return "JOIN_RESULT_NOT_JOINABLE_INVITE_ONLY";
  case JoinResult::NOT_JOINABLE_FRIENDS_ONLY:
    return "JOIN_RESULT_NOT_JOINABLE_FRIENDS_ONLY";
  case JoinResult::NOT_JOINABLE_SOLO_MODE:
    return "JOIN_RESULT_NOT_JOINABLE_SOLO_MODE";
  case JoinResult::NO_JOIN_IN_PROGRESS:
    return "JOIN_RESULT_NO_JOIN_IN_PROGRESS";
  case JoinResult::GAME_PAUSED:
    return "JOIN_RESULT_GAME_PAUSED";
  case JoinResult::CHUNK_MP_REQUIRED:
    return "JOIN_RESULT_CHUNK_MP_REQUIRED";
  case JoinResult::CHUNK_ZM_REQUIRED:
    return "JOIN_RESULT_CHUNK_ZM_REQUIRED";
  case JoinResult::CHUNK_CP_REQUIRED:
    return "JOIN_RESULT_CHUNK_CP_REQUIRED";
  case JoinResult::CHUNK_MP_REQUIRED_HOST:
    return "JOIN_RESULT_CHUNK_MP_REQUIRED_HOST";
  case JoinResult::CHUNK_ZM_REQUIRED_HOST:
    return "JOIN_RESULT_CHUNK_ZM_REQUIRED_HOST";
  case JoinResult::CHUNK_CP_REQUIRED_HOST:
    return "JOIN_RESULT_CHUNK_CP_REQUIRED_HOST";
  case JoinResult::SPLITSCREEN_NOT_ALLOWED:
    return "JOIN_RESULT_SPLITSCREEN_NOT_ALLOWED";
  case JoinResult::IN_PUBLIC_PROBATION:
    return "JOIN_RESULT_IN_PUBLIC_PROBATION";
  case JoinResult::IN_ARENA_PROBATION:
    return "JOIN_RESULT_IN_ARENA_PROBATION";
  case JoinResult::OVER_MAX_PARTY_LIMIT:
    return "JOIN_RESULT_OVER_MAX_PARTY_LIMIT";
  case JoinResult::NO_PARTIES:
    return "JOIN_RESULT_NO_PARTIES";
  case JoinResult::LOBBY_FULL:
    return "JOIN_RESULT_LOBBY_FULL";
  case JoinResult::NETWORK_MODE_MISMATCH:
    return "JOIN_RESULT_NETWORK_MODE_MISMATCH";
  case JoinResult::MISMATCH_PLAYLISTID:
    return "JOIN_RESULT_MISMATCH_PLAYLISTID";
  case JoinResult::MISMATCH_PLAYLIST_VERSION_TO_NEW:
    return "JOIN_RESULT_MISMATCH_PLAYLIST_VERSION_TO_NEW";
  case JoinResult::MISMATCH_PLAYLIST_VERSION_TO_OLD:
    return "JOIN_RESULT_MISMATCH_PLAYLIST_VERSION_TO_OLD";
  case JoinResult::MISMATCH_PROTOCOL_VERSION:
    return "JOIN_RESULT_MISMATCH_PROTOCOL_VERSION";
  case JoinResult::MISMATCH_NETFIELD_CHECKSUM:
    return "JOIN_RESULT_MISMATCH_NETFIELD_CHECKSUM";
  case JoinResult::MISMATCH_FFOTD_VERSION_TO_NEW:
    return "JOIN_RESULT_MISMATCH_FFOTD_VERSION_TO_NEW";
  case JoinResult::MISMATCH_FFOTD_VERSION_TO_OLD:
    return "JOIN_RESULT_MISMATCH_FFOTD_VERSION_TO_OLD";
  case JoinResult::MIGRATE_IN_PROGRESS:
    return "JOIN_RESULT_MIGRATE_IN_PROGRESS";
  case JoinResult::COULD_NOT_RESERVE:
    return "JOIN_RESULT_COULD_NOT_RESERVE";
  case JoinResult::BAD_DLC_BITS:
    return "JOIN_RESULT_BAD_DLC_BITS";
  case JoinResult::BAD_MPHD_BITS:
    return "JOIN_RESULT_BAD_MPHD_BITS";
  case JoinResult::VM_FAILURE_1:
    return "JOIN_RESULT_VM_FAILURE_1";
  case JoinResult::VM_FAILURE_2:
    return "JOIN_RESULT_VM_FAILURE_2";
  case JoinResult::VM_FAILURE_3:
    return "JOIN_RESULT_VM_FAILURE_3";
  case JoinResult::VM_FAILURE_4:
    return "JOIN_RESULT_VM_FAILURE_4";
  case JoinResult::STARTER_PACK_RESTRICT:
    return "JOIN_RESULT_STARTER_PACK_RESTRICT";
  case JoinResult::INVALID_PASSWORD:
    return "JOIN_RESULT_INVALID_PASSWORD";
  case JoinResult::BANNED:
    return "JOIN_RESULT_BANNED";
  case JoinResult::BANNED_MEMBER:
    return "JOIN_RESULT_BANNED_MEMBER";
  case JoinResult::UNDER_MIN_PARTY_LIMIT:
    return "JOIN_RESULT_UNDER_MIN_PARTY_LIMIT";
  default:
    return "JOIN_RESULT_INVALID";
  }
}

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

inline constexpr const char *serialize(const JoinClientState state) {
  switch (state) {
  case JoinClientState::IDLE:
    return "JOIN_CLIENT_STATE_IDLE";
  case JoinClientState::CONNECT_TO_NEXT_HOST:
    return "JOIN_CLIENT_STATE_CONNECT_TO_NEXT_HOST";
  case JoinClientState::ASSOCIATING:
    return "JOIN_CLIENT_STATE_ASSOCIATING";
  case JoinClientState::HANDSHAKING:
    return "JOIN_CLIENT_STATE_HANDSHAKING";
  case JoinClientState::WAITING_FOR_AGREEMENT:
    return "JOIN_CLIENT_STATE_WAITING_FOR_AGREEMENT";
  case JoinClientState::CONNECTED_FAILED:
    return "JOIN_CLIENT_STATE_CONNECTED_FAILED";
  case JoinClientState::CONNECTION_SUCCESS:
    return "JOIN_CLIENT_STATE_CONNECTION_SUCCESS";
  case JoinClientState::ENDING_HOST:
    return "JOIN_CLIENT_STATE_ENDING_HOST";
  case JoinClientState::CLEANUP:
    return "JOIN_CLIENT_STATE_CLEANUP";
  default:
    return "JOIN_CLIENT_STATE_INVALID";
  }
}

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

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"xuid\": {:016X}, \"lobbyID\": {:016X}, \"skillRating\": {}, "
        "\"skillVariance\": {}, \"publicProbationEndTime\": {}, "
        "\"arenaProbationEndTime\": {} }}",
        xuid, lobbyID, skillRating, skillVariance, publicProbationEndTime,
        arenaProbationEndTime);
  }
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
  // JoinHost is packed, so it isn't aligned on its own
  uint8_t _padding1F71[7];
  JoinHost potentialHost;
  Agreement agreement;
  Agreement debugAgreement;
  JoinType joinType;
  XUID probedXuid;
  JoinResult joinResult;

  IMPL_SLICE_METHODS(hostList, hostCount, HostList);

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"state\": {}, \"actionId\": {}, \"startTime\": {}, \"duration\": "
        "{}, "
        "\"controllerIndex\": {}, \"sourceLobbyType\": {}, "
        "\"targetLobbyType\": {}, "
        "\"joinComplete\": {}, \"hostList\": {}, \"hostCount\": {}, "
        "\"processedCount\": {}, "
        "\"isFinalized\": {}, \"potentialHost\": {}, \"agreement\": {}, "
        "\"debugAgreement\": {}, "
        "\"joinType\": {}, \"probedXuid\": {:016X}, \"joinResult\": {} }}",
        join::serialize(state), actionId, startTime, duration,
        game::serialize(controllerIndex), lobby::serialize(sourceLobbyType),
        lobby::serialize(targetLobbyType), ::serialize(joinComplete),
        ::serialize(getHostList()), hostCount, processedCount,
        ::serialize(isFinalized), potentialHost.serialize(),
        agreement.serialize(), debugAgreement.serialize(),
        join::serialize(joinType), probedXuid, join::serialize(joinResult));
  }
};
ASSERT_OFFSET(Join, potentialHost, 0x1F78);
ASSERT_OFFSET(Join, joinResult, 0x2838);

struct JoinClient {
  JoinClientState state;
  uint8_t _padding04[4];
  JoinHost potentialHost;
  LobbyType sourceLobbyType;
  ControllerIndex_t controllerIndex;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format(
        R"({{ "state": {}, "potentialHost": {}, "sourceLobbyType": {}, "controllerIndex": {} }})",
        join::serialize(state), potentialHost.serialize(),
        lobby::serialize(sourceLobbyType), game::serialize(controllerIndex));
  }
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
