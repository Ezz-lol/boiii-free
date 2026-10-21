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

enum class PackageType : uint32_t {
  NONE = 0x0,
  WRITE = 0x1,
  READ = 0x2,
};

struct LobbyMsg {
  game::net::msg::msg_t msg;
  MsgType msgType;
  char encodeFlags;
  PackageType packageType;
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
