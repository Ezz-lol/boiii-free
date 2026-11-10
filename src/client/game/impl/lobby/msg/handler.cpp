#include <std_include.hpp>

#include "game/impl/lobby/data.hpp"
#include "game/impl/lobby/msg/host.hpp"
#include "game/impl/lobby/msg/msg.hpp"
#include "game/impl/lobby/reservation.hpp"
#include "game/impl/lobby/serverlist.hpp"
#include "game/impl/lobby/spinner.hpp"
#include "utils/string.hpp"

#include "game/utils.hpp"

#include "handler.hpp"

namespace game {
namespace lobby {
namespace msg {
namespace {
// Matches the lobby component's patch to the engine's join handler, which this
// handler replaces
LobbyNetworkMode get_join_network_mode() {
  return game::is_client() ? LobbyNetworkMode::LIVE
                           : base::LobbyBase_GetNetworkMode();
}
} // namespace

// Host <- Client
void HandleAgreementResponse(ControllerIndex_t controllerIndex,
                             net::netadr_t *fromAdr, XUID fromXUID,
                             LobbyMsg *lobbyMsg) {

  Msg_JoinAgreementResponse msgData{};
  if (msgData.package(*lobbyMsg) &&
      join::s_join->state == join::JoinSourceState::WAITING_FOR_AGREEMENT) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Host received join agreement response "
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif
    join::Agreement_Update(&join::s_join->agreement, msgData.xuid,
                           msgData.clientAgrees);
    if (join::s_join->agreement.requestCount > 0 &&
        join::s_join->agreement.requestCount ==
            join::s_join->agreement.responseCount) {
      if (join::s_join->agreement.responseCount ==
          join::s_join->agreement.agreeCount) {
        join::LobbyHostMsg_SendJoinComplete(
            join::s_join->controllerIndex, join::s_join->sourceLobbyType, true);
        *join::s_successTimeout =
            sys::Sys_Milliseconds() + get_join_completionWindow().value_or(0);
        debug::LobbyDebug_JoinStateChanged(join::s_joinNames->connectionSuccess,
                                           "clients agree");
        spinner::LobbySpinner_Update_Impl(spinner::SpinnerSystem::JOIN_PROGRESS,
                                          6);
        join::s_join->state = join::JoinSourceState::CONNECTION_SUCCESS;
      } else {
        join::s_join->joinResult =
            join::JoinResult::SEND_AGREEMENT_REQUEST_FAILED;
        debug::LobbyDebug_JoinStateChanged(join::s_joinNames->connectionFailed,
                                           "clients disagree");
        join::LobbyTaskUpdate(join::s_join.get());
        join::s_join->state = join::JoinSourceState::CONNECTION_FAILED;
      }
      join::Host_ChangeState(join::s_join->sourceLobbyType);

      Agreement &agreement = join::s_join->agreement;

      Agreement &debugAgreement = join::s_join->debugAgreement;

      debugAgreement = agreement;
      agreement = {};
      debug::LobbyDebug_AgreementCount(0);
    }
  }
#ifndef NDEBUG
  else {
    game::trace(
        "[Lobby][Message][{}] Host received invalid join agreement response "
        "from client ({:016X} @ {}): {}. s_join->state: {}",
        game::serialize(controllerIndex), fromXUID, fromAdr->serialize(),
        msgData.serialize(), join::serialize(join::s_join->state));
  }
#endif
}

// P2P
void LobbyPeerToPeer_HandleConnectivityTest(ControllerIndex_t controllerIndex,
                                            net::netadr_t *fromAdr,
                                            XUID fromXUID, LobbyMsg *lobbyMsg) {
  Msg_ConnectionTest msgData{};
  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Peer received connectivity test "
                "from peer ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    session::LobbySession *const session =
        client::LobbyClient_GetSession(msgData.lobbyType);
    const int32_t currentConnectivityBits =
        client::LobbyClientSession_GetConnectivityBits(session->type);
    const int32_t newConnectivityBits =
        (1 << +msgData.clientIndex) | currentConnectivityBits;
    client::LobbyClientSession_SetConnectivityBits(session->type,
                                                   newConnectivityBits);
    if (currentConnectivityBits != newConnectivityBits) {
      client::SendUpdatedConnectivityToHost(controllerIndex, session,
                                            newConnectivityBits);
    }
  }
#ifndef NDEBUG
  else {
    game::trace("[Lobby][Message][{}] Peer received invalid connectivity test "
                "from peer ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
  }
#endif
}

// P2P
void HandleDemoState(ControllerIndex_t controllerIndex, net::netadr_t *fromAdr,
                     XUID fromXUID, LobbyMsg *lobbyMsg) {
  Msg_DemoState msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Peer received demo state "
                "from peer ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    demo::Demo_ReadHeartbeat(
        com::Com_ControllerIndex_GetLocalClientNum(controllerIndex), &msgData);
  }
#ifndef NDEBUG
  else {
    game::trace("[Lobby][Message][{}] Peer received invalid demo state "
                "from peer ({:016X} @ {})",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize());
  }
#endif
}

// Host <- Client
void HandleMemberInfo(ControllerIndex_t controllerIndex, net::netadr_t *fromAdr,
                      XUID fromXUID, LobbyMsg *lobbyMsg) {
  Msg_JoinMemberInfo msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Host received member info "
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    uint64_t joinNonce = 0;
    if (reservation::ReservationIsValid(
            *reservation::s_reservation,
            msgData.fixedClientInfoMsg.fixedClientInfo.xuid,
            msgData.reservationKey, joinNonce)) {
      debug::LobbyDebug_ReservationProcessed(
          msgData.fixedClientInfoMsg.fixedClientInfo.xuid,
          msgData.reservationKey);
      XUID probedXuid = 0;
      reservation::ReservationStatus *status =
          reservation::GetReservationStatus(
              *reservation::s_reservation,
              msgData.fixedClientInfoMsg.fixedClientInfo.xuid);
      if (status != nullptr) {
        probedXuid = status->probedXuid;
      }
      lobby::host::LobbyHost_AddJoiningClient(controllerIndex, fromAdr,
                                              joinNonce, &msgData, probedXuid);
      // Join completed
      reservation::ReservationRemove(
          *reservation::s_reservation,
          msgData.fixedClientInfoMsg.fixedClientInfo.xuid);
    }
  }
#ifndef NDEBUG
  else {
    game::trace("[Lobby][Message][{}] Host received invalid member info "
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
  }
#endif
}

// Host <- Client
void HandleServerInfo(ControllerIndex_t controllerIndex, net::netadr_t *fromAdr,
                      XUID fromXUID, LobbyMsg *lobbyMsg) {

  serverlist::LobbyServerList &serverList = *serverlist::s_lobbyServerList;
  if (com::Com_IsRunningUILevel() &&
      serverList.count < std::size(serverList.serverInfo)) {

    Msg_InfoResponseLobbyServer &msgData =
        serverList.serverInfo[serverList.count];
    msgData = {};
    if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
      game::trace("[Lobby][Message][{}] Host received server info "
                  "from client ({:016X} @ {}): {}",
                  game::serialize(controllerIndex), fromXUID,
                  fromAdr->serialize(), msgData.serialize());
#endif

      if (serverList.refreshTime == -1) {
        serverList.refreshTime = 0;
      }
      const LobbyMainMode mainMode = msgData.lobby.client.lobbyParams.mainMode;
      if (msgData.clientPlatform == LOBBY_PLATFORM &&
          (*serverlist::s_lobbyMainModeFilter == LobbyMainMode::INVALID ||
           mainMode == *serverlist::s_lobbyMainModeFilter) &&
          serverlist::CheckForDuplicate(msgData, serverList) == nullptr) {
        // Success
        ++serverList.count;
      }
    }
#ifndef NDEBUG
    else {
      game::trace("[Lobby][Message][{}] Host received invalid server info "
                  "from client ({:016X} @ {}): {}",
                  game::serialize(controllerIndex), fromXUID,
                  fromAdr->serialize(), msgData.serialize());
    }
#endif
  }
}

// Host <- client
void HandleJoinRequest(ControllerIndex_t controllerIndex,
                       net::netadr_t *fromAdr, XUID fromXUID,
                       LobbyMsg *lobbyMsg) {

  ControllerIndex_t activeControllerIndex = controllerIndex;
  net::netadr_t targetAdr = {};
  if (fromAdr != nullptr) {
    targetAdr = *fromAdr;
  }

  Msg_JoinParty msgData{};
  if (msgData.package(*lobbyMsg)) {

#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Host received join request "
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    session::LobbySession *const session =
        LobbyHostData_GetSession(msgData.targetLobby);
    const LobbyType lobbyType = session->type;
    const LobbyMainMode mainMode = base::LobbyBase_GetMainMode();
    const LobbyNetworkMode networkMode = get_join_network_mode();

    Msg_JoinResponse joinResponse{};
    strscpy(joinResponse.name, session->host.info.name);
    joinResponse.serverLocation = 999;

    joinResponse.lobbyParams.mainMode = mainMode;
    joinResponse.lobbyType = msgData.targetLobby;
    joinResponse.lobbyParams.networkMode = networkMode;
    joinResponse.response = join::JoinResult::SUCCESS;

    bool isXUIDLocalPlayer = live::user::LiveUser_IsXUIDLocalPlayer(fromXUID);
    bool isPermaBanned =
        !isXUIDLocalPlayer && live::user::LiveUser_IsPermaBanned(fromXUID);

    int32_t memberCount = msgData.memberCount;
    bool hasBannedMember = false;
    if (memberCount > 0) {
      for (const join::JoinPartyMember &member : msgData.getMembers()) {
        if (!isXUIDLocalPlayer &&
            live::user::LiveUser_IsPermaBanned(member.xuid)) {
          hasBannedMember = true;
        }
      }
      activeControllerIndex = controllerIndex;
    }

    const bool joinable = lobby::host::LobbyHost_IsJoinable(
        lobbyType, isXUIDLocalPlayer, &joinResponse.response);
    bool privacyAllowsJoin = lobby::host::LobbyHost_PartyPrivacyCheck(
        activeControllerIndex, lobbyType,
        static_cast<const join::JoinType>(msgData.joinType), fromXUID,
        &joinResponse);

    bool skipFurtherChecks = false;

    if (joinable && privacyAllowsJoin) {
      if (ugc::UGC_ActiveMod_Loaded() || isXUIDLocalPlayer ||
          networkMode == LobbyNetworkMode::LIVE) {
        if (isPermaBanned) {
          joinResponse.response = join::JoinResult::BANNED;
        } else if (hasBannedMember) {
          joinResponse.response = join::JoinResult::BANNED_MEMBER;
        } else {
          if (networkMode == LobbyNetworkMode::LAN) {
            if (msgData.networkMode != LobbyNetworkMode::LAN ||
                (msgData.joinType != join::JoinType::PARTY &&
                 msgData.joinType != join::JoinType::PLAYLIST)) {
              joinResponse.response = join::JoinResult::NETWORK_MODE_MISMATCH;
              skipFurtherChecks = true;
            }
          } else if (networkMode == LobbyNetworkMode::LIVE) {
            if (game::is_client()) {
              const int32_t currentPlaylistVersion =
                  playlist::Playlist_GetVersionNumber();
              if (msgData.playlistVersion != currentPlaylistVersion) {
                joinResponse.response =
                    msgData.playlistVersion < currentPlaylistVersion
                        ? join::JoinResult::MISMATCH_PLAYLIST_VERSION_TO_NEW
                        : join::JoinResult::MISMATCH_PLAYLIST_VERSION_TO_OLD;
                skipFurtherChecks = true;
              }
            }
            if (!skipFurtherChecks) {
              const int32_t currentFFOTDVersion =
                  live::storage::LiveStorage_GetFFOTDVersion();
              if (msgData.ffotdVersion != currentFFOTDVersion) {
                joinResponse.response =
                    msgData.ffotdVersion < currentFFOTDVersion
                        ? join::JoinResult::MISMATCH_FFOTD_VERSION_TO_NEW
                        : join::JoinResult::MISMATCH_FFOTD_VERSION_TO_OLD;

                skipFurtherChecks = true;
              }
            }
          }

          if (!skipFurtherChecks) {
            if (msgData.netFieldChecksum ==
                net::msg::crc::MsgCRC_NetFieldChecksum()) {
              joinResponse.response =
                  lobby::host::party::LobbyHost_IsPartyAllowed(
                      activeControllerIndex, lobbyType, msgData.memberCount);
              if (joinResponse.response == join::JoinResult::SUCCESS) {
                joinResponse.response = lobby::host::LobbyHost_CanFit(
                    activeControllerIndex, lobbyType, msgData.members,
                    msgData.memberCount, fromXUID, msgData.probedXuid);
              }
            } else {
              joinResponse.response =
                  join::JoinResult::MISMATCH_NETFIELD_CHECKSUM;
            }
          }
        }
      } else {
        joinResponse.response = join::JoinResult::NOT_JOINABLE_OFFLINE_MODE;
      }
    }

    join::JoinRequestLuaCheck(activeControllerIndex, fromXUID, &msgData,
                              &joinResponse);

    if (joinResponse.response == join::JoinResult::SUCCESS) {
      bool reserveSuccess = join::LobbyJoin_Reserve(
          activeControllerIndex, msgData.members, msgData.memberCount,
          msgData.joinNonce, msgData.probedXuid, &joinResponse.reservationKey);
      if (!reserveSuccess) {
        joinResponse.response = join::JoinResult::COULD_NOT_RESERVE;
      }
    }

    net::NetChanMsgType_e globalChannel =
        netchan::LobbyNetChan_GetGlobalChannel(LobbyGlobalChannel::JOIN);
    LobbyMsg_SendToAdr<Msg_JoinResponse>(activeControllerIndex, globalChannel,
                                         LobbyModule::HOST, targetAdr, fromXUID,
                                         joinResponse);
  }
#ifndef NDEBUG
  else {
    game::trace("[Lobby][Message][{}] Host received invalid join request "
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
  }
#endif
}

// P2P
void HandleP2PVoicePacket(ControllerIndex_t controllerIndex,
                          net::netadr_t *fromAdr, XUID fromXUID,
                          LobbyMsg *lobbyMsg) {
  Msg_VoicePacket msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Peer received voice packet "
                "from peer ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    session::LobbySession *const session =
        client::LobbyClient_GetSession(msgData.lobbyType);
    if (voice::LobbyVoice_HandleVoicePacket(controllerIndex, session,
                                            &msgData)) {
      const int32_t currentConnectivityBits =
          client::LobbyClientSession_GetConnectivityBits(session->type);
      const ClientNum_t clientIndex =
          session::LobbySession_FindClientIndexByXuid(session, fromXUID);
      if (validate(clientIndex)) {
        const int32_t newConnectivityBits =
            (1 << clientIndex) | currentConnectivityBits;
        client::LobbyClientSession_SetConnectivityBits(session->type,
                                                       newConnectivityBits);
        if (currentConnectivityBits != newConnectivityBits) {
          client::SendUpdatedConnectivityToHost(controllerIndex, session,
                                                newConnectivityBits);
        }
      }
    }
  }
#ifndef NDEBUG
  else {
    game::trace("[Lobby][Message][{}] Peer received invalid voice packet "
                "from peer ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
  }
#endif
}

// Host <- Client
void HandleHostClientVoicePacket(ControllerIndex_t controllerIndex,
                                 net::netadr_t *fromAdr, XUID fromXUID,
                                 LobbyMsg *lobbyMsg) {
  Msg_VoicePacket msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Host received voice packet "
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    voice::LobbyVoice_HandleVoicePacket(
        controllerIndex, LobbyHostData_GetSession(msgData.lobbyType), &msgData);
  }
#ifndef NDEBUG
  else {
    game::trace("[Lobby][Message][{}] Host received invalid voice packet "
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
  }
#endif
}

// Host <- Client
void HandleJoinResponse(ControllerIndex_t controllerIndex,
                        net::netadr_t *fromAdr, XUID fromXUID,
                        LobbyMsg *lobbyMsg) {
  Msg_JoinResponse msgData{};

  if (msgData.package(*lobbyMsg) &&
      join::s_join->state == join::JoinSourceState::HANDSHAKING) {
    const char *failReason = nullptr;
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Host received join response "
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    if (*fromAdr == join::s_join->potentialHost.info.netAdr) {
      if (static_cast<uint32_t>(msgData.lobbyType) >
          static_cast<uint32_t>(LobbyType::GAME)) {
        join::s_join->joinResult = join::JoinResult::INVALID_LOBBY;
        failReason = "host response invalid";
      } else {
        join::s_join->potentialHost.lobbyType = msgData.lobbyType;
        strscpy(join::s_join->potentialHost.info.name, msgData.name);
        join::s_join->joinResult = msgData.response;
        join::s_join->potentialHost.reservationKey = msgData.reservationKey;
        join::s_join->potentialHost.info.serverLocation =
            msgData.serverLocation;
        join::s_join->potentialHost.lobbyParams = msgData.lobbyParams;
        switch (msgData.response) {
        case join::JoinResult::SUCCESS: {
          if (host::LobbyHostMsg_SendAgreementRequest_Impl(
                  join::s_join->controllerIndex,
                  &join::s_join->potentialHost.info, msgData.reservationKey,
                  join::s_join->agreement.nonce, join::s_join->sourceLobbyType,
                  join::s_join->targetLobbyType, msgData.lobbyParams) &&
              join::Agreement_Add(&join::s_join->agreement,
                                  join::s_join->sourceLobbyType)) {
            debug::LobbyDebug_JoinStateChanged(
                join::s_joinNames->waitingForAgreement,
                utils::string::va(
                    "reservation key %lu",
                    static_cast<uint32_t>(msgData.reservationKey)));
            spinner::LobbySpinner_Update_Impl(
                spinner::SpinnerSystem::JOIN_PROGRESS, 4);
            join::s_join->state = join::JoinSourceState::WAITING_FOR_AGREEMENT;
            join::Host_ChangeState(join::s_join->sourceLobbyType);
            return;
          }
          join::s_join->joinResult =
              join::JoinResult::SEND_AGREEMENT_REQUEST_FAILED;
          failReason = "MENU_JOIN_RESULT_NOT_JOINABLE";
          break;
        }
        case join::JoinResult::OVER_MAX_PARTY_LIMIT:
          failReason = "EXE_TO_MANY_LOCAL_PLAYERS";
          break;
        case join::JoinResult::LOBBY_FULL:
          failReason = "MENU_JOIN_RESULT_LOBBY_FULL";
          break;
        case join::JoinResult::NETWORK_MODE_MISMATCH:
          failReason = "MENU_JOIN_RESULT_NETWORK_MODE_MISMATCH";
          break;
        case join::JoinResult::MISMATCH_PLAYLISTID:
          failReason = "MENU_JOIN_RESULT_MISMATCH_PLAYLISTID";
          break;
        case join::JoinResult::MISMATCH_PLAYLIST_VERSION_TO_NEW:
        case join::JoinResult::MISMATCH_FFOTD_VERSION_TO_NEW:
          failReason = "MENU_JOIN_RESULT_MISMATCH_PLAYLIST_VERSION_TO_NEW";
          break;
        case join::JoinResult::MISMATCH_PLAYLIST_VERSION_TO_OLD:
        case join::JoinResult::MISMATCH_FFOTD_VERSION_TO_OLD:
          failReason = "MENU_JOIN_RESULT_MISMATCH_PLAYLIST_VERSION_TO_OLD";
          break;
        case join::JoinResult::MISMATCH_PROTOCOL_VERSION:
          failReason = "MENU_JOIN_RESULT_MISMATCH_PROTOCOL_VERSION";
          break;
        case join::JoinResult::MISMATCH_NETFIELD_CHECKSUM:
          failReason = "MENU_JOIN_RESULT_MISMATCH_NETFIELD_CHECKSUM";
          break;
        case join::JoinResult::MIGRATE_IN_PROGRESS:
          failReason = "MENU_JOIN_RESULT_MIGRATE_IN_PROGRESS";
          break;
        case join::JoinResult::COULD_NOT_RESERVE: {
          if (join::s_join->potentialHost.retryCount <
              get_join_retryCount().value_or(0)) {
            ++join::s_join->potentialHost.retryCount;
            join::s_join->potentialHost.retryTime =
                sys::Sys_Milliseconds() + get_join_retryDelay().value_or(0);
            debug::LobbyDebug_JoinStateChanged(
                join::s_joinNames->associating,
                "host is temporarily unavailable but may be active "
                "later. retrying..");
            spinner::LobbySpinner_Update_Impl(
                spinner::SpinnerSystem::JOIN_PROGRESS, 2);
            join::s_join->state = join::JoinSourceState::ASSOCIATING;
            join::Host_ChangeState(join::s_join->sourceLobbyType);
            return;
          }
          failReason = "MENU_JOIN_RESULT_COULD_NOT_RESERVE";
          break;
        }
        default:
          failReason = "MENU_JOIN_RESULT_NOT_JOINABLE";
          break;
        }
      }
      debug::LobbyDebug_JoinStateChanged(join::s_joinNames->connectionFailed,
                                         failReason);
      join::LobbyTaskUpdate(join::s_join.get());
      join::s_join->state = join::JoinSourceState::CONNECTION_FAILED;
      join::Host_ChangeState(join::s_join->sourceLobbyType);
    }
  }
#ifndef NDEBUG
  else {
    game::trace("[Lobby][Message][{}] Host received invalid join response "
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
  }
#endif
}

// P2P
void HandleMigrateStart(ControllerIndex_t controllerIndex,
                        net::netadr_t *fromAdr, XUID fromXUID,
                        LobbyMsg *lobbyMsg) {
  Msg_LobbyMigrateStart msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Peer received migration start "
                "from peer ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    session::LobbySession *const session =
        client::LobbyClient_GetSession(msgData.lobbyType);
    if (msgData.migrateTo == 0) {
      migrate::LobbyMigrate_HandleFindBestCommand(session, &msgData);
    } else {
      migrate::LobbyMigrate_HandleLobbyMigrateTo(session, msgData.migrateTo);
    }
  }
#ifndef NDEBUG
  else {
    game::trace("[Lobby][Message][{}] Peer received invalid migration start "
                "from peer ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
  }
#endif
}

// Host <- Client
void Host_HandleClientContent(ControllerIndex_t controllerIndex,
                              net::netadr_t *fromAdr, XUID fromXUID,
                              LobbyMsg *lobbyMsg) {
  Msg_ClientContent msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Host received client content "
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    if (LobbyHost_IsHost(msgData.lobbyType)) {
      const clientcontent::ClientContentType contentType =
          clientcontent::ClientContentType_GetContentTypeFromDatamask(
              msgData.dataMask);
      session::LobbySession *const session =
          LobbyHostData_GetSession(msgData.lobbyType);
      session::SessionClient *const sessionLobbyList =
          lobby::host::LobbyHostData_GetSessionLobbyList(msgData.lobbyType);
      session::SessionClient *const clientByXuid =
          session::LobbySession_GetClientByXuid(session, fromXUID);

      if (clientByXuid != nullptr && clientByXuid->activeClient != nullptr) {
        clientcontent::ClientContent *clientContent =
            active::LobbyActiveList_GetClientContent(
                clientByXuid->activeClient);
        const uint32_t currentCACPSBufferCrc = com::Com_BlockChecksum32(
            msgData.clientContentData, sizeof(msgData.clientContentData), 0);

        if (clientContent != nullptr &&
            clientcontent::ClientContent_GetChecksum(
                contentType, clientContent) != currentCACPSBufferCrc) {
          if (com::Com_IsInGame()) {
            active::LobbyActiveList_ClearClientContentForClient(
                clientByXuid->activeClient, contentType, fromXUID);
          }

          clientcontent::ClientContent_SetData(contentType, clientContent,
                                               msgData.clientContentData,
                                               msgData.compressedBufferSize);
          clientcontent::ClientContent_SetCompressedDataSize(
              contentType, clientContent, msgData.compressedBufferSize);
          clientcontent::ClientContent_SetChecksum(contentType, clientContent,
                                                   currentCACPSBufferCrc);

          clientContent->sessionMode = msgData.sessionMode;
          clientContent->gameMode = msgData.gameMode;

          // First pass: mark session lobby list entries dirty
          uint32_t trackingIndex = 0;
          for (ClientNum_t i = ClientNum_t::CLIENT_INDEX_0;
               i < CLIENT_INDEX_COUNT; ++i) {
            session::SessionClient *currentClient = &sessionLobbyList[i];
            if (currentClient->xuid != 0) {
              clientcontent::ClientContentTracking *const
                  contentTrackingByIndex =
                      active::LobbyActiveList_GetClientContentTrackingByIndex(
                          clientByXuid->activeClient, contentType,
                          trackingIndex);
              contentTrackingByIndex->state =
                  clientcontent::ClientContentTransferState::DIRTY;
              contentTrackingByIndex->toXUID = currentClient->xuid;

              ++trackingIndex;
            }
          }

          // Second pass: iterate through lobby list to update tracking
          // records
          for (ClientNum_t j = ClientNum_t::CLIENT_INDEX_0;
               j < CLIENT_INDEX_COUNT; ++j) {
            session::SessionClient *targetClient = &sessionLobbyList[j];
            if (targetClient->xuid != 0) {
              clientcontent::ClientContentTracking *clientContentTracking =
                  active::LobbyActiveList_GetClientContentTracking(
                      targetClient->activeClient, contentType);

              uint32_t foundIndex = 0;
              ClientNum_t innerCount = ClientNum_t::CLIENT_INDEX_0;

              clientcontent::ClientContentTracking *clientContentTracking2 =
                  clientContentTracking;
              while (fromXUID != clientContentTracking2->toXUID) {
                ++innerCount;
                ++foundIndex;
                clientContentTracking2++;

                if (innerCount >= ClientNum_t::CLIENT_INDEX_COUNT) {
                  foundIndex = 0;
                  ClientNum_t zeroSearchCount = ClientNum_t::CLIENT_INDEX_0;
                  while (clientContentTracking->toXUID != 0) {
                    ++zeroSearchCount;
                    ++foundIndex;
                    clientContentTracking++;
                    if (zeroSearchCount >= ClientNum_t::CLIENT_INDEX_COUNT) {
                      foundIndex = ClientNum_t::INVALID_CLIENT_INDEX;
                      break;
                    }
                  }
                  break;
                }
              }

              if (foundIndex != ClientNum_t::INVALID_CLIENT_INDEX) {
                clientcontent::ClientContentTracking *targetTracking =
                    active::LobbyActiveList_GetClientContentTrackingByIndex(
                        targetClient->activeClient, contentType, foundIndex);
                targetTracking->state =
                    clientcontent::ClientContentTransferState::DIRTY;

                targetTracking->toXUID = fromXUID;
              }
            }
          }
        }
      }
    }
  }
#ifndef NDEBUG
  else {
    game::trace("[Lobby][Message][{}] Host received invalid client content "
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
  }
#endif
}

// Host <- Client
void HandleModifiedStats(ControllerIndex_t controllerIndex,
                         net::netadr_t *fromAdr, XUID fromXUID,
                         LobbyMsg *lobbyMsg) {

  Msg_ModifiedStats msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Host received modified stats "
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    sv::SV_ReceiveModifiedStats(fromXUID, msgData.statsBuffer,
                                msgData.statsSize);
  }
#ifndef NDEBUG
  else {
    game::trace("[Lobby][Message][{}] Host received invalid modified stats "
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
  }
#endif
}

// Host <- Client
void HandleClientDisconnect(ControllerIndex_t controllerIndex,
                            net::netadr_t *fromAdr, XUID fromXUID,
                            LobbyMsg *lobbyMsg) {
  Msg_LobbyClientDisconnect msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Host received disconnect "
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    session::LobbySession *const session =
        LobbyHostData_GetSession(msgData.lobbyType);
    session::SessionClient *const client =
        session::LobbySession_GetClientByXuid(session, msgData.xuid);
    if (client != nullptr && client->activeClient != nullptr) {
      lobby::host::LobbyHost_RemoveClient(msgData.lobbyType, msgData.xuid,
                                          "received client disconnect message");
    }
  }
#ifndef NDEBUG
  else {
    game::trace("[Lobby][Message][{}] Host received invalid disconnect "
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
  }
#endif
}

// Host <- Client
void HandlePeerToPeerUpdate(ControllerIndex_t controllerIndex,
                            net::netadr_t *fromAdr, XUID fromXUID,
                            LobbyMsg *lobbyMsg) {
  Msg_PeerToPeerInfo msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Host received peer to peer update "
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    session::LobbySession *const session =
        LobbyHostData_GetSession(msgData.lobbyType);
    if (LobbyHost_IsHost(msgData.lobbyType)) {
      session::SessionClient *const client =
          session::LobbySession_GetClientByXuid(session, msgData.clientXuid);
      if (client != nullptr && client->activeClient != nullptr) {
        const int32_t prevLastCaughtTalking =
            client->voiceInfo.lastCaughtTalking;
        client->voiceInfo.lastCaughtTalking = msgData.connectivityBits;
        if (prevLastCaughtTalking != msgData.connectivityBits) {
          session::LobbySession_DirtySet(session, 1);
        }
      }
    }
  }
#ifndef NDEBUG
  else {
    game::trace(
        "[Lobby][Message][{}] Host received invalid peer to peer update "
        "from client ({:016X} @ {}): {}",
        game::serialize(controllerIndex), fromXUID, fromAdr->serialize(),
        msgData.serialize());
  }
#endif
}

// Host <- Client
void HandleClientReliableData(ControllerIndex_t controllerIndex,
                              net::netadr_t *fromAdr, XUID fromXUID,
                              LobbyMsg *lobbyMsg) {
  Msg_ClientReliableData msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Host received reliable data"
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    session::LobbySession *const session =
        LobbyHostData_GetSession(msgData.lobbyType);

    if (LobbyHost_IsHost(msgData.lobbyType)) {
      if ((msgData.dataMask & (1ul << 1)) != 0 &&
          session->leader.xuid == fromXUID) {
        session::LobbySession_SetLeader(session, msgData.xuidNewLeader);
      }
      if ((msgData.dataMask & (1ul << 11)) != 0) {
        session::LobbySession_SetPlatformSessionID(session,
                                                   msgData.platformSessionID);
      }
      if ((msgData.dataMask & (1ul << 2)) != 0) {
        lobby::host::LobbyHostMsg_SendDisconnectClient(
            controllerIndex, msgData.lobbyType, msgData.disconnectClientXuid,
            msgData.disconnectClient);
      }
      if ((msgData.dataMask & (1ul << 3)) != 0) {
        ui::LobbyUI_SetLeaderActivity(msgData.leaderActivity);
      }

      if (msgData.lobbyType == LobbyType::GAME) {
        session::LobbyGameData &lobbyGame = session->lobbyData.lobbyGame;

        if (session->lobbyData.type != nullptr &&
            *session->lobbyData.type == LobbyType::GAME) {
          const ClientNum_t clientIndex =
              session::LobbySession_FindClientIndexByXuid(session, fromXUID);

          if (validate(clientIndex)) {
            session::GameClientData &clientData = lobbyGame.client[clientIndex];

            if ((msgData.dataMask & (1ul << 5)) != 0) {
              clientData.team = msgData.team;
            }
            if ((msgData.dataMask & (1ul << 6)) != 0) {
              clientData.mapVote = msgData.mapVote;
            }
            if ((msgData.dataMask & (1ul << 7)) != 0) {
              clientData.readyUp = msgData.readyUp;
            }

            const bool hasItemVote = (msgData.dataMask & (1ul << 8)) != 0;
            const bool hasCharacterSelection =
                (msgData.dataMask & (1ul << 9)) != 0;
            const bool hasEditComplete = (msgData.dataMask & (1ul << 10)) != 0;

            if ((!hasItemVote && !hasCharacterSelection && !hasEditComplete) ||
                (clientData.pregameState ==
                     LobbyClientPregameState::SELECTION_FORCED ||
                 clientData.pregameState ==
                     LobbyClientPregameState::SELECTION_COMPLETE)) {
              session::LobbySession_DirtySet(session, 1);
            } else {
              clientData.pregameState = LobbyClientPregameState::SELECTED;

              uint32_t voteIndex = 0;
              if (hasItemVote) {
                VoteType itemVoteType = VoteType::INVALID;
                switch (msgData.itemVote.itemType) {
                case VoteItemType::ITEM:
                  itemVoteType = data::LobbyData_GetItemVoteType(
                      msgData.itemVote.itemIndex);
                  break;
                case VoteItemType::ATTACHMENT:
                  itemVoteType = data::LobbyData_GetAttachmentVoteType(
                      msgData.itemVote.attachment);
                  break;
                case VoteItemType::ITEM_ATTACHMENT:
                  itemVoteType = data::LobbyData_GetItemAttachmentVoteType(
                      msgData.itemVote.itemIndex, msgData.itemVote.attachment);
                  break;
                case VoteItemType::ATTACHMENT_GROUP:
                  itemVoteType = data::LobbyData_GetAttachmentGroupVoteType(
                      msgData.itemVote.itemGroup, msgData.itemVote.attachment);
                  break;
                default:
                  break;
                }

                const uint32_t maxVotes =
                    com::gts::Com_GametypeSettings_GetUInt(
                        "pregameItemMaxVotes", false);
                voteIndex = clientData.itemVoteCount;
                if (clientData.itemVoteCount < maxVotes &&
                    clientData.itemVoteCount < 12) {
                  if (validate(itemVoteType)) {
                    clientData.pregameState =
                        LobbyClientPregameState::SELECTING;
                  } else if (msgData.itemVote.voteType != VoteType::INVALID) {
                    ++clientData.itemVoteCount;
                    lobbyGame.pregameVotes[lobbyGame.pregameVoteCount] =
                        msgData.itemVote;
                    lobbyGame.pregameVotes[lobbyGame.pregameVoteCount]
                        .clientXuid = fromXUID;
                    ++lobbyGame.pregameVoteCount;

                    if (clientData.itemVoteCount < maxVotes) {
                      clientData.pregameState =
                          LobbyClientPregameState::SELECTING;
                    }
                  }
                }
              }

              if (hasCharacterSelection) {
                clientData.characterSelection = msgData.characterSelection;
              }

              if (hasEditComplete) {
                clientData.pregameEditComplete = msgData.editComplete;
                if (!msgData.editComplete) {
                  clientData.pregameState = LobbyClientPregameState::SELECTING;
                }
              }

              // Sets current thread ID to `GetCurrentThread()` output in TLS if
              // not already set. This could also just be here as a leftover
              // fragment of a conditionally compiled conditional - not sure.
              // TODO :can this be removed?
              sys::Sys_IsMainThread();

              {
                lua::hks::lua_State *vm = *lua::hks::s_lobbyLuaVM;
                lua::LuaScopedEvent luaVM(vm, 2, "LobbyVM",
                                          "OnClientSelectionReceived");

                if (!luaVM._finished) {
                  const ControllerIndex_t primaryController =
                      com::Com_ControllerIndexes_GetPrimary();
                  lua::Lua_SetTableInt(
                      "controller",
                      static_cast<lua::hks::lua_Integer>(primaryController),
                      luaVM._vm);
                  lua::Lua_SetTableID64("xuid", fromXUID, luaVM._vm);
                  lua::Lua_SetTableBool("forced", false, luaVM._vm);
                  lua::Lua_SetTableInt("position", clientData.pregamePosition,
                                       luaVM._vm);

                  if (hasItemVote) {
                    lua::Lua_SetTableInt("voteIndex",
                                         static_cast<int32_t>(voteIndex),
                                         luaVM._vm);
                    lua::Lua_SetTableInt(
                        "voteCount",
                        static_cast<int32_t>(clientData.itemVoteCount),
                        luaVM._vm);
                  }
                  if (hasCharacterSelection) {
                    lua::Lua_SetTableInt(
                        "characterIndex",
                        static_cast<int32_t>(
                            clientData.characterSelection.characterIndex),
                        luaVM._vm);
                    lua::Lua_SetTableInt(
                        "loadoutType",
                        +clientData.characterSelection.loadoutType, luaVM._vm);
                  }
                  luaVM._finished = true;
                }
              }

              session::LobbySession_DirtySet(session, 1);
            }
          }
        }
      }
    }
  }
#ifndef NDEBUG
  else {
    game::trace("[Lobby][Message][{}] Host received invalid reliable data"
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
  }
#endif
}

// Host <- Client
void HandleLobbyClientHeartbeat(ControllerIndex_t controllerIndex,
                                net::netadr_t *fromAdr, XUID fromXUID,
                                LobbyMsg *lobbyMsg) {
  Msg_LobbyClientHeartbeat msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Host received heartbeat "
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    session::LobbySession *const session =
        LobbyHostData_GetSession(msgData.lobbyType);
    if (session != nullptr) {
      migrate::LobbyMigrateHost_ReadClientMigrateInfo(
          session, &msgData.migrateInfo, fromXUID);
    }
    bool mutableClientInfoChanged = false;
    session::LobbySession_UpdateClient(
        session, fromXUID, &msgData.mutableClientInfoMsg.mutableClientInfo,
        &mutableClientInfoChanged);
    if (mutableClientInfoChanged) {
      session::LobbySession_SetAllLobbySessionsDirty(7);
    }
  }
#ifndef NDEBUG
  else {
    game::trace("[Lobby][Message][{}] Host received invalid heartbeat "
                "from client ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
  }
#endif
}

// P2P
void HandleMigrateBandwidthTest(ControllerIndex_t controllerIndex,
                                net::netadr_t *fromAdr, XUID fromXUID,
                                LobbyMsg *lobbyMsg) {
  Msg_LobbyMigrateBandwidthTest msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Peer received migrate bandwidth test "
                "from peer ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    session::LobbySession *const session =
        client::LobbyClient_GetSession(msgData.lobbyType);
    migrate::LobbyMigrateClient_HandleTestPacket(
        controllerIndex, session, fromAdr, msgData.peerXuid, &msgData);
  }
#ifndef NDEBUG
  else {
    game::trace(
        "[Lobby][Message][{}] Peer received invalid migrate bandwidth test "
        "from peer ({:016X} @ {}): {}",
        game::serialize(controllerIndex), fromXUID, fromAdr->serialize(),
        msgData.serialize());
  }
#endif
}

// Client <- Host
void HandleLobbyStatePrivate(ControllerIndex_t controllerIndex,
                             net::netadr_t *fromAdr, XUID fromXUID,
                             LobbyMsg *lobbyMsg) {
  Msg_LobbyStatePrivate msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Client received lobby state private "
                "from host ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    if (IsLobbyStateValid(controllerIndex, fromAdr, fromXUID,
                          msgData.core.lobbyType,
                          msgData.core.migrateInfo.indexBits)) {
      HandleLobbyStateCore(controllerIndex, fromAdr, fromXUID, &msgData.core);
      session::LobbySession *const session =
          client::LobbyClientData_GetSession(msgData.core.lobbyType);
      client::LobbyClient_UpdatePrivateData(controllerIndex, session,
                                            &msgData.dataPrivate);
      ui::LobbyUI_SetUIScreenFromState(msgData.core.lobbyType,
                                       msgData.core.uiScreen);
      session::LobbySession_DirtySet(session, 2);
    }
  }
#ifndef NDEBUG
  else {
    game::trace(
        "[Lobby][Message][{}] Client received invalid lobby state private "
        "from host ({:016X} @ {}): {}",
        game::serialize(controllerIndex), fromXUID, fromAdr->serialize(),
        msgData.serialize());
  }
#endif
}

// Client <- Host
void HandleLobbyStateGame(ControllerIndex_t controllerIndex,
                          net::netadr_t *fromAdr, XUID fromXUID,
                          LobbyMsg *lobbyMsg) {
  Msg_LobbyStateGame msgData{};

  if (msgData.package(*lobbyMsg) && msgData.core.lobbyType == LobbyType::GAME) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Client received lobby state game "
                "from host ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    if (IsLobbyStateValid(controllerIndex, fromAdr, fromXUID, LobbyType::GAME,
                          msgData.core.migrateInfo.indexBits)) {
      HandleLobbyStateCore(controllerIndex, fromAdr, fromXUID, &msgData.core);
      const bool isHost = client::LobbyClient_IsHost(LobbyType::GAME);
      if (base::LobbyBase_GetMainMode() == LobbyMainMode::CP && !isHost) {
        cp_queued_level->set(msgData.dataGame.cpQueuedLevel);
        if (game::is_client()) {
          cinematic::Cinematic_SetLoadingMovieSkippedState(
              msgData.dataGame.loadingMovieSkipped);
        }
      }
      session::LobbySession *const session =
          client::LobbyClientData_GetSession(LobbyType::GAME);
      session::LobbyGameData &lobbyGame = session->lobbyData.lobbyGame;
      if (*session->lobbyData.type == LobbyType::GAME) {
        const uint32_t updated = client::LobbyClient_UpdateGameData(
            controllerIndex, session, &msgData.dataGame);
        ui::LobbyUI_SetUIScreenFromState(LobbyType::GAME,
                                         msgData.core.uiScreen);
        if (updated != 0) {
          sys::Sys_IsMainThread();
          lua::hks::lua_State *lobbyVM = *lua::hks::s_lobbyLuaVM;
          lua::LuaScopedEvent event(lobbyVM, 2, "LobbyVM",
                                    "GameLobbyGameServerDataUpdate");
          event._finished = true;
        }
        if ((updated & 2) != 0) {
          sys::Sys_IsMainThread();
          lua::hks::lua_State *lobbyVM = *lua::hks::s_lobbyLuaVM;
          {
            lua::LuaScopedEvent event(lobbyVM, 2, "LobbyVM",
                                      "UpdateLobbyStatusInfo");
            if (!event._finished) {
              lua::Lua_SetTableInt("controller", controllerIndex, event._vm);
              lua::Lua_SetTableInt("lobbyType", +session->type, event._vm);
              lua::Lua_SetTableInt("status", lobbyGame.status, event._vm);
              lua::Lua_SetTableInt("statusValue", lobbyGame.statusValue,
                                   event._vm);
              lua::Lua_SetTableInt("pregameState",
                                   static_cast<int32_t>(lobbyGame.pregameState),
                                   event._vm);
              event._finished = true;
            }
          }
        }

        if ((updated & 4) != 0) {
          sys::Sys_IsMainThread();
          lua::hks::lua_State *lobbyVM = *lua::hks::s_lobbyLuaVM;
          {
            lua::LuaScopedEvent event(lobbyVM, 2, "LobbyVM",
                                      "GameLobbyClientDataUpdate");
            if (!event._finished) {
              lua::Lua_SetTableInt("controller", +controllerIndex, event._vm);
              lua::Lua_BeginTable("clients", event._vm);
              for (ClientNum_t clientIdx = ClientNum_t::CLIENT_INDEX_0;
                   clientIdx < std::size(lobbyGame.client); ++clientIdx) {
                const session::GameClientData &client =
                    lobbyGame.client[+clientIdx];
                const session::SessionClient &sessionClient =
                    session->client[+clientIdx];
                if (sessionClient.xuid != 0) {
                  lua::Lua_BeginTable(+clientIdx + 1, event._vm);
                  lua::Lua_SetTableID64("xuid", sessionClient.xuid, event._vm);
                  lua::Lua_SetTableInt(
                      "team", static_cast<int32_t>(client.team), event._vm);
                  lua::Lua_SetTableInt("mapVote", +client.mapVote, event._vm);
                  lua::Lua_SetTableInt("readyUp",
                                       static_cast<int32_t>(client.readyUp),
                                       event._vm);
                  lua::Lua_SetTableInt("pregamePosition",
                                       client.pregamePosition, event._vm);
                  lua::Lua_SetTableInt(
                      "pregameState", static_cast<int32_t>(client.pregameState),
                      event._vm);
                  lua::Lua_SetTableInt(
                      "restrictedItemVoteCount",
                      static_cast<int32_t>(client.itemVoteCount), event._vm);
                  lua::Lua_BeginTable("CharacterDraft", event._vm);
                  lua::Lua_SetTableInt(
                      "characterIndex",
                      static_cast<int32_t>(
                          client.characterSelection.characterIndex),
                      event._vm);
                  lua::Lua_SetTableInt("loadoutType",
                                       +client.characterSelection.loadoutType,
                                       event._vm);
                  lua::Lua_EndTable(event._vm);
                  lua::Lua_EndTable(event._vm);
                }
              }

              lua::Lua_EndTable(event._vm);
              lua::Lua_BeginTable("RestrictedItemVotes", event._vm);
              for (const session::ItemVote &vote :
                   lobbyGame.getPregameVotes()) {
                lua::Lua_SetTableInt(
                    "itemType", static_cast<int32_t>(vote.itemType), event._vm);
                lua::Lua_SetTableInt("itemIndex",
                                     static_cast<int32_t>(vote.itemIndex),
                                     event._vm);
                lua::Lua_SetTableInt("itemGroup", +vote.itemGroup, event._vm);
                lua::Lua_SetTableInt("attachmentIndex", +vote.attachment,
                                     event._vm);
                lua::Lua_SetTableInt(
                    "voteType", static_cast<int32_t>(vote.voteType), event._vm);
              }

              lua::Lua_EndTable(event._vm);
              event._finished = true;
            }
          }
        }
        if ((msgData.core.lobbyMode == LobbyMode::CUSTOM ||
             msgData.core.lobbyMode == LobbyMode::FREERUN) &&
            !isHost && msgData.compressedBufferSize > 0 &&
            com::gts::Com_GametypeSettings_Read(msgData.compressedBuffer,
                                                msgData.compressedBufferSize)) {
          lua::lobby::vm::LobbyVM_OnGametypeSettingsChange(session);
        }
        session::LobbySession_DirtySet(session, 2);
      }
    }
  }
#ifndef NDEBUG
  else {
    game::trace("[Lobby][Message][{}] Client received invalid lobby state game "
                "from host ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
  }
#endif
}

// Client <- Host
void HandleLobbyHostHeartbeat(ControllerIndex_t controllerIndex,
                              net::netadr_t *fromAdr, XUID fromXUID,
                              LobbyMsg *lobbyMsg) {

  Msg_LobbyHostHeartbeat msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Client received heartbeat "
                "from host ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    session::LobbySession *const session =
        client::LobbyClient_GetSession(msgData.lobbyType);
    if (session != nullptr && fromXUID == session->host.info.xuid) {
      client::LobbyClientData_UpdateLastHostMsgTimestamp(msgData.lobbyType,
                                                         true);
      migrate::LobbyMigrateClient_ReadMigrateInfo(session, &msgData.migrateInfo,
                                                  false);
      if (client::LobbyClientLaunch_IsInGame()) {
        const std::optional<std::string_view> map = data::LobbyData_GetMap();
        if (!map.has_value() || !map.value().starts_with("cp_sh")) {
          return;
        }
        com::Com_UpdateScriptBunkCollectibles.call_safe(controllerIndex);
      }
      client::LobbyClientMsg_SendClientHeartbeat(controllerIndex,
                                                 session->type);
    }
  }
#ifndef NDEBUG
  else {
    game::trace("[Lobby][Message][{}] Client received invalid heartbeat "
                "from host ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
  }
#endif
}

// Client <- Host
void HandleHostDisconnect(ControllerIndex_t controllerIndex,
                          net::netadr_t *fromAdr, XUID fromXUID,
                          LobbyMsg *lobbyMsg) {
  Msg_LobbyHostDisconnect msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Client received disconnect host "
                "from host ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    if (!lobby::host::LobbyHost_IsHost(msgData.lobbyType)) {
      session::LobbySession *const session =
          client::LobbyClientData_GetSession(msgData.lobbyType);
      if (!migrate::LobbyMigrateClient_MigrateActive(session) &&
          session->active != session::SessionActive::INACTIVE &&
          session->host.info.xuid == msgData.xuid) {
        if (migrate::LobbyMigrate_CanLobbyMigrate(session)) {
          if (!client::LobbyClientLaunch_IsInGame()) {
            session::LobbySession_RemoveClient(
                session, fromXUID,
                "Host left remove from the lobby and migrate.");
            migrate::LobbyMigrateClient_SetMigrateActive(session, true);
          }
        } else {
          lua::lobby::vm::LobbyVM_LobbyHostLeftEvent(
              controllerIndex, session->type, session->host.maxClients);
        }
      }
    }
  }
#ifndef NDEBUG
  else {
    game::trace("[Lobby][Message][{}] Client received invalid disconnect host "
                "from host ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
  }
#endif
}

// Client <- Host
void HandleHostDisconnectClient(ControllerIndex_t controllerIndex,
                                net::netadr_t *fromAdr, XUID fromXUID,
                                LobbyMsg *lobbyMsg) {
  Msg_LobbyDisconnectClient msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Client received disconnect client "
                "from host ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    session::LobbySession *const session =
        client::LobbyClientData_GetSession(msgData.lobbyType);
    if (session->active != session::SessionActive::INACTIVE) {
      if (session->host.info.xuid == fromXUID &&
          session::LobbySession_GetClientByXuid(
              session, msgData.disconnectClientXuid) != nullptr) {
        lua::lobby::vm::LobbyVM_OnDisconnect(session,
                                             msgData.disconnectClientXuid,
                                             msgData.disconnectClient, nullptr);
      }
    }
  }
#ifndef NDEBUG
  else {
    game::trace(
        "[Lobby][Message][{}] Client received invalid disconnect client "
        "from host ({:016X} @ {}): {}",
        game::serialize(controllerIndex), fromXUID, fromAdr->serialize(),
        msgData.serialize());
  }
#endif
}

// Client <- Host
void HandleHostLeaveWithParty(ControllerIndex_t controllerIndex,
                              net::netadr_t *fromAdr, XUID fromXUID,
                              LobbyMsg *lobbyMsg) {
  Msg_LobbyHostLeaveWithParty msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Client received host leave with party "
                "from host ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    if (!lobby::host::LobbyHost_IsHost(msgData.lobbyType)) {
      session::LobbySession *const session =
          client::LobbyClientData_GetSession(msgData.lobbyType);
      if (!migrate::LobbyMigrateClient_MigrateActive(session) &&
          session->active != session::SessionActive::INACTIVE &&
          session->host.info.xuid == fromXUID) {
        lua::lobby::vm::LobbyVM_OnLeaveWithParty(session);
      }
    }
  }
#ifndef NDEBUG
  else {
    game::trace(
        "[Lobby][Message][{}] Client received invalid host leave with party "
        "from host ({:016X} @ {}): {}",
        game::serialize(controllerIndex), fromXUID, fromAdr->serialize(),
        msgData.serialize());
  }
#endif
}

// Client <- Host
void HandleAgreementRequest(ControllerIndex_t controllerIndex,
                            net::netadr_t *fromAdr, XUID fromXUID,
                            LobbyMsg *lobbyMsg) {
  Msg_JoinAgreementRequest msgData{};

  join::JoinClient &joinClient = (*join::s_joinClient)[controllerIndex];
  if (joinClient.state == join::JoinClientState::IDLE) {
    if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
      game::trace("[Lobby][Message][{}] Client received join agreement request "
                  "from host ({:016X} @ {}): {}. joinClient: {}",
                  game::serialize(controllerIndex), fromXUID,
                  fromAdr->serialize(), msgData.serialize(),
                  joinClient.serialize());
#endif

      joinClient.potentialHost.info.xuid = msgData.hostXuid;
      strscpy(joinClient.potentialHost.info.name, msgData.hostName);
      joinClient.potentialHost.info.serializedAdr.addrBuff =
          msgData.serializedAdr.addrBuff;
      joinClient.potentialHost.info.secKey = msgData.secKey;
      joinClient.potentialHost.info.secId = msgData.secId;
      joinClient.potentialHost.reservationKey = msgData.reservationKey;
      joinClient.sourceLobbyType = msgData.sourceLobbyType;
      joinClient.potentialHost.lobbyType = msgData.destinationLobbyType;
      joinClient.potentialHost.lobbyParams = msgData.destinationLobbyParams;
      joinClient.potentialHost.info.serverLocation = msgData.serverLoc;
      if (join::LobbyJoin_ConnectToHost(&joinClient.potentialHost.info)) {
        joinClient.state = join::JoinClientState::CONNECT_TO_NEXT_HOST;
        join::Client_ChangeState(joinClient.sourceLobbyType);
      } else {
        join::LobbyJoin_DisconnectFromHost(&joinClient.potentialHost.info);
        join::SendDisagree(&joinClient);
      }
    }
#ifndef NDEBUG
    else {
      game::trace(
          "[Lobby][Message][{}] Client received invalid join agreement request "
          "from host ({:016X} @ {}): {}",
          game::serialize(controllerIndex), fromXUID, fromAdr->serialize(),
          msgData.serialize());
    }
#endif
  }
}

// Client <- Host
void HandleJoinComplete(ControllerIndex_t controllerIndex,
                        net::netadr_t *fromAdr, XUID fromXUID,
                        LobbyMsg *lobbyMsg) {

  join::JoinClient &client = (*join::s_joinClient)[controllerIndex];
  Msg_JoinComplete msgData{};

  if (msgData.package(*lobbyMsg) &&
      !client.potentialHost.info.netAdr.is_zero_ip()) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Client received join complete "
                "from host ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    if (msgData.shouldCommit) {
      Msg_JoinMemberInfo memberInfoMsg{};

      client::LobbyClient_GetFixedClientInfo(
          controllerIndex, *net::g_qport,
          &memberInfoMsg.fixedClientInfoMsg.fixedClientInfo);

      client::LobbyClient_GetMutableClientInfo(
          controllerIndex,
          &memberInfoMsg.mutableClientInfoMsg.mutableClientInfo);

      lua::lobby::vm::LobbyVM_OnExperimentReset(controllerIndex);

      const uint32_t serverLocation = client.potentialHost.info.serverLocation;
      // Unsure what this `999` magic number represents.
      if (serverLocation != 999) {
        memberInfoMsg.fixedClientInfoMsg.fixedClientInfo.latencyBand =
            static_cast<uint8_t>(std::min<int32_t>(
                live::qos::Live_Qos_GetPingForDatacenter(
                    live::qos::Live_Qos_GetDatacenterById(serverLocation)),
                static_cast<int32_t>(std::numeric_limits<uint8_t>::max())));
      }

      dw::net::bdCommonAddr *commonAdr = dw::net::dwGetLocalCommonAddr();

      auto commonAddrGuard = gsl_lite::finally([&commonAdr]() {
        if (commonAdr != nullptr && commonAdr->remove_ref() == 0) {
          commonAdr->release(1);
          commonAdr = nullptr;
        }
      });

      if (commonAdr != nullptr) {
        commonAdr->serialize(memberInfoMsg.serializedAdr.addrBuff);
        memberInfoMsg.reservationKey = client.potentialHost.reservationKey;
        memberInfoMsg.targetLobby = client.potentialHost.lobbyType;

        const net::NetChanMsgType_e globalChannel =
            lobby::netchan::LobbyNetChan_GetGlobalChannel(
                LobbyGlobalChannel::JOIN);

        LobbyMsg_SendToAdr<Msg_JoinMemberInfo>(
            controllerIndex, globalChannel, LobbyModule::HOST,
            client.potentialHost.info.netAdr, client.potentialHost.info.xuid,
            memberInfoMsg);

        const eModes newMode = lobby::types::LobbyTypes_MainModeToESessionMode(
            client.potentialHost.lobbyParams.mainMode);

        const LocalClientNum_t localClientNum =
            com::Com_ControllerIndex_GetLocalClientNum(controllerIndex);

        if (com::Com_SessionMode_IsMode(eModes::COUNT)) {
          const eModes currentMode = eModes::COUNT;
          com::Com_SwitchMode(localClientNum, currentMode, newMode, 2u);
        } else if (!com::Com_SessionMode_IsMode(newMode)) {
          const eModes currentMode = com::Com_SessionMode_GetMode();
          com::Com_SwitchMode(localClientNum, currentMode, newMode, 2u);
        }

        client::LobbyClient_Start(0, client.potentialHost.lobbyType,
                                  client.potentialHost.lobbyParams.mainMode,
                                  client.potentialHost.lobbyParams.networkMode,
                                  &client.potentialHost.info);
      }

      dw::net::bdCommonAddr *memberCommonAdr =
          memberInfoMsg.fixedClientInfoMsg.fixedClientInfo.commonAdr.m_ptr;
      if (memberCommonAdr != nullptr && memberCommonAdr->remove_ref() == 0) {

        memberCommonAdr->release(1);
      }
    } else {
      join::LobbyJoin_DisconnectFromHost(&client.potentialHost.info);
    }
  }
#ifndef NDEBUG
  else {
    game::trace("[Lobby][Message][{}] Client received invalid join complete "
                "from host ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
  }
#endif
}

// Client <- Host
void HandleMigrateAnnounceHost(ControllerIndex_t controllerIndex,
                               net::netadr_t *fromAdr, XUID fromXUID,
                               LobbyMsg *lobbyMsg) {
  Msg_LobbyMigrateAnnounceHost msgData{};

  if (msgData.package(*lobbyMsg) && !fromAdr->is_zero_ip()) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Client received migrate announce host "
                "from host ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    migrate::LobbyMigrateClient_HandleHostAnnounce(
        client::LobbyClientData_GetSession(msgData.lobbyType), &msgData,
        fromXUID);
  }
#ifndef NDEBUG
  else {
    game::trace(
        "[Lobby][Message][{}] Client received invalid migrate announce host "
        "from host ({:016X} @ {}): {}",
        game::serialize(controllerIndex), fromXUID, fromAdr->serialize(),
        msgData.serialize());
  }
#endif
}

// Client <- Host
void HandleIngameMigrateTo(ControllerIndex_t controllerIndex,
                           net::netadr_t *fromAdr, XUID fromXUID,
                           LobbyMsg *lobbyMsg) {
  Msg_LobbyIngameMigrateMigrateTo msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Client received in-game migrate to "
                "from host ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif
    migrate::LobbyInGameMigrateClient_HandleMigrateTo(
        controllerIndex, client::LobbyClientData_GetSession(msgData.lobbyType),
        &msgData);
  }
#ifndef NDEBUG
  else {
    game::trace(
        "[Lobby][Message][{}] Client received invalid in-game migrate to "
        "from host ({:016X} @ {}): {}",
        game::serialize(controllerIndex), fromXUID, fromAdr->serialize(),
        msgData.serialize());
  }
#endif
}

// Client <- Host
void HandleIngameMigrateNewHost(ControllerIndex_t controllerIndex,
                                net::netadr_t *fromAdr, XUID fromXUID,
                                LobbyMsg *lobbyMsg) {
  Msg_LobbyIngameMigrateNewHost msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Client received in-game migrate new host "
                "from host ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    migrate::LobbyInGameMigrateClient_HandleSessionInfoMsg(
        controllerIndex, client::LobbyClientData_GetSession(msgData.lobbyType),
        &msgData);
  }

#ifndef NDEBUG
  else {
    game::trace(
        "[Lobby][Message][{}] Client received invalid in-game migrate new host "
        "from host ({:016X} @ {}): {}",
        game::serialize(controllerIndex), fromXUID, fromAdr->serialize(),
        msgData.serialize());
  }
#endif
}

// Client <- Host
void Client_HandleClientContent(ControllerIndex_t controllerIndex,
                                net::netadr_t *fromAdr, XUID fromXUID,
                                LobbyMsg *lobbyMsg) {
  Msg_ClientContent msgData{};

  if (msgData.package(*lobbyMsg)) {
#ifndef NDEBUG
    game::trace("[Lobby][Message][{}] Client received client content "
                "from host ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
#endif

    clientcontent::ClientContentType contentType =
        clientcontent::ClientContentType_GetContentTypeFromDatamask(
            msgData.dataMask);
    com::Com_BlockChecksum32(msgData.clientContentData,
                             msgData.compressedBufferSize, 0);

    session::LobbySession *const session =
        client::LobbyClientData_GetSession(msgData.lobbyType);

    session::SessionClient *const client =
        session::LobbySession_GetClientByXuid(session, msgData.clientXUID);
    if (client != nullptr && client->activeClient != nullptr) {
      clientcontent::ClientContent *const content =
          active::LobbyActiveList_GetClientContent(client->activeClient);

      clientcontent::ClientContent_ClearData(contentType, content);
      clientcontent::ClientContent_SetCompressedDataSize(
          contentType, content, msgData.compressedBufferSize);
      clientcontent::ClientContent_SetData(contentType, content,
                                           msgData.clientContentData,
                                           msgData.compressedBufferSize);
      clientcontent::ClientContent_SetDataCompressed(contentType, content,
                                                     true);

      content->sessionMode = msgData.sessionMode;
      content->gameMode = msgData.gameMode;

      client::LobbyClientMsg_DecompressClientContentDataReceived(
          content, msgData.lobbyType, fromXUID, msgData.clientXUID,
          contentType);

      if (*ui_lobbyDebugVis && ui_lobbyDebugVis->get_int() != 0) {
        session::LobbySession_DirtySet(session, 4);
      }
    }
  }
#ifndef NDEBUG
  else {
    game::trace("[Lobby][Message][{}] Client received invalid client content "
                "from host ({:016X} @ {}): {}",
                game::serialize(controllerIndex), fromXUID,
                fromAdr->serialize(), msgData.serialize());
  }
#endif
}

} // namespace msg
} // namespace lobby
} // namespace game
