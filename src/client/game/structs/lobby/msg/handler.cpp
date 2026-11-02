#include <std_include.hpp>
#include <utility>

#include "game/game.hpp"

namespace game {
namespace lobby {

namespace msg {

bool MsgMutableClientInfo::package(LobbyMsg &lobbyMsg) {
  bool result =
      lobbyMsg.package_uint64("lobbyid", &mutableClientInfo.lobbyID) &&
      lobbyMsg.package_string(
          "clantag", mutableClientInfo.clantag,
          static_cast<int32_t>(std::size(mutableClientInfo.clantag)));

  for (size_t progIdx = 3; progIdx > 0; --progIdx) {
    ClientProgression &prog = mutableClientInfo.progression[progIdx];
    result &= lobbyMsg.package_uchar("rank", &prog.rank) &&
              lobbyMsg.package_uchar("prestige", &prog.prestige) &&
              lobbyMsg.package_ushort("paragonRank", &prog.paragonRank) &&
              lobbyMsg.package_uchar("paragonIconId", &prog.paragonIconId) &&
              lobbyMsg.package_uchar(
                  "chunk", reinterpret_cast<uint8_t *>(
                               &mutableClientInfo.chunkStatuses[progIdx]));
  }
  return result &&
         lobbyMsg.package_bool("isStarterPack",
                               &mutableClientInfo.isStarterPack) &&
         lobbyMsg.package_int("arenapoints", &mutableClientInfo.arenaPoints) &&
         lobbyMsg.package_float("arenaskill", &mutableClientInfo.arenaSkill) &&
         lobbyMsg.package_float("arenavar", &mutableClientInfo.arenaVariance) &&
         lobbyMsg.package_glob(
             "bubblegumbuffs", &mutableClientInfo.bubbleGumInfo.bubbleGumBuffs,
             sizeof(mutableClientInfo.bubbleGumInfo.bubbleGumBuffs)) &&
         lobbyMsg.package_glob(

             "bgbcount",
             mutableClientInfo.bubbleGumInfo.bubbleGumBuffConsumableCount,
             sizeof(mutableClientInfo.bubbleGumInfo
                        .bubbleGumBuffConsumableCount)) &&
         lobbyMsg.package_uchar(
             "chartype", &mutableClientInfo.heroInfo.selectedCharacterType) &&
         lobbyMsg.package_uchar("headtype",
                                &mutableClientInfo.heroInfo.selectedHeadType) &&
         lobbyMsg.package_uchar(
             "charmode", &mutableClientInfo.heroInfo.selectedCharacterMode) &&
         lobbyMsg.package_uchar(

             "charloadoutslot",
             &mutableClientInfo.heroInfo.selectedLoadoutSlot) &&
         lobbyMsg.package_glob(

             "charitem", mutableClientInfo.heroInfo.selectedCharacterItem) &&
         lobbyMsg.package_glob(

             "charitemcolors",
             mutableClientInfo.heroInfo.selectedCharacterItemColors,
             sizeof(mutableClientInfo.heroInfo.selectedCharacterItemColors)) &&
         lobbyMsg.package_glob(

             "attachment", mutableClientInfo.showcaseWeapon.attachment,
             sizeof(mutableClientInfo.showcaseWeapon.attachment)) &&
         lobbyMsg.package_glob(

             "attachmentvar",
             mutableClientInfo.showcaseWeapon.attachmentVariant) &&
         lobbyMsg.package_uchar("camoindex",
                                &mutableClientInfo.showcaseWeapon.camoIndex) &&
         lobbyMsg.package_int("paintjobslot",
                              &mutableClientInfo.showcaseWeapon.paintjobSlot) &&
         lobbyMsg.package_int(
             "paintjobindex",
             &mutableClientInfo.showcaseWeapon.paintjobIndex) &&
         lobbyMsg.package_int("weaponindex",
                              &mutableClientInfo.showcaseWeapon.weaponIndex) &&
         lobbyMsg.package_int("variantindex",
                              &mutableClientInfo.showcaseWeapon.variantIndex) &&
         lobbyMsg.package_int("sortindex",
                              &mutableClientInfo.showcaseWeapon.sortIndex) &&
         lobbyMsg.package_glob("collectibles", mutableClientInfo.collectibles,
                               sizeof(mutableClientInfo.collectibles)) &&
         lobbyMsg.package_uint("flags", &mutableClientInfo.flags) &&
         lobbyMsg.package_int("backgroundId",
                              &mutableClientInfo.backgroundId) &&
         lobbyMsg.package_float("skillrating",
                                &mutableClientInfo.skillRating) &&
         lobbyMsg.package_float("skillvariance",
                                &mutableClientInfo.skillVariance) &&
         lobbyMsg.package_glob("musicunlocks", mutableClientInfo.musicUnlocks,
                               sizeof(mutableClientInfo.musicUnlocks)) &&
         lobbyMsg.package_char(
             "defaultemblemid",
             reinterpret_cast<char *>(&mutableClientInfo.defaultEmblemIndex)) &&
         lobbyMsg.package_char(
             "easterEggBits",
             reinterpret_cast<char *>(&mutableClientInfo.easterEggBits)) &&
         lobbyMsg.package_ushort("unlockedmedals",
                                 &mutableClientInfo.unlockedMedals) &&
         lobbyMsg.package_uint("profilenonce",
                               &mutableClientInfo.profileNonce) &&
         lobbyMsg.package_ushort("ddlbuffsize",
                                 &mutableClientInfo.ddlBuffSize) &&
         lobbyMsg.package_glob("ddlbuff", mutableClientInfo.getDdlBuff()) &&
         lobbyMsg.package_uint("pprobation",
                               &mutableClientInfo.publicProbationEndTime) &&
         lobbyMsg.package_uint("aprobation",
                               &mutableClientInfo.arenaProbationEndTime) &&
         lobbyMsg.package_uint("dlcBits", &mutableClientInfo.dlcBits.mask) &&
         lobbyMsg.package_uint("xpgroupmask",
                               &mutableClientInfo.doubleXPGroupMask) &&
         lobbyMsg.package_bool("issplitscreen",
                               &mutableClientInfo.isSplitscreenClient) &&
         lobbyMsg.package_bool("inplatparty",
                               &mutableClientInfo.isInPlatformParty);
}

bool Msg_LobbyState::package(LobbyMsg &lobbyMsg) {
  bool result =
      lobbyMsg.package_int("statenum", &stateNum) &&
      lobbyMsg.package_int("networkmode",
                           reinterpret_cast<int32_t *>(&lobbyNetworkMode)) &&
      lobbyMsg.package_int("mainmode",
                           reinterpret_cast<int32_t *>(&lobbyMainMode)) &&
      lobbyMsg.package_int("partyprivacy",
                           reinterpret_cast<int32_t *>(&partyPrivacy)) &&
      lobbyMsg.package_int("lobbytype",
                           reinterpret_cast<int32_t *>(&lobbyType)) &&
      lobbyMsg.package_int("lobbymode",
                           reinterpret_cast<int32_t *>(&lobbyMode)) &&
      lobbyMsg.package_int("sessionstatus",
                           reinterpret_cast<int32_t *>(&status)) &&
      lobbyMsg.package_int("uiscreen", &uiScreen) &&
      lobbyMsg.package_uchar("leaderactivity",
                             reinterpret_cast<uint8_t *>(&uiLeaderActivity)) &&
      lobbyMsg.package_string("key", key,
                              static_cast<int32_t>(std::size(key))) &&
      lobbyMsg.package_xuid("leader", &clientLeader) &&
      lobbyMsg.package_xuid("platformsession", &platformSessionID) &&
      lobbyMsg.package_int("maxclients",
                           reinterpret_cast<int32_t *>(&maxClients)) &&
      lobbyMsg.package_bool("isadvertised", &isAdvertised) &&
      lobbyMsg.package_int("clientcount",
                           reinterpret_cast<int32_t *>(&clientCount)) &&
      lobbyMsg.package_string(

          "sessionid", platformData.platformSessionData.sessionId,
          static_cast<int32_t>(
              std::size(platformData.platformSessionData.sessionId))) &&
      lobbyMsg.package_string(

          "sessioninfo", platformData.platformSessionData.sessionInfo,
          static_cast<int32_t>(
              std::size(platformData.platformSessionData.sessionInfo))) &&
      lobbyMsg.package_string(
          "ugcName", ugcInfo.publisherId,
          static_cast<int32_t>(std::size(ugcInfo.publisherId))) &&
      lobbyMsg.package_uint("ugcVersion", &ugcInfo.version);
  lobbyMsg.package_arrayStart("clientlist");
  for (size_t clientIdx = 0; lobbyMsg.package_element(clientIdx < clientCount);
       ++clientIdx) {
    MsgClientLobbyInfo &client = clientList[clientIdx];
    result &= lobbyMsg.package_xuid("xuid", &client.xuid) &&
              lobbyMsg.package_uchar("clientNum", reinterpret_cast<uint8_t *>(
                                                      &client.clientNum)) &&
              lobbyMsg.package_string(
                  "gamertag", client.gamertag,
                  static_cast<int32_t>(std::size(client.gamertag))) &&
              lobbyMsg.package_bool("isGuest", &client.isGuest) &&
              lobbyMsg.package_uint64("lobbyid", &client.lobbyID) &&
              lobbyMsg.package_int("connectbit", &client.connectivityBits) &&
              lobbyMsg.package_int("score", &client.score) &&
              lobbyMsg.package_glob("address", client.serializedAdr.addrBuff,
                                    sizeof(client.serializedAdr.addrBuff)) &&
              lobbyMsg.package_int("qport", &client.qport) &&
              lobbyMsg.package_uchar("band", &client.latencyBand) &&
              lobbyMsg.package_uint(
                  "netsrc", reinterpret_cast<uint32_t *>(&client.localNetID)) &&
              lobbyMsg.package_uint("joinorder", &client.joinOrder) &&
              lobbyMsg.package_uint("dlcBits", &client.dlcBits) &&
              client.mutableClientInfoMsg.package(lobbyMsg);
  }
  result &= lobbyMsg.package_uchar("migratebits", &migrateInfo.indexBits) &&
            lobbyMsg.package_int("lasthosttimems", &migrateInfo.lasthostTimeMS);
  lobbyMsg.package_arrayStart("nomineelist");
  for (size_t nomineeIdx = 0;
       result && lobbyMsg.package_element(
                     nomineeIdx < std::size(migrateInfo.migrateNominees));
       ++nomineeIdx) {
    result &=
        lobbyMsg.package_xuid("xuid", &migrateInfo.migrateNominees[nomineeIdx]);
  }
  return result;
}

bool Msg_LobbyStateGame::package(LobbyMsg &lobbyMsg) {
  bool result =
      core.package(lobbyMsg) &&
      lobbyMsg.package_int("serverstatus", reinterpret_cast<int32_t *>(
                                               &dataGame.serverStatus)) &&
      lobbyMsg.package_int("launchnonce", &dataGame.launchNonce) &&
      lobbyMsg.package_uint64("matchhashlow", &dataGame.matchHash.low) &&
      lobbyMsg.package_uint64("matchhashhigh", &dataGame.matchHash.high) &&
      lobbyMsg.package_int("status", &dataGame.status) &&
      lobbyMsg.package_int("statusvalue", &dataGame.statusValue) &&
      lobbyMsg.package_int("gamemode",
                           reinterpret_cast<int32_t *>(&dataGame.gameMode)) &&
      lobbyMsg.package_string(
          "gametype", dataGame.gameType,
          static_cast<int32_t>(std::size(dataGame.gameType))) &&
      lobbyMsg.package_string(
          "map", dataGame.mapName,
          static_cast<int32_t>(std::size(dataGame.mapName))) &&
      lobbyMsg.package_string(
          "ugcName", dataGame.ugcInfo.publisherId,
          static_cast<int32_t>(std::size(dataGame.ugcInfo.publisherId))) &&
      lobbyMsg.package_uint("ugcVersion", &dataGame.ugcInfo.version);
  if (core.lobbyMainMode == LobbyMainMode::CP) {
    result &=
        lobbyMsg.package_string(
            "cpqueuedlevel", dataGame.cpQueuedLevel,
            static_cast<int32_t>(std::size(dataGame.cpQueuedLevel))) &&
        lobbyMsg.package_bool("movieskipped", &dataGame.loadingMovieSkipped);
  }
  switch (core.lobbyMode) {
  case LobbyMode::PUBLIC: {
    lobbyMsg.package_arrayStart("clientlist");
    for (size_t clientIdx = 0;
         lobbyMsg.package_element(clientIdx < std::size(dataGame.client));

         ++clientIdx) {
      session::GameClientData &client = dataGame.client[clientIdx];
      result &= lobbyMsg.package_int(
                    "team", reinterpret_cast<int32_t *>(&client.team)) &&
                lobbyMsg.package_int(
                    "mapvote", reinterpret_cast<int32_t *>(&client.mapVote)) &&
                lobbyMsg.package_bool("readyup", &client.readyUp);
    }
    return result && lobbyMsg.package_int("plistid", &dataGame.playlistId) &&
           lobbyMsg.package_int("plistcurr", &dataGame.playlistCurrentEntry) &&
           lobbyMsg.package_glob("plistentries", dataGame.playlistLastEntries,
                                 sizeof(dataGame.playlistLastEntries)) &&
           lobbyMsg.package_uchar("plistnext", &dataGame.playlistNext) &&
           lobbyMsg.package_uchar("plistprev", &dataGame.playlistPrev) &&
           lobbyMsg.package_uchar("plistprevcount",
                                  &dataGame.playlistPrevCount);
  }
  case LobbyMode::CUSTOM: {
    result &= lobbyMsg.package_int(
        "votecount", reinterpret_cast<int32_t *>(&dataGame.pregameVoteCount));
    lobbyMsg.package_arrayStart("votes");

    for (size_t voteIdx = 0; lobbyMsg.package_element(true); ++voteIdx) {
      session::ItemVote &vote = dataGame.pregameVotes[voteIdx];
      result &=
          lobbyMsg.package_short("itemtype",
                                 reinterpret_cast<int16_t *>(&vote.itemType)) &&
          lobbyMsg.package_uint("item", &vote.itemIndex) &&
          lobbyMsg.package_short(
              "itemgroup", reinterpret_cast<int16_t *>(&vote.itemGroup)) &&
          lobbyMsg.package_short(
              "attachment", reinterpret_cast<int16_t *>(&vote.attachment)) &&
          lobbyMsg.package_short("votetype",
                                 reinterpret_cast<int16_t *>(&vote.voteType)) &&
          lobbyMsg.package_xuid("votexuid", &vote.clientXuid);
    }
    lobbyMsg.package_arrayStart("clientlist");
    for (size_t clientIdx = 0;
         lobbyMsg.package_element(clientIdx < std::size(dataGame.client));
         ++clientIdx) {

      session::GameClientData &client = dataGame.client[clientIdx];
      result &=
          lobbyMsg.package_int("team",
                               reinterpret_cast<int32_t *>(&client.team)) &&
          lobbyMsg.package_int("pregamepos", &client.pregamePosition) &&
          lobbyMsg.package_int("pregamestate", reinterpret_cast<int32_t *>(
                                                   &client.pregameState)) &&
          lobbyMsg.package_uint("clvotecount", &client.itemVoteCount) &&
          lobbyMsg.package_uint("character", reinterpret_cast<uint32_t *>(
                                                 &client.characterSelection)) &&
          lobbyMsg.package_uint("loadout",
                                reinterpret_cast<uint32_t *>(
                                    client.characterSelection.loadoutType));
    }

    return result &&
           lobbyMsg.package_int("settingssize", &compressedBufferSize) &&
           compressedBufferSize > 0 &&
           lobbyMsg.package_glob("settings", getCompressedBuffer()) &&
           lobbyMsg.package_int("compstate", reinterpret_cast<int32_t *>(
                                                 &dataGame.pregameState)) &&
           lobbyMsg.package_string("customname",
                                   dataGame.customGameDetails.name,
                                   static_cast<int32_t>(std::size(
                                       dataGame.customGameDetails.name))) &&
           lobbyMsg.package_bool(
               "customofficial",
               &dataGame.customGameDetails.isOfficial.value) &&
           lobbyMsg.package_int("plistid", &dataGame.playlistId) &&
           lobbyMsg.package_int("plistcurr", &dataGame.playlistCurrentEntry) &&
           lobbyMsg.package_glob("plistentries", dataGame.playlistLastEntries,
                                 sizeof(dataGame.playlistLastEntries));
  }
  case LobbyMode::THEATER: {
    return result &&
           lobbyMsg.package_string(
               "demoname", dataGame.demoName,
               static_cast<int32_t>(std::size(dataGame.demoName))) &&
           lobbyMsg.package_uint64("fileid", &dataGame.fileId) &&
           lobbyMsg.package_int("filesize", &dataGame.fileSize) &&
           lobbyMsg.package_bool("isuserfile", &dataGame.isUserFile) &&
           lobbyMsg.package_bool("islivestream", &dataGame.isLiveStream) &&
           lobbyMsg.package_int(
               "mainmode", reinterpret_cast<int32_t *>(&dataGame.mainMode)) &&
           lobbyMsg.package_string(
               "demomode", dataGame.demoMode,
               static_cast<int32_t>(std::size(dataGame.demoMode))) &&
           lobbyMsg.package_string(
               "filecategory", dataGame.fileCategory,
               static_cast<int32_t>(std::size(dataGame.fileCategory))) &&
           lobbyMsg.package_xuid("authorxuid", &dataGame.authorXUID);
  }
  case LobbyMode::ARENA: {
    result &=
        lobbyMsg.package_int(
            "compstate", reinterpret_cast<int32_t *>(&dataGame.pregameState)) &&
        lobbyMsg.package_int("votecount", reinterpret_cast<int32_t *>(
                                              &dataGame.pregameVoteCount));
    lobbyMsg.package_arrayStart("votes");
    for (size_t voteIdx = 0;
         lobbyMsg.package_element(voteIdx < dataGame.pregameVoteCount);
         ++voteIdx) {
      session::ItemVote &vote = dataGame.pregameVotes[voteIdx];
      result &=
          lobbyMsg.package_short("itemtype",
                                 reinterpret_cast<int16_t *>(&vote.itemType)) &&
          lobbyMsg.package_uint("item", &vote.itemIndex) &&
          lobbyMsg.package_short(
              "itemgroup", reinterpret_cast<int16_t *>(&vote.itemGroup)) &&
          lobbyMsg.package_short(
              "attachment", reinterpret_cast<int16_t *>(&vote.attachment)) &&
          lobbyMsg.package_short("votetype",
                                 reinterpret_cast<int16_t *>(&vote.voteType)) &&
          lobbyMsg.package_xuid("votexuid", &vote.clientXuid);
    }

    lobbyMsg.package_arrayStart("clientlist");
    for (size_t clientIdx = 0; lobbyMsg.package_element(
             std::cmp_less_equal(clientIdx, std::size(dataGame.client)));
         ++clientIdx) {

      session::GameClientData &client = dataGame.client[clientIdx];
      result &=
          lobbyMsg.package_int("team",
                               reinterpret_cast<int32_t *>(&client.team)) &&
          lobbyMsg.package_int("pregamepos", &client.pregamePosition) &&
          lobbyMsg.package_int("pregamestate", reinterpret_cast<int32_t *>(
                                                   &client.pregameState)) &&
          lobbyMsg.package_uint("clvotecount", &client.itemVoteCount) &&
          lobbyMsg.package_uint("character", reinterpret_cast<uint32_t *>(
                                                 &client.characterSelection)) &&
          lobbyMsg.package_uint("loadout",
                                reinterpret_cast<uint32_t *>(
                                    client.characterSelection.loadoutType));
    }
    return result && lobbyMsg.package_int("plistid", &dataGame.playlistId) &&
           lobbyMsg.package_int("plistcurr", &dataGame.playlistCurrentEntry) &&
           lobbyMsg.package_glob("plistentries", dataGame.playlistLastEntries,
                                 sizeof(dataGame.playlistLastEntries));
  }
  default: {
    return result;
  }
  }
}

bool Msg_LobbyHostHeartbeat::package(LobbyMsg &lobbyMsg) {
  bool result =
      lobbyMsg.package_int("heartbeatnum", &heartbeatNum) &&
      lobbyMsg.package_int("lobbytype",
                           reinterpret_cast<int32_t *>(&lobbyType)) &&
      lobbyMsg.package_int("lasthosttimems", &migrateInfo.lasthostTimeMS);
  lobbyMsg.package_arrayStart("nomineelist");
  for (size_t nomineeIdx = 0;
       result && lobbyMsg.package_element(
                     nomineeIdx < std::size(migrateInfo.migrateNominees));
       ++nomineeIdx) {
    result &=
        lobbyMsg.package_xuid("xuid", &migrateInfo.migrateNominees[nomineeIdx]);
  }
  return result;
}

bool Msg_JoinParty::package(LobbyMsg &lobbyMsg) {
  bool result =
      lobbyMsg.package_int("targetlobby",
                           reinterpret_cast<int32_t *>(&targetLobby)) &&
      lobbyMsg.package_int("sourcelobby",
                           reinterpret_cast<int32_t *>(&sourceLobby)) &&
      lobbyMsg.package_int("jointype",
                           reinterpret_cast<int32_t *>(&joinType)) &&
      lobbyMsg.package_xuid("probedxuid", &probedXuid) &&
      lobbyMsg.package_int("playlistid", &playlistID) &&
      lobbyMsg.package_int("playlistver", &playlistVersion) &&
      lobbyMsg.package_int("ffotdver", &ffotdVersion) &&
      lobbyMsg.package_short("networkmode",
                             reinterpret_cast<int16_t *>(&networkMode)) &&
      lobbyMsg.package_uint("netchecksum", &netFieldChecksum) &&
      lobbyMsg.package_int("protocol", &protocolVersion) &&
      lobbyMsg.package_int("changelist", &changelist) &&
      lobbyMsg.package_int("pingband", &pingBand) &&
      lobbyMsg.package_uint("dlcbits", &dlcBits) &&
      lobbyMsg.package_uint64("joinnonce", &joinNonce);
  for (bool &status : chunkStatus) {
    result &=
        lobbyMsg.package_uchar("chunk", reinterpret_cast<uint8_t *>(&status));
  }
  result &= lobbyMsg.package_bool("isStarterPack", &isStarterPack) &&
            /* Note: this matches Treyarch's package implementation here, but
               why the hell does it send the raw password verbatim? This is a
               bit silly. */
            lobbyMsg.package_string("password", password) &&
            lobbyMsg.package_int("membercount", &memberCount);
  lobbyMsg.package_arrayStart("members");
  for (size_t partyMemberIdx = 0;
       lobbyMsg.package_element(std::cmp_less(partyMemberIdx, memberCount));
       ++partyMemberIdx) {
    join::JoinPartyMember &partyMember = members[partyMemberIdx];
    result &=
        lobbyMsg.package_xuid("xuid", &partyMember.xuid) &&
        lobbyMsg.package_uint64("lobbyid", &partyMember.lobbyID) &&
        lobbyMsg.package_float("skillrating", &partyMember.skillRating) &&
        lobbyMsg.package_float("skillvariance", &partyMember.skillVariance) &&
        lobbyMsg.package_uint("pprobation",
                              &partyMember.publicProbationEndTime) &&
        lobbyMsg.package_uint("aprobation", &partyMember.arenaProbationEndTime);
  }
  return result && lobbyMsg.package_int("splitscreen", &splitscreenClients);
}

bool Msg_InfoResponseLobbyServer::package(LobbyMsg &lobbyMsg) {
  bool result = lobbyMsg.package_uchar(
      "platform", reinterpret_cast<uint8_t *>(&clientPlatform));
  lobbyMsg.package_arrayStart("lobbies");

  result &= lobbyMsg.package(lobby) && lobbyMsg.package_xuid("xuid", &xuid) &&
            lobbyMsg.package_string("gamertag", gamertag) &&
            lobbyMsg.package_int("sessionmode",
                                 reinterpret_cast<int32_t *>(&sessionMode)) &&
            lobbyMsg.package_string("mapname", mapName) &&
            lobbyMsg.package_string("gametype", gameType) &&
            lobbyMsg.package_bool("ingame", &isInGame) &&
            lobbyMsg.package_uint("maxclients",
                                  reinterpret_cast<uint32_t *>(&maxClients)) &&
            lobbyMsg.package_uint("clientcount",
                                  reinterpret_cast<uint32_t *>(&clientCount));
  lobbyMsg.package_arrayStart("clientlist");
  for (size_t lanClientIdx = 0;
       lobbyMsg.package_element(lanClientIdx < clientCount); ++lanClientIdx) {
    LANClientInfo &lanClient = client[lanClientIdx];
    result &= lobbyMsg.package_xuid("xuid", &lanClient.xuid) &&
              lobbyMsg.package_string("gamertag", lanClient.gamertag);
  }

  return result;
}

bool Msg_InfoResponse::package(LobbyMsg &lobbyMsg) {
  bool result =
      lobbyMsg.package_uint("nonce", &nonce) &&
      lobbyMsg.package_int("uiscreen", &uiScreen) &&
      lobbyMsg.package_uchar("nattype", reinterpret_cast<uint8_t *>(&natType));
  lobbyMsg.package_arrayStart("lobbies");
  return result && lobbyMsg.package(lobby);
}

bool Msg_ClientReliableData::package(LobbyMsg &lobbyMsg) {

  bool result = lobbyMsg.package_uint("datamask", &dataMask) &&
                lobbyMsg.package_int("lobbytype",
                                     reinterpret_cast<int32_t *>(&lobbyType));
  // TODO: define as bitpacked union
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  if ((dataMask & 2) != 0) {
    result &= lobbyMsg.package_xuid("newleader", &xuidNewLeader);
  }
  // TODO: define as bitpacked union
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  if ((dataMask & 0x800) != 0) {
    result &= lobbyMsg.package_xuid("platformsession", &platformSessionID);
  }
  // TODO: define as bitpacked union
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  if ((dataMask & 4) != 0) {
    result &= lobbyMsg.package_xuid("discclientxuid", &disconnectClientXuid) &&
              lobbyMsg.package_int(
                  "discclient", reinterpret_cast<int32_t *>(&disconnectClient));
  }
  // TODO: define as bitpacked union
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  if ((dataMask & 8) != 0) {
    result &= lobbyMsg.package_uchar(
        "leaderactivity", reinterpret_cast<uint8_t *>(&leaderActivity));
  }
  if (lobbyType == LobbyType::GAME) {
    // TODO: define as bitpacked union
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    if ((dataMask & 0x20) != 0) {
      result &=
          lobbyMsg.package_int("team", reinterpret_cast<int32_t *>(&team));
    }
    // TODO: define as bitpacked union
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    if ((dataMask & 0x40) != 0) {
      result &= lobbyMsg.package_int("mapvote",
                                     reinterpret_cast<int32_t *>(&mapVote));
    }
    // TODO: define as bitpacked union
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    if ((dataMask & 0xFF) > 0x7F) {
      result &= lobbyMsg.package_bool("readyup", &readyUp);
    }
    // TODO: define as bitpacked union
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    if ((dataMask & 0x100) != 0) {
      result &=
          lobbyMsg.package_int(
              "itemtype", reinterpret_cast<int32_t *>(&itemVote.itemType)) &&
          lobbyMsg.package_uint("item", &itemVote.itemIndex) &&
          lobbyMsg.package_int(
              "itemgroup", reinterpret_cast<int32_t *>(&itemVote.itemGroup)) &&
          lobbyMsg.package_int("attachment", reinterpret_cast<int32_t *>(
                                                 &itemVote.attachment)) &&
          lobbyMsg.package_int("votetype",
                               reinterpret_cast<int32_t *>(&itemVote.voteType));
    }
    // TODO: define as bitpacked union
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    if ((dataMask & 0x200) != 0) {
      result &=
          lobbyMsg.package_uint(
              "character", reinterpret_cast<uint32_t *>(&characterSelection)) &&
          lobbyMsg.package_uint(
              "loadout",
              reinterpret_cast<uint32_t *>(&characterSelection.loadoutType));
    }

    // TODO: define as bitpacked union
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    if ((dataMask & 0x400) != 0) {
      return result && lobbyMsg.package_bool("editcomplete", &editComplete);
    }
  }
  return result;
}

} // namespace msg
} // namespace lobby
} // namespace game
