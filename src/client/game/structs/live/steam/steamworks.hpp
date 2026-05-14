#pragma once

#include <cstdint>
namespace game {
namespace live {
namespace steam {
namespace steamworks {

/*
 TODO: can we just use the SteamWorks SDK for the boilerplate steam structs,
 types, and enums?
*/

// Start SteamWorks boilerplate
enum EResult : int32_t {
  k_EResultOK = 0x1,
  k_EResultFail = 0x2,
  k_EResultNoConnection = 0x3,
  k_EResultInvalidPassword = 0x5,
  k_EResultLoggedInElsewhere = 0x6,
  k_EResultInvalidProtocolVer = 0x7,
  k_EResultInvalidParam = 0x8,
  k_EResultFileNotFound = 0x9,
  k_EResultBusy = 0xA,
  k_EResultInvalidState = 0xB,
  k_EResultInvalidName = 0xC,
  k_EResultInvalidEmail = 0xD,
  k_EResultDuplicateName = 0xE,
  k_EResultAccessDenied = 0xF,
  k_EResultTimeout = 0x10,
  k_EResultBanned = 0x11,
  k_EResultAccountNotFound = 0x12,
  k_EResultInvalidSteamID = 0x13,
  k_EResultServiceUnavailable = 0x14,
  k_EResultNotLoggedOn = 0x15,
  k_EResultPending = 0x16,
  k_EResultEncryptionFailure = 0x17,
  k_EResultInsufficientPrivilege = 0x18,
  k_EResultLimitExceeded = 0x19,
  k_EResultRevoked = 0x1A,
  k_EResultExpired = 0x1B,
  k_EResultAlreadyRedeemed = 0x1C,
  k_EResultDuplicateRequest = 0x1D,
  k_EResultAlreadyOwned = 0x1E,
  k_EResultIPNotFound = 0x1F,
  k_EResultPersistFailed = 0x20,
  k_EResultLockingFailed = 0x21,
  k_EResultLogonSessionReplaced = 0x22,
  k_EResultConnectFailed = 0x23,
  k_EResultHandshakeFailed = 0x24,
  k_EResultIOFailure = 0x25,
  k_EResultRemoteDisconnect = 0x26,
  k_EResultShoppingCartNotFound = 0x27,
  k_EResultBlocked = 0x28,
  k_EResultIgnored = 0x29,
  k_EResultNoMatch = 0x2A,
  k_EResultAccountDisabled = 0x2B,
  k_EResultServiceReadOnly = 0x2C,
  k_EResultAccountNotFeatured = 0x2D,
  k_EResultAdministratorOK = 0x2E,
  k_EResultContentVersion = 0x2F,
  k_EResultTryAnotherCM = 0x30,
  k_EResultPasswordRequiredToKickSession = 0x31,
  k_EResultAlreadyLoggedInElsewhere = 0x32,
  k_EResultSuspended = 0x33,
  k_EResultCancelled = 0x34,
  k_EResultDataCorruption = 0x35,
  k_EResultDiskFull = 0x36,
  k_EResultRemoteCallFailed = 0x37,
  k_EResultPasswordUnset = 0x38,
  k_EResultExternalAccountUnlinked = 0x39,
  k_EResultPSNTicketInvalid = 0x3A,
  k_EResultExternalAccountAlreadyLinked = 0x3B,
  k_EResultRemoteFileConflict = 0x3C,
  k_EResultIllegalPassword = 0x3D,
  k_EResultSameAsPreviousValue = 0x3E,
  k_EResultAccountLogonDenied = 0x3F,
  k_EResultCannotUseOldPassword = 0x40,
  k_EResultInvalidLoginAuthCode = 0x41,
  k_EResultAccountLogonDeniedNoMail = 0x42,
  k_EResultHardwareNotCapableOfIPT = 0x43,
  k_EResultIPTInitError = 0x44,
  k_EResultParentalControlRestricted = 0x45,
  k_EResultFacebookQueryError = 0x46,
  k_EResultExpiredLoginAuthCode = 0x47,
  k_EResultIPLoginRestrictionFailed = 0x48,
  k_EResultAccountLockedDown = 0x49,
  k_EResultAccountLogonDeniedVerifiedEmailRequired = 0x4A,
  k_EResultNoMatchingURL = 0x4B,
};

struct CCallbackBase;

struct /*VFT*/ CCallbackBase_vtbl {
  void(__thiscall *Run)(CCallbackBase *_this, void *, bool, uint64_t);
  void(__thiscall *Run_2)(CCallbackBase *_this, void *);
  int32_t(__thiscall *GetCallbackSizeBytes)(CCallbackBase *_this);
};

struct CCallbackBase {
  CCallbackBase_vtbl *__vftable /*VFT*/;
  uint8_t m_nCallbackFlags;
  int32_t m_iCallback;
};

template <typename T, typename A> class CCallResult : CCallbackBase {
  uint64_t m_hAPICall;
  T *m_pObj;
  void(__thiscall *m_Func)(T *_this, A *, bool);
};

struct EncryptedAppTicketResponse_t {
  EResult m_eResult;
};

struct GameOverlayActivated_t {
  uint8_t m_bActive;
};

struct DlcInstalled_t {
  uint32_t m_nAppID;
};

struct SteamIDComponent_t {
  uint32_t m_unAccountID : 32;
  uint32_t m_unAccountInstance : 20;
  uint32_t m_EAccountType : 4;
  int32_t m_EUniverse : 8;
};

union SteamID_t {
  SteamIDComponent_t m_comp;
  uint64_t m_unAll64Bits;
};

struct CSteamID {
  SteamID_t m_steamid;
};

#pragma pack(push, 1)
struct PersonaStateChange_t {
  uint64_t m_ulSteamID;
  int32_t m_nChangeFlags;
  uint8_t _padding[4];
};
#pragma pack(pop)

template <typename T, typename A, uint32_t V> struct CCallback : CCallbackBase {
  T *m_pObj;
  void(__thiscall *m_Func)(T *_this, A *);
};

struct GameRichPresenceJoinRequested_t {
  CSteamID m_steamIDFriend;
  char m_rgchConnect[256];
};

enum ECallbackType {
  k_iSteamUserCallbacks = 100,
  k_iSteamGameServerCallbacks = 200,
  k_iSteamFriendsCallbacks = 300,
  k_iSteamBillingCallbacks = 400,
  k_iSteamMatchmakingCallbacks = 500,
  k_iSteamContentServerCallbacks = 600,
  k_iSteamUtilsCallbacks = 700,
  k_iClientFriendsCallbacks = 800,
  k_iClientUserCallbacks = 900,
  k_iSteamAppsCallbacks = 1000,
  k_iSteamUserStatsCallbacks = 1100,
  k_iSteamNetworkingCallbacks = 1200,
  k_iClientRemoteStorageCallbacks = 1300,
  k_iSteamUserItemsCallbacks = 1400,
  k_iSteamGameServerItemsCallbacks = 1500,
  k_iClientUtilsCallbacks = 1600,
  k_iSteamGameCoordinatorCallbacks = 1700,
  k_iSteamGameServerStatsCallbacks = 1800,
  k_iSteam2AsyncCallbacks = 1900,
  k_iSteamGameStatsCallbacks = 2000,
  k_iClientHTTPCallbacks = 2100,
  k_iClientScreenshotsCallbacks = 2200,
  k_iSteamScreenshotsCallbacks = 2300,
  k_iClientAudioCallbacks = 2400,
  k_iSteamUnifiedMessagesCallbacks = 2500,
  k_iClientUnifiedMessagesCallbacks = 2600,
  k_iClientControllerCallbacks = 2700,
  k_iSteamControllerCallbacks = 2800,
  k_iClientParentalSettingsCallbacks = 2900,
  k_iClientDeviceAuthCallbacks = 3000,
  k_iClientNetworkDeviceManagerCallbacks = 3100,
  k_iClientMusicCallbacks = 3200,
  k_iClientRemoteClientManagerCallbacks = 3300,
  k_iClientUGCCallbacks = 3400,
  k_iSteamStreamClientCallbacks = 3500,
  k_IClientProductBuilderCallbacks = 3600,
  k_iClientShortcutsCallbacks = 3700,
  k_iClientRemoteControlManagerCallbacks = 3800,
  k_iSteamAppListCallbacks = 3900,
  k_iSteamMusicCallbacks = 4000,
  k_iSteamMusicRemoteCallbacks = 4100,
  k_iClientVRCallbacks = 4200,
  k_iClientReservedCallbacks = 4300,
  k_iSteamReservedCallbacks = 4400,
  k_iSteamHTMLSurfaceCallbacks = 4500,
  k_iClientVideoCallbacks = 4600,
  k_iClientInventoryCallbacks = 4700,
};

typedef uint32_t AppId_t;
struct FriendRichPresenceUpdate_t {
  enum { k_iCallback = k_iSteamFriendsCallbacks + 36 };

  CSteamID m_steamIDFriend; // friend who's rich presence has changed
  AppId_t m_nAppID; // the appID of the game (should always be the current game)
};

struct MicroTxnAuthorizationResponse_t {
  enum { k_iCallback = k_iSteamUserCallbacks + 52 };

  uint32_t m_unAppID;    // AppID for this microtransaction
  uint64_t m_ulOrderID;  // OrderID provided for the microtransaction
  uint8_t m_bAuthorized; // if user authorized transaction
};
} // namespace steamworks
} // namespace steam
} // namespace live
} // namespace game