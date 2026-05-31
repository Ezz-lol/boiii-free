#pragma once

#include <cstdint>

#include "../../steam.hpp"
namespace game {
namespace live {
namespace steam {

// Verified on client. Fields - their types and offsets - and total size.
// Not verified on dedicated server, but also may not exist there.
#pragma pack(push, 1)
struct LiveSteamClient {
  game::steam::EResult resultOnRequestEncryptedAppTicket;
  int32_t ticketTime;
  int32_t numFriendsNotifications;
  uint8_t _padding0C[4];
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
};
ASSERT_SIZE(LiveSteamClient, 0xF8);
#pragma pack(pop)
} // namespace steam
} // namespace live
} // namespace game