#pragma once

#include <cstdint>
namespace steam {
struct encrypted_app_ticket_response final {
  enum { callback_id = 154 };

  int32_t m_e_result;
};

struct get_auth_session_ticket_response {
  enum { callback_id = 163 };

  uint32_t m_h_auth_ticket;
  int32_t m_e_result;
};

class user {
public:
  ~user() = default;

  virtual int32_t GetHSteamUser();
  virtual bool LoggedOn();
  virtual steam_id GetSteamID();

  virtual int32_t InitiateGameConnection(void *pAuthBlob, int32_t cbMaxAuthBlob,
                                         steam_id steamIDGameServer,
                                         uint32_t unIPServer,
                                         uint16_t usPortServer, bool bSecure);
  virtual void TerminateGameConnection(uint32_t unIPServer,
                                       uint16_t usPortServer);
  virtual void TrackAppUsageEvent(steam_id gameID, int32_t eAppUsageEvent,
                                  const char *pchExtraInfo = "");
  virtual bool GetUserDataFolder(char *pchBuffer, int32_t cubBuffer);
  virtual void StartVoiceRecording();
  virtual void StopVoiceRecording();
  virtual int GetAvailableVoice(uint32_t *pcbCompressed,
                                uint32_t *pcbUncompressed,
                                uint32_t nUncompressedVoiceDesiredSampleRate);
  virtual int32_t GetVoice(bool bWantCompressed, void *pDestBuffer,
                           uint32_t cbDestBufferSize, uint32_t *nBytesWritten,
                           bool bWantUncompressed,
                           void *pUncompressedDestBuffer,
                           uint32_t cbUncompressedDestBufferSize,
                           uint32_t *nUncompressBytesWritten,
                           uint32_t nUncompressedVoiceDesiredSampleRate);
  virtual int32_t DecompressVoice(void *pCompressed, uint32_t cbCompressed,
                                  void *pDestBuffer, uint32_t cbDestBufferSize,
                                  uint32_t *nBytesWritten);
  virtual uint32_t GetVoiceOptimalSampleRate();
  virtual uint32_t GetAuthSessionTicket(void *pTicket, int32_t cbMaxTicket,
                                        uint32_t *pcbTicket);
  virtual int32_t BeginAuthSession(const void *pAuthTicket,
                                   int32_t cbAuthTicket, steam_id steamID);
  virtual void EndAuthSession(steam_id steamID);
  virtual void CancelAuthTicket(uint32_t hAuthTicket);
  virtual uint32_t UserHasLicenseForApp(steam_id steamID, uint32_t appID);
  virtual bool BIsBehindNAT();
  virtual void AdvertiseGame(steam_id steamIDGameServer, uint32_t unIPServer,
                             uint16_t usPortServer);
  virtual uint64_t RequestEncryptedAppTicket(void *pUserData,
                                             int32_t cbUserData);
  virtual bool GetEncryptedAppTicket(void *pTicket, int32_t cbMaxTicket,
                                     uint32_t *pcbTicket);

  virtual int32_t GetGameBadgeLevel(int nSeries, bool bFoil);
  virtual int32_t GetPlayerSteamLevel();
  virtual uint64_t RequestStoreAuthURL(const char *pchRedirectURL);
  virtual bool BIsPhoneVerified();
  virtual bool BIsTwoFactorEnabled();
  virtual bool BIsPhoneIdentifying();
  virtual bool BIsPhoneRequiringVerification();
};
} // namespace steam