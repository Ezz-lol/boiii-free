#pragma once

#include <cstdint>

#include <game/structs/macros.hpp>
#include <game/structs/steam.hpp>

namespace game {
namespace live {
namespace steam {

// Verified on client. Fields - their types and offsets - and total size.
// Not verified on dedicated server, but also may not exist there.
PACKED(struct LiveSteamClient {
  EResult resultOnRequestEncryptedAppTicket;
  int32_t ticketTime;
  int32_t numFriendsNotifications;
  DEF_PAD(0C, 4);
  game::steam::CCallResult<LiveSteamClient,
                           game::steam::EncryptedAppTicketResponse_t>
      m_SteamCallResultEncryptedAppTicket;
  game::steam::CCallback<LiveSteamClient, game::steam::GameOverlayActivated_t,
                         0>
      m_CallbackOverlayActivated;
  game::steam::CCallback<LiveSteamClient, game::steam::DlcInstalled_t, 0>
      m_CallbackDLCInstalled;
  game::steam::CCallback<LiveSteamClient, game::steam::PersonaStateChange_t, 0>
      m_CallbackSteamFriendsStatusChange;
  game::steam::CCallback<LiveSteamClient,
                         game::steam::GameRichPresenceJoinRequested_t, 0>
      m_CallbackRichPresenceJoinRequested;
  game::steam::CCallback<LiveSteamClient,
                         game::steam::FriendRichPresenceUpdate_t, 0>
      m_CallbackFriendRichPresenceUpdate;
  game::steam::CCallback<LiveSteamClient,
                         game::steam::MicroTxnAuthorizationResponse_t, 0>
      m_CallbackMicroTxnAuthorizationResponse;
});
ASSERT_SIZE(LiveSteamClient, 0xF8);

enum class SteamServerState : uint32_t {
  INIT_FAILED = 0x0,
  DISCONNECTED = 0x1,
  CONNECTING = 0x2,
  CONNECTED = 0x3,
  SECURED = 0x4,
};
IMPL_ENUM_OPERATORS(SteamServerState);

enum class SteamServerRequestType : int32_t {
  INTERNET = 0,
  LAN = 1,
  FRIENDS = 2,
  FAVORITES = 3,
  HISTORY = 4,
  SPECTATOR = 5,
  ZOMBIES = 6,
  COUNT = 7,
};
IMPL_ENUM_OPERATORS(SteamServerRequestType);

enum class SteamServerSortType : int32_t {
  PROTECTED_ASCENDING = 0,
  PROTECTED_DESCENDING = 1,
  DEDICATED_ASCENDING = 2,
  DEDICATED_DESCENDING = 3,
  RANKED_ASCENDING = 4,
  RANKED_DESCENDING = 5,
  NAME_ASCENDING = 6,
  NAME_DESCENDING = 7,
  MAP_ASCENDING = 8,
  MAP_DESCENDING = 9,
  HARDCORE_ASCENDING = 10,
  HARDCORE_DESCENDING = 11,
  GAMETYPE_ASCENDING = 12,
  GAMETYPE_DESCENDING = 13,
  PLAYERS_ASCENDING = 14,
  PLAYERS_DESCENDING = 15,
  PING_ASCENDING = 16,
  PING_DESCENDING = 17,
  COUNT = 18,
};
IMPL_ENUM_OPERATORS(SteamServerSortType);

enum class SteamServerFilterType : int32_t {
  MAP = 0,
  GAMETYPE = 1,
  MOD = 2,
  KEYWORDS = 3,
  COUNT = 4,
};
IMPL_ENUM_OPERATORS(SteamServerFilterType);

enum class SteamServerAttributeFilter : int32_t {
  DEDICATED = 0,
  NOTDEDICATED = 1,
  RANKED = 2,
  UNRANKED = 3,
  HARDCORE = 4,
  CORE = 5,
  COUNT = 6,
};
IMPL_ENUM_OPERATORS(SteamServerAttributeFilter);

// TODO
struct LiveSteamServer;

} // namespace steam
} // namespace live
} // namespace game
