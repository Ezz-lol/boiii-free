#pragma once

#include <cstdint>

#include <game/structs/core.hpp>
#include <game/structs/lobby/core.hpp>

namespace game {
namespace live {
typedef int32_t StorageSlot;

#pragma pack(push, 1)

struct SteamIDComponent_t // sizeof=0x8
{
  uint32_t m_unAccountID : 32;
  uint32_t m_unAccountInstance : 20;
  uint32_t m_EAccountType : 4;
  int32_t m_EUniverse : 8;
};

union SteamID_t // sizeof=0x8
{
  SteamIDComponent_t m_comp;
  uint64_t m_unAll64Bits;
};

struct CSteamID // sizeof=0x8
{
  SteamID_t m_steamid;
};
ASSERT_SIZE(CSteamID, 8);

union AuthRequestFlags {
  struct {
    uint32_t authBlobRequested : 1;
    uint32_t authBlobReceived : 1;
    uint32_t authBlobAuthenticated : 1;
    /*
      Note: the following three bits correspond exactly to Steam's
      `EUserHasLicenseForAppResult` enum:
      ```
      enum EUserHasLicenseForAppResult
      {
          k_EUserHasLicenseResultHasLicense = 0x0,
          k_EUserHasLicenseResultDoesNotHaveLicense = 0x1,
          k_EUserHasLicenseResultNoAuth = 0x2,
      };
      ```
    */
    uint32_t appLicenseVerificationSuccessful : 1;
    /*
       This flag is verified to exist separately from
       `appLicenseVerificationSuccessful` - `appLicenseVerificationSuccessful`
       does not act as a boolean flag. This is probably so that the engine can
       differentiate between an incomplete app license verification, and a
       failed app license verification.
    */
    uint32_t appLicenseVerificationFailed : 1;
    uint32_t authSessionTerminated : 1;
    uint32_t reserved : 26;
  };
  uint32_t value;
};

enum class AuthConnectionStatus { FAIL = 0x6 };

struct LiveUserAuthData {
  game::XUID liveUserID;
  // Maybe just padding
  time32_t lastPump;
  bool isStarterPack;
  bool liveAuthorized;
  // Very likely just padding
  uint8_t _unknown0E[2];
  AuthRequestFlags authFlags;
  AuthConnectionStatus connectionStatus;
  // Maybe? this is only ever used to pass to a steam callback for XUID
  // (liveUserID) generation
  CSteamID steamID;
};

ASSERT_SIZE(LiveUserAuthData, 32);

ASSERT_OFFSET(LiveUserAuthData, liveUserID, 0);
ASSERT_OFFSET(LiveUserAuthData, isStarterPack, 12);
ASSERT_OFFSET(LiveUserAuthData, liveAuthorized, 13);
ASSERT_OFFSET(LiveUserAuthData, authFlags, 16);
ASSERT_OFFSET(LiveUserAuthData, steamID, 24);

typedef array<LiveUserAuthData, game::lobby::MAX_PLAYERS> LiveUserAuthPool;
#pragma pack(pop)

enum class LiveAllocTypes : int32_t {
  LIVE_ALLOC_DW = 0x0,
  LIVE_ALLOC_TOMCRYPT = 0x1,
  LIVE_ALLOC_STRINGED = 0x2,
  LIVE_ALLOC_JSON = 0x3,
  LIVE_ALLOC_METPLAYER = 0x4,
  LIVE_ALLOC_STORAGE = 0x5,
  LIVE_ALLOC_NP_WEBAPI = 0x6,
  LIVE_ALLOC_NUM_TYPES = 0x7,
};

} // namespace live
} // namespace game