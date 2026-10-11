#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace voice {

// Named `LobbyVoice_UnMuteClient` in engine - override
WEAK symbol<void(ControllerIndex_t controllerIndex, LobbyType lobbyType,
                 ClientNum_t clientNum)>
    LobbyVoice_UnMuteClient_ByClientNum{0x0141EE9040, 0x141EF59B0, 0x1404A37A0};
// Named `LobbyVoice_MuteClient` in engine - override
WEAK symbol<void(ControllerIndex_t controllerIndex, LobbyType lobbyType,
                 ClientNum_t clientNum)>
    LobbyVoice_MuteClient_ByClientNum{0x141EE5A40, 0x141EF23B0, 0x1404A32E0};
WEAK symbol<void(LobbyType lobbyType, XUID xuid)>
    LobbyVoice_VoiceChatNotAllowedForClient{0x141EE9340, 0x141EF5CB0,
                                            0x1404A3910};
WEAK symbol<void(LobbyType lobbyType, XUID xuid)>
    LobbyVoice_VoiceChatAllowedForClient{0x141EE9310, 0x141EF5C80, 0x1404A38E0};

// Named `LobbyVoice_UnMuteClient` in engine - override
WEAK symbol<void(ControllerIndex_t controllerIndex, LobbyType lobbyType,
                 XUID xuid)>
    LobbyVoice_UnMuteClient_ByXUID{0x141EE90E0, 0x141EF5A50, 0x1404A3860};
// Named `LobbyVoice_MuteClient` in engine - override
WEAK symbol<void(ControllerIndex_t controllerIndex, LobbyType lobbyType,
                 XUID xuid)>
    LobbyVoice_MuteClient_ByXUID{0x141EE5B40, 0x141EF24B0, 0x1404A33F0};

WEAK symbol<bool(LocalClientNum_t localClientNum, ClientNum_t talker)>
    LobbyVoice_ShouldIgnorePlayer{0x141EE5C50, 0x141EF25C0, 0x1404A3500};
WEAK symbol<bool(ControllerIndex_t controllerIndex,
                 session::LobbySession *lobbySession, XUID talkerXuid,
                 cl::voice::voiceCommunication_t *vc)>
    BuildAndSendVoicePacket{0x141EE4A20, 0x0, 0x1404A2340};
WEAK symbol<bool(ControllerIndex_t controllerIndex,
                 session::LobbySession *lobbySession, int routBits,
                 ClientNum_t peerIndex, Msg_VoicePacket *voicePacket)>
    SendVoicePacket{0x141EE9370, 0x0, 0x1404A3940};
WEAK symbol<bool(ControllerIndex_t controllerIndex, LobbyType lobbyType)>
    CanProcessVoiceData{0x141EE4CB0, 0x0, 0x1404A2580};
WEAK symbol<void(ControllerIndex_t controllerIndex, net::netadr_t *adr,
                 XUID xuid, LobbyMsg *msg)>
    HandleVoicePacket{0x141ED3410, 0x0, 0x140491620};
WEAK symbol<void(
    ControllerIndex_t controllerIndex, session::LobbySession *lobbySession,
    session::SessionClient *outClient, XUID ourXuid, int *routeList)>
    LobbyVoice_BuildHostRouteList{0x141EE4CF0, 0x0, 0x1404A25E0};
WEAK symbol<void(
    ControllerIndex_t controllerIndex, session::LobbySession *lobbySession,
    session::SessionClient *outClient, XUID ourXuid, int *routeList)>
    LobbyVoice_BuildRouteList{0x141EE4DF0, 0x0, 0x1404A26E0};
WEAK symbol<void(ControllerIndex_t controllerIndex)> LobbyVoice_ClearMuteList{
    0x141EE52C0, 0x0, 0x1404A2BC0};
WEAK symbol<ClientNum_t(session::LobbySession *lobbySession, XUID ourXuid,
                        int *routeList, session::SessionClient *peer)>
    LobbyVoice_FindGoodRouteForClientNum{0x141EE5300, 0x0, 0x1404A2C00};
WEAK symbol<bool(ControllerIndex_t controllerIndex,
                 session::LobbySession *lobbySession, Msg_VoicePacket *msgData)>
    LobbyVoice_HandleVoicePacket{0x141EE54B0, 0x0, 0x1404A2DC0};
WEAK symbol<bool(ControllerIndex_t controllerIndex, LobbyType lobbyType,
                 ClientNum_t clientNum)>
    LobbyVoice_IsClientMuted{0x141EE5720, 0x0, 0x1404A3050};
WEAK symbol<bool(ControllerIndex_t controllerIndex, LobbyType lobbyType,
                 ClientNum_t clientNum)>
    LobbyVoice_IsClientNumConnected{0x141EE5870, 0x0, 0x0};
WEAK symbol<bool(ControllerIndex_t controllerIndex, LobbyType lobbyType,
                 ClientNum_t clientNum)>
    LobbyVoice_IsClientNumTalking{0x141EE58E0, 0x0, 0x1404A31B0};
WEAK symbol<bool(ControllerIndex_t controllerIndex, LobbyID myLobbyId,
                 LobbyID thereLobbyId)>
    LobbyVoice_IsMuteAllButParty_And_NotInParty{0x141EE59E0, 0x0, 0x1404A3280};
WEAK symbol<void(ControllerIndex_t controllerIndex, LobbyType lobbyType,
                 XUID xuid, bool muteOn)>
    LobbyVoice_MuteNonPartyClients{0x141EE5B80, 0x0, 0x1404A3430};
WEAK symbol<bool(
    ControllerIndex_t controllerIndex, session::LobbySession *lobbySession,
    session::SessionClient *peer, ClientNum_t player, bool validAddressCheck)>
    LobbyVoice_ShouldSendPeerVoiceData{0x141EE7510, 0x0, 0x1404A3580};
WEAK symbol<void(ControllerIndex_t controllerIndex, LobbyType lobbyType,
                 ClientNum_t clientNum)>
    LobbyVoice_ToggleMute{0x141EE8ED0, 0x0, 0x0};
WEAK symbol<void(ControllerIndex_t controllerIndex,
                 cl::voice::voiceCommunication_t *vc)>
    LobbyVoice_TransmitQueuedVoiceData{0x141EE8FC0, 0x0, 0x0};
WEAK symbol<void(ControllerIndex_t controllerIndex, LobbyType lobbyType)>
    LobbyVoice_Update{0x141EE9110, 0x0, 0x0};
WEAK symbol<void(LobbyType lobbyType)> LobbyVoice_VoiceChatAllowedForAll{
    0x141EE92C0, 0x0, 0x0};
WEAK symbol<array<uint8_t, 1222>> tempVoicePacketBuf{0x145778CD0, 0x0,
                                                     0x142CEE840};
WEAK symbol<bool(ControllerIndex_t controllerIndex,
                 session::LobbySession *lobbySession,
                 cl::voice::voiceCommunication_t *vc)>
    TransmitQueuedVoiceData{0x141EE9410, 0x0, 0x1404A3A10};
WEAK symbol<int(const ControllerIndex_t controllerIndex,
                SharedVoiceSessionData *sharedData,
                cl::voice::ClientVoicePacket_t *voiceData)>
    Voice_GetLocalVoiceData{0x1422BADE0, 0x0, 0x0};

WEAK symbol<bool(ControllerIndex_t localControllerIndex, LobbyType lobbyType,
                 XUID xuid)>
    Voice_IsXuidTalking{0x141EE5960, 0x0, 0x0};

WEAK symbol<SharedVoiceSessionData *(LobbyType lobbyType)>
    LobbyClientSession_GetSharedVoiceData{0x141EBF6A0, 0x0, 0x0};

inline void LobbyVoice_UnMuteClient(ControllerIndex_t controllerIndex,
                                    LobbyType lobbyType,
                                    ClientNum_t clientNum) {
  LobbyVoice_UnMuteClient_ByClientNum(controllerIndex, lobbyType, clientNum);
}

inline void LobbyVoice_MuteClient(ControllerIndex_t controllerIndex,
                                  LobbyType lobbyType, ClientNum_t clientNum) {
  LobbyVoice_MuteClient_ByClientNum(controllerIndex, lobbyType, clientNum);
}

inline void LobbyVoice_UnMuteClient(ControllerIndex_t controllerIndex,
                                    LobbyType lobbyType, XUID xuid) {
  LobbyVoice_UnMuteClient_ByXUID(controllerIndex, lobbyType, xuid);
}

inline void LobbyVoice_MuteClient(ControllerIndex_t controllerIndex,
                                  LobbyType lobbyType, XUID xuid) {
  LobbyVoice_MuteClient_ByXUID(controllerIndex, lobbyType, xuid);
}

typedef fastcallPtr_t<void(ControllerIndex_t controllerIndex,
                           LobbyType lobbyType, ClientNum_t clientNum)>
    LobbyVoice_ClientNum_Func;

template <const LobbyVoice_ClientNum_Func Func>
inline void LobbyVoice_ForEachLobbyClientNum() {
  for (LobbyType lobbyType = LobbyType::FIRST; lobbyType < LobbyType::LAST;
       ++lobbyType) {
    for (ControllerIndex_t controllerIndex = CONTROLLER_INDEX_FIRST;
         controllerIndex < CONTROLLER_INDEX_COUNT; ++controllerIndex) {
      for (ClientNum_t clientNum = CLIENT_INDEX_FIRST;
           clientNum < CLIENT_INDEX_COUNT; ++clientNum) {
        Func(controllerIndex, lobbyType, clientNum);
      }
    }
  }
}

inline void UnMuteAllClients() {
  return LobbyVoice_ForEachLobbyClientNum<LobbyVoice_UnMuteClient>();
}

inline void MuteAllClients() {
  return LobbyVoice_ForEachLobbyClientNum<LobbyVoice_MuteClient>();
}
} // namespace voice
} // namespace lobby
} // namespace game
