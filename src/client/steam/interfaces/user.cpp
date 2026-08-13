#include <std_include.hpp>

#include "../steam.hpp"
#include "../../component/auth.hpp"

namespace steam {
namespace {
std::string auth_ticket;

steam_id generate_steam_id() { return {.bits = auth::get_guid()}; }
} // namespace

int user::GetHSteamUser() { return 1; }

bool user::LoggedOn() { return true; }

steam_id user::GetSteamID() {
  static const steam_id id = generate_steam_id();
  return id;
}

int user::InitiateGameConnection(void *pAuthBlob, int32_t cbMaxAuthBlob,
                                 steam_id steamIDGameServer,
                                 uint32_t unIPServer, uint16_t usPortServer,
                                 bool bSecure) {
  return 0;
}

void user::TerminateGameConnection(uint32_t unIPServer, uint16_t usPortServer) {
}

void user::TrackAppUsageEvent(steam_id gameID, int32_t eAppUsageEvent,
                              const char *pchExtraInfo) {}

bool user::GetUserDataFolder(char *pchBuffer, int32_t cubBuffer) {
  return false;
}

void user::StartVoiceRecording() {}

void user::StopVoiceRecording() {}

int user::GetAvailableVoice(uint32_t *pcbCompressed, uint32_t *pcbUncompressed,
                            uint32_t nUncompressedVoiceDesiredSampleRate) {
  return 0;
}

int user::GetVoice(bool bWantCompressed, void *pDestBuffer,
                   uint32_t cbDestBufferSize, uint32_t *nBytesWritten,
                   bool bWantUncompressed, void *pUncompressedDestBuffer,
                   uint32_t cbUncompressedDestBufferSize,
                   uint32_t *nUncompressBytesWritten,
                   uint32_t nUncompressedVoiceDesiredSampleRate) {
  return 2;
}

int user::DecompressVoice(void *pCompressed, uint32_t cbCompressed,
                          void *pDestBuffer, uint32_t cbDestBufferSize,
                          uint32_t *nBytesWritten) {
  return 0;
}

uint32_t user::GetVoiceOptimalSampleRate() { return 0; }

uint32_t user::GetAuthSessionTicket(void *pTicket, int32_t cbMaxTicket,
                                    uint32_t *pcbTicket) {
  static uint32_t ticket = 0;
  *pcbTicket = 1;

  const uint64_t result = callbacks::register_call();
  get_auth_session_ticket_response *response =
      static_cast<get_auth_session_ticket_response *>(
          calloc(1, sizeof(get_auth_session_ticket_response)));
  response->m_h_auth_ticket = ++ticket;
  response->m_e_result = 1; // k_EResultOK;

  callbacks::return_call(response, sizeof(get_auth_session_ticket_response),
                         get_auth_session_ticket_response::callback_id, result);
  return response->m_h_auth_ticket;
}

int32_t user::BeginAuthSession(const void *pAuthTicket, int32_t cbAuthTicket,
                               steam_id steamID) {
  return game::steam::k_EBeginAuthSessionResultOK;
}

void user::EndAuthSession(steam_id steamID) {}

void user::CancelAuthTicket(uint32_t hAuthTicket) {}

uint32_t user::UserHasLicenseForApp(steam_id steamID, uint32_t appID) {
  return game::steam::EUserHasLicenseForAppResult::
      k_EUserHasLicenseResultHasLicense;
}

bool user::BIsBehindNAT() { return false; }

void user::AdvertiseGame(steam_id steamIDGameServer, uint32_t unIPServer,
                         uint16_t usPortServer) {}

uint64_t user::RequestEncryptedAppTicket(void *pUserData, int32_t cbUserData) {
  const steam_id id = this->GetSteamID();

  auth_ticket = "BOIII";
  auth_ticket.resize(32);
  auth_ticket.append(static_cast<char *>(pUserData), 24); // key
  auth_ticket.append(reinterpret_cast<const char *>(&id.bits),
                     sizeof(id.bits));                         // user id
  auth_ticket.append(&static_cast<char *>(pUserData)[24], 64); // user name

  // Create the call response
  const uint64_t result = callbacks::register_call();
  encrypted_app_ticket_response *retvals =
      static_cast<encrypted_app_ticket_response *>(
          calloc(1, sizeof(encrypted_app_ticket_response)));
  //::Utils::Memory::AllocateArray<EncryptedAppTicketResponse>();
  retvals->m_e_result = 1;

  // Return the call response
  callbacks::return_call(retvals, sizeof(encrypted_app_ticket_response),
                         encrypted_app_ticket_response::callback_id, result);

  return result;
}

bool user::GetEncryptedAppTicket(void *pTicket, int32_t cbMaxTicket,
                                 uint32_t *pcbTicket) {
  if (cbMaxTicket < 0 || auth_ticket.empty())
    return false;

  const size_t size = auth_ticket.size();
  if (static_cast<size_t>(cbMaxTicket) < size) {
    *pcbTicket = static_cast<unsigned>(size);
    return false;
  }

  std::memcpy(pTicket, auth_ticket.data(), size);
  *pcbTicket = static_cast<unsigned>(size);

  return true;
}

int user::GetGameBadgeLevel(int nSeries, bool bFoil) { return 1; }

int user::GetPlayerSteamLevel() { return 0; }

uint64_t user::RequestStoreAuthURL(const char *pchRedirectURL) { return 0; }

bool user::BIsPhoneVerified() { return true; }

bool user::BIsTwoFactorEnabled() { return true; }

bool user::BIsPhoneIdentifying() { return false; }

bool user::BIsPhoneRequiringVerification() { return false; }
} // namespace steam