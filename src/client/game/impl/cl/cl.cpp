#include "cl.hpp"
#include <cstring>
#include <cstdio>

#include "../../utils.hpp"
#include "../cg/cg.hpp"
#include "../../../component/auth.hpp"
#include "../../../../common/utils/string.hpp"

namespace game {
namespace cl {

int32_t CL_HighestPriorityStatPacket_Impl(clientConnection_t *clc,
                                          int32_t numCompressedStatsPackets) {
  int32_t realtime = cls->realtime;
  int32_t oldestPacket = -1;

  if (numCompressedStatsPackets > 0) {
    for (int32_t i = 0; i < numCompressedStatsPackets; ++i) {
      bool isSet = false;

      if (i >= 64) {
        isSet = (clc->statPacketsToSend[1] & (1ULL << (i - 64))) != 0;
      } else {
        isSet = (clc->statPacketsToSend[0] & (1ULL << i)) != 0;
      }

      if (isSet) {
        if (clc->statPacketSendTime[i] == 0) {
          return i; // Send immediately
        }

        if (clc->statPacketSendTime[i] < realtime) {
          realtime = clc->statPacketSendTime[i];
          oldestPacket = i;
        }
      }
    }
  }

  return oldestPacket;
}

clientConnection_t *
CL_GetLocalClientConnection(LocalClientNum_t localClientNum) {
  return &(*cl::clientConnections)->connections[localClientNum];
}

void CL_CheckForResend_Impl(game::LocalClientNum_t localClientNum) {
  clientConnection_t *clc = CL_GetLocalClientConnection(localClientNum);

  ControllerIndex_t controllerIndex =
      com::Com_LocalClient_GetControllerIndex(localClientNum);
  connstate_t connectionState =
      cg::clientUIActives->actives[localClientNum].connectionState;

  if (clc->transferBuffer && connectionState != connstate_t::SENDINGDATA) {
    com::Com_LiveDeallocate(live::LiveAllocTypes::LIVE_ALLOC_STORAGE,
                            static_cast<void *>(clc->transferBuffer));
    clc->transferBuffer = nullptr;
  }

  if (clc->loadoutNonceReceived < clc->loadoutNonceSent &&
      sys::Sys_Milliseconds() > clc->loadoutNextRetry) {
    CL_LoadoutChanged(localClientNum, true);
  }

  // Determine if we need to throttle packet resends
  bool shouldWait = false;
  switch (connectionState) {
  case connstate_t::SENDINGDATA:
    shouldWait = (cls->realtime - clc->lastPacketSentTime) < 100;
    break;
  case connstate_t::CONFIRMLOADING:
  case connstate_t::CHALLENGING:
  case connstate_t::CONNECTING:
    shouldWait = (cls->realtime - clc->connectTime) < 3000;
    break;
  default:
    return;
  }

  if (shouldWait || demo::Demo_IsPlaying()) {
    return;
  }

  clc->connectTime = cls->realtime;
  if (db::DB_GetLoadedFraction() >= 1.0f) {
    clc->connectPacketCount++;
  }

  net::netsrc_t networkId = com::Com_LocalClient_GetNetworkID(localClientNum);
  /*
      We can also instead use the original buffer in the engine:
      0x14573A1E0 ; min_msg_buf CL_CheckForResend(LocalClientNum_t)::msgBuffer
  */
  static net::msg::min_msg_buf msgBuffer;

  switch (connectionState) {
  case connstate_t::CONFIRMLOADING: {
    net::msg::msg_t sb;
    net::msg::MSG_Init(&sb, msgBuffer, sizeof(msgBuffer));
    net::msg::MSG_WriteString(&sb, "cfl");
    net::msg::MSG_WriteInt64(&sb,
                             live::user::LiveUser_GetXuid(controllerIndex));
    net::msg::MSG_WriteShort(&sb, static_cast<int16_t>(clc->qport));

    net::netadr_t toAdr = clc->serverAddress;
    if (!net::NET_OutOfBandData(networkId, &toAdr, sb.data, sb.cursize)) {
      if (!*com::com_errorEntered) {
        com::Com_Error_("q:\\t7\\pc\\code\\src\\client\\cl_main.cpp", 2305,
                        errorParm::SERVERDISCONNECT, "EXE_DISCONNECTED");
      }
    }
    return;
  }

  case connstate_t::SENDINGDATA: {

    net::msg::msg_t sb;
    net::msg::MSG_Init(&sb, msgBuffer, sizeof(msgBuffer));
    net::msg::MSG_WriteString(&sb, "stats");

    if (!clc->transferBuffer) {
      clc->transferBuffer = static_cast<uint8_t *>(com::Com_LiveRealloc(
          live::LiveAllocTypes::LIVE_ALLOC_STORAGE,
          static_cast<void *>(clc->transferBuffer), 0x25800));
      clc->transferBufferCompressedSize =
          live::storage::Storage_SerializeTransferData(
              controllerIndex, clc->transferBuffer, 0x25800);
    }

    int32_t numPackets = (clc->transferBufferCompressedSize + 1216) / 1216;
    int32_t oldestPacketIndex =
        CL_HighestPriorityStatPacket_Impl(clc, numPackets);

    uint8_t *packetData = &clc->transferBuffer[1216 * oldestPacketIndex];
    net::msg::MSG_WriteShort(&sb, static_cast<int16_t>(clc->qport));
    net::msg::MSG_WriteByte(&sb, static_cast<int8_t>(oldestPacketIndex));
    net::msg::MSG_WriteByte(&sb, static_cast<int8_t>(numPackets));

    int32_t sizeToWrite = 1216;
    if (0x25800 - (1216 * oldestPacketIndex) <= 1216) {
      sizeToWrite = 0x25800 - (1216 * oldestPacketIndex);
    }
    net::msg::MSG_WriteData(&sb, packetData, sizeToWrite);

    clc->statPacketSendTime[oldestPacketIndex] = cls->realtime;
    clc->lastPacketSentTime = cls->realtime;

    net::netadr_t serverAddress = clc->serverAddress;
    if (!net::NET_OutOfBandData(networkId, &serverAddress, sb.data,
                                sb.cursize)) {
      if (!*com::com_errorEntered) {
        com::Com_Error_("q:\\t7\\pc\\code\\src\\client\\cl_main.cpp", 2434,
                        errorParm::SERVERDISCONNECT, "EXE_DISCONNECTED");
      }
    }
    return;
  }

  case connstate_t::CONNECTING: {
    constexpr int32_t infoStrLen = 1024;
    char s[infoStrLen];
    std::memset(s, 0, sizeof(s));

    /*
       TODO: why does this case cause the function to hang indefinitely?

       We likely do not need this anyway, as double XP events are not currently
       used in the boiii client; temporary removal should not present an issue.
       However, for the sake of equivalency, this should be uncommented as soon
       as this issue is investigated and fixed.
    */
    // if (Dvar_GetBool(*testmissingcontentpacks)) {
    //   uint32_t oldMask = *live::g_oldDoubleXPMask[controllerIndex];
    //   if (oldMask) {
    //     uint32_t newMask =
    //         live::Live_DoubleXPGetGroupMaskForController(controllerIndex);
    //     if (oldMask != newMask) {
    //       dw::bdRemoteTaskRef eventObj;
    //       dw::dwRecordEventFormat(
    //           &eventObj, controllerIndex,
    //           dw::RecordEventType::DW_EVENT_DOUBLE_XP_FLAG_CHANGED,
    //           "Client sending new DoubleXPMask. old XP Mask: "
    //           "%i New XP Mask: %i",
    //           oldMask, newMask);
    //
    //       if (eventObj.m_ptr) {
    //
    //         if (InterlockedDecrement(&eventObj.m_ptr->m_refCount) == 1) {
    //           (*eventObj.m_ptr->releaseFunc)(eventObj.m_ptr, 1);
    //         }
    //         eventObj.m_ptr = nullptr;
    //       }
    //     }
    //   }
    // }

    info::Info_SetValueForKey(
        s, "name",
        utils::string::va("%s",
                          live::user::LiveUser_GetClientName(controllerIndex)));
    info::Info_SetValueForKey(s, "protocol", utils::string::va("%i", 18532));
    info::Info_SetValueForKey(
        s, "netfieldchk",
        utils::string::va("%i", net::msgcrc::MsgCRC_NetFieldChecksum()));
    const char *modeAbbrev =
        com::Com_SessionMode_GetAbbreviationForCurrentMode();
    info::Info_SetValueForKey(s, "sessionmode",
                              modeAbbrev && *modeAbbrev ? modeAbbrev : "");
    info::Info_SetValueForKey(s, "challenge",
                              utils::string::va("%i", clc->challenge));
    info::Info_SetValueForKey(
        s, "invited",
        utils::string::va(
            "%i",
            (cg::clientUIActives->actives[localClientNum].flags & 4) != 0));
    info::Info_SetValueForKey(
        s, "xuid", live::user::LiveUser_GetXuidString(controllerIndex));
    info::Info_SetValueForKey(s, "qport", utils::string::va("%i", clc->qport));
    info::Info_SetValueForKey(
        s, "migrating",
        cg::clientUIActives->actives[localClientNum].migrationState ==
                cg::clientMigState_t::CMSTATE_INACTIVE
            ? "0"
            : "1");

    // BEGIN boiii-specific fields
    if (auth::password && *auth::password->current.value.string) {
      info::Info_SetValueForKey(s, "password",
                                auth::password->current.value.string);
    }

    const char *clan_abbrev = game::live::LiveStats_GetClanTagText(0);
    if (clan_abbrev && *clan_abbrev) {
      info::Info_SetValueForKey(s, "clanAbbrev", clan_abbrev);
    }
    // END boiii-specific fields

    constexpr int32_t connectPrefixLen = 8; // "connect "
    constexpr int32_t infoStrQuotesLen = 2;
    constexpr int32_t destLen =
        infoStrLen + connectPrefixLen + infoStrQuotesLen;
    char dest[destLen];

    int32_t writtenLength = std::snprintf(dest, destLen, "connect \"%s\"", s);

    net::netadr_t serverAddress = clc->serverAddress;
    // ORIGINAL:
    // if (!net::NET_OutOfBandData(networkId, &serverAddress,
    //                             reinterpret_cast<const uint8_t *>(dest),
    //                             writtenLength)) {
    // PATCHED:
    if (!auth::send_fragmented_connect_packet(
            controllerIndex, networkId, &serverAddress,
            reinterpret_cast<const char *>(dest), writtenLength)) {
      if (!*com::com_errorEntered) {
        com::Com_Error_("q:\\t7\\pc\\code\\src\\client\\cl_main.cpp", 2391,
                        errorParm::SERVERDISCONNECT, "EXE_DISCONNECTED");
      }
    }
    return;
  }

  case connstate_t::CHALLENGING: {
    if (dw::dwGetConnectionTaskStatus(&clc->serverAddress) !=
        dw::taskCompleteResults::TASK_NOTCOMPLETE) {
      net::netadr_t adr = clc->serverAddress;
      // ORIGINAL:
      // net::NET_OutOfBandPrint(networkId, &adr, "getchallenge");
      /*
          # PATCH

          ## Problem Statement

          Generated challenges are generated and stored per requesting IP
         address, not per `netadr_t`. As such, if a local client requests a
         challenge, receives and stores the generated challenge, but does not
         attempt connection using this challenge prior to another local client
         requesting a new challenge, the challenge stored for the first local
         client's address will be overwritten server-side. When the first client
         attempts connection with the signature generated with this challenge,
         the server will attempt and fail verification of the signature, as it
         is verifying against the challenge generated for the second local
         client's request.

          ## Solution

          Request a challenge once per server connection, and reuse it for all
         connecting, local clients.
      */

      if (localClientNum == game::LOCAL_CLIENT_0) {
        net::NET_OutOfBandPrint(networkId, &adr, "getchallenge");
      }
    }
    return;
  }

  default: {
    com::Com_Error_("q:\\t7\\pc\\code\\src\\client\\cl_main.cpp", 2441,
                    errorParm::DROP, "CL_CheckForResend: bad connstate");
    return;
  }
  }
}

void CL_AllocateClientMemory_Impl(hunk::HunkUser *hunk,
                                  LocalClientNum_t maxLocalClients,
                                  ClientNum_t maxClients,
                                  clientAllocFlags_t flags) {
  int32_t maxClientSnapshots = CL_SNAPSHOT_COUNT * maxClients;

  *clients = reinterpret_cast<ClientActives *>(hunk::Hunk_UserAlloc(
      hunk, sizeof(clientActive_t) * maxLocalClients, 0x10, "clients"));

  *clientConnections = reinterpret_cast<ClientConnections *>(
      hunk::Hunk_UserAlloc(hunk, sizeof(clientConnection_t) * maxLocalClients,
                           0x10, "clientConnections"));

  for (game::LocalClientNum_t localClientNum = game::LOCAL_CLIENT_0;
       localClientNum < maxLocalClients; ++localClientNum) {
    clOutPacketsPool_t *outPackets =
        reinterpret_cast<clOutPacketsPool_t *>(hunk::Hunk_UserAlloc(
            hunk, sizeof(clOutPacketsPool_t), 0x10, "clients.outPackets"));
    clSnapshotPool_t *snapshots =
        reinterpret_cast<clSnapshotPool_t *>(hunk::Hunk_UserAlloc(
            hunk, sizeof(clSnapshotPool_t), 0x10, "clients.snapshots"));
    clEntityStatePool_t *parseEntitiesBuf =
        reinterpret_cast<clEntityStatePool_t *>(hunk::Hunk_UserAlloc(
            hunk, sizeof(clEntityStatePool_t), 0x10, "clients.parseEntities"));
    clClientStates_t *parseClientsBuf = reinterpret_cast<clClientStates_t *>(
        hunk::Hunk_UserAlloc(hunk, sizeof(clClientStates_t) * maxClients, 0x10,
                             "clients.parseClients"));
    clActorStatePool_t *parseActorsBuf =
        reinterpret_cast<clActorStatePool_t *>(hunk::Hunk_UserAlloc(
            hunk, sizeof(clActorStatePool_t), 0x10, "clients.parseActors"));
    clMatchStatePool_t *parseMatchStatesBuf =
        reinterpret_cast<clMatchStatePool_t *>(
            hunk::Hunk_UserAlloc(hunk, sizeof(clMatchStatePool_t), 0x10,
                                 "clients.parseMatchStates"));
    clCasterStatePool_t *parseCasterStatesBuf =
        reinterpret_cast<clCasterStatePool_t *>(
            hunk::Hunk_UserAlloc(hunk, sizeof(clCasterStatePool_t), 0x10,
                                 "clients.parseCasterStates"));
    clCasterClientStatePool_t *parseCasterClientsBuf =
        reinterpret_cast<clCasterClientStatePool_t *>(
            hunk::Hunk_UserAlloc(hunk, sizeof(clCasterClientStatePool_t), 0x10,
                                 "clients.parseCasterClientStates"));

    if (!flags.dryRun) {
      clientActive_t *client = &(*clients)->actives[localClientNum];

      // Assign buffers and capacities
      client->parseCasterStatesBuf = parseCasterStatesBuf;
      client->maxParseCasterStates = CL_SNAPSHOT_COUNT;
      client->numParseCasterStates = CL_SNAPSHOT_COUNT;

      client->parseActorsBuf = parseActorsBuf;
      client->maxParseActors = CL_ACTOR_COUNT;
      client->numParseActors = CL_ACTOR_COUNT;

      client->outPackets = outPackets;
      client->snapshots = snapshots;
      client->packetBackupCount = CL_SNAPSHOT_COUNT;
      client->packetBackupMask = PACKET_BACKUP_MASK;

      client->parseEntitiesBuf = parseEntitiesBuf;
      client->maxParseEntities = CL_ENTITY_COUNT;
      client->numParseEntities = CL_ENTITY_COUNT;

      client->parseClientsBuf = parseClientsBuf;
      client->maxParseClients = maxClientSnapshots;
      client->numParseClients = maxClientSnapshots;

      client->parseMatchStatesBuf = parseMatchStatesBuf;
      client->maxParseMatchStates = CL_SNAPSHOT_COUNT;
      client->numParseMatchStates = CL_SNAPSHOT_COUNT;

      client->parseCasterClientsBuf = parseCasterClientsBuf;
      client->maxParseCasterClients = maxClientSnapshots;
      client->numParseCasterClients = maxClientSnapshots;

      // Initialize connection transfer buffers
      clientConnection_t *connection =
          CL_GetLocalClientConnection(localClientNum);
      connection->transferBuffer = nullptr;
      connection->transferBufferCompressedSize = 0;
    }
  }

  if (!flags.dryRun) {
    memset(cls_gamestate, 0, sizeof(gameState_t));
    *cls_serverId = 0;
    for (game::LocalClientNum_t localClientNum = game::LOCAL_CLIENT_0;
         localClientNum < game::LOCAL_CLIENT_COUNT; ++localClientNum) {
      cg::clientUIActives->actives[localClientNum].connectionState =
          connstate_t::DISCONNECTED;
    }
  }
}

void AllocatePerLocalClientMemory_Impl(LocalClientNum_t maxLocalClients,
                                       ClientNum_t maxClients,
                                       clientAllocFlags_t flags) {

  fx::FX_ShutdownLensFlareSystem();
  CL_FreePerLocalClientMemory(true);
  flags.dryRun = 1;

  hunk::HunkUserNull nullUser;
  hunk::HunkUser *user = hunk::Hunk_UserCreateNull(&nullUser);
  cg::CG_AllocateClientMemory_Impl(user, maxLocalClients);
  fx::FX_AllocateClientMemory(user, maxLocalClients, maxClients, flags);
  CL_AllocateClientMemory_Impl(user, maxLocalClients, maxClients, flags);
  Checkpoint_Init(user, flags);
  int32_t clientAlignment = (std::max)(nullUser.alignment, 4);
  int32_t clientSize = nullUser.size + 1024;
  pmem::PMem_BeginAlloc(*pmem::PerLocalClientMemoryName, PMemStack::GAME,
                        EMemTrack::CLIENT);

  void *localClientHunkBuf = pmem::_PMem_Alloc(
      clientSize, clientAlignment, PMemPool::MAIN, PMemStack::GAME, 0,
      EMemTrack::CLIENT, "q:\\t7\\code\\src\\client_mp\\cl_main_mp.cpp", 0);
  pmem::PMem_EndAlloc(*pmem::PerLocalClientMemoryName, PMemStack::GAME);
  *hunk::s_localClientHunk = hunk::Hunk_UserCreateFromBuffer(
      localClientHunkBuf, static_cast<size_t>(clientSize),
      hunk::HU_ALLOCATION_SCHEME::HU_SCHEME_DEFAULT, 8u, nullptr,
      "clientOnlyHunk", 0x1A);
  hunk::Hunk_UserDefaultReset(*hunk::s_localClientHunk);
  flags.dryRun = 0;
  cg::CG_AllocateClientMemory_Impl(*hunk::s_localClientHunk, maxLocalClients);
  fx::FX_AllocateClientMemory(*hunk::s_localClientHunk, maxLocalClients,
                              maxClients, flags);
  CL_AllocateClientMemory_Impl(*hunk::s_localClientHunk, maxLocalClients,
                               maxClients, flags);
  Checkpoint_Init(*hunk::s_localClientHunk, flags);
  *cl_maxLocalClients = maxLocalClients;
  *cl_allocatedClients = maxClients;
  *cl_lastAllocFlags = flags;
}

} // namespace cl
} // namespace game