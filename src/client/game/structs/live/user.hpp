#pragma once

#include <cstdint>
#include "../core.hpp"
#include "../quake/core.hpp"
#include "../dw/dw.hpp"

namespace game {
namespace live {
namespace user {

enum class EUserTier : int32_t {
  USER_TIER_NONE = 0x0,
  USER_TIER_SUPER = 0x1,
  USER_TIER_DEVELOPER = 0x2,
  USER_TIER_FRIEND = 0x3,
  USER_TIER_DEDICATED = 0x4,
  USER_TIER_ENEMY = 0x29A,
};

enum class XUSER_SIGNIN_STATE : int32_t {
  eXUserSigninState_NotSignedIn = 0x0,
  eXUserSigninState_SignedInLocally = 0x1,
  eXUserSigninState_SignedInToLive = 0x2,
};

enum class CONNECTION_STATE : int32_t {
  CONNECTION_STATUS_ERROR = 0x0,
  CONNECTION_STATUS_NOT_CONNECTED = 0x1,
  CONNECTION_STATUS_AUTHORIZING = 0x2,
  CONNECTION_STATUS_CONNECTING = 0x3,
  CONNECTION_STATUS_CONNECTED = 0x4,
  CONNECTION_STATUS_IN_BACKOFF = 0x5,
  CONNECTION_STATUS_DNS_RESOLVED = 0x6,
  CONNECTION_STATUS_DW_CROSS_AUTH_BEGIN = 0x7,
  CONNECTION_STATUS_DISABLE_CONNECT = 0x8,
  CONNECTION_STATUS_ANTICHEAT_DISABLE_CONNECT_NO_AUTO_RECONNECT = 0x9,
  CONNECTION_STATUS_ABORTED_QUEUE = 0xA,
  CONNECTION_STATUS_QUEUED = 0xB,
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
  userDataRef data[2];
};

} // namespace user
} // namespace live
} // namespace game