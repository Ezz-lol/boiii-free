#pragma once

#include <cstdint>
#include "../core.hpp"
#include "../lobby.hpp"

namespace game {
namespace live {
typedef int32_t StorageSlot;

#pragma pack(push, 1)

enum class LiveAuthFlags_t : uint32_t {
  AUTH_BLOB_REQUESTED = 0x1,
  AUTH_BLOB_RECEIVED = 0x2,
  AUTH_BLOB_SENT_TO_STEAM = 0x4,
  AUTH_APPROVE = 0x8,
  AUTH_DENY = 0x10,
};

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
static_assert(sizeof(CSteamID) == 8, "CSteamID size must be 8 bytes");

union LiveAuthFlags {
  struct {
    uint32_t authBlobRequested : 1;
    uint32_t authBlobReceived : 1;
    uint32_t authBlobSentToSteam : 1;
    uint32_t authApprove : 1;
    uint32_t authDeny : 1;
  };
  uint32_t value;
};
struct LiveUserAuthData {
  game::XUID liveUserID;
  // Maybe just padding
  uint8_t _unknown08[4];
  bool isStarterPack;
  bool liveAuthorized;
  // Very likely just padding
  uint8_t _unknown0E[2];
  LiveAuthFlags liveAuthFlags;
  // Also maybe just padding
  uint8_t _unknown14[4];
  // Maybe? this is only ever used to pass to a steam callback for XUID
  // (liveUserID) generation
  CSteamID steamID;
};

static_assert(sizeof(LiveUserAuthData) == 32,
              "LiveUserAuthData has wrong size");

static_assert(offsetof(LiveUserAuthData, liveUserID) == 0,
              "liveUserID offset is wrong");
static_assert(offsetof(LiveUserAuthData, isStarterPack) == 12,
              "isStarterPack offset is wrong");
static_assert(offsetof(LiveUserAuthData, liveAuthorized) == 13,
              "liveAuthorized offset is wrong");
static_assert(offsetof(LiveUserAuthData, liveAuthFlags) == 16,
              "liveAuthFlags offset is wrong");
static_assert(offsetof(LiveUserAuthData, steamID) == 24,
              "steamID offset is wrong");

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