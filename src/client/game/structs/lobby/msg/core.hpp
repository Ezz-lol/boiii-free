#pragma once

#include <cstdint>

#include "game/structs/core.hpp"
#include "game/structs/net/core.hpp"
#include "game/structs/net/msg.hpp"
namespace game {
namespace lobby {

namespace msg {

struct LobbyMsg;
typedef fastcall_t<void(ControllerIndex_t controllerIndex, net::netadr_t *adr,
                        XUID xuid, LobbyMsg *msg)>
    LobbyMsg_MsgHandleCallback;
typedef LobbyMsg_MsgHandleCallback *LobbyMsg_MsgHandleCallbackPtr;

enum class LobbyMsgElementType : uint32_t {
  INT32 = 0x0,
  UINT32 = 0x1,
  INT16 = 0x2,
  UINT16 = 0x3,
  INT8 = 0x4,
  UINT8 = 0x5,
  INT64 = 0x6,
  UINT64 = 0x7,
  FLOAT = 0x8,
  XUID = 0x9,
  STRING = 0xA,
  GLOB = 0xB,
  ARRAY_BEGIN = 0xC,
  ARRAY_ELEMENT = 0xD,
  ARRAY_END = 0xE,
  DEBUG_START = 0xF,
  DEBUG_END = 0x10,
  COUNT = 0x11,
};

enum class MsgType : int32_t {
  NONE = -1,
  INFO_REQUEST = 0,
  INFO_RESPONSE = 1,
  LOBBY_STATE_PRIVATE = 2,
  LOBBY_STATE_GAME = 3,
  LOBBY_STATE_GAMEPUBLIC = 4,
  LOBBY_STATE_GAMECUSTOM = 5,
  LOBBY_STATE_GAMETHEATER = 6,
  LOBBY_HOST_HEARTBEAT = 7,
  LOBBY_HOST_DISCONNECT = 8,
  LOBBY_HOST_DISCONNECT_CLIENT = 9,
  LOBBY_HOST_LEAVE_WITH_PARTY = 0xA,
  LOBBY_CLIENT_HEARTBEAT = 0xB,
  LOBBY_CLIENT_DISCONNECT = 0xC,
  LOBBY_CLIENT_RELIABLE_DATA = 0xD,
  LOBBY_CLIENT_CONTENT = 0xE,
  LOBBY_MODIFIED_STATS = 0xF,
  JOIN_LOBBY = 0x10,
  JOIN_RESPONSE = 0x11,
  JOIN_AGREEMENT_REQUEST = 0x12,
  JOIN_AGREEMENT_RESPONSE = 0x13,
  JOIN_COMPLETE = 0x14,
  JOIN_MEMBER_INFO = 0x15,
  SERVERLIST_INFO = 0x16,
  PEER_TO_PEER_CONNECTIVITY_TEST = 0x17,
  PEER_TO_PEER_INFO = 0x18,
  LOBBY_MIGRATE_TEST = 0x19,
  MIGRATE_ANNOUNCE_HOST = 0x1A,
  MIGRATE_START = 0x1B,
  INGAME_MIGRATE_TO = 0x1C,
  MIGRATE_NEW_HOST = 0x1D,
  VOICE_PACKET = 0x1E,
  VOICE_RELAY_PACKET = 0x1F,
  DEMO_STATE = 0x20,
  COUNT = 0x21,
};
IMPL_ENUM_OPERATORS(MsgType);

inline constexpr std::string_view lobbyMsgName[+MsgType::COUNT] = {
    "InfoRequest",
    "InfoResponse",
    "StatePrivate",
    "StateGame",
    "StateGamePublic",
    "StateGameCustom",
    "StateGameTheater",
    "HostHeartbeat",
    "HostDisconnect",
    "HostDisconnectClient",
    "HostLeaveWithParty",
    "ClientHeartbeat",
    "ClientDisconnect",
    "ClientReliableData",
    "ClientContent",
    "ClientStatsModified",
    "JoinLobby",
    "JoinResponse",
    "JoinAgreementRequest",
    "JoinAgreementResponse",
    "JoinComplete",
    "JoinMemberInfo",
    "ServerlistInfo",
    "P2PConnectivityTest",
    "P2PInfo",
    "MigrateBandwidthTest",
    "AnnounceHost",
    "StartMigrate",
    "MigrateHostTo",
    "NewIngameHost",
    "VoicePacket",
    "VoiceRelayPacket",
    "DemoState",
};
inline constexpr auto MAX_MSG_NAME_STRING_LENGTH = 0x20;
typedef char msgName_t[MAX_MSG_NAME_STRING_LENGTH];

enum class PackageType : uint32_t {
  NONE = 0x0,
  WRITE = 0x1,
  READ = 0x2,
};

inline constexpr auto LOBBYMSG_MAX_KEY_LENGTH = 0x10;
typedef char lobbyMsgKey_t[LOBBYMSG_MAX_KEY_LENGTH];

union LobbyMsgEncodeFlags {
  uint8_t packed;
  struct {
    uint8_t isArray : 1;
    uint8_t encodeKey : 1;
    uint8_t : 6;
  };

  inline constexpr void clear() noexcept { packed = 0; }
};
struct LobbyMsg {
  net::msg::msg_t msg;
  MsgType msgType;
  LobbyMsgEncodeFlags encodeFlags;
  PackageType packageType;

  bool package_int(const std::string_view &key, int32_t *val);
  bool package_bool(const std::string_view &key, bool *val);
  bool package_char(const std::string_view &key, char *val);
  bool package_glob(const std::string_view &key, void *val, int numbytes);
  bool package_uint(const std::string_view &key, uint32_t *val);
  bool package_xuid(const std::string_view &key, XUID *val);
  bool package_float(const std::string_view &key, float *val);
  bool package_short(const std::string_view &key, int16_t *val);
  bool package_uchar(const std::string_view &key, uint8_t *val);
  bool package_string(const std::string_view &key, char *val, int maxChar);
  bool package_uint64(const std::string_view &key, uint64_t *val);
  bool package_ushort(const std::string_view &key, uint16_t *val);
  bool package_element(bool addElement);
  bool package_arrayStart(const std::string_view &key);
  bool prep_readMsg();
  bool prep_readMsg(net::msg::msg_t *msg);
  bool prep_readMsg(uint8_t *data, int length);
  bool prep_writeMsg(uint8_t *data, int length, MsgType type);
  bool read_char(const std::string_view &expectedKey, char *val);
  bool read_int(const std::string_view &expectedKey, int32_t *val);
  bool read_bool(const std::string_view &expectedKey, bool *val);
  bool read_glob(const std::string_view &expectedKey, void *data,
                 int32_t numBytes);
  bool read_uint(const std::string_view &expectedKey, uint32_t *val);
  bool read_xuid(const std::string_view &expectedKey, XUID *val);
  bool read_float(const std::string_view &expectedKey, float *val);
  bool read_short(const std::string_view &expectedKey, int16_t *val);
  bool read_uchar(const std::string_view &expectedKey, uint8_t *val);
  bool read_string(const std::string_view &expectedKey, char *outStr,
                   uint32_t maxChar);
  bool read_uint64(const std::string_view &expectedKey, uint64_t *val);
  bool read_ushort(const std::string_view &expectedKey, uint16_t *val);
  bool read_arrayBegin(const std::string_view &expectedKey);
  LobbyMsgElementType read_elementType();
  bool write_char(const std::string_view &key, char val);
  bool write_int(const std::string_view &key, int32_t val);
  bool write_glob(const std::string_view &key, void *val, int32_t numbytes);
  bool write_uint(const std::string_view &key, uint32_t val);
  bool write_xuid(const std::string_view &key, XUID val);
  bool write_float(const std::string_view &key, float val);
  bool write_short(const std::string_view &key, int16_t val);
  bool write_uchar(const std::string_view &key, uint8_t val);
  bool write_string(const std::string_view &key, const std::string_view &val);
  bool write_uint64(const std::string_view &key, uint64_t val);
  bool write_ushort(const std::string_view &key, uint16_t val);
};

enum class MsgState : uint32_t {
  IDLE = 0x0,
  WAITING_FOR_ACK = 0x1,
};

struct EVENT_HANDLE {
  bool manualReset;
  uint8_t _padding1[7];
};

enum class netchanMessageState_e : uint32_t {
  COMPLETE = 0x0,
  FAILED = 0x1,
  TIMEOUT = 0x2,
};

struct netchanMsgConfig_s {
  EVENT_HANDLE completedEvent;
  netchanMessageState_e status;
  int32_t timeoutMS;
};

typedef netchanMsgConfig_s netchanMsgConfig_t;

// Verified
struct NetChanMsg {
  int32_t sentTime;
  MsgState msgState;
  MsgType msgType;
  netchanMsgConfig_t config;
  int32_t timeout;
  XUID toXUID;
  XUID clientXUID;
};
ASSERT_SIZE(NetChanMsg, 0x30);

struct LobbyMsgHandler {
  MsgType msgType;
  LobbyMsg_MsgHandleCallback *function;
};
} // namespace msg
} // namespace lobby
} // namespace game
