#pragma once

#include <cstdint>
#include "../core.hpp"
#include "../quake/core.hpp"
#include "../dw/dw.hpp"

namespace game {
namespace live {
namespace user {

enum class EUserTier : int32_t {
  NONE = 0x0,
  SUPER = 0x1,
  DEVELOPER = 0x2,
  FRIEND = 0x3,
  DEDICATED = 0x4,
  ENEMY = 0x29A,
};

enum class XUSER_SIGNIN_STATE : int32_t {
  NotSignedIn = 0x0,
  SignedInLocally = 0x1,
  SignedInToLive = 0x2,
};

/*
  These are probably mostly incorrect.

  In the version of the engine this was generated from,
  CONNECTED was 0x4, as opposed to the drastically different value
  of 0xA seen in the latest version of the engine.

  All enumerations other than CONNECTED should be assumed to be incorrect
  unless updated to be otherwise.
*/
enum class CONNECTION_STATE : int32_t {
  CONNECTION_STATE_ERROR = 0x0,
  NOT_CONNECTED = 0x1,
  AUTHORIZING = 0x2,
  CONNECTING = 0x3,
  // Verified
  CONNECTED = 0xA,
  IN_BACKOFF = 0x5,
  DNS_RESOLVED = 0x6,
  DW_CROSS_AUTH_BEGIN = 0x7,
  DISABLE_CONNECT = 0x8,
  ANTICHEAT_DISABLE_CONNECT_NO_AUTO_RECONNECT = 0x9,
  // TODO: what is this now? CONNECTED is verified to be 0xA
  // ABORTED_QUEUE = 0xA,
  QUEUED = 0xB,
};

#pragma pack(push, 1)
struct userDataPlatform_t {
  uint8_t _unknown[0x4C];
};

#pragma pack(pop)

#pragma pack(push, 1)
struct userData_t {
  XUID xuid;
  char gamertag[32];
  bool isActive;
  bool isGuest;
  uint8_t _padding2A[2];
  EUserTier tier;
  XUSER_SIGNIN_STATE signInState;
  CONNECTION_STATE connectionState;
  qboolean onConnectCalled;
  dw::net::bdInetAddr authAddr;
  dw::net::bdInetAddr lsgAddr;
  int errorCount;
  int backoffRecoverTime;
  ControllerIndex_t controller;
  bool isUnderAge;
  bool isContentRestricted;
  bool isChatRestricted;
  char xuidString[48];
  uint8_t _padding183[1];
  userDataPlatform_t platform;
};
static_assert(sizeof(userData_t) == 0x1D0, "sizeof(userData_t) != 0x1D0");
#pragma pack(pop)

typedef userData_t *userDataRef;

struct UserDataForControllerMap {
  LocalClientPool<userDataRef> data;
};

} // namespace user
} // namespace live
} // namespace game