#pragma once

#include "core.hpp"
#include "net/net.hpp"
#include <cstdint>
#include <optional>

namespace game {
namespace lobby {

constexpr ClientNum_t MIN_PLAYERS = CLIENT_INDEX_0;
constexpr ClientNum_t MAX_PLAYERS = ClientNum_t::CLIENT_INDEX_COUNT;
template <typename T> using LobbyClientPool = array<T, MAX_PLAYERS>;
template <typename T>
using LobbyClientOptionalPool = array<std::optional<T>, MAX_PLAYERS>;
template <typename T> using AtomicLobbyClientPool = atomicarray<T, MAX_PLAYERS>;
template <typename T>
using AtomicLobbyClientOptionalPool =
    atomicarray<std::optional<T>, MAX_PLAYERS>;

enum class LobbyType : int32_t {
  INVALID = -1,
  PRIVATE = 0x0,
  GAME = 0x1,
  TRANSITION = 0x2,
  COUNT = 0x3,
  FIRST = 0x0,
  LAST = 0x2,
  AUTO = 0x3,
};
IMPL_ENUM_OPERATORS(LobbyType);

enum class LobbyClientType : int32_t {
  INVALID = -1,
  ALL = 0x0,
  LOCAL = 0x1,
  REMOTE = 0x2,
};
IMPL_ENUM_OPERATORS(LobbyClientType);

enum class LobbyNetworkMode : int32_t {
  INVALID = -1,
  LOCAL = 0x0,
  LAN = 0x1,
  LIVE = 0x2,
  COUNT = 0x3,
};
IMPL_ENUM_OPERATORS(LobbyNetworkMode);

enum class LobbyMainMode {
  INVALID = -1,
  CP = 0x0,
  MP = 0x1,
  ZM = 0x2,
  COUNT = 0x3,
};
IMPL_ENUM_OPERATORS(LobbyMainMode);

struct LobbyParams {
  LobbyNetworkMode networkMode;
  LobbyMainMode mainMode;
};

enum class JoinType : uint32_t {
  NORMAL = 0x0,
  PLAYLIST = 0x1,
  FRIEND = 0x2,
  INVITE = 0x3,
  PARTY = 0x4,
  COUNT = 0x5,
};
IMPL_ENUM_OPERATORS(JoinType);

struct JoinHost {
  net::HostInfo info;
  LobbyType lobbyType;
  LobbyParams lobbyParams;
  uint64_t reservationKey;
  int32_t retryTime;
  int32_t retryCount;
};

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
  NOT_JOINABLE_NOT_HOSTING = 0xE,
  NOT_JOINABLE_NOT_IDLE = 0xF,
  NOT_JOINABLE_CLOSED = 0x10,
  NOT_JOINABLE_INVITE_ONLY = 0x11,
  NOT_JOINABLE_FRIENDS_ONLY = 0x12,
  LOBBY_FULL = 0x13,
  NETWORK_MODE_MISMATCH = 0x14,
  MISMATCH_PLAYLISTID = 0x15,
  MISMATCH_PLAYLIST_VERSION_TO_NEW = 0x16,
  MISMATCH_PLAYLIST_VERSION_TO_OLD = 0x17,
  MISMATCH_PROTOCOL_VERSION = 0x18,
  MISMATCH_NETFIELD_CHECKSUM = 0x19,
  MISMATCH_FFOTD_VERSION_TO_NEW = 0x1A,
  MISMATCH_FFOTD_VERSION_TO_OLD = 0x1B,
  MIGRATE_IN_PROGRESS = 0x1C,
  COULD_NOT_RESERVE = 0x1D,
  COUNT = 0x1E,
};

using joinCompleteCallback = void (*)(int, JoinResult);

struct AgreementStatus {
  XUID xuid;
  name_t name;
  bool responded;
  bool agrees;
  int32_t startTime;
  int32_t responseTime;
};

struct Agreement {
  int32_t nonce;
  LobbyClientPool<AgreementStatus> status;
  int32_t requestCount;
  int32_t responseCount;
  int32_t agreeCount;
};

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
  JoinResult joinResult;
};

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
};
enum class JoinClientState : int32_t {
  JOIN_CLIENT_STATE_IDLE = 0x0,
  JOIN_CLIENT_STATE_CONNECT_TO_NEXT_HOST = 0x1,
  JOIN_CLIENT_STATE_ASSOCIATING = 0x2,

  JOIN_CLIENT_STATE_HANDSHAKING = 0x3,
  JOIN_CLIENT_STATE_WAITING_FOR_AGREEMENT = 0x4,
  JOIN_CLIENT_STATE_CONNECTED_FAILED = 0x5,
  JOIN_CLIENT_STATE_CONNECTION_SUCCESS = 0x6,
  JOIN_CLIENT_STATE_ENDING_HOST = 0x7,
  JOIN_CLIENT_STATE_CLEANUP = 0x8,
  JOIN_CLIENT_STATE_COUNT = 0x9,
};
IMPL_ENUM_OPERATORS(JoinClientState);

// =====================================================================
// LobbyMsg struct - matches in-memory layout at the point of inspection.
// game::net::msg::msg_t is 0x38 bytes, followed by msgType at offset 0x38.
// =====================================================================
struct LobbyMsg {
  game::net::msg::msg_t msg; // 0x00 - 0x37
  int32_t msgType;           // 0x38
  char encodeFlags;          // 0x3C
  int32_t packageType;       // 0x40
};

} // namespace lobby
} // namespace game