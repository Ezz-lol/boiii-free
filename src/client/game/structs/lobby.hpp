#pragma once

#include "core.hpp"
#include "net.hpp"
#include <cstdint>

namespace game {
namespace lobby {
enum LobbyType {
  LOBBY_TYPE_INVALID = -1,
  LOBBY_TYPE_PRIVATE = 0x0,
  LOBBY_TYPE_GAME = 0x1,
  LOBBY_TYPE_TRANSITION = 0x2,
  LOBBY_TYPE_COUNT = 0x3,
  LOBBY_TYPE_FIRST = 0x0,
  LOBBY_TYPE_LAST = 0x2,
  LOBBY_TYPE_AUTO = 0x3,
};

enum LobbyClientType {
  LOBBY_CLIENT_TYPE_INVALID = -1,
  LOBBY_CLIENT_TYPE_ALL = 0x0,
  LOBBY_CLIENT_TYPE_LOCAL = 0x1,
  LOBBY_CLIENT_TYPE_REMOTE = 0x2,
};

enum LobbyNetworkMode {
  LOBBY_NETWORKMODE_INVALID = -1,
  LOBBY_NETWORKMODE_LOCAL = 0x0,
  LOBBY_NETWORKMODE_LAN = 0x1,
  LOBBY_NETWORKMODE_LIVE = 0x2,
  LOBBY_NETWORKMODE_COUNT = 0x3,
};

enum LobbyMainMode {
  LOBBY_MAINMODE_INVALID = -1,
  LOBBY_MAINMODE_CP = 0x0,
  LOBBY_MAINMODE_MP = 0x1,
  LOBBY_MAINMODE_ZM = 0x2,
  LOBBY_MAINMODE_COUNT = 0x3,
};

struct LobbyParams {
  LobbyNetworkMode networkMode;
  LobbyMainMode mainMode;
};

enum JoinType {
  JOIN_TYPE_NORMAL = 0x0,
  JOIN_TYPE_PLAYLIST = 0x1,
  JOIN_TYPE_FRIEND = 0x2,
  JOIN_TYPE_INVITE = 0x3,
  JOIN_TYPE_PARTY = 0x4,
  JOIN_TYPE_COUNT = 0x5,
};

struct JoinHost {
  net::HostInfo info;
  LobbyType lobbyType;
  LobbyParams lobbyParams;
  uint64_t reservationKey;
  int32_t retryTime;
  int32_t retryCount;
};

enum JoinSourceState {
  JOIN_SOURCE_STATE_IDLE = 0x0,
  JOIN_SOURCE_STATE_CONNECT_TO_NEXT_HOST = 0x1,
  JOIN_SOURCE_STATE_ASSOCIATING = 0x2,
  JOIN_SOURCE_STATE_HANDSHAKING = 0x3,
  JOIN_SOURCE_STATE_WAITING_FOR_AGREEMENT = 0x4,
  JOIN_SOURCE_STATE_CONNECTION_FAILED = 0x5,
  JOIN_SOURCE_STATE_CONNECTION_SUCCESS = 0x6,
  JOIN_SOURCE_STATE_ENDING_HOST = 0x7,
  JOIN_SOURCE_STATE_CLEANUP = 0x8,
  JOIN_SOURCE_STATE_COUNT = 0x9,
};

enum JoinResult {
  JOIN_RESULT_INVALID = 0x0,
  JOIN_RESULT_SUCCESS = 0x1,
  JOIN_RESULT_CONNECT_TO_HOST_FAILURE = 0x2,
  JOIN_RESULT_PROBE_SEND_FAILURE = 0x3,
  JOIN_RESULT_PROBE_TIMEOUT = 0x4,
  JOIN_RESULT_PROBE_INVALID_LOBBY = 0x5,
  JOIN_RESULT_PROBE_INVALID_INFO = 0x6,
  JOIN_RESULT_PROBE_RESULT_INVALID = 0x7,
  JOIN_RESULT_INVALID_LOBBY = 0x8,
  JOIN_RESULT_SEND_AGREEMENT_REQUEST_FAILED = 0x9,
  JOIN_RESULT_HANDSHAKE_WINDOW_EXPIRED = 0xA,
  JOIN_RESULT_AGREEMENT_WINDOW_EXPIRED = 0xB,
  JOIN_RESULT_JOIN_DISABLED = 0xC,
  JOIN_RESULT_JOIN_ALREADY_IN_PROGRESS = 0xD,
  JOIN_RESULT_NOT_JOINABLE_NOT_HOSTING = 0xE,
  JOIN_RESULT_NOT_JOINABLE_NOT_IDLE = 0xF,
  JOIN_RESULT_NOT_JOINABLE_CLOSED = 0x10,
  JOIN_RESULT_NOT_JOINABLE_INVITE_ONLY = 0x11,
  JOIN_RESULT_NOT_JOINABLE_FRIENDS_ONLY = 0x12,
  JOIN_RESULT_LOBBY_FULL = 0x13,
  JOIN_RESULT_NETWORK_MODE_MISMATCH = 0x14,
  JOIN_RESULT_MISMATCH_PLAYLISTID = 0x15,
  JOIN_RESULT_MISMATCH_PLAYLIST_VERSION_TO_NEW = 0x16,
  JOIN_RESULT_MISMATCH_PLAYLIST_VERSION_TO_OLD = 0x17,
  JOIN_RESULT_MISMATCH_PROTOCOL_VERSION = 0x18,
  JOIN_RESULT_MISMATCH_NETFIELD_CHECKSUM = 0x19,
  JOIN_RESULT_MISMATCH_FFOTD_VERSION_TO_NEW = 0x1A,
  JOIN_RESULT_MISMATCH_FFOTD_VERSION_TO_OLD = 0x1B,
  JOIN_RESULT_MIGRATE_IN_PROGRESS = 0x1C,
  JOIN_RESULT_COULD_NOT_RESERVE = 0x1D,
  JOIN_RESPONSE_COUNT = 0x1E,
};

using joinCompleteCallback = void (*)(int, JoinResult);

struct AgreementStatus {
  XUID xuid;
  char name[32];
  bool responded;
  bool agrees;
  int32_t startTime;
  int32_t responseTime;
};

struct Agreement {
  int32_t nonce;
  AgreementStatus status[18];
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
  char mapname[32];
  char description[64];
  char gamemode[16];
  char modname[32];
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
} // namespace lobby
} // namespace game