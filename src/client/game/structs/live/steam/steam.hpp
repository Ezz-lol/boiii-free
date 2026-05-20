#pragma once
#include "steamworks.hpp" // IWYU pragma: export

#include <cstdint>
namespace game {
namespace live {
namespace steam {

// Verified on client. Fields - their types and offsets - and total size.
// Not verified on dedicated server, but also may not exist there.
#pragma pack(push, 1)
struct LiveSteamClient {
  steamworks::EResult resultOnRequestEncryptedAppTicket;
  int32_t ticketTime;
  int32_t numFriendsNotifications;
  uint8_t _padding0C[4];
  steamworks::CCallResult<LiveSteamClient,
                          steamworks::EncryptedAppTicketResponse_t>
      m_SteamCallResultEncryptedAppTicket;
  steamworks::CCallback<LiveSteamClient, steamworks::GameOverlayActivated_t, 0>
      m_CallbackOverlayActivated;
  steamworks::CCallback<LiveSteamClient, steamworks::DlcInstalled_t, 0>
      m_CallbackDLCInstalled;
  steamworks::CCallback<LiveSteamClient, steamworks::PersonaStateChange_t, 0>
      m_CallbackSteamFriendsStatusChange;
  steamworks::CCallback<LiveSteamClient,
                        steamworks::GameRichPresenceJoinRequested_t, 0>
      m_CallbackRichPresenceJoinRequested;
  steamworks::CCallback<LiveSteamClient, steamworks::FriendRichPresenceUpdate_t,
                        0>
      m_CallbackFriendRichPresenceUpdate;
  steamworks::CCallback<LiveSteamClient,
                        steamworks::MicroTxnAuthorizationResponse_t, 0>
      m_CallbackMicroTxnAuthorizationResponse;
};
static_assert(sizeof(LiveSteamClient) == 0xF8,
              "sizeof(LiveSteamClient) must equal 0xF8");
#pragma pack(pop)
} // namespace steam
} // namespace live
} // namespace game