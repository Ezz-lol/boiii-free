#pragma once

#include <game/structs/core.hpp>
#include <game/structs/dw/net/core.hpp>
#include <game/structs/net/net.hpp>

#include <cstdint>
#include <optional>

namespace game {
namespace lobby {

typedef int32_t LobbyUIScreen;
typedef int32_t LobbyLeaderActivity;

enum class LobbyGlobalChannel : uint32_t {
  JOIN = 0x0,
  CLIENT_CONTENT = 0x1,
  COUNT = 0x2,
};
IMPL_ENUM_OPERATORS(LobbyGlobalChannel);

[[nodiscard]] inline constexpr const char *
serialize(const LobbyGlobalChannel channel) {
  switch (channel) {
  case LobbyGlobalChannel::JOIN:
    return "LOBBY_GLOBAL_CHANNEL_JOIN";
  case LobbyGlobalChannel::CLIENT_CONTENT:
    return "LOBBY_GLOBAL_CHANNEL_CLIENT_CONTENT";
  case LobbyGlobalChannel::COUNT:
    return "LOBBY_GLOBAL_CHANNEL_COUNT";
  default:
    return "LOBBY_GLOBAL_CHANNEL_INVALID";
  }
}

enum class LobbyListComparisonType : int32_t {
  EQUAL_TO_OR_LESS_THAN = -2,
  LESS_THAN = -1,
  EQUAL = 0,
  GREATER_THAN = 1,
  EQUAL_TO_OR_GREATER_THAN = 2,
  NOT_EQUAL = 3,
};

[[nodiscard]] inline constexpr const char *
serialize(const LobbyListComparisonType type) {
  switch (type) {
  case LobbyListComparisonType::EQUAL_TO_OR_LESS_THAN:
    return "COMPARISON_EQUAL_TO_OR_LESS_THAN";
  case LobbyListComparisonType::LESS_THAN:
    return "COMPARISON_LESS_THAN";
  case LobbyListComparisonType::EQUAL:
    return "COMPARISON_EQUAL";
  case LobbyListComparisonType::GREATER_THAN:
    return "COMPARISON_GREATER_THAN";
  case LobbyListComparisonType::EQUAL_TO_OR_GREATER_THAN:
    return "COMPARISON_EQUAL_TO_OR_GREATER_THAN";
  case LobbyListComparisonType::NOT_EQUAL:
    return "COMPARISON_NOT_EQUAL";
  default:
    return "COMPARISON_INVALID";
  }
}

enum class LobbyTaskType : int32_t {
  HOST_CREATE = 0,
  HOST_ADDLOCAL = 1,
  HOST_END = 2,
  HOST_SEARCH = 3,
  HOST_ADVERTISE = 4,
  HOST_GAMERTAGSTOXUIDS = 5,
  CLIENT_END = 6,
  CLIENT_LAUNCH_GAME = 7,
  COUNT = 8,
};

[[nodiscard]] inline constexpr const char *serialize(const LobbyTaskType type) {
  switch (type) {
  case LobbyTaskType::HOST_CREATE:
    return "LOBBY_TASK_HOST_CREATE";
  case LobbyTaskType::HOST_ADDLOCAL:
    return "LOBBY_TASK_HOST_ADDLOCAL";
  case LobbyTaskType::HOST_END:
    return "LOBBY_TASK_HOST_END";
  case LobbyTaskType::HOST_SEARCH:
    return "LOBBY_TASK_HOST_SEARCH";
  case LobbyTaskType::HOST_ADVERTISE:
    return "LOBBY_TASK_HOST_ADVERTISE";
  case LobbyTaskType::HOST_GAMERTAGSTOXUIDS:
    return "LOBBY_TASK_HOST_GAMERTAGSTOXUIDS";
  case LobbyTaskType::CLIENT_END:
    return "LOBBY_TASK_CLIENT_END";
  case LobbyTaskType::CLIENT_LAUNCH_GAME:
    return "LOBBY_TASK_CLIENT_LAUNCH_GAME";
  case LobbyTaskType::COUNT:
    return "LOBBY_TASK_COUNT";
  default:
    return "LOBBY_TASK_INVALID";
  }
}
enum class LobbyTaskState : int32_t {
  IDLE = 0,
  HOSTCREATE_PENDING = 1,
  HOSTCREATE_DWNET = 2,
  HOSTCREATE_SESSION = 3,
  HOSTEND_PENDING = 4,
  HOSTEND_ONLINE = 5,
  HOSTEND_DISCONNECT = 6,
  HOSTEND_COOLOFF = 7,
  HOSTEND_SHUTDOWN = 8,
  HOSTSEARCH_PENDING = 9,
  HOSTSEARCH_INPROGRESS = 10,
  HOSTSEARCH_BEGINJOIN = 11,
  HOSTSEARCH_JOINING = 12,
  HOSTADVERTISE_PENDING = 13,
  HOSTADVERTISE_INPROGRESS = 14,
  HOSTPLATFORMSESSIONCREATE = 15,
  HOSTPLATFORMSESSIONUPDATE = 16,
  HOSTPLATFORMSESSIONINVITE = 17,
  HOSTPLATFORMSESSIONGETSESSIONINFO = 18,
  HOSTPLATFORMSESSIONGETINVITEINFO = 19,
  HOSTPLATFORMSESSIONLEAVE = 20,
  HOSTPLATFORMSESSIONSEARCH = 21,
  HOSTPLATFORMSESSIONJOIN = 22,
  HOSTPLATFORMSESSIONSETDURANGOJOINRESTRICTION = 23,
  HOSTGAMERTAGSTOXUIDS = 24,
  CLIENTEND_PENDING = 25,
  CLIENTEND_DISCONNECT = 26,
  CLIENTEND_COOLOFF = 27,
  CLIENTEND_SHUTDOWN = 28,
  HOSTLAUNCH_INIT = 29,
  HOSTLAUNCH_PUMP = 30,
  HOSTLAUNCH_PRELOAD = 31,
  HOSTLAUNCH_GO = 32,
  CLIENTLAUNCH_INIT = 33,
  CLIENTLAUNCH_PUMP = 34,
  CLIENTLAUNCH_GO = 35,
  SUCCESS = 36,
  FAILURE = 37,
};

[[nodiscard]] inline constexpr const char *
serialize(const LobbyTaskState state) {
  switch (state) {
  case LobbyTaskState::IDLE:
    return "LOBBY_TASK_STATE_IDLE";
  case LobbyTaskState::HOSTCREATE_PENDING:
    return "LOBBY_TASK_STATE_HOSTCREATE_PENDING";
  case LobbyTaskState::HOSTCREATE_DWNET:
    return "LOBBY_TASK_STATE_HOSTCREATE_DWNET";
  case LobbyTaskState::HOSTCREATE_SESSION:
    return "LOBBY_TASK_STATE_HOSTCREATE_SESSION";
  case LobbyTaskState::HOSTEND_PENDING:
    return "LOBBY_TASK_STATE_HOSTEND_PENDING";
  case LobbyTaskState::HOSTEND_ONLINE:
    return "LOBBY_TASK_STATE_HOSTEND_ONLINE";
  case LobbyTaskState::HOSTEND_DISCONNECT:
    return "LOBBY_TASK_STATE_HOSTEND_DISCONNECT";
  case LobbyTaskState::HOSTEND_COOLOFF:
    return "LOBBY_TASK_STATE_HOSTEND_COOLOFF";
  case LobbyTaskState::HOSTEND_SHUTDOWN:
    return "LOBBY_TASK_STATE_HOSTEND_SHUTDOWN";
  case LobbyTaskState::HOSTSEARCH_PENDING:
    return "LOBBY_TASK_STATE_HOSTSEARCH_PENDING";
  case LobbyTaskState::HOSTSEARCH_INPROGRESS:
    return "LOBBY_TASK_STATE_HOSTSEARCH_INPROGRESS";
  case LobbyTaskState::HOSTSEARCH_BEGINJOIN:
    return "LOBBY_TASK_STATE_HOSTSEARCH_BEGINJOIN";
  case LobbyTaskState::HOSTSEARCH_JOINING:
    return "LOBBY_TASK_STATE_HOSTSEARCH_JOINING";
  case LobbyTaskState::HOSTADVERTISE_PENDING:
    return "LOBBY_TASK_STATE_HOSTADVERTISE_PENDING";
  case LobbyTaskState::HOSTADVERTISE_INPROGRESS:
    return "LOBBY_TASK_STATE_HOSTADVERTISE_INPROGRESS";
  case LobbyTaskState::HOSTPLATFORMSESSIONCREATE:
    return "LOBBY_TASK_STATE_HOSTPLATFORMSESSIONCREATE";
  case LobbyTaskState::HOSTPLATFORMSESSIONUPDATE:
    return "LOBBY_TASK_STATE_HOSTPLATFORMSESSIONUPDATE";
  case LobbyTaskState::HOSTPLATFORMSESSIONINVITE:
    return "LOBBY_TASK_STATE_HOSTPLATFORMSESSIONINVITE";
  case LobbyTaskState::HOSTPLATFORMSESSIONGETSESSIONINFO:
    return "LOBBY_TASK_STATE_HOSTPLATFORMSESSIONGETSESSIONINFO";
  case LobbyTaskState::HOSTPLATFORMSESSIONGETINVITEINFO:
    return "LOBBY_TASK_STATE_HOSTPLATFORMSESSIONGETINVITEINFO";
  case LobbyTaskState::HOSTPLATFORMSESSIONLEAVE:
    return "LOBBY_TASK_STATE_HOSTPLATFORMSESSIONLEAVE";
  case LobbyTaskState::HOSTPLATFORMSESSIONSEARCH:
    return "LOBBY_TASK_STATE_HOSTPLATFORMSESSIONSEARCH";
  case LobbyTaskState::HOSTPLATFORMSESSIONJOIN:
    return "LOBBY_TASK_STATE_HOSTPLATFORMSESSIONJOIN";
  case LobbyTaskState::HOSTPLATFORMSESSIONSETDURANGOJOINRESTRICTION:
    return "LOBBY_TASK_STATE_HOSTPLATFORMSESSIONSETDURANGOJOINRESTRICTION";
  case LobbyTaskState::HOSTGAMERTAGSTOXUIDS:
    return "LOBBY_TASK_STATE_HOSTGAMERTAGSTOXUIDS";
  case LobbyTaskState::CLIENTEND_PENDING:
    return "LOBBY_TASK_STATE_CLIENTEND_PENDING";
  case LobbyTaskState::CLIENTEND_DISCONNECT:
    return "LOBBY_TASK_STATE_CLIENTEND_DISCONNECT";
  case LobbyTaskState::CLIENTEND_COOLOFF:
    return "LOBBY_TASK_STATE_CLIENTEND_COOLOFF";
  case LobbyTaskState::CLIENTEND_SHUTDOWN:
    return "LOBBY_TASK_STATE_CLIENTEND_SHUTDOWN";
  case LobbyTaskState::HOSTLAUNCH_INIT:
    return "LOBBY_TASK_STATE_HOSTLAUNCH_INIT";
  case LobbyTaskState::HOSTLAUNCH_PUMP:
    return "LOBBY_TASK_STATE_HOSTLAUNCH_PUMP";
  case LobbyTaskState::HOSTLAUNCH_PRELOAD:
    return "LOBBY_TASK_STATE_HOSTLAUNCH_PRELOAD";
  case LobbyTaskState::HOSTLAUNCH_GO:
    return "LOBBY_TASK_STATE_HOSTLAUNCH_GO";
  case LobbyTaskState::CLIENTLAUNCH_INIT:
    return "LOBBY_TASK_STATE_CLIENTLAUNCH_INIT";
  case LobbyTaskState::CLIENTLAUNCH_PUMP:
    return "LOBBY_TASK_STATE_CLIENTLAUNCH_PUMP";
  case LobbyTaskState::CLIENTLAUNCH_GO:
    return "LOBBY_TASK_STATE_CLIENTLAUNCH_GO";
  case LobbyTaskState::SUCCESS:
    return "LOBBY_TASK_STATE_SUCCESS";
  case LobbyTaskState::FAILURE:
    return "LOBBY_TASK_STATE_FAILURE";
  default:
    return "LOBBY_TASK_STATE_INVALID";
  }
}

enum class LobbyType : int32_t {
  INVALID = -1,
  PRIVATE = 0,
  GAME = 1,
  COUNT = 2,
  FIRST = 0,
  LAST = 1,
};
constexpr bool validate(const LobbyType lobbyType) {
  return lobbyType == LobbyType::PRIVATE || lobbyType == LobbyType::GAME;
}
IMPL_ENUM_OPERATORS(LobbyType);

constexpr const char *serialize(const LobbyType lobbyType) {
  switch (lobbyType) {
  case LobbyType::PRIVATE:
    return "LOBBYTYPE_PRIVATE";
  case LobbyType::GAME:
    return "LOBBYTYPE_GAME";
  default:
    return "LOBBYTYPE_INVALID";
  }
}

enum class LobbyClientType : int32_t {
  INVALID = -1,
  ALL = 0,
  LOCAL = 1,
  REMOTE = 2,
  SPLITSCREEN_ALL = 3,
  SPLITSCREEN_REMOTE = 4,
  GUEST = 5,
};
IMPL_ENUM_OPERATORS(LobbyClientType);

[[nodiscard]] inline constexpr const char *
serialize(const LobbyClientType type) {
  switch (type) {
  case LobbyClientType::INVALID:
    return "LOBBY_CLIENT_TYPE_INVALID";
  case LobbyClientType::ALL:
    return "LOBBY_CLIENT_TYPE_ALL";
  case LobbyClientType::LOCAL:
    return "LOBBY_CLIENT_TYPE_LOCAL";
  case LobbyClientType::REMOTE:
    return "LOBBY_CLIENT_TYPE_REMOTE";
  case LobbyClientType::SPLITSCREEN_ALL:
    return "LOBBY_CLIENT_TYPE_SPLITSCREEN_ALL";
  case LobbyClientType::SPLITSCREEN_REMOTE:
    return "LOBBY_CLIENT_TYPE_SPLITSCREEN_REMOTE";
  case LobbyClientType::GUEST:
    return "LOBBY_CLIENT_TYPE_GUEST";
  default:
    return "LOBBY_CLIENT_TYPE_UNKNOWN";
  }
}

enum class LobbyNetworkMode : int32_t {
  INVALID = -1,
  UNKNOWN = 0,
  LAN = 1,
  LIVE = 2,
  COUNT = 3,
  LOCAL = 1,
};
IMPL_ENUM_OPERATORS(LobbyNetworkMode);

[[nodiscard]] inline constexpr const char *
serialize(const LobbyNetworkMode mode) {
  switch (mode) {
  case LobbyNetworkMode::INVALID:
    return "LOBBY_NETWORK_MODE_INVALID";
  case LobbyNetworkMode::UNKNOWN:
    return "LOBBY_NETWORK_MODE_UNKNOWN";
  case LobbyNetworkMode::LAN:
    return "LOBBY_NETWORK_MODE_LAN";
  case LobbyNetworkMode::LIVE:
    return "LOBBY_NETWORK_MODE_LIVE";
  case LobbyNetworkMode::COUNT:
    return "LOBBY_NETWORK_MODE_COUNT";
  default:
    return "LOBBY_NETWORK_MODE_UNKNOWN";
  }
}

constexpr bool validate(const LobbyNetworkMode networkMode) {
  return networkMode >= LobbyNetworkMode::UNKNOWN &&
         networkMode <= LobbyNetworkMode::COUNT;
}

enum class LobbyMainMode : int32_t {
  INVALID = -1,
  CP = 0,
  MP = 1,
  ZM = 2,
  COUNT = 3,
};
IMPL_ENUM_OPERATORS(LobbyMainMode);

[[nodiscard]] inline constexpr const char *serialize(const LobbyMainMode mode) {
  switch (mode) {
  case LobbyMainMode::INVALID:
    return "LOBBY_MAIN_MODE_INVALID";
  case LobbyMainMode::CP:
    return "LOBBY_MAIN_MODE_CP";
  case LobbyMainMode::MP:
    return "LOBBY_MAIN_MODE_MP";
  case LobbyMainMode::ZM:
    return "LOBBY_MAIN_MODE_ZM";
  case LobbyMainMode::COUNT:
    return "LOBBY_MAIN_MODE_COUNT";
  default:
    return "LOBBY_MAIN_MODE_UNKNOWN";
  }
}

constexpr bool validate(const LobbyMainMode mainMode) {
  return mainMode >= LobbyMainMode::CP && mainMode <= LobbyMainMode::COUNT;
}

enum class LobbyModule : int32_t {
  INVALID = -1,
  HOST = 0x0,
  CLIENT = 0x1,
  COUNT = 0x2,
  FIRST = 0x0,
  LAST = 0x1,
  PEER_TO_PEER = 0x3,
};
IMPL_ENUM_OPERATORS(LobbyModule);

[[nodiscard]] inline constexpr const char *serialize(const LobbyModule module) {
  switch (module) {
  case LobbyModule::INVALID:
    return "LOBBY_MODULE_INVALID";
  case LobbyModule::HOST:
    return "LOBBY_MODULE_HOST";
  case LobbyModule::CLIENT:
    return "LOBBY_MODULE_CLIENT";
  case LobbyModule::COUNT:
    return "LOBBY_MODULE_COUNT";
  case LobbyModule::PEER_TO_PEER:
    return "LOBBY_MODULE_PEER_TO_PEER";
  default:
    return "LOBBY_MODULE_UNKNOWN";
  }
}

enum class LobbyMode : int32_t {
  INVALID = -1,
  PUBLIC = 0,
  CUSTOM = 1,
  THEATER = 2,
  ARENA = 3,
  FREERUN = 4,
  COUNT = 5,
};
IMPL_ENUM_OPERATORS(LobbyMode);

[[nodiscard]] inline constexpr const char *serialize(const LobbyMode mode) {
  switch (mode) {
  case LobbyMode::INVALID:
    return "LOBBY_MODE_INVALID";
  case LobbyMode::PUBLIC:
    return "LOBBY_MODE_PUBLIC";
  case LobbyMode::CUSTOM:
    return "LOBBY_MODE_CUSTOM";
  case LobbyMode::THEATER:
    return "LOBBY_MODE_THEATER";
  case LobbyMode::ARENA:
    return "LOBBY_MODE_ARENA";
  case LobbyMode::FREERUN:
    return "LOBBY_MODE_FREERUN";
  case LobbyMode::COUNT:
    return "LOBBY_MODE_COUNT";
  default:
    return "LOBBY_MODE_UNKNOWN";
  }
}

constexpr bool validate(const LobbyMode mode) {
  return mode >= LobbyMode::PUBLIC && mode <= LobbyMode::COUNT;
}

enum class PartyPrivacy : int32_t {
  OPEN = 0,
  FRIENDS_ONLY = 1,
  INVITE_ONLY = 2,
  CLOSED = 3,
  COUNT = 4,
};
IMPL_ENUM_OPERATORS(PartyPrivacy);

[[nodiscard]] inline constexpr const char *
serialize(const PartyPrivacy privacy) {
  switch (privacy) {
  case PartyPrivacy::OPEN:
    return "PARTY_PRIVACY_OPEN";
  case PartyPrivacy::FRIENDS_ONLY:
    return "PARTY_PRIVACY_FRIENDS_ONLY";
  case PartyPrivacy::INVITE_ONLY:
    return "PARTY_PRIVACY_INVITE_ONLY";
  case PartyPrivacy::CLOSED:
    return "PARTY_PRIVACY_CLOSED";
  case PartyPrivacy::COUNT:
    return "PARTY_PRIVACY_COUNT";
  default:
    return "PARTY_PRIVACY_UNKNOWN";
  }
}

constexpr bool validate(const PartyPrivacy privacy) {
  return privacy >= PartyPrivacy::OPEN && privacy < PartyPrivacy::COUNT;
}

enum class LobbyJoinable : int32_t {
  NO_NOT_IN_LOBBY = 0,
  YES = 1,
  YES_FRIENDS_ONLY = 2,
  NO_PARTY_PRIVACY_INVITE_ONLY = 3,
  NO_PARTY_PRIVACY_CLOSED = 4,
  NO_PARTY_FULL = 5,
  NO_GAME_FULL = 6,
  NO_PLAYING_LOCAL = 7,
  NO_GAME_PAUSED = 8,
  NO = 9,
  NO_YOU_NEED_DLC = 10,
  COUNT = 11,
};
IMPL_ENUM_OPERATORS(LobbyJoinable);

[[nodiscard]] inline constexpr const char *
serialize(const LobbyJoinable joinable) {
  switch (joinable) {
  case LobbyJoinable::NO_NOT_IN_LOBBY:
    return "LOBBY_JOINABLE_NO_NOT_IN_LOBBY";
  case LobbyJoinable::YES:
    return "LOBBY_JOINABLE_YES";
  case LobbyJoinable::YES_FRIENDS_ONLY:
    return "LOBBY_JOINABLE_YES_FRIENDS_ONLY";
  case LobbyJoinable::NO_PARTY_PRIVACY_INVITE_ONLY:
    return "LOBBY_JOINABLE_NO_PARTY_PRIVACY_INVITE_ONLY";
  case LobbyJoinable::NO_PARTY_PRIVACY_CLOSED:
    return "LOBBY_JOINABLE_NO_PARTY_PRIVACY_CLOSED";
  case LobbyJoinable::NO_PARTY_FULL:
    return "LOBBY_JOINABLE_NO_PARTY_FULL";

  case LobbyJoinable::NO_GAME_FULL:
    return "LOBBY_JOINABLE_NO_GAME_FULL";
  case LobbyJoinable::NO_PLAYING_LOCAL:
    return "LOBBY_JOINABLE_NO_PLAYING_LOCAL";
  case LobbyJoinable::NO_GAME_PAUSED:
    return "LOBBY_JOINABLE_NO_GAME_PAUSED";
  case LobbyJoinable::NO:
    return "LOBBY_JOINABLE_NO";
  case LobbyJoinable::NO_YOU_NEED_DLC:
    return "LOBBY_JOINABLE_NO_YOU_NEED_DLC";
  case LobbyJoinable::COUNT:
    return "LOBBY_JOINABLE_COUNT";
  default:
    return "LOBBY_JOINABLE_UNKNOWN";
  }
}

enum class LobbyHostType : int32_t {
  INVALID = -1,
  P2P = 0,
  DEDICATED = 1,
  COUNT = 2,
};
IMPL_ENUM_OPERATORS(LobbyHostType);

[[nodiscard]] inline constexpr const char *serialize(const LobbyHostType type) {
  switch (type) {
  case LobbyHostType::INVALID:
    return "LOBBY_HOST_TYPE_INVALID";
  case LobbyHostType::P2P:
    return "LOBBY_HOST_TYPE_P2P";
  case LobbyHostType::DEDICATED:
    return "LOBBY_HOST_TYPE_DEDICATED";
  case LobbyHostType::COUNT:
    return "LOBBY_HOST_TYPE_COUNT";
  default:
    return "LOBBY_HOST_TYPE_UNKNOWN";
  }
}

enum class LobbyMapVote : int32_t {
  INVALID = -1,
  NONE = 0,
  NEXT = 1,
  PREVIOUS = 2,
  RANDOM = 3,
};
IMPL_ENUM_OPERATORS(LobbyMapVote);

[[nodiscard]] inline constexpr const char *serialize(const LobbyMapVote vote) {
  switch (vote) {
  case LobbyMapVote::INVALID:
    return "LOBBY_MAP_VOTE_INVALID";
  case LobbyMapVote::NONE:
    return "LOBBY_MAP_VOTE_NONE";
  case LobbyMapVote::NEXT:
    return "LOBBY_MAP_VOTE_NEXT";
  case LobbyMapVote::PREVIOUS:
    return "LOBBY_MAP_VOTE_PREVIOUS";
  case LobbyMapVote::RANDOM:
    return "LOBBY_MAP_VOTE_RANDOM";
  default:
    return "LOBBY_MAP_VOTE_UNKNOWN";
  }
}

constexpr bool validate(const LobbyMapVote vote) {
  return vote >= LobbyMapVote::NONE && vote <= LobbyMapVote::RANDOM;
}

enum class LobbyProcessComplete : int32_t {
  INVALID = -1,
  SUCCESS = 0,
  FAILURE = 1,
  LOBBY_PROCESS_COMPLETE_ERROR = 2,
};
IMPL_ENUM_OPERATORS(LobbyProcessComplete);

[[nodiscard]] inline constexpr const char *
serialize(const LobbyProcessComplete complete) {
  switch (complete) {
  case LobbyProcessComplete::INVALID:
    return "LOBBY_PROCESS_COMPLETE_INVALID";
  case LobbyProcessComplete::SUCCESS:
    return "LOBBY_PROCESS_COMPLETE_SUCCESS";
  case LobbyProcessComplete::FAILURE:
    return "LOBBY_PROCESS_COMPLETE_FAILURE";
  case LobbyProcessComplete::LOBBY_PROCESS_COMPLETE_ERROR:
    return "LOBBY_PROCESS_COMPLETE_ERROR";
  default:
    return "LOBBY_PROCESS_COMPLETE_UNKNOWN";
  }
}

enum class LobbyDisconnectClient : int32_t {
  INVALID = -1,
  DROP = 0x0,
  KICK = 0x1,
  BADDLC = 0x2,
  KICK_PARTY = 0x3,
  HOSTRELOAD = 0x4,
  NOPARTYCHAT = 0x5,
  LOBBY_DISCONNECT_CLIENT_ERROR = 0x6,
  MISSING_CONTENT = 0x7,
};
IMPL_ENUM_OPERATORS(LobbyDisconnectClient);

constexpr bool validate(const LobbyDisconnectClient disc) {
  return disc >= LobbyDisconnectClient::DROP &&
         disc <= LobbyDisconnectClient::MISSING_CONTENT;
}

[[nodiscard]] inline constexpr const char *
serialize(const LobbyDisconnectClient disc) {
  switch (disc) {
  case LobbyDisconnectClient::INVALID:
    return "LOBBY_DISCONNECT_CLIENT_INVALID";
  case LobbyDisconnectClient::DROP:
    return "LOBBY_DISCONNECT_CLIENT_DROP";
  case LobbyDisconnectClient::KICK:
    return "LOBBY_DISCONNECT_CLIENT_KICK";
  case LobbyDisconnectClient::BADDLC:
    return "LOBBY_DISCONNECT_CLIENT_BADDLC";
  case LobbyDisconnectClient::KICK_PARTY:
    return "LOBBY_DISCONNECT_CLIENT_KICK_PARTY";
  case LobbyDisconnectClient::HOSTRELOAD:
    return "LOBBY_DISCONNECT_CLIENT_HOSTRELOAD";
  case LobbyDisconnectClient::NOPARTYCHAT:
    return "LOBBY_DISCONNECT_CLIENT_NOPARTYCHAT";
  case LobbyDisconnectClient::LOBBY_DISCONNECT_CLIENT_ERROR:
    return "LOBBY_DISCONNECT_CLIENT_ERROR";
  case LobbyDisconnectClient::MISSING_CONTENT:
    return "LOBBY_DISCONNECT_CLIENT_MISSING_CONTENT";
  default:
    return "LOBBY_DISCONNECT_CLIENT_UNKNOWN";
  }
}

enum class MutableClientFlag : uint32_t {
  HEADSET_PRESENT = 0x0,
  MATURE_CONTENT = 0x1,
  IS_ONBOARDING = 0x2,
  COUNT = 0x3,
};
IMPL_ENUM_OPERATORS(MutableClientFlag);

[[nodiscard]] inline constexpr const char *
serialize(const MutableClientFlag flag) {
  switch (flag) {
  case MutableClientFlag::HEADSET_PRESENT:
    return "MUTABLE_CLIENT_FLAG_HEADSET_PRESENT";
  case MutableClientFlag::MATURE_CONTENT:
    return "MUTABLE_CLIENT_FLAG_MATURE_CONTENT";
  case MutableClientFlag::IS_ONBOARDING:
    return "MUTABLE_CLIENT_FLAG_IS_ONBOARDING";
  case MutableClientFlag::COUNT:
    return "MUTABLE_CLIENT_FLAG_COUNT";
  default:
    return "MUTABLE_CLIENT_FLAG_UNKNOWN";
  }
}

constexpr ClientNum_t MIN_PLAYERS = CLIENT_INDEX_0;
constexpr ClientNum_t MAX_PLAYERS = ClientNum_t::CLIENT_INDEX_COUNT;
template <typename T> using LobbyClientPool = array<T, MAX_PLAYERS>;
template <typename T>
using LobbyClientOptionalPool = array<std::optional<T>, MAX_PLAYERS>;
template <typename T> using AtomicLobbyClientPool = atomicarray<T, MAX_PLAYERS>;
template <typename T>
using AtomicLobbyClientOptionalPool =
    atomicarray<std::optional<T>, MAX_PLAYERS>;

// Verified
struct LobbyParams {
  LobbyNetworkMode networkMode = LobbyNetworkMode::INVALID;
  LobbyMainMode mainMode = LobbyMainMode::INVALID;

  inline std::string serialize() const noexcept {
    return std::format(R"({{ "networkMode": {}, "mainMode": {} }})",
                       lobby::serialize(networkMode),
                       lobby::serialize(mainMode));
  }
};

// Verified
PACKED(struct JoinHost {
  net::HostInfo info;
  LobbyType lobbyType;
  LobbyParams lobbyParams;
  uint8_t _padding8C[4];
  uint64_t reservationKey;
  int32_t retryTime;
  int32_t retryCount;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"info\": {}, \"lobbyType\": {}, \"lobbyParams\": {}, "
        "\"reservationKey\": {}, \"retryTime\": {}, \"retryCount\": {} }}",
        info.serialize(), lobby::serialize(lobbyType), lobbyParams.serialize(),
        reservationKey, retryTime, retryCount);
  }
});
ASSERT_SIZE(JoinHost, 0xA0);

struct AgreementStatus {
  XUID xuid = 0;
  name_t name = {0};
  bool responded = false;
  bool agrees = false;
  int32_t startTime = 0;
  int32_t responseTime = 0;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"xuid\": {:016X}, \"name\": {}, \"responded\": {}, "
        "\"agrees\": {}, \"startTime\": {}, \"responseTime\": {} }}",
        xuid, ::serialize(name), ::serialize(responded), ::serialize(agrees),
        startTime, responseTime);
  }
};

// Verified
struct Agreement {
  int32_t nonce = 0;
  LobbyClientPool<AgreementStatus> status = {{}};
  int32_t requestCount = 0;
  int32_t responseCount = 0;
  int32_t agreeCount = 0;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"nonce\": {}, \"status\": {}, \"requestCount\": {}, "
        "\"responseCount\": {}, \"agreeCount\": {} }}",
        nonce, ::serialize(status), requestCount, responseCount, agreeCount);
  }
};
ASSERT_SIZE(Agreement, 0x408);

struct ServerInfo {
  uint16_t m_usConnectionPort;
  uint16_t m_usQueryPort;
  uint32_t m_unIP;
  int32_t m_nPing;
  uint8_t unk[0x22];
  name_t mapname;
  char description[64];
  char gamemode[16];
  name_t modname;
  int32_t playerCount;
  int32_t maxPlayers;
  int32_t unk2;
  int32_t unk3;
  int32_t unk4;
  bool dedicated;
  bool ranked;
  bool hardcore;
  bool zombies;
  char servername[64];
  char tags[128];
  int32_t unk5;
  int32_t unk6;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"m_usConnectionPort\": {}, \"m_usQueryPort\": {}, "
                       "\"m_unIP\": {}, \"m_nPing\": {}, "
                       "\"unk\": {}, \"mapname\": {}, \"description\": {}, "
                       "\"gamemode\": {}, \"modname\": {}, "
                       "\"playerCount\": {}, \"maxPlayers\": {}, \"unk2\": {}, "
                       "\"unk3\": {}, \"unk4\": {}, "
                       "\"dedicated\": {}, \"ranked\": {}, \"hardcore\": {}, "
                       "\"zombies\": {}, \"servername\": {}, "
                       "\"tags\": {}, \"unk5\": {}, \"unk6\": {} }}",
                       m_usConnectionPort, m_usQueryPort, m_unIP, m_nPing,
                       ::serialize(unk), ::serialize(mapname),
                       ::serialize(description), ::serialize(gamemode),
                       ::serialize(modname), playerCount, maxPlayers, unk2,
                       unk3, unk4, ::serialize(dedicated), ::serialize(ranked),
                       ::serialize(hardcore), ::serialize(zombies),
                       ::serialize(servername), ::serialize(tags), unk5, unk6);
  }
};

typedef uint64_t LobbyID;

// Verified
PACKED(struct ClientProgression {
  uint8_t rank = 0;
  uint8_t prestige = 0;
  uint16_t paragonRank = 0;
  uint8_t paragonIconId = 0;
  uint8_t _padding05[1];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"rank\": {:X}, \"prestige\": {:X}, "
                       "\"paragonRank\": {:X}, \"paragonIconId\": {:X} }}",
                       rank, prestige, paragonRank, paragonIconId);
  }
});
ASSERT_SIZE(ClientProgression, 0x6);

template <typename T> using BubbleGumBuffLobbyArray = array<T, 5>;

// Verified
struct BubbleGumBuffsLobbyClientInfo {
  BubbleGumBuffLobbyArray<uint8_t> bubbleGumBuffs = {0};
  BubbleGumBuffLobbyArray<uint8_t> bubbleGumBuffConsumableCount = {0};

  inline std::string serialize() const noexcept {
    return std::format(
        R"({{ "bubbleGumBuffs": {}, "bubbleGumBuffConsumableCount": {} }})",
        ::serialize(bubbleGumBuffs), ::serialize(bubbleGumBuffConsumableCount));
  }
};
ASSERT_SIZE(BubbleGumBuffsLobbyClientInfo, 0xA);

// Verified
struct CharacterCustomizationLobbyClientInfo {
  uint8_t selectedCharacterType = 0;
  uint8_t selectedCharacterMode = 0;
  uint8_t selectedHeadType = 0;
  uint8_t selectedLoadoutSlot = 0;
  uint8_t selectedCharacterItem[2] = {0};
  vec3<vec2<uint8_t>> selectedCharacterItemColors = {
      .x = {.v = {0}}, .y = {.v = {0}}, .z = {.v = {0}}};

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"selectedCharacterType\": {:X}, \"selectedCharacterMode\": {:X}, "
        "\"selectedHeadType\": {:X}, "
        "\"selectedLoadoutSlot\": {:X}, \"selectedCharacterItem\": {}, "
        "\"selectedCharacterItemColors\": {} }}",
        selectedCharacterType, selectedCharacterMode, selectedHeadType,
        selectedLoadoutSlot, ::serialize(selectedCharacterItem),
        selectedCharacterItemColors.serialize());
  }
};
ASSERT_SIZE(CharacterCustomizationLobbyClientInfo, 0xC);

// Verified
PACKED(struct Variant {
  char variantName[17] = {0};
  uint8_t _padding11[3];
  int32_t attachment[8] = {0};
  uint8_t attachmentVariant[8] = {0};
  uint8_t camoIndex = 0;
  uint8_t _padding3D[3];
  int32_t paintjobSlot = 0;
  int32_t paintjobIndex = 0;
  int32_t weaponIndex = 0;
  int32_t variantIndex = 0;
  int32_t sortIndex = 0;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"variantName\": {}, \"attachment\": {}, \"attachmentVariant\": "
        "{}, "
        "\"camoIndex\": {:X}, \"paintjobSlot\": {:X}, \"paintjobIndex\": {:X}, "
        "\"weaponIndex\": {:X}, \"variantIndex\": {:X}, \"sortIndex\": {:X} }}",
        ::serialize(variantName), ::serialize(attachment),
        ::serialize(attachmentVariant), camoIndex, paintjobSlot, paintjobIndex,
        weaponIndex, variantIndex, sortIndex);
  }
});
ASSERT_SIZE(Variant, 0x54);

// Verified
struct CollectibleInfo {
  bool isSet = false;
  name_t mapName = {0};
  uint8_t collectibleIndex = 0;

  inline std::string serialize() const noexcept {
    return std::format(
        R"({{ "isSet": {}, "mapName": {}, "collectibleIndex": {:X} }})",
        ::serialize(isSet), ::serialize(mapName), collectibleIndex);
  }
};
ASSERT_SIZE(CollectibleInfo, 0x22);

// Verified
PACKED(struct MutableClientInfo {
  LobbyID lobbyID = 0;
  char clantag[5] = {0};
  uint8_t _padding0D[1];
  ClientProgression progression[3] = {{}};
  BubbleGumBuffsLobbyClientInfo bubbleGumInfo = {};
  uint8_t _padding2A[2];
  int32_t arenaPoints = 0;
  float arenaSkill = 0.0f;
  float arenaVariance = 0.0f;
  CharacterCustomizationLobbyClientInfo heroInfo = {};
  Variant showcaseWeapon = {};
  CollectibleInfo collectibles[9] = {{}};
  uint8_t _padding1CA[2];
  // TODO: define flags as bit-packed union
  uint32_t flags = 0;
  BGEmblemBackgroundID backgroundId = 0;
  uint32_t musicUnlocks[3] = {0};
  float skillRating = 0.0f;
  float skillVariance = 0.0f;
  int8_t defaultEmblemIndex = 0;
  int8_t easterEggBits = 0;
  uint16_t unlockedMedals = 0;
  uint32_t profileNonce = 0;
  uint8_t ddlBuff[512] = {0};
  uint16_t ddlBuffSize = 0;
  uint8_t _padding3F2[2];
  union {
    uint32_t probationEndTime[2];
    struct {
      // Serialized as "pprobation" in lobby message packet
      uint32_t publicProbationEndTime = 0;
      // Serialized as "aprobation" in lobby message packet
      uint32_t arenaProbationEndTime = 0;
    };
  };
  bool chunkStatuses[3] = {false};
  bool isStarterPack = false;
  ContentFlags dlcBits = {};
  uint32_t doubleXPGroupMask = 0;
  bool isSplitscreenClient = false;
  bool isInPlatformParty = false;
  bool isInFrontend = false;
  uint8_t _padding40B[1];
  connstate_t connectionState = connstate_t::DISCONNECTED;

  static inline constexpr auto MAX_DDL_BUFF_SIZE = 0x200;

  IMPL_SLICE_METHODS(
      ddlBuff, std::min<uint16_t>(ddlBuffSize, MAX_DDL_BUFF_SIZE), DdlBuff);

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"lobbyID\": {:016X}, \"clantag\": {}, \"progression\": {}, "
        "\"bubbleGumInfo\": {}, "
        "\"arenaPoints\": {}, \"arenaSkill\": {}, \"arenaVariance\": {}, "
        "\"heroInfo\": {}, "
        "\"showcaseWeapon\": {}, \"collectibles\": {}, \"flags\": {:08X}, "
        "\"backgroundId\": {}, "
        "\"musicUnlocks\": {}, \"skillRating\": {}, \"skillVariance\": {}, "
        "\"defaultEmblemIndex\": {}, "
        "\"easterEggBits\": {:X}, \"unlockedMedals\": {}, \"profileNonce\": "
        "{}, "
        "\"ddlBuffSize\": {}, "
        "\"publicProbationEndTime\": {}, \"arenaProbationEndTime\": {}, "
        "\"chunkStatuses\": {}, "
        "\"isStarterPack\": {}, \"dlcBits\": {}, \"doubleXPGroupMask\": "
        "{:08X}, \"isSplitscreenClient\": {}, "
        "\"isInPlatformParty\": {}, \"isInFrontend\": {}, \"connectionState\": "
        "{} }}",
        lobbyID, ::serialize(clantag), ::serialize(progression),
        bubbleGumInfo.serialize(), arenaPoints, arenaSkill, arenaVariance,
        heroInfo.serialize(), showcaseWeapon.serialize(),
        ::serialize(collectibles), flags, backgroundId,
        ::serialize(musicUnlocks), skillRating, skillVariance,
        defaultEmblemIndex, easterEggBits, unlockedMedals, profileNonce,
        ddlBuffSize, publicProbationEndTime, arenaProbationEndTime,
        ::serialize(chunkStatuses), ::serialize(isStarterPack),
        dlcBits.serialize(), doubleXPGroupMask,
        ::serialize(isSplitscreenClient), ::serialize(isInPlatformParty),
        ::serialize(isInFrontend), game::serialize(connectionState));
  }
});
ASSERT_SIZE(MutableClientInfo, 0x410);

struct SharedDataCenterInfo {
  uint16_t shortId = 0;
  uint8_t rtt[10] = {0};

  inline std::string serialize() const noexcept {
    return std::format(R"({{ "shortId": {}, "rtt": {} }})", shortId,
                       ::serialize(rtt));
  }
};
ASSERT_SIZE(SharedDataCenterInfo, 0xC);

// Verified
PACKED(struct FixedClientInfo {
  XUID xuid = 0;
  name_t gamertag = {0};
  dw::net::bdCommonAddrRef commonAdr = {};
  int32_t qport = 0;
  uint8_t latencyBand = 0;
  bool isGuest = false;
  uint8_t _padding36[2];
  int32_t granted = 0;
  int32_t consumed = 0;
  int32_t date = 0;
  int32_t daily = 0;
  uint8_t connectionType = 0;
  uint8_t _padding49[1];
  SharedDataCenterInfo datacenter[10] = {{}};
  uint16_t countryCode = 0;
  float latitude = 0.0f;
  float longitude = 0.0f;
  bool experimentActive = false;
  uint8_t _paddingCD[3];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"xuid\": {:016X}, \"gamertag\": {}, \"commonAdr\": {}, \"qport\": "
        "{}, "
        "\"latencyBand\": {}, \"isGuest\": {}, \"granted\": {}, \"consumed\": "
        "{}, \"date\": {}, "
        "\"daily\": {}, \"connectionType\": {}, \"datacenter\": {}, "
        "\"countryCode\": {}, "
        "\"latitude\": {}, \"longitude\": {}, \"experimentActive\": {} }}",
        xuid, ::serialize(gamertag), ::serialize(commonAdr), qport, latencyBand,
        ::serialize(isGuest), granted, consumed, date, daily, connectionType,
        ::serialize(datacenter), countryCode, latitude, longitude,
        ::serialize(experimentActive));
  }
});
ASSERT_SIZE(FixedClientInfo, 0xD0);

// Verified
PACKED(struct LANClientInfo {
  XUID xuid = 0;
  name_t gamertag = {0};

  inline std::string serialize() const noexcept {
    return std::format("{{ \"xuid\": {:016X}, \"gamertag\": {} }}", xuid,
                       ::serialize(gamertag));
  }
});
ASSERT_SIZE(LANClientInfo, 0x28);

template <typename T> union LobbyModulePool {
  typedef uint8_t index_t;

  T pool[+LobbyModule::COUNT] = {{}};

  struct {
    T host;
    T client;
  };

  constexpr void assert_range([[maybe_unused]] index_t index) const {
    assert(index < +LobbyModule::COUNT &&
           "index to LobbyModulePool must satisfy range "
           "LobbyModule::HOST <= "
           "index < LobbyModule::COUNT");
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

  operator const array<T, +LobbyModule::COUNT> &() const noexcept {
    return pool;
  }

  operator array<T, +LobbyModule::COUNT> &() noexcept { return pool; }

  operator std::span<const T, +LobbyModule::COUNT>() const noexcept {
    return std::span<const T, +LobbyModule::COUNT>(pool);
  }

  operator std::span<T, +LobbyModule::COUNT>() noexcept {
    return std::span<T, +LobbyModule::COUNT>(pool);
  }

  static constexpr auto size() noexcept { return +LobbyModule::COUNT; }

  [[nodiscard]] inline std::string serialize() const noexcept {
    return ::serialize(pool);
  }
};
} // namespace lobby
} // namespace game
