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

enum class CONNECTION_STATE : int32_t {
  CONNECTION_STATE_ERROR = 0x0,
  CONNECTION_STATE_NOT_CONNECTED = 0x1,
  CONNECTION_STATE_AUTHORIZING = 0x2,
  CONNECTION_STATE_CONNECTING = 0x3,
  // Verified
  CONNECTION_STATE_CONNECTED = 0xA,
  CONNECTION_STATE_IN_BACKOFF = 0x5,
  CONNECTION_STATE_DNS_RESOLVED = 0x6,
  CONNECTION_STATE_DW_CROSS_AUTH_BEGIN = 0x7,
  CONNECTION_STATE_DISABLE_CONNECT = 0x8,
  CONNECTION_STATE_ANTICHEAT_DISABLE_CONNECT_NO_AUTO_RECONNECT = 0x9,
  // TODO: what is this now? CONNECTED is verified to be 0xA
  // ABORTED_QUEUE = 0xA,
  CONNECTION_STATE_QUEUED = 0xB,
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