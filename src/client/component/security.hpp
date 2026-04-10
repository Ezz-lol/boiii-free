#pragma once
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <functional>

#include "game/game.hpp"

namespace ezzsec {
// =====================================================================
// Lobby message type enum
// =====================================================================
enum MsgType : int32_t {
  MESSAGE_TYPE_NONE = -1,
  MESSAGE_TYPE_INFO_REQUEST = 0,
  MESSAGE_TYPE_INFO_RESPONSE = 1,
  MESSAGE_TYPE_LOBBY_STATE_PRIVATE = 2,
  MESSAGE_TYPE_LOBBY_STATE_GAME = 3,
  MESSAGE_TYPE_LOBBY_STATE_GAMEPUBLIC = 4,
  MESSAGE_TYPE_LOBBY_STATE_GAMECUSTOM = 5,
  MESSAGE_TYPE_LOBBY_STATE_GAMETHEATER = 6,
  MESSAGE_TYPE_LOBBY_HOST_HEARTBEAT = 7,
  MESSAGE_TYPE_LOBBY_HOST_DISCONNECT = 8,
  MESSAGE_TYPE_LOBBY_HOST_DISCONNECT_CLIENT = 9,
  MESSAGE_TYPE_LOBBY_HOST_LEAVE_WITH_PARTY = 0xA,
  MESSAGE_TYPE_LOBBY_CLIENT_HEARTBEAT = 0xB,
  MESSAGE_TYPE_LOBBY_CLIENT_DISCONNECT = 0xC,
  MESSAGE_TYPE_LOBBY_CLIENT_RELIABLE_DATA = 0xD,
  MESSAGE_TYPE_LOBBY_CLIENT_CONTENT = 0xE,
  MESSAGE_TYPE_LOBBY_MODIFIED_STATS = 0xF,
  MESSAGE_TYPE_JOIN_LOBBY = 0x10,
  MESSAGE_TYPE_JOIN_RESPONSE = 0x11,
  MESSAGE_TYPE_JOIN_AGREEMENT_REQUEST = 0x12,
  MESSAGE_TYPE_JOIN_AGREEMENT_RESPONSE = 0x13,
  MESSAGE_TYPE_JOIN_COMPLETE = 0x14,
  MESSAGE_TYPE_JOIN_MEMBER_INFO = 0x15,
  MESSAGE_TYPE_SERVERLIST_INFO = 0x16,
  MESSAGE_TYPE_PEER_TO_PEER_CONNECTIVITY_TEST = 0x17,
  MESSAGE_TYPE_PEER_TO_PEER_INFO = 0x18,
  MESSAGE_TYPE_LOBBY_MIGRATE_TEST = 0x19,
  MESSAGE_TYPE_MIGRATE_ANNOUNCE_HOST = 0x1A,
  MESSAGE_TYPE_MIGRATE_START = 0x1B,
  MESSAGE_TYPE_INGAME_MIGRATE_TO = 0x1C,
  MESSAGE_TYPE_MIGRATE_NEW_HOST = 0x1D,
  MESSAGE_TYPE_VOICE_PACKET = 0x1E,
  MESSAGE_TYPE_VOICE_RELAY_PACKET = 0x1F,
  MESSAGE_TYPE_DEMO_STATE = 0x20,
  MESSAGE_TYPE_COUNT = 0x21,
};

// =====================================================================
// LobbyMsg struct - matches in-memory layout at the point of inspection.
// game::msg_t is 0x38 bytes, followed by msgType at offset 0x38.
// =====================================================================
struct LobbyMsg {
  game::msg_t msg;     // 0x00 - 0x37
  int32_t msgType;     // 0x38
  char encodeFlags;    // 0x3C
  int32_t packageType; // 0x40
};

// =====================================================================
// Function pointer typedefs for LobbyMsgRW functions
// =====================================================================
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-attributes"
using tLobbyMsgRW_PackageInt = bool(__fastcall *)(void *lobbyMsg,
                                                  const char *key,
                                                  int32_t *val);
using tLobbyMsgRW_PackageUChar = bool(__fastcall *)(void *lobbyMsg,
                                                    const char *key, char *val);
using tLobbyMsgRW_PackageString = bool(__fastcall *)(void *lobbyMsg,
                                                     const char *key,
                                                     const char *val,
                                                     int maxLength);
using tLobbyMsgRW_PackageXuid = bool(__fastcall *)(void *lobbyMsg,
                                                   const char *key,
                                                   uint64_t *val);
using tLobbyMsgRW_PackageBool = bool(__fastcall *)(void *lobbyMsg,
                                                   const char *key, char *val);
using tLobbyMsgRW_PackageUInt = bool(__fastcall *)(void *lobbyMsg,
                                                   const char *key,
                                                   uint32_t *val);
using tLobbyMsgRW_PackageShort = bool(__fastcall *)(void *lobbyMsg,
                                                    const char *key,
                                                    int16_t *val);
using tLobbyMsgRW_PackageUInt64 = bool(__fastcall *)(void *lobbyMsg,
                                                     const char *key,
                                                     uint64_t *val);
using tLobbyMsgRW_PackageArrayStart = bool(__fastcall *)(void *lobbyMsg,
                                                         const char *key);
using tLobbyMsgRW_PackageElement = bool(__fastcall *)(void *lobbyMsg,
                                                      bool addElement);
using tLobbyMsgRW_PackageGlob = bool(__fastcall *)(void *lobbyMsg,
                                                   const char *key,
                                                   const char *val,
                                                   int maxLength);
using tLobbyMsgRW_PackageFloat = bool(__fastcall *)(void *lobbyMsg,
                                                    const char *key,
                                                    float *val);
using tMsgMutableClientInfo_Package = bool(__fastcall *)(void *outRequest,
                                                         void *lobbyMsg);
using tLobbyMsgRW_PrepReadData = bool(__fastcall *)(LobbyMsg *, char *, int);
using tMSG_ReadData = void(__fastcall *)(game::msg_t *, char *, int);
#pragma clang diagnostic pop

// =====================================================================
// Lazy-initialized function pointers
// Uses boiii-free's _g operator for address relocation.
//
// T7Patch REBASE(x) = PEB_base + x
// boiii-free 0x140000000+x _g = game_base + ((0x140000000+x) - 0x140000000) =
// game_base + x
// =====================================================================
namespace fn {
inline tLobbyMsgRW_PackageInt LobbyMsgRW_PackageInt = nullptr;
inline tLobbyMsgRW_PackageUChar LobbyMsgRW_PackageUChar = nullptr;
inline tLobbyMsgRW_PackageString LobbyMsgRW_PackageString = nullptr;
inline tLobbyMsgRW_PackageXuid LobbyMsgRW_PackageXuid = nullptr;
inline tLobbyMsgRW_PackageBool LobbyMsgRW_PackageBool = nullptr;
inline tLobbyMsgRW_PackageUInt LobbyMsgRW_PackageUInt = nullptr;
inline tLobbyMsgRW_PackageShort LobbyMsgRW_PackageShort = nullptr;
inline tLobbyMsgRW_PackageUInt64 LobbyMsgRW_PackageUInt64 = nullptr;
inline tLobbyMsgRW_PackageArrayStart LobbyMsgRW_PackageArrayStart = nullptr;
inline tLobbyMsgRW_PackageElement LobbyMsgRW_PackageElement = nullptr;
inline tLobbyMsgRW_PackageGlob LobbyMsgRW_PackageGlob = nullptr;
inline tLobbyMsgRW_PackageFloat LobbyMsgRW_PackageFloat = nullptr;
inline tMsgMutableClientInfo_Package MsgMutableClientInfo_Package = nullptr;
inline tLobbyMsgRW_PrepReadData LobbyMsgRW_PrepReadData = nullptr;
inline tMSG_ReadData MSG_ReadData = nullptr;
inline bool initialized = false;

inline void initialize() {
  if (initialized)
    return;

  // These offsets are client-only.
  // The _g operator: val_g = game_base + (val - 0x140000000)
  LobbyMsgRW_PackageInt =
      reinterpret_cast<tLobbyMsgRW_PackageInt>(0x141EF66D0_g);
  LobbyMsgRW_PackageUChar =
      reinterpret_cast<tLobbyMsgRW_PackageUChar>(0x141EF6800_g);
  LobbyMsgRW_PackageString =
      reinterpret_cast<tLobbyMsgRW_PackageString>(0x141EF6770_g);
  LobbyMsgRW_PackageXuid =
      reinterpret_cast<tLobbyMsgRW_PackageXuid>(0x141EF6940_g);
  LobbyMsgRW_PackageBool =
      reinterpret_cast<tLobbyMsgRW_PackageBool>(0x141EF6580_g);
  LobbyMsgRW_PackageUInt =
      reinterpret_cast<tLobbyMsgRW_PackageUInt>(0x141EF68A0_g);
  LobbyMsgRW_PackageShort =
      reinterpret_cast<tLobbyMsgRW_PackageShort>(0x141EF6750_g);
  LobbyMsgRW_PackageUInt64 =
      reinterpret_cast<tLobbyMsgRW_PackageUInt64>(0x141EF6820_g);
  LobbyMsgRW_PackageArrayStart =
      reinterpret_cast<tLobbyMsgRW_PackageArrayStart>(0x141EF6510_g);
  LobbyMsgRW_PackageElement =
      reinterpret_cast<tLobbyMsgRW_PackageElement>(0x141EF65C0_g);
  LobbyMsgRW_PackageGlob =
      reinterpret_cast<tLobbyMsgRW_PackageGlob>(0x141EF66B0_g);
  LobbyMsgRW_PackageFloat =
      reinterpret_cast<tLobbyMsgRW_PackageFloat>(0x141EF6630_g);
  MsgMutableClientInfo_Package =
      reinterpret_cast<tMsgMutableClientInfo_Package>(0x141ED47D0_g);
  LobbyMsgRW_PrepReadData =
      reinterpret_cast<tLobbyMsgRW_PrepReadData>(0x141EF69C0_g);
  MSG_ReadData = reinterpret_cast<tMSG_ReadData>(0x1422154B0_g);

  initialized = true;
}
} // namespace fn

// =====================================================================
// Scratch buffers for packet inspection
// =====================================================================
inline thread_local char requestOut[0x20000]{};
inline thread_local char lobbyMsgCpy[0x50]{};

// =====================================================================
// MSG_JoinParty_Package_Inspect
// Validates JoinParty (0x10) lobby messages for buffer overflow attacks.
// Returns 0 if packet is safe, non-zero if malicious.
// =====================================================================
inline int MSG_JoinParty_Package_Inspect(char *_this, char *lobbyMsg) {
  bool packageOK =
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "targetlobby",
                                reinterpret_cast<int32_t *>(_this)) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "sourcelobby",
                                reinterpret_cast<int32_t *>(_this + 4)) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "jointype",
                                reinterpret_cast<int32_t *>(_this + 8)) &&
      fn::LobbyMsgRW_PackageXuid(lobbyMsg, "probedxuid",
                                 reinterpret_cast<uint64_t *>(_this + 16)) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "playlistid",
                                reinterpret_cast<int32_t *>(_this + 612)) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "playlistver",
                                reinterpret_cast<int32_t *>(_this + 616)) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "ffotdver",
                                reinterpret_cast<int32_t *>(_this + 620)) &&
      fn::LobbyMsgRW_PackageShort(lobbyMsg, "networkmode",
                                  reinterpret_cast<int16_t *>(_this + 624)) &&
      fn::LobbyMsgRW_PackageUInt(lobbyMsg, "netchecksum",
                                 reinterpret_cast<uint32_t *>(_this + 628)) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "protocol",
                                reinterpret_cast<int32_t *>(_this + 632)) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "changelist",
                                reinterpret_cast<int32_t *>(_this + 636)) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "pingband",
                                reinterpret_cast<int32_t *>(_this + 640)) &&
      fn::LobbyMsgRW_PackageUInt(lobbyMsg, "dlcbits",
                                 reinterpret_cast<uint32_t *>(_this + 644)) &&
      fn::LobbyMsgRW_PackageUInt64(lobbyMsg, "joinnonce",
                                   reinterpret_cast<uint64_t *>(_this + 648));

  for (int i = 0; packageOK && (i < 3); i++) {
    packageOK = packageOK &&
                fn::LobbyMsgRW_PackageUChar(lobbyMsg, "chunk", _this + i + 689);
  }

  packageOK =
      packageOK &&
      fn::LobbyMsgRW_PackageBool(lobbyMsg, "isStarterPack", _this + 656) &&
      fn::LobbyMsgRW_PackageString(lobbyMsg, "password", _this + 657, 0x20) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "membercount",
                                reinterpret_cast<int32_t *>(_this + 24)) &&
      fn::LobbyMsgRW_PackageArrayStart(lobbyMsg, "members");

  if (!packageOK) {
    return 1; // bad packet
  }

  if (*reinterpret_cast<int32_t *>(_this + 24) > 18) {
    return 2; // crash attempt via BoF
  }

  if (fn::LobbyMsgRW_PackageElement(
          lobbyMsg, *reinterpret_cast<int32_t *>(_this + 24) > 0)) {
    if (*reinterpret_cast<int32_t *>(_this + 24) <= 0) {
      return 3; // crash attempt via BoF (element despite 0 members)
    }

    for (int i = 0; i < *reinterpret_cast<int32_t *>(_this + 24); i++) {
      packageOK =
          packageOK &&
          fn::LobbyMsgRW_PackageXuid(
              lobbyMsg, "xuid", reinterpret_cast<uint64_t *>(_this + 616)) &&
          fn::LobbyMsgRW_PackageUInt64(
              lobbyMsg, "lobbyid", reinterpret_cast<uint64_t *>(_this + 616)) &&
          fn::LobbyMsgRW_PackageFloat(lobbyMsg, "skillrating",
                                      reinterpret_cast<float *>(_this + 616)) &&
          fn::LobbyMsgRW_PackageFloat(lobbyMsg, "skillvariance",
                                      reinterpret_cast<float *>(_this + 616)) &&
          fn::LobbyMsgRW_PackageUInt(
              lobbyMsg, "pprobation",
              reinterpret_cast<uint32_t *>(_this + 616)) &&
          fn::LobbyMsgRW_PackageUInt(lobbyMsg, "aprobation",
                                     reinterpret_cast<uint32_t *>(_this + 616));

      if (!packageOK) {
        return 4; // bad packet
      }

      bool expected = (i + 1) < *reinterpret_cast<int32_t *>(_this + 24);
      bool result = fn::LobbyMsgRW_PackageElement(lobbyMsg, expected);
      if (result && !expected) {
        return 5; // BoF attempt via package element overflow
      }
    }
  }

  return 0; // packet is safe
}

// =====================================================================
// MSG_LobbyState_Package_Inspect
// Validates LobbyState (0x2) lobby messages.
// Returns 0 if packet is safe, non-zero if malicious.
// =====================================================================
inline int MSG_LobbyState_Package_Inspect(char *__this, char *lobbyMsg) {
  int32_t *_this = reinterpret_cast<int32_t *>(__this);

  bool packageOK =
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "statenum", _this) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "networkmode", _this + 1) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "mainmode", _this + 2) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "partyprivacy", _this + 3) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "lobbytype", _this + 4) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "lobbymode", _this + 5) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "sessionstatus", _this + 6) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "uiscreen", _this + 7) &&
      fn::LobbyMsgRW_PackageUChar(lobbyMsg, "leaderactivity",
                                  reinterpret_cast<char *>(_this) + 32) &&
      fn::LobbyMsgRW_PackageString(lobbyMsg, "key",
                                   reinterpret_cast<char *>(_this + 9), 0x20) &&
      fn::LobbyMsgRW_PackageXuid(lobbyMsg, "leader",
                                 reinterpret_cast<uint64_t *>(_this + 9)) &&
      fn::LobbyMsgRW_PackageXuid(lobbyMsg, "platformsession",
                                 reinterpret_cast<uint64_t *>(_this + 10)) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "maxclients", _this + 22) &&
      fn::LobbyMsgRW_PackageBool(lobbyMsg, "isadvertised",
                                 reinterpret_cast<char *>(_this) + 92) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "clientcount", _this + 24) &&
      fn::LobbyMsgRW_PackageString(lobbyMsg, "sessionid",
                                   reinterpret_cast<char *>(_this + 5320),
                                   0x40) &&
      fn::LobbyMsgRW_PackageString(lobbyMsg, "sessioninfo",
                                   reinterpret_cast<char *>(_this + 5336),
                                   0x40) &&
      fn::LobbyMsgRW_PackageString(
          lobbyMsg, "ugcName", reinterpret_cast<char *>(_this + 5352), 0x20) &&
      fn::LobbyMsgRW_PackageUInt(lobbyMsg, "ugcVersion",
                                 reinterpret_cast<uint32_t *>(_this + 5360));

  if (!packageOK) {
    return 1; // bad packet
  }

  fn::LobbyMsgRW_PackageArrayStart(lobbyMsg, "clientlist");

  if (_this[24] > 18) {
    return 2; // crash attempt via BoF
  }

  int index = 0;
  bool hasNextElement =
      fn::LobbyMsgRW_PackageElement(lobbyMsg, index < _this[24]);

  while (hasNextElement && index < _this[24]) {
    int32_t offset = 292 * index;
    int64_t offset2 = reinterpret_cast<int64_t>(&_this[offset + 26]);

    packageOK =
        packageOK &&
        fn::LobbyMsgRW_PackageXuid(lobbyMsg, "xuid",
                                   reinterpret_cast<uint64_t *>(offset2)) &&
        fn::LobbyMsgRW_PackageUChar(lobbyMsg, "clientNum",
                                    reinterpret_cast<char *>(offset2 + 8)) &&
        fn::LobbyMsgRW_PackageString(lobbyMsg, "gamertag",
                                     reinterpret_cast<char *>(offset2 + 9),
                                     0x20) &&
        fn::LobbyMsgRW_PackageBool(lobbyMsg, "isGuest",
                                   reinterpret_cast<char *>(offset2 + 41)) &&
        fn::LobbyMsgRW_PackageUInt64(
            lobbyMsg, "lobbyid", reinterpret_cast<uint64_t *>(offset2 + 48)) &&
        fn::LobbyMsgRW_PackageInt(
            lobbyMsg, "connectbit",
            reinterpret_cast<int32_t *>(offset2 + 1108)) &&
        fn::LobbyMsgRW_PackageInt(
            lobbyMsg, "score", reinterpret_cast<int32_t *>(offset2 + 1104)) &&
        fn::LobbyMsgRW_PackageGlob(lobbyMsg, "address",
                                   reinterpret_cast<char *>(offset2 + 1113),
                                   37) &&
        fn::LobbyMsgRW_PackageInt(lobbyMsg, "qport",
                                  reinterpret_cast<int32_t *>(offset2 + 56)) &&
        fn::LobbyMsgRW_PackageUChar(lobbyMsg, "band",
                                    reinterpret_cast<char *>(offset2 + 60)) &&
        fn::LobbyMsgRW_PackageUInt(
            lobbyMsg, "netsrc", reinterpret_cast<uint32_t *>(offset2 + 1152)) &&
        fn::LobbyMsgRW_PackageUInt(
            lobbyMsg, "joinorder",
            reinterpret_cast<uint32_t *>(offset2 + 1156)) &&
        fn::LobbyMsgRW_PackageUInt(
            lobbyMsg, "dlcBits",
            reinterpret_cast<uint32_t *>(offset2 + 1160)) &&
        fn::MsgMutableClientInfo_Package(reinterpret_cast<char *>(offset2 + 64),
                                         lobbyMsg);

    if (!packageOK) {
      return 101; // bad packet in client list
    }

    index++;
    hasNextElement = fn::LobbyMsgRW_PackageElement(lobbyMsg, index < _this[24]);
  }

  if (hasNextElement) {
    return 102; // Crash attempt via BoF and PackElem RO exploit
  }

  packageOK =
      packageOK &&
      fn::LobbyMsgRW_PackageUChar(lobbyMsg, "migratebits",
                                  reinterpret_cast<char *>(_this) + 21128) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "lasthosttimems", _this + 5283);

  if (!packageOK) {
    return 3; // bad packet
  }

  fn::LobbyMsgRW_PackageArrayStart(lobbyMsg, "nomineelist");

  int32_t count = 0;
  bool hasNextElement2 = fn::LobbyMsgRW_PackageElement(lobbyMsg, count < 18);

  while (hasNextElement2 && (count < 18)) {
    packageOK = packageOK &&
                fn::LobbyMsgRW_PackageXuid(
                    lobbyMsg, "xuid",
                    reinterpret_cast<uint64_t *>(&_this[0x14A4 + (2 * count)]));
    count++;
    hasNextElement2 = fn::LobbyMsgRW_PackageElement(lobbyMsg, count < 18);
  }

  if (hasNextElement2) {
    return 103; // crash attempt via PERO nominee
  }

  if (!packageOK) {
    return 4; // bad packet
  }

  return 0;
}

// =====================================================================
// MSG_LobbyStateGame_Package_Inspect
// Validates LobbyStateGame (0x3) lobby messages.
// This calls MSG_LobbyState_Package_Inspect first, then validates
// game-specific fields (gametype, map, votes, etc.)
// Returns 0 if packet is safe, non-zero if malicious.
// =====================================================================
inline int MSG_LobbyStateGame_Package_Inspect(char *__this, char *lobbyMsg) {
  int lspiResult = MSG_LobbyState_Package_Inspect(__this, lobbyMsg);
  if (lspiResult) {
    return lspiResult;
  }

  int32_t *_this = reinterpret_cast<int32_t *>(__this);
  bool packageOK =
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "serverstatus", _this + 5362);

  if (!packageOK) {
    return 11;
  }

  packageOK =
      packageOK &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "launchnonce", _this + 5363) &&
      fn::LobbyMsgRW_PackageUInt64(
          lobbyMsg, "matchhashlow",
          reinterpret_cast<uint64_t *>(_this + 2682)) &&
      fn::LobbyMsgRW_PackageUInt64(
          lobbyMsg, "matchhashhigh",
          reinterpret_cast<uint64_t *>(_this + 2683)) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "status", _this + 5394) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "statusvalue", _this + 5395) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "gamemode", _this + 5368) &&
      fn::LobbyMsgRW_PackageString(
          lobbyMsg, "gametype", reinterpret_cast<char *>(_this + 5369), 0x20) &&
      fn::LobbyMsgRW_PackageString(
          lobbyMsg, "map", reinterpret_cast<char *>(_this + 5377), 0x20) &&
      fn::LobbyMsgRW_PackageString(
          lobbyMsg, "ugcName", reinterpret_cast<char *>(_this + 5385), 0x20) &&
      fn::LobbyMsgRW_PackageUInt(lobbyMsg, "ugcVersion",
                                 reinterpret_cast<uint32_t *>(_this + 5393));

  if (!_this[2]) {
    packageOK =
        packageOK &&
        fn::LobbyMsgRW_PackageString(lobbyMsg, "cpqueuedlevel",
                                     reinterpret_cast<char *>(_this + 7293),
                                     0x20) &&
        fn::LobbyMsgRW_PackageBool(lobbyMsg, "movieskipped",
                                   reinterpret_cast<char *>(_this) + 29204);
  }

  int32_t lobbyMode = _this[5];

  if (!lobbyMode) // LOBBY_MODE_PUBLIC
  {
    fn::LobbyMsgRW_PackageArrayStart(lobbyMsg, "clientlist");
    int index = 0;
    bool hasNextElement = fn::LobbyMsgRW_PackageElement(lobbyMsg, index < 18);
    while (hasNextElement && (index < 18)) {
      int64_t currentClient = reinterpret_cast<int64_t>(&_this[9 * index]);
      packageOK = packageOK &&
                  fn::LobbyMsgRW_PackageInt(
                      lobbyMsg, "team",
                      reinterpret_cast<int32_t *>(currentClient + 28524)) &&
                  fn::LobbyMsgRW_PackageInt(
                      lobbyMsg, "mapvote",
                      reinterpret_cast<int32_t *>(currentClient + 28528)) &&
                  fn::LobbyMsgRW_PackageBool(
                      lobbyMsg, "readyup",
                      reinterpret_cast<char *>(currentClient + 28532));

      if (!packageOK) {
        return 21; // bad pack
      }

      index++;
      hasNextElement = fn::LobbyMsgRW_PackageElement(lobbyMsg, index < 18);
    }

    if (hasNextElement) {
      return 22; // PERO
    }

    packageOK =
        packageOK &&
        fn::LobbyMsgRW_PackageInt(lobbyMsg, "plistid", _this + 7126) &&
        fn::LobbyMsgRW_PackageInt(lobbyMsg, "plistcurr", _this + 7127) &&
        fn::LobbyMsgRW_PackageGlob(lobbyMsg, "plistentries",
                                   reinterpret_cast<char *>(_this) + 28512,
                                   8) &&
        fn::LobbyMsgRW_PackageUChar(lobbyMsg, "plistnext",
                                    reinterpret_cast<char *>(_this) + 28520) &&
        fn::LobbyMsgRW_PackageUChar(lobbyMsg, "plistprev",
                                    reinterpret_cast<char *>(_this) + 28521) &&
        fn::LobbyMsgRW_PackageUChar(lobbyMsg, "plistprevcount",
                                    reinterpret_cast<char *>(_this) + 28522);

    if (!packageOK) {
      return 5; // bad packet
    }

    return 0; // PackageOK
  }

  lobbyMode--;
  if (!lobbyMode) // LOBBY_MODE_CUSTOM
  {
    packageOK = packageOK &&
                fn::LobbyMsgRW_PackageInt(lobbyMsg, "votecount", _this + 5397);
    fn::LobbyMsgRW_PackageArrayStart(lobbyMsg, "votes");

    int32_t numVotes = *reinterpret_cast<int32_t *>(_this + 5397);
    if (packageOK && (numVotes > 216)) {
      return 26; // crash attempt via votecount BoF
    }

    int index = 0;
    bool hasNextElement =
        fn::LobbyMsgRW_PackageElement(lobbyMsg, index < numVotes);

    while (hasNextElement && (index < numVotes)) {
      int64_t currentVote = reinterpret_cast<int64_t>(&_this[8 * index + 5398]);
      packageOK =
          packageOK &&
          fn::LobbyMsgRW_PackageShort(
              lobbyMsg, "itemtype",
              reinterpret_cast<int16_t *>(currentVote + 8)) &&
          fn::LobbyMsgRW_PackageUInt(
              lobbyMsg, "item",
              reinterpret_cast<uint32_t *>(currentVote + 12)) &&
          fn::LobbyMsgRW_PackageShort(
              lobbyMsg, "itemgroup",
              reinterpret_cast<int16_t *>(currentVote + 16)) &&
          fn::LobbyMsgRW_PackageShort(
              lobbyMsg, "attachment",
              reinterpret_cast<int16_t *>(currentVote + 20)) &&
          fn::LobbyMsgRW_PackageShort(
              lobbyMsg, "votetype",
              reinterpret_cast<int16_t *>(currentVote + 24)) &&
          fn::LobbyMsgRW_PackageXuid(lobbyMsg, "votexuid",
                                     reinterpret_cast<uint64_t *>(currentVote));

      if (!packageOK) {
        return 6; // bad packet
      }

      index++;
      hasNextElement =
          fn::LobbyMsgRW_PackageElement(lobbyMsg, index < numVotes);
    }

    if (hasNextElement) {
      return 23; // PERO
    }

    fn::LobbyMsgRW_PackageArrayStart(lobbyMsg, "clientlist");
    index = 0;
    hasNextElement = fn::LobbyMsgRW_PackageElement(lobbyMsg, index < 18);
    while (hasNextElement && (index < 18)) {
      int64_t currentClient =
          reinterpret_cast<int64_t>(&_this[index + 8 * index + 7131]);
      packageOK =
          packageOK &&
          fn::LobbyMsgRW_PackageInt(
              lobbyMsg, "team", reinterpret_cast<int32_t *>(currentClient)) &&
          fn::LobbyMsgRW_PackageInt(
              lobbyMsg, "pregamepos",
              reinterpret_cast<int32_t *>(currentClient + 4)) &&
          fn::LobbyMsgRW_PackageInt(
              lobbyMsg, "pregamestate",
              reinterpret_cast<int32_t *>(currentClient + 3)) &&
          fn::LobbyMsgRW_PackageUInt(
              lobbyMsg, "clvotecount",
              reinterpret_cast<uint32_t *>(currentClient + 6)) &&
          fn::LobbyMsgRW_PackageUInt(
              lobbyMsg, "character",
              reinterpret_cast<uint32_t *>(currentClient + 7)) &&
          fn::LobbyMsgRW_PackageUInt(
              lobbyMsg, "loadout",
              reinterpret_cast<uint32_t *>(currentClient + 8));

      if (!packageOK) {
        return 7; // bad packet
      }

      index++;
      hasNextElement = fn::LobbyMsgRW_PackageElement(lobbyMsg, index < 18);
    }

    if (hasNextElement) {
      return 24; // PERO
    }

    packageOK = packageOK && fn::LobbyMsgRW_PackageInt(lobbyMsg, "settingssize",
                                                       _this + 7350);

    if ((_this[7350] <= 0) || (_this[7350] > 0xC000)) {
      return 25; // BoF attempt
    }

    return 0; // PackageOK
  }

  lobbyMode--;
  if (!lobbyMode) // LOBBY_MODE_THEATER
  {
    return 0; // PackageOK
  }

  if (lobbyMode == 1) // LOBBY_MODE_ARENA
  {
    packageOK =
        packageOK &&
        fn::LobbyMsgRW_PackageInt(lobbyMsg, "compstate", _this + 5396) &&
        fn::LobbyMsgRW_PackageInt(lobbyMsg, "votecount", _this + 5397);

    if (!packageOK) {
      return 10;
    }

    int32_t numVotes = _this[5397];
    if (_this[5397] > 216) {
      return 27; // crash attempt via votecount BoF
    }

    int index = 0;
    bool hasNextElement =
        fn::LobbyMsgRW_PackageElement(lobbyMsg, index < numVotes);

    while (hasNextElement && (index < numVotes)) {
      int64_t currentVote = reinterpret_cast<int64_t>(&_this[8 * index + 5398]);
      packageOK =
          packageOK &&
          fn::LobbyMsgRW_PackageShort(
              lobbyMsg, "itemtype",
              reinterpret_cast<int16_t *>(currentVote + 8)) &&
          fn::LobbyMsgRW_PackageUInt(
              lobbyMsg, "item",
              reinterpret_cast<uint32_t *>(currentVote + 12)) &&
          fn::LobbyMsgRW_PackageShort(
              lobbyMsg, "itemgroup",
              reinterpret_cast<int16_t *>(currentVote + 16)) &&
          fn::LobbyMsgRW_PackageShort(
              lobbyMsg, "attachment",
              reinterpret_cast<int16_t *>(currentVote + 20)) &&
          fn::LobbyMsgRW_PackageShort(
              lobbyMsg, "votetype",
              reinterpret_cast<int16_t *>(currentVote + 24)) &&
          fn::LobbyMsgRW_PackageXuid(lobbyMsg, "votexuid",
                                     reinterpret_cast<uint64_t *>(currentVote));

      if (!packageOK) {
        return 8;
      }

      index++;
      hasNextElement =
          fn::LobbyMsgRW_PackageElement(lobbyMsg, index < numVotes);
    }

    if (hasNextElement) {
      return 28; // PERO
    }

    fn::LobbyMsgRW_PackageArrayStart(lobbyMsg, "clientlist");
    index = 0;
    hasNextElement = fn::LobbyMsgRW_PackageElement(lobbyMsg, index < 18);

    while (hasNextElement && (index < 18)) {
      int64_t currentClient =
          reinterpret_cast<int64_t>(&_this[index + 8 * index + 7131]);
      packageOK =
          packageOK &&
          fn::LobbyMsgRW_PackageInt(
              lobbyMsg, "team", reinterpret_cast<int32_t *>(currentClient)) &&
          fn::LobbyMsgRW_PackageInt(
              lobbyMsg, "pregamepos",
              reinterpret_cast<int32_t *>(currentClient + 4)) &&
          fn::LobbyMsgRW_PackageInt(
              lobbyMsg, "pregamestate",
              reinterpret_cast<int32_t *>(currentClient + 3)) &&
          fn::LobbyMsgRW_PackageUInt(
              lobbyMsg, "clvotecount",
              reinterpret_cast<uint32_t *>(currentClient + 6)) &&
          fn::LobbyMsgRW_PackageUInt(
              lobbyMsg, "character",
              reinterpret_cast<uint32_t *>(currentClient + 7)) &&
          fn::LobbyMsgRW_PackageUInt(
              lobbyMsg, "loadout",
              reinterpret_cast<uint32_t *>(currentClient + 8));

      if (!packageOK) {
        return 9;
      }

      index++;
      hasNextElement = fn::LobbyMsgRW_PackageElement(lobbyMsg, index < 18);
    }

    if (hasNextElement) {
      return 29; // PERO
    }

    return 0; // PackageOK
  }

  return 0; // PackageOK (unknown lobby mode, pass through)
}

// MSG_InfoResponse_Inspect
// Validates InfoResponse (0x1) lobby messages.
inline int MSG_InfoResponse_Inspect(char *__this, char *lobbyMsg) {
  uint32_t nonce = 0;
  int32_t uiScreen = 0;
  char natType = 0;

  bool packIsOK = fn::LobbyMsgRW_PackageUInt(lobbyMsg, "nonce", &nonce) &&
                  fn::LobbyMsgRW_PackageInt(lobbyMsg, "uiscreen", &uiScreen) &&
                  fn::LobbyMsgRW_PackageUChar(lobbyMsg, "nattype", &natType);

  if (!packIsOK)
    return 1;

  fn::LobbyMsgRW_PackageArrayStart(lobbyMsg, "lobbies");

  bool hasNextElement = fn::LobbyMsgRW_PackageElement(lobbyMsg, TRUE);
  for (int32_t cResponse = 0; hasNextElement && (cResponse < 2); cResponse++) {
    fn::LobbyMsgRW_PackageArrayStart(lobbyMsg, "lobbies");

    bool hasNextElement2 = fn::LobbyMsgRW_PackageElement(lobbyMsg, TRUE);
    for (int32_t i = 0; hasNextElement2 && (i < 2); i++) {
      char isValid = 0;
      packIsOK =
          packIsOK && fn::LobbyMsgRW_PackageBool(lobbyMsg, "valid", &isValid);
      if (!packIsOK)
        return 2;

      if (!isValid) {
        hasNextElement2 = fn::LobbyMsgRW_PackageElement(lobbyMsg, (i < 1));
        continue;
      }

      uint64_t hostXuid = 0;
      char hostName[32]{};
      int32_t networkMode = 0, mainMode = 0;
      char secId[8]{}, secKey[16]{}, addrBuf[37]{};
      char ugcName[32]{};
      uint32_t ugcVersion = 0;

      packIsOK =
          packIsOK &&
          fn::LobbyMsgRW_PackageXuid(lobbyMsg, "hostxuid", &hostXuid) &&
          fn::LobbyMsgRW_PackageString(lobbyMsg, "hostname", hostName, 32) &&
          fn::LobbyMsgRW_PackageInt(lobbyMsg, "networkmode", &networkMode) &&
          fn::LobbyMsgRW_PackageInt(lobbyMsg, "mainmode", &mainMode) &&
          fn::LobbyMsgRW_PackageGlob(lobbyMsg, "secid", secId, 8) &&
          fn::LobbyMsgRW_PackageGlob(lobbyMsg, "seckey", secKey, 16) &&
          fn::LobbyMsgRW_PackageGlob(lobbyMsg, "addrbuff", addrBuf, 37) &&
          fn::LobbyMsgRW_PackageString(lobbyMsg, "ugcName", ugcName, 32) &&
          fn::LobbyMsgRW_PackageUInt(lobbyMsg, "ugcVersion", &ugcVersion);

      if (!packIsOK)
        return 3;

      hasNextElement2 = fn::LobbyMsgRW_PackageElement(lobbyMsg, (i < 1));
    }

    if (hasNextElement2)
      return 5;

    hasNextElement = fn::LobbyMsgRW_PackageElement(lobbyMsg, (cResponse < 1));
  }

  if (hasNextElement)
    return 4;

  return 0;
}

// =====================================================================
// MSG_HostHeartbeat_Inspect
// Validates HostHeartbeat (0x7) lobby messages.
// Returns 0 if packet is safe, non-zero if malicious.
// =====================================================================
inline int MSG_HostHeartbeat_Inspect(char *__this, char *lobbyMsg) {
  bool packageOK =
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "heartbeatnum",
                                reinterpret_cast<int32_t *>(__this)) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "lobbytype",
                                reinterpret_cast<int32_t *>(__this)) &&
      fn::LobbyMsgRW_PackageInt(lobbyMsg, "lasthosttimems",
                                reinterpret_cast<int32_t *>(__this));

  if (!packageOK) {
    return 1; // invalid packet
  }

  fn::LobbyMsgRW_PackageArrayStart(lobbyMsg, "nomineelist");

  int32_t count = 0;
  bool hasNextElement = fn::LobbyMsgRW_PackageElement(lobbyMsg, count < 18);
  while (hasNextElement && (count < 18)) {
    packageOK = packageOK &&
                fn::LobbyMsgRW_PackageXuid(
                    lobbyMsg, "xuid", reinterpret_cast<uint64_t *>(__this));
    count++;
    hasNextElement = fn::LobbyMsgRW_PackageElement(lobbyMsg, count < 18);
  }

  if (hasNextElement) {
    return 101; // crash attempt via PERO nominee
  }

  if (!packageOK) {
    return 2; // bad packet
  }

  return 0;
}

// =====================================================================
// Packet callback map and registration
// =====================================================================

using packet_callback_t =
    std::function<void(int32_t *lobbyMsgTypePtr, char *lobbyMsg)>;

inline std::unordered_map<uint8_t, packet_callback_t> &get_packet_callbacks() {
  static std::unordered_map<uint8_t, packet_callback_t> callbacks;
  static bool registered = false;

  if (!registered) {
    registered = true;

    // 0x10 = MESSAGE_TYPE_JOIN_LOBBY (JoinParty)
    callbacks[0x10] = [](int32_t *lobbyMsgTypePtr, char *lobbyMsg) {
      memset(requestOut, 0, sizeof(requestOut));
      memset(lobbyMsgCpy, 0, sizeof(lobbyMsgCpy));
      memcpy_s(lobbyMsgCpy, sizeof(lobbyMsgCpy), lobbyMsg, 0x44);

      auto result = MSG_JoinParty_Package_Inspect(requestOut, lobbyMsgCpy);
      if (result) {
        *lobbyMsgTypePtr = 0xFF; // drop
        printf("[Security] Dropped malicious JoinParty packet (reason: %d)\n",
               result);
      }
    };

    // 0xF = MESSAGE_TYPE_LOBBY_MODIFIED_STATS (always drop)
    callbacks[0xF] = [](int32_t *lobbyMsgTypePtr,
                        [[maybe_unused]] char *lobbyMsg) {
      printf("[Security] Dropped packet type 0x%X (blocked)\n",
             *lobbyMsgTypePtr);
      *lobbyMsgTypePtr = 0xFF;
    };

    // Auto-drop dangerous packet types
    callbacks[0x1E] =
        callbacks[0xF]; // MESSAGE_TYPE_VOICE_PACKET (voice crash exploit)
    callbacks[0x16] = callbacks[0xF]; // MESSAGE_TYPE_SERVERLIST_INFO
    callbacks[0x17] =
        callbacks[0xF]; // MESSAGE_TYPE_PEER_TO_PEER_CONNECTIVITY_TEST
    callbacks[0x18] = callbacks[0xF]; // MESSAGE_TYPE_PEER_TO_PEER_INFO
    callbacks[0x1F] =
        callbacks[0xF]; // MESSAGE_TYPE_VOICE_RELAY_PACKET (relay crash vector)
    callbacks[0x20] =
        callbacks[0xF]; // MESSAGE_TYPE_DEMO_STATE (demo state crash vector)

    // 0x7 = MESSAGE_TYPE_LOBBY_HOST_HEARTBEAT
    callbacks[MESSAGE_TYPE_LOBBY_HOST_HEARTBEAT] = [](int32_t *lobbyMsgTypePtr,
                                                      char *lobbyMsg) {
      memset(requestOut, 0, sizeof(requestOut));
      memset(lobbyMsgCpy, 0, sizeof(lobbyMsgCpy));
      memcpy_s(lobbyMsgCpy, sizeof(lobbyMsgCpy), lobbyMsg, 0x44);

      auto result = MSG_HostHeartbeat_Inspect(requestOut, lobbyMsgCpy);
      if (result) {
        *lobbyMsgTypePtr = 0xFF;
        printf(
            "[Security] Dropped malicious HostHeartbeat packet (reason: %d)\n",
            result);
      }
    };

    // 0x2 = MESSAGE_TYPE_LOBBY_STATE_PRIVATE
    callbacks[0x2] = [](int32_t *lobbyMsgTypePtr, char *lobbyMsg) {
      memset(requestOut, 0, sizeof(requestOut));
      memset(lobbyMsgCpy, 0, sizeof(lobbyMsgCpy));
      memcpy_s(lobbyMsgCpy, sizeof(lobbyMsgCpy), lobbyMsg, 0x44);

      auto result = MSG_LobbyState_Package_Inspect(requestOut, lobbyMsgCpy);
      if (result) {
        *lobbyMsgTypePtr = 0xFF;
        printf("[Security] Dropped malicious LobbyState packet (reason: %d)\n",
               result);
      }
    };

    // 0x3 = MESSAGE_TYPE_LOBBY_STATE_GAME
    callbacks[0x3] = [](int32_t *lobbyMsgTypePtr, char *lobbyMsg) {
      memset(requestOut, 0, sizeof(requestOut));
      memset(lobbyMsgCpy, 0, sizeof(lobbyMsgCpy));
      memcpy_s(lobbyMsgCpy, sizeof(lobbyMsgCpy), lobbyMsg, 0x44);

      auto result = MSG_LobbyStateGame_Package_Inspect(requestOut, lobbyMsgCpy);
      if (result) {
        *lobbyMsgTypePtr = 0xFF;
        printf(
            "[Security] Dropped malicious LobbyStateGame packet (reason: %d)\n",
            result);
      }
    };

    // 0x1 = MESSAGE_TYPE_INFO_RESPONSE
    callbacks[0x1] = [](int32_t *lobbyMsgTypePtr, char *lobbyMsg) {
      memset(requestOut, 0, sizeof(requestOut));
      memset(lobbyMsgCpy, 0, sizeof(lobbyMsgCpy));
      memcpy_s(lobbyMsgCpy, sizeof(lobbyMsgCpy), lobbyMsg, 0x44);

      auto result = MSG_InfoResponse_Inspect(requestOut, lobbyMsgCpy);
      if (result) {
        *lobbyMsgTypePtr = 0xFF;
        printf(
            "[Security] Dropped malicious InfoResponse packet (reason: %d)\n",
            result);
      }
    };

    // 0x4, 0x5, 0x6 share LobbyStateGame structure
    callbacks[0x4] = callbacks[0x3]; // MESSAGE_TYPE_LOBBY_STATE_GAME_PUBLIC
    callbacks[0x5] = callbacks[0x3]; // MESSAGE_TYPE_LOBBY_STATE_GAME_CUSTOM
    callbacks[0x6] = callbacks[0x2]; // MESSAGE_TYPE_LOBBY_STATE_GAME_THEATER
                                     // (uses LobbyState format)

    // 0xD = MESSAGE_TYPE_LOBBY_CLIENT_RELIABLE_DATA
    // Can be abused to send oversized reliable data causing buffer overflows
    callbacks[0xD] = [](int32_t *lobbyMsgTypePtr, char *lobbyMsg) {
      // Validate reliable data has sane structure
      memset(requestOut, 0, sizeof(requestOut));
      memset(lobbyMsgCpy, 0, sizeof(lobbyMsgCpy));
      memcpy_s(lobbyMsgCpy, sizeof(lobbyMsgCpy), lobbyMsg, 0x44);

      auto *lm = reinterpret_cast<LobbyMsg *>(lobbyMsgCpy);
      // Check the underlying msg_t for overflow indicators
      if (lm->msg.overflowed || lm->msg.cursize > 0x4000) {
        *lobbyMsgTypePtr = 0xFF;
        printf("[Security] Dropped oversized reliable data packet (size: %d)\n",
               lm->msg.cursize);
      }
    };
  }

  return callbacks;
}

// =====================================================================
// InspectPacket - main entry point called from network.cpp
//
// This examines the msg_t, extracts the lobby message data, prepares
// it for reading, checks the message type against our callback map,
// and drops any packets that fail validation.
//
// Returns true if the packet should be DROPPED (malicious),
// false if the packet is OK and should be processed normally.
// =====================================================================
inline bool InspectPacket(game::msg_t *msg) {
  // Only run on client builds - offsets are client-only
  if (game::is_server()) {
    return false;
  }

  fn::initialize();

  if (!msg || !msg->data) {
    return false; // nothing to inspect
  }

  // We need enough data to read
  if (msg->cursize <= msg->readcount) {
    return false;
  }

  const uint32_t remaining =
      static_cast<uint32_t>(msg->cursize - msg->readcount);
  if (remaining == 0 || remaining > 0x10000) {
    return false;
  }

  // Read the lobby message data from the msg_t
  char data[0x2000]{};
  const uint32_t readSize = (remaining < sizeof(data))
                                ? remaining
                                : static_cast<uint32_t>(sizeof(data));

  // Make a copy of msg so we don't disturb the original read position
  game::msg_t msgCopy{};
  memcpy(&msgCopy, msg, sizeof(game::msg_t));

  fn::MSG_ReadData(&msgCopy, data, static_cast<int>(readSize));

  if (msgCopy.overflowed) {
    return false; // can't read, let the game handle it
  }

  // Try to parse as a lobby message
  LobbyMsg lobbyMsg{};
  if (!fn::LobbyMsgRW_PrepReadData(&lobbyMsg, data,
                                   static_cast<int>(readSize))) {
    return false; // not a lobby message or failed to parse
  }

  // Check if we have a callback for this message type
  auto &callbacks = get_packet_callbacks();
  const auto it = callbacks.find(static_cast<uint8_t>(lobbyMsg.msgType));
  if (it != callbacks.end()) {
    it->second(&lobbyMsg.msgType, reinterpret_cast<char *>(&lobbyMsg));
    if (lobbyMsg.msgType == 0xFF) {
      return true; // packet was flagged as malicious, DROP it
    }
  }

  return false; // packet is OK
}
} // namespace ezzsec