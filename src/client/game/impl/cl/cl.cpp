#include <std_include.hpp>
#include "cl.hpp"

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

inline clientConnection_t *
CL_GetLocalClientConnection(LocalClientNum_t LocalClientNum) {
  game::cl::clientConnection_t(*clientConnections)[2] =
      *game::cl::clientConnections;
  game::cl::clientConnection_t *clientConnection =
      &(*clientConnections)[static_cast<int32_t>(LocalClientNum)];

  return clientConnection;
}

void CL_CheckForResend_Impl(game::LocalClientNum_t localClientNum) {
  clientConnection_t *clc = CL_GetLocalClientConnection(localClientNum);

  ControllerIndex_t controllerIndex =
      com::Com_LocalClient_GetControllerIndex(localClientNum);
  connstate_t connectionState =
      cg::clientUIActives->actives[localClientNum].connectionState;

  if (clc->transferBuffer && connectionState != CA_SENDINGDATA) {
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
  case CA_SENDINGDATA:
    shouldWait = (cls->realtime - clc->lastPacketSentTime) < 100;
    break;
  case CA_CONFIRMLOADING:
  case CA_CHALLENGING:
  case CA_CONNECTING:
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
  case CA_CONFIRMLOADING: {
    net::msg::msg_t sb;
    net::msg::MSG_Init(&sb, msgBuffer, sizeof(msgBuffer));
    net::msg::MSG_WriteString(&sb, "cfl");
    net::msg::MSG_WriteInt64(&sb,
                             live::user::LiveUser_GetXuid(controllerIndex));
    net::msg::MSG_WriteShort(&sb, static_cast<int16_t>(clc->qport));

    net::netadr_t toAdr = clc->serverAddress;
    if (!net::NET_OutOfBandData(networkId, &toAdr, sb.data, sb.cursize)) {
      if (!*com::com_errorEntered) {
        com::Com_Error_("q:\\t7\\pc\\code\\src\\client\\cl_main.cpp", 2305, 2,
                        "EXE_DISCONNECTED");
      }
    }
    return;
  }

  case CA_SENDINGDATA: {

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
        com::Com_Error_("q:\\t7\\pc\\code\\src\\client\\cl_main.cpp", 2434, 2,
                        "EXE_DISCONNECTED");
      }
    }
    return;
  }

  case CA_CHALLENGING: {
    const int32_t infoStrLen = 1024;
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
    std::string protocolStr = utils::string::va("%i", 18532);
    info::Info_SetValueForKey(s, "protocol", protocolStr.c_str());
    std::string netfieldchkStr =
        utils::string::va("%i", net::msgcrc::MsgCRC_NetFieldChecksum());
    info::Info_SetValueForKey(s, "netfieldchk", netfieldchkStr.c_str());
    const char *modeAbbrev =
        com::Com_SessionMode_GetAbbreviationForCurrentMode();
    std::string sessionmodeStr =
        modeAbbrev && *modeAbbrev ? utils::string::va("%s", modeAbbrev) : "";
    info::Info_SetValueForKey(s, "sessionmode", sessionmodeStr.c_str());
    std::string challengeStr = utils::string::va("%i", clc->challenge);
    info::Info_SetValueForKey(s, "challenge", challengeStr.c_str());
    std::string invitedStr = utils::string::va(
        "%i", (cg::clientUIActives->actives[localClientNum].flags & 4) != 0);
    info::Info_SetValueForKey(s, "invited", invitedStr.c_str());
    info::Info_SetValueForKey(
        s, "xuid", live::user::LiveUser_GetXuidString(controllerIndex));
    std::string qportStr = utils::string::va("%i", clc->qport);
    info::Info_SetValueForKey(s, "qport", qportStr.c_str());
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

    const int32_t connectPrefixLen = 8; // "connect "
    const int32_t infoStrQuotesLen = 2;

    char dest[infoStrLen + connectPrefixLen + infoStrQuotesLen];

    int32_t writtenLength =
        std::snprintf(dest, sizeof(dest), "connect \"%s\"", s);

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
        com::Com_Error_("q:\\t7\\pc\\code\\src\\client\\cl_main.cpp", 2391, 2,
                        "EXE_DISCONNECTED");
      }
    }
    return;
  }

  case CA_CONNECTING: {
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
    com::Com_Error_("q:\\t7\\pc\\code\\src\\client\\cl_main.cpp", 2441, 1,
                    "CL_CheckForResend: bad connstate");
    return;
  }
  }
}

} // namespace cl
} // namespace game