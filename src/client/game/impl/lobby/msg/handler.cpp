#include <std_include.hpp>

#include "game/impl/lobby/msg/msg.hpp"
#include "game/impl/lobby/reservation.hpp"
#include "game/impl/lobby/serverlist.hpp"
#include "utils/string.hpp"

#include "game/utils.hpp"

#include "handler.hpp"

namespace game {
namespace lobby {
namespace msg {
void HandleAgreementResponse(ControllerIndex_t controllerIndex,
                             net::netadr_t *fromAdr, XUID fromXUID,
                             LobbyMsg *lobbyMsg) {

  Msg_JoinAgreementResponse msgData{};
  if (msgData.package(*lobbyMsg) &&
      join::s_join->state == join::JoinSourceState::WAITING_FOR_AGREEMENT) {
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
        if (game::is_client()) {
          spinner::LobbySpinner_Update(spinner::SpinnerSystem::JOIN_PROGRESS,
                                       6);
        }
        join::s_join->state = join::JoinSourceState::CONNECTION_SUCCESS;
      } else {
        join::s_join->joinResult =
            join::JoinResult::SEND_AGREEMENT_REQUEST_FAILED;
        debug::LobbyDebug_JoinStateChanged(join::s_joinNames->connectionFailed,
                                           "clients disagree");
        join::LobbyTaskUpdate(join::s_join.get());
        join::s_join->state = join::JoinSourceState::CONNECTION_FAILED;
      }
      join::ChangeState(join::s_join->sourceLobbyType);

      Agreement &agreement = join::s_join->agreement;

      Agreement &debugAgreement = join::s_join->debugAgreement;

      debugAgreement = agreement;
      agreement = {};
      debug::LobbyDebug_AgreementCount(0);
    }
  }
}
void LobbyPeerToPeer_HandleConnectivityTest(ControllerIndex_t controllerIndex,
                                            net::netadr_t *fromAdr,
                                            XUID fromXUID, LobbyMsg *lobbyMsg) {
  Msg_ConnectionTest msgData{};
  if (msgData.package(*lobbyMsg)) {
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
}
void HandleDemoState(ControllerIndex_t controllerIndex, net::netadr_t *fromAdr,
                     XUID fromXUID, LobbyMsg *lobbyMsg) {
  Msg_DemoState msgData{};

  if (msgData.package(*lobbyMsg)) {
    demo::Demo_ReadHeartbeat(
        com::Com_ControllerIndex_GetLocalClientNum(controllerIndex), &msgData);
  }
}

void HandleMemberInfo(ControllerIndex_t controllerIndex, net::netadr_t *fromAdr,
                      XUID fromXUID, LobbyMsg *lobbyMsg) {
  Msg_JoinMemberInfo msgData{};
  if (msgData.package(*lobbyMsg)) {
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
      host::LobbyHost_AddJoiningClient(controllerIndex, fromAdr, joinNonce,
                                       &msgData, probedXuid);
      // Join completed
      reservation::ReservationRemove(
          *reservation::s_reservation,
          msgData.fixedClientInfoMsg.fixedClientInfo.xuid);
    }
  }
}
void HandleServerInfo(ControllerIndex_t controllerIndex, net::netadr_t *fromAdr,
                      XUID fromXUID, LobbyMsg *lobbyMsg) {

  serverlist::LobbyServerList &serverList = *serverlist::s_lobbyServerList;
  if (com::Com_IsRunningUILevel() &&
      serverList.count < std::size(serverList.serverInfo)) {

    Msg_InfoResponseLobbyServer &msgData =
        serverList.serverInfo[serverList.count];
    msgData = {};
    if (msgData.package(*lobbyMsg)) {
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
  }
}
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
    session::LobbySession *const session =
        LobbyHostData_GetSession(msgData.targetLobby);
    const LobbyType lobbyType = session->type;
    const LobbyMainMode mainMode = base::LobbyBase_GetMainMode();
    const LobbyNetworkMode networkMode = base::LobbyBase_GetNetworkMode();

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

    bool joinable = host::LobbyHost_IsJoinable(lobbyType, isXUIDLocalPlayer,
                                               &joinResponse.response);
    bool privacyAllowsJoin = host::LobbyHost_PartyPrivacyCheck(
        activeControllerIndex, lobbyType,
        static_cast<const join::JoinType>(msgData.joinType), fromXUID,
        &joinResponse);

    bool skipFurtherChecks = false;

    if (joinable && privacyAllowsJoin) {
      if (ugc::UGC_ActiveMod_Loaded() || isXUIDLocalPlayer ||
          base::LobbyBase_GetNetworkMode() == LobbyNetworkMode::LIVE) {
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
              joinResponse.response = host::party::LobbyHost_IsPartyAllowed(
                  activeControllerIndex, lobbyType, msgData.memberCount);
              if (joinResponse.response == join::JoinResult::SUCCESS) {
                joinResponse.response = host::LobbyHost_CanFit(
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
}

void HandleP2PVoicePacket(ControllerIndex_t controllerIndex,
                          net::netadr_t *fromAdr, XUID fromXUID,
                          LobbyMsg *lobbyMsg) {
  Msg_VoicePacket msgData{};

  if (msgData.package(*lobbyMsg)) {
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
}
void HandleHostClientVoicePacket(ControllerIndex_t controllerIndex,
                                 net::netadr_t *fromAdr, XUID fromXUID,
                                 LobbyMsg *lobbyMsg) {
  Msg_VoicePacket msgData{};

  if (msgData.package(*lobbyMsg)) {
    voice::LobbyVoice_HandleVoicePacket(
        controllerIndex, LobbyHostData_GetSession(msgData.lobbyType), &msgData);
  }
}
void HandleJoinResponse(ControllerIndex_t controllerIndex,
                        net::netadr_t *fromAdr, XUID fromXUID,
                        LobbyMsg *lobbyMsg) {
  const char *failReason = nullptr;
  Msg_JoinResponse msgData{};
  if (msgData.package(*lobbyMsg) &&
      join::s_join->state == join::JoinSourceState::HANDSHAKING) {
    if (net::NetAdr_Compare(fromAdr,
                            &join::s_join->potentialHost.info.netAdr)) {
      if (msgData.lobbyType > LobbyType::GAME) {
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
        case join::JoinResult::SUCCESS:
          if (join::LobbyHostMsg_SendAgreementRequest(
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
            if (game::is_client()) {
              spinner::LobbySpinner_Update(
                  spinner::SpinnerSystem::JOIN_PROGRESS, 4);
            }
            join::s_join->state = join::JoinSourceState::WAITING_FOR_AGREEMENT;
            join::ChangeState(join::s_join->sourceLobbyType);
          }
          join::s_join->joinResult =
              join::JoinResult::SEND_AGREEMENT_REQUEST_FAILED;
          failReason = "MENU_JOIN_RESULT_NOT_JOINABLE";
          break;
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
        case join::JoinResult::COULD_NOT_RESERVE:
          if (join::s_join->potentialHost.retryCount <
              get_join_retryCount().value_or(0)) {
            ++join::s_join->potentialHost.retryCount;
            join::s_join->potentialHost.retryTime =
                sys::Sys_Milliseconds() + get_join_retryDelay().value_or(0);
            debug::LobbyDebug_JoinStateChanged(
                join::s_joinNames->associating,
                "host is temporarily unavailable but may be active "
                "later. retrying..");
            if (game::is_client()) {
              spinner::LobbySpinner_Update(
                  spinner::SpinnerSystem::JOIN_PROGRESS, 2);
            }
            join::s_join->state = join::JoinSourceState::ASSOCIATING;
            join::ChangeState(join::s_join->sourceLobbyType);
          }
          failReason = "MENU_JOIN_RESULT_COULD_NOT_RESERVE";
          break;
        default:
          failReason = "MENU_JOIN_RESULT_NOT_JOINABLE";
          break;
        }
      }
      debug::LobbyDebug_JoinStateChanged(join::s_joinNames->connectionFailed,
                                         failReason);
      join::LobbyTaskUpdate(join::s_join.get());
      join::s_join->state = join::JoinSourceState::CONNECTION_FAILED;
      join::ChangeState(join::s_join->sourceLobbyType);
    }
  }
}
void HandleMigrateStart(ControllerIndex_t controllerIndex,
                        net::netadr_t *fromAdr, XUID fromXUID,
                        LobbyMsg *lobbyMsg) {
  Msg_LobbyMigrateStart msgData{};

  if (msgData.package(*lobbyMsg)) {
    session::LobbySession *const session =
        client::LobbyClient_GetSession(msgData.lobbyType);
    if (msgData.migrateTo == 0) {
      migrate::LobbyMigrate_HandleFindBestCommand(session, &msgData);
    } else {
      migrate::LobbyMigrate_HandleLobbyMigrateTo(session, msgData.migrateTo);
    }
  }
}

void HandleClientContent(ControllerIndex_t controllerIndex,
                         net::netadr_t *fromAdr, XUID fromXUID,
                         LobbyMsg *lobbyMsg) {
  Msg_ClientContent msgData{};

  if (msgData.package(*lobbyMsg)) {
    LobbyType lobbyType = msgData.lobbyType;
    const clientcontent::ClientContentType contentType =
        clientcontent::ClientContentType_GetContentTypeFromDatamask(
            msgData.dataMask);

    if (LobbyHost_IsHost(lobbyType)) {
      session::LobbySession *const session =
          LobbyHostData_GetSession(lobbyType);
      session::SessionClient *const sessionLobbyList =
          host::LobbyHostData_GetSessionLobbyList(lobbyType);
      session::SessionClient *const clientByXuid =
          session::LobbySession_GetClientByXuid(session, fromXUID);

      if (clientByXuid != nullptr) {
        active::ActiveClient *activeClient = clientByXuid->activeClient;
        clientcontent::ClientContent *clientContent =
            active::LobbyActiveList_GetClientContent(activeClient);
        const uint32_t currentCACPSBufferCrc = com::Com_BlockChecksum32(
            msgData.clientContentData, sizeof(msgData.clientContentData), 0);

        if (clientcontent::ClientContent_GetChecksum(
                contentType, clientContent) != currentCACPSBufferCrc) {
          if (com::Com_IsInGame()) {
            active::LobbyActiveList_ClearClientContentForClient(
                activeClient, contentType, fromXUID);
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
              clientcontent::ClientContentTracking *contentTrackingByIndex =
                  active::LobbyActiveList_GetClientContentTrackingByIndex(
                      activeClient, contentType, trackingIndex);
              ++trackingIndex;
              contentTrackingByIndex->state =
                  clientcontent::ClientContentTransferState::DIRTY;

              contentTrackingByIndex->toXUID = currentClient->xuid;
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
}
void HandleModifiedStats(ControllerIndex_t controllerIndex,
                         net::netadr_t *fromAdr, XUID fromXUID,
                         LobbyMsg *lobbyMsg) {

  Msg_ModifiedStats msgData{};

  if (msgData.package(*lobbyMsg)) {
    sv::SV_ReceiveModifiedStats(fromXUID, msgData.statsBuffer,
                                msgData.statsSize);
  }
}
void HandleClientDisconnect(ControllerIndex_t controllerIndex,
                            net::netadr_t *fromAdr, XUID fromXUID,
                            LobbyMsg *lobbyMsg) {
  Msg_LobbyClientDisconnect msgData{};

  if (msgData.package(*lobbyMsg)) {
    session::LobbySession *const session =
        LobbyHostData_GetSession(msgData.lobbyType);
    session::SessionClient *const client =
        session::LobbySession_GetClientByXuid(session, msgData.xuid);
    if (client != nullptr && client->activeClient != nullptr) {
      host::LobbyHost_RemoveClient(msgData.lobbyType, msgData.xuid,
                                   "received client disconnect message");
    }
  }
}
void HandlePeerToPeerUpdate(ControllerIndex_t controllerIndex,
                            net::netadr_t *fromAdr, XUID fromXUID,
                            LobbyMsg *lobbyMsg) {
  Msg_PeerToPeerInfo msgData{};

  if (msgData.package(*lobbyMsg)) {
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
}
void HandleClientReliableData(ControllerIndex_t controllerIndex,
                              net::netadr_t *adr, XUID fromXUID,
                              LobbyMsg *lobbyMsg) {
  Msg_ClientReliableData msgData{};

  if (msgData.package(*lobbyMsg)) {
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
        host::LobbyHostMsg_SendDisconnectClient(
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

            const bool hasVoteFlag = (msgData.dataMask & (1ul << 24)) != 0;
            const bool hasCharacterSelection =
                (msgData.dataMask & (1ul << 9)) != 0;
            const bool isByteDataZero = (msgData.dataMask & 0xFFul) == 0;

            if (((msgData.dataMask & (1ul << 8)) == 0 &&
                 !hasCharacterSelection && isByteDataZero) ||
                (clientData.pregameState ==
                     LobbyClientPregameState::SELECTION_FORCED ||
                 clientData.pregameState ==
                     LobbyClientPregameState::SELECTION_COMPLETE)) {
              session::LobbySession_DirtySet(session, 1);
            } else {

              VoteType itemVoteType = VoteType::INVALID;
              switch (msgData.itemVote.itemType) {
              case VoteItemType::ITEM:
                itemVoteType =
                    data::LobbyData_GetItemVoteType(msgData.itemVote.itemIndex);
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

              if (!validate(itemVoteType)) {
                const uint32_t maxVotes =
                    com::gts::Com_GametypeSettings_GetUInt(
                        "pregameItemMaxVotes", false);
                if (clientData.itemVoteCount < maxVotes &&
                    clientData.itemVoteCount < 12) {
                  if (msgData.itemVote.voteType != VoteType::INVALID) {
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

              clientData.pregameState = LobbyClientPregameState::SELECTED;

              if (hasCharacterSelection) {
                clientData.characterSelection = msgData.characterSelection;
              }

              if (isByteDataZero) {
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

                  if (hasVoteFlag) {
                    lua::Lua_SetTableInt("voteIndex", 0, luaVM._vm);
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
}
void HandleLobbyClientHeartbeat(ControllerIndex_t controllerIndex,
                                net::netadr_t *fromAdr, XUID fromXUID,
                                LobbyMsg *lobbyMsg) {
  Msg_LobbyClientHeartbeat msgData{};

  if (msgData.package(*lobbyMsg)) {
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
}
void HandleMigrateBandwidthTest(ControllerIndex_t controllerIndex,
                                net::netadr_t *fromAdr, XUID fromXUID,
                                LobbyMsg *lobbyMsg) {
  Msg_LobbyMigrateBandwidthTest msgData{};

  if (msgData.package(*lobbyMsg)) {
    session::LobbySession *const session =
        client::LobbyClient_GetSession(msgData.lobbyType);
    migrate::LobbyMigrateClient_HandleTestPacket(
        controllerIndex, session, fromAdr, msgData.peerXuid, &msgData);
  }
}

} // namespace msg
} // namespace lobby
} // namespace game
