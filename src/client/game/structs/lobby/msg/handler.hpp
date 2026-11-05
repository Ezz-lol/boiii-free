#pragma once

#include <cstdint>

#include <game/structs/lobby/core.hpp>
#include <game/structs/lobby/info.hpp>
#include <game/structs/lobby/join.hpp>
#include <game/structs/lobby/msg/core.hpp>
#include <game/structs/lobby/session.hpp>

namespace game {
namespace lobby {

namespace msg {

#ifndef DEFINE_LOBBYMSGHANDLER_TABLE_METHODS
#define DEFINE_LOBBYMSGHANDLER_TABLE_METHODS()                                 \
  static inline constexpr auto size() noexcept { return COUNT; }               \
                                                                               \
  inline constexpr void assert_range([[maybe_unused]] size_t index) const {    \
    assert(index < COUNT &&                                                    \
           "index to handler table must be < handler table length");           \
  }                                                                            \
  template <IntegralLike<size_t> Index>                                        \
  inline constexpr const LobbyMsgHandler &get(Index index_arg)                 \
      const noexcept {                                                         \
    const size_t index = static_cast<size_t>(index_arg);                       \
    assert_range(index);                                                       \
    return handlers[index];                                                    \
  }                                                                            \
  template <IntegralLike<size_t> Index>                                        \
  inline constexpr LobbyMsgHandler &get(Index index_arg) noexcept {            \
    const size_t index = static_cast<size_t>(index_arg);                       \
    assert_range(index);                                                       \
    return handlers[index];                                                    \
  }                                                                            \
                                                                               \
  template <IntegralLike<size_t> Index>                                        \
  inline constexpr const LobbyMsgHandler &operator[](Index index)              \
      const noexcept {                                                         \
    return get(index);                                                         \
  }                                                                            \
  template <IntegralLike<size_t> Index>                                        \
  inline constexpr LobbyMsgHandler &operator[](Index index) noexcept {         \
    return get(index);                                                         \
  }                                                                            \
                                                                               \
  inline operator const LobbyMsgHandler *() const noexcept {                   \
    return handlers;                                                           \
  }                                                                            \
  inline operator LobbyMsgHandler *() noexcept { return handlers; }            \
                                                                               \
  inline operator const void *() const noexcept { return handlers; }           \
  inline operator void *() noexcept { return handlers; }                       \
                                                                               \
  inline operator const array<LobbyMsgHandler, COUNT> &() const noexcept {     \
    return handlers;                                                           \
  }                                                                            \
  inline operator array<LobbyMsgHandler, COUNT> &() noexcept {                 \
    return handlers;                                                           \
  }                                                                            \
  inline operator const std::span<const LobbyMsgHandler, COUNT>()              \
      const noexcept {                                                         \
    return std::span<const LobbyMsgHandler, COUNT>(handlers);                  \
  }                                                                            \
  inline operator std::span<LobbyMsgHandler, COUNT>() noexcept {               \
    return std::span<LobbyMsgHandler, COUNT>(handlers);                        \
  }
#endif

// Client-side, received from host
union LobbyClientHostMsgHandlersTable {
  struct {
    LobbyMsgHandler LobbyStatePrivate;
    LobbyMsgHandler LobbyStateGame;
    LobbyMsgHandler LobbyHostHeartbeat;
    LobbyMsgHandler LobbyHostDisconnect;
    LobbyMsgHandler LobbyHostDisconnectClient;
    LobbyMsgHandler LobbyHostLeaveWithParty;
    LobbyMsgHandler JoinAgreementRequest;
    LobbyMsgHandler JoinComplete;
    LobbyMsgHandler LobbyMigrateAnnounceHost;
    LobbyMsgHandler IngameMigrateTo;
    LobbyMsgHandler IngameMigrateNewHost;
    LobbyMsgHandler LobbyClientContent;
  };

  static constexpr auto COUNT = 12;
  LobbyMsgHandler handlers[COUNT];

  DEFINE_LOBBYMSGHANDLER_TABLE_METHODS();
};

// Host-side, received from client
union LobbyHostClientMsgHandlersTable {
  struct {
    LobbyMsgHandler JoinRequest;
    LobbyMsgHandler JoinMemberInfo;
    LobbyMsgHandler JoinAgreementResponse;
    LobbyMsgHandler JoinResponse;
    LobbyMsgHandler ServerListInfo;
    LobbyMsgHandler PeerToPeerInfo;
    LobbyMsgHandler LobbyClientHeartbeat;
    LobbyMsgHandler LobbyClientDisconnect;
    LobbyMsgHandler LobbyClientReliableData;
    LobbyMsgHandler LobbyClientContent;
    LobbyMsgHandler LobbyModifiedStats;
    LobbyMsgHandler VoicePacket;
  };

  static constexpr auto COUNT = 12;
  LobbyMsgHandler handlers[COUNT];

  DEFINE_LOBBYMSGHANDLER_TABLE_METHODS();
};

union LobbyP2PMsgHandlersTable {
  struct {
    LobbyMsgHandler ConnectivityTest;
    LobbyMsgHandler MigrateBandwidthTest;
    LobbyMsgHandler MigrateStart;
    LobbyMsgHandler VoicePacket;
    LobbyMsgHandler DemoState;
  };
  static constexpr auto COUNT = 5;
  LobbyMsgHandler handlers[COUNT];

  DEFINE_LOBBYMSGHANDLER_TABLE_METHODS();
};

// Verified
PACKED(struct Msg_InfoResponse {
  uint32_t nonce = 0;
  LobbyUIScreen uiScreen = 0;
  dw::net::bdNATType natType = dw::net::bdNATType::NAT_UNKNOWN;
  uint8_t _padding09[7];
  info::InfoResponseLobby lobby[2] = {{}};

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::INFO_RESPONSE;
  bool package(LobbyMsg & lobbyMsg);
});
ASSERT_SIZE(Msg_InfoResponse, 0x150);

PACKED(struct Msg_JoinParty {
  LobbyType targetLobby = LobbyType::INVALID;
  LobbyType sourceLobby = LobbyType::INVALID;
  join::JoinType joinType = join::JoinType::COUNT;
  uint8_t _padding0C[4];
  XUID probedXuid = 0;
  ClientNum_t memberCount = ClientNum_t::CLIENT_INDEX_0;
  uint8_t _padding1C[4];
  LobbyClientPool<join::JoinPartyMember> members = {{}};
  LocalClientNum_t splitscreenClients = LocalClientNum_t::INVALID_LOCAL_CLIENT;
  int32_t playlistID = 0;
  int32_t playlistVersion = 0;
  int32_t ffotdVersion = 0;
  LobbyNetworkMode networkMode = LobbyNetworkMode::INVALID;
  uint32_t netFieldChecksum = 0;
  int32_t protocolVersion = 0;
  int32_t changelist = 0;
  int32_t pingBand = 0;
  ContentFlags dlcBits = {};
  uint64_t joinNonce = 0;
  bool isStarterPack = false;
  char password[32] = {0};
  bool chunkStatus[3] = {false};
  uint8_t _padding2B4[4];

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::JOIN_LOBBY;

  bool package(LobbyMsg & lobbyMsg);

  IMPL_SLICE_METHODS(members, memberCount, Members);
});
ASSERT_SIZE(Msg_JoinParty, 0x2B8);

// Verified
struct MsgMutableClientInfo {
  MutableClientInfo mutableClientInfo = {};

  bool package(LobbyMsg &lobbyMsg);
};
ASSERT_SIZE(MsgMutableClientInfo, 0x410);

// Verified
PACKED(struct MsgClientLobbyInfo {
  XUID xuid = 0;
  ClientNum8_t clientNum = ClientNum8_t::INVALID_CLIENT_INDEX_8;
  name_t gamertag = {0};
  bool isGuest = false;
  uint8_t _padding2A[6];
  LobbyID lobbyID = 0;
  int32_t qport = 0;
  uint8_t latencyBand = 0;
  uint8_t _padding3D[3];
  MsgMutableClientInfo mutableClientInfoMsg = {};
  int32_t score = 0;
  int32_t connectivityBits = 0;
  net::SerializedAdr serializedAdr = {};
  uint8_t _padding47E[2];
  net::netsrc_t localNetID = net::netsrc_t::NS_NULL;
  uint32_t joinOrder = 0;
  ContentFlags dlcBits = {};
  uint8_t _padding48C[4];
});
ASSERT_SIZE(MsgClientLobbyInfo, 0x490);

// Verified
PACKED(struct MsgHostMigrateInfo {
  uint8_t indexBits = 0;
  uint8_t _padding01[3];
  int32_t lasthostTimeMS = 0;
  LobbyClientPool<XUID> migrateNominees = {0};
});
ASSERT_SIZE(MsgHostMigrateInfo, 0x98);

// Verified
struct MsgPlatformSessionData {
  session::PlatformSessionData platformSessionData = {};
};

// Verified
PACKED(struct Msg_LobbyState {
  int32_t stateNum = 0;
  LobbyNetworkMode lobbyNetworkMode = LobbyNetworkMode::INVALID;
  LobbyMainMode lobbyMainMode = LobbyMainMode::INVALID;
  PartyPrivacy partyPrivacy = PartyPrivacy::CLOSED;
  LobbyType lobbyType = LobbyType::INVALID;
  LobbyMode lobbyMode = LobbyMode::INVALID;
  session::SessionStatus status = session::SessionStatus::INVALID;
  int32_t uiScreen = 0;
  LobbyLeaderActivity uiLeaderActivity = 0;
  str32_t key = {0};
  uint8_t _padding44[4];
  XUID clientLeader = 0;
  XUID platformSessionID = 0;
  ClientNum_t maxClients = ClientNum_t::INVALID_CLIENT_INDEX;
  bool isAdvertised = false;
  uint8_t _padding5D[3];
  ClientNum_t clientCount = ClientNum_t::INVALID_CLIENT_INDEX;
  uint8_t _padding64[4];
  LobbyClientPool<MsgClientLobbyInfo> clientList = {{}};
  MsgHostMigrateInfo migrateInfo = {};
  MsgPlatformSessionData platformData = {};
  info::LobbyUGCInfo ugcInfo = {};
  uint8_t _padding53C4[4];

  bool package(LobbyMsg & lobbyMsg);

  IMPL_SLICE_METHODS(clientList, clientCount, ClientList);
});
// Verified
ASSERT_SIZE(Msg_LobbyState, 0x53C8);

// Verified
PACKED(struct Msg_LobbyStateGame {
  Msg_LobbyState core = {};
  session::GameData dataGame = {};
  int32_t compressedBufferSize = 0;
  session::gametypesettings_t compressedBuffer = {0};
  uint8_t _padding132DC[4];

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::LOBBY_STATE_GAME;

  bool package(LobbyMsg & lobbyMsg);

  IMPL_SLICE_METHODS(compressedBuffer, compressedBufferSize, CompressedBuffer);
});
ASSERT_SIZE(Msg_LobbyStateGame, 0x132E0);
ASSERT_OFFSET(Msg_LobbyStateGame, core, 0x0);
ASSERT_OFFSET(Msg_LobbyStateGame, dataGame, 0x53C8);
ASSERT_OFFSET(Msg_LobbyStateGame, compressedBufferSize, 0x72D8);
ASSERT_OFFSET(Msg_LobbyStateGame, compressedBuffer, 0x72DC);

struct PrivateData {
  int32_t playlistId = 0;
};

// Verified
PACKED(struct Msg_LobbyStatePrivate {
  Msg_LobbyState core = {};
  PrivateData dataPrivate = {};
  uint8_t _padding53CC[4];

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::LOBBY_STATE_PRIVATE;

  inline bool package(LobbyMsg & lobbyMsg) {
    return core.package(lobbyMsg) &&
           (core.lobbyMode != LobbyMode::ARENA ||
            lobbyMsg.package_int("plistid", &dataPrivate.playlistId));
  }
});
ASSERT_SIZE(Msg_LobbyStatePrivate, 0x53D0);

// Verified
PACKED(struct Msg_JoinAgreementResponse {
  XUID xuid = 0;
  bool clientAgrees = false;
  uint8_t _padding09[7];

  static inline constexpr MsgType MESSAGE_TYPE =
      MsgType::JOIN_AGREEMENT_RESPONSE;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_xuid("xuid", &xuid) &&
           lobbyMsg.package_bool("agree", &clientAgrees);
  }
});
ASSERT_SIZE(Msg_JoinAgreementResponse, 0x10);

// Verified
PACKED(struct Msg_ModifiedStats {
  int32_t statsSize = 0;
  uint8_t statsBuffer[0x10000] = {0};

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::LOBBY_MODIFIED_STATS;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_int("statssize", &statsSize) &&
           statsSize <= std::size(statsBuffer) &&
           lobbyMsg.package_glob("statsbuffer", statsBuffer, statsSize);
  }

  IMPL_SLICE_METHODS(statsBuffer, statsSize, StatsBuffer);
});
ASSERT_SIZE(Msg_ModifiedStats, 0x10004);

// Verified
PACKED(struct Msg_ClientReliableData {
  // TODO: concretely define as bit-packed union
  uint32_t dataMask = 0;
  LobbyType lobbyType = LobbyType::INVALID;
  XUID xuidNewLeader = 0;
  XUID disconnectClientXuid = 0;
  LobbyDisconnectClient disconnectClient = LobbyDisconnectClient::INVALID;
  LobbyLeaderActivity leaderActivity = 0;
  XUID platformSessionID = 0;
  team_t team = team_t::TEAM_FREE;
  LobbyMapVote mapVote = LobbyMapVote::INVALID;
  bool readyUp = false;
  uint8_t _padding31[7];
  session::ItemVote itemVote = {};
  session::CharacterSelection characterSelection = {};
  bool editComplete = false;
  uint8_t _padding61[7];

  static inline constexpr MsgType MESSAGE_TYPE =
      MsgType::LOBBY_CLIENT_RELIABLE_DATA;

  bool package(LobbyMsg & lobbyMsg);
});
ASSERT_SIZE(Msg_ClientReliableData, 0x68);

// Verified
PACKED(struct Msg_VoicePacket {
  LobbyType lobbyType = LobbyType::INVALID;
  uint8_t talkerIndex = 0;
  uint8_t _padding05[3];
  int32_t relayBits = 0;
  uint16_t sizeofVoiceData = 0;
  uint8_t numPacketsInData = 0;
  // A buffer of contiguous voice packets with cumulative size <= 1198 bytes.
  // Each packet contains its size as its first 4 bytes, little-endian encoded,
  // followed by the specified number of packet bytes.
  uint8_t voiceData[1198] = {0};
  uint8_t _padding4BD[3];

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::VOICE_PACKET;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_int("lobbytype", &lobbyType) &&
           validate(lobbyType) &&
           lobbyMsg.package_uchar("talker", &talkerIndex) &&
           lobbyMsg.package_int("relaybits", &relayBits) &&
           lobbyMsg.package_ushort("sizeofvoicedata", &sizeofVoiceData) &&
           sizeofVoiceData <= std::size(voiceData) &&
           lobbyMsg.package_uchar("numpackets", &numPacketsInData) &&
           lobbyMsg.package_glob("voicedata", voiceData, sizeofVoiceData);
  }

  IMPL_SLICE_METHODS(voiceData, sizeofVoiceData, VoiceData);
});
ASSERT_SIZE(Msg_VoicePacket, 0x4C0);

// Verified
struct MsgFixedClientInfo {
  FixedClientInfo fixedClientInfo;

  bool package(LobbyMsg &lobbyMsg) {
    return lobbyMsg.package_xuid("xuid", &fixedClientInfo.xuid) &&
           lobbyMsg.package_string("name", fixedClientInfo.gamertag) &&
           lobbyMsg.package_int("qport", &fixedClientInfo.qport) &&
           lobbyMsg.package_bool("isGuest", &fixedClientInfo.isGuest) &&
           lobbyMsg.package_uchar("band", &fixedClientInfo.latencyBand) &&
           lobbyMsg.package_int("granted", &fixedClientInfo.granted) &&
           lobbyMsg.package_int("consumed", &fixedClientInfo.consumed) &&
           lobbyMsg.package_int("date", &fixedClientInfo.date) &&
           lobbyMsg.package_int("daily", &fixedClientInfo.daily) &&
           lobbyMsg.package_uchar("connectionType",
                                  &fixedClientInfo.connectionType) &&
           lobbyMsg.package_glob("datacenter", fixedClientInfo.datacenter,
                                 sizeof(fixedClientInfo.datacenter)) &&
           lobbyMsg.package_ushort("countryCode",
                                   &fixedClientInfo.countryCode) &&
           lobbyMsg.package_float("latitude", &fixedClientInfo.latitude) &&
           lobbyMsg.package_float("longitude", &fixedClientInfo.longitude) &&
           lobbyMsg.package_bool("experimentActive",
                                 &fixedClientInfo.experimentActive);
  }
};

// Verified
PACKED(struct Msg_ClientContent {
  uint32_t dataMask = 0;
  LobbyType lobbyType = LobbyType::INVALID;
  uint8_t clientContentData[0x27C00] = {0};
  uint16_t compressedBufferSize = 0;
  uint8_t _padding27C0A[6];
  XUID clientXUID = 0;
  eModes sessionMode = eModes::INVALID;
  eGameModes gameMode = eGameModes::INVALID;

  IMPL_SLICE_METHODS(clientContentData, compressedBufferSize,
                     ClientContentData);

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::LOBBY_CLIENT_CONTENT;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_uint("datamask", &dataMask) &&
           lobbyMsg.package_int("lobbytype", &lobbyType) &&
           validate(lobbyType) &&
           lobbyMsg.package_xuid("clientxuid", &clientXUID) &&
           lobbyMsg.package_ushort("buffersize", &compressedBufferSize) &&
           compressedBufferSize <= std::size(clientContentData) &&
           lobbyMsg.package_glob("buffer", getClientContentData()) &&
           lobbyMsg.package_uchar("sessionmode",
                                  reinterpret_cast<uint8_t *>(&sessionMode)) &&
           validate(sessionMode) &&
           lobbyMsg.package_uchar("gamemode",
                                  reinterpret_cast<uint8_t *>(&gameMode)) &&
           validate(gameMode);
  }
});
ASSERT_SIZE(Msg_ClientContent, 0x27C20);

// Verified
PACKED(struct Msg_LobbyMigrateBandwidthTest {
  LobbyType lobbyType = LobbyType::INVALID;
  uint8_t _padding04[4];
  XUID peerXuid = 0;
  bool isAck = false;
  uint8_t sentPacketCountOrSequence = 0;
  uint8_t _padding11[2];
  int32_t timeMS = 0;

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::LOBBY_MIGRATE_TEST;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_int("lobbytype", &lobbyType) &&
           validate(lobbyType) && lobbyMsg.package_xuid("xuid", &peerXuid) &&
           lobbyMsg.package_bool("isack", &isAck) &&
           lobbyMsg.package_int("timems", &timeMS) &&
           lobbyMsg.package_uchar("seqorcount", &sentPacketCountOrSequence);
  }
});
ASSERT_SIZE(Msg_LobbyMigrateBandwidthTest, 0x18);

PACKED(struct Msg_LobbyDisconnectClient {
  LobbyType lobbyType = LobbyType::INVALID;
  uint8_t _padding04[4];
  XUID disconnectClientXuid = 0;
  LobbyDisconnectClient disconnectClient = LobbyDisconnectClient::INVALID;
  uint8_t _padding14[4];

  static inline constexpr MsgType MESSAGE_TYPE =
      MsgType::LOBBY_HOST_DISCONNECT_CLIENT;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_int("lobbytype", &lobbyType) &&
           validate(lobbyType) &&
           lobbyMsg.package_xuid("discclientxuid", &disconnectClientXuid) &&
           lobbyMsg.package_int("discclient", &disconnectClient) &&
           validate(disconnectClient);
  }
});
ASSERT_SIZE(Msg_LobbyDisconnectClient, 0x18);

// Verified
PACKED(struct Msg_ConnectionTest {
  LobbyType lobbyType = LobbyType::INVALID;
  ClientNum_t clientIndex = ClientNum_t::INVALID_CLIENT_INDEX;

  static inline constexpr MsgType MESSAGE_TYPE =
      MsgType::PEER_TO_PEER_CONNECTIVITY_TEST;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_int("lobbytype", &lobbyType) &&
           validate(lobbyType) && lobbyMsg.package_int("index", &clientIndex) &&
           validate(clientIndex);
  }
});
ASSERT_SIZE(Msg_ConnectionTest, 0x8);

// Verified
PACKED(struct Msg_DemoState {
  LobbyType lobbyType = LobbyType::INVALID;
  bool paused = false;
  uint8_t _padding05[3];
  int32_t serverTime = 0;
  int32_t timeScale = 0;
  ClientNum_t client = ClientNum_t::INVALID_CLIENT_INDEX;
  int32_t keyframeIndex = 0;
  int32_t lastJumpedKeyframe = 0;
  int32_t keyframeJumpCount = 0;
  int32_t lastKeyframeTime = 0;

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::DEMO_STATE;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_int("lobbytype", &lobbyType) &&
           validate(lobbyType) && lobbyMsg.package_bool("paused", &paused) &&
           lobbyMsg.package_int("servertime", &serverTime) &&
           lobbyMsg.package_int("timescale", &timeScale) &&
           lobbyMsg.package_int("client", &client) && validate(client) &&
           lobbyMsg.package_int("kframeindex", &keyframeIndex) &&
           lobbyMsg.package_int("lstjumpedkframe", &lastJumpedKeyframe) &&
           lobbyMsg.package_int("kframejumpcount", &keyframeJumpCount) &&
           lobbyMsg.package_int("lstkframetime", &lastKeyframeTime);
  }
});
ASSERT_SIZE(Msg_DemoState, 0x24);

// Verified
PACKED(struct Msg_LobbyClientDisconnect {
  LobbyType lobbyType = LobbyType::INVALID;
  uint8_t _padding04[4];
  XUID xuid = 0;

  static inline constexpr MsgType MESSAGE_TYPE =
      MsgType::LOBBY_CLIENT_DISCONNECT;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_int("lobbytype", &lobbyType) &&
           validate(lobbyType) && lobbyMsg.package_xuid("xuid", &xuid);
  }
});
ASSERT_SIZE(Msg_LobbyClientDisconnect, 0x10);

// Verified
PACKED(struct Msg_LobbyHostHeartbeat {
  int32_t heartbeatNum = 0;
  LobbyType lobbyType = LobbyType::INVALID;
  MsgHostMigrateInfo migrateInfo = {};

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::LOBBY_HOST_HEARTBEAT;

  bool package(LobbyMsg & lobbyMsg);
});
ASSERT_SIZE(Msg_LobbyHostHeartbeat, 0xA0);

// Verified
PACKED(struct Msg_LobbyIngameMigrateMigrateTo {
  LobbyType lobbyType = LobbyType::INVALID;
  uint8_t _padding04[4];
  XUID newHostXuid = 0;

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::INGAME_MIGRATE_TO;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_int("lobbytype", &lobbyType) &&
           validate(lobbyType) &&
           lobbyMsg.package_xuid("newhost", &newHostXuid);
  }
});
ASSERT_SIZE(Msg_LobbyIngameMigrateMigrateTo, 0x10);

// Verified
PACKED(struct Msg_InfoRequest {
  uint32_t nonce = 0;

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::INFO_REQUEST;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_uint("nonce", &nonce);
  }
});
ASSERT_SIZE(Msg_InfoRequest, 0x4);

PACKED(struct Msg_InfoResponseLobbyServer {
  clientplatform_t clientPlatform = clientplatform_t::PC;
  uint8_t _padding04[4];
  LobbyModulePool<info::InfoResponseLobby> lobby = {{}};
  XUID xuid = 0;
  name_t gamertag = {0};
  eModes sessionMode = eModes::INVALID;
  name_t mapName = {0};
  name_t gameType = {0};
  bool isInGame = false;
  uint8_t _padding1B5[3];
  ClientNum_t maxClients = ClientNum_t::INVALID_CLIENT_INDEX;
  ClientNum_t clientCount = ClientNum_t::INVALID_CLIENT_INDEX;
  LobbyClientPool<LANClientInfo> client = {{}};

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::SERVERLIST_INFO;

  bool package(LobbyMsg & lobbyMsg);

  IMPL_SLICE_METHODS(client, clientCount, Client);
});
ASSERT_SIZE(Msg_InfoResponseLobbyServer, 0x490);

// Verified
PACKED(struct Msg_JoinAgreementRequest {
  XUID hostXuid = 0;
  name_t hostName = {0};
  net::bdSecurityID secId = {};
  net::bdSecurityKey secKey = {};
  net::SerializedAdr serializedAdr = {};
  uint8_t _padding66[2];
  LobbyType sourceLobbyType = LobbyType::INVALID;
  LobbyType destinationLobbyType = LobbyType::INVALID;
  LobbyParams destinationLobbyParams = {};
  uint64_t reservationKey = 0;
  int32_t agreementNonce = 0;
  int32_t serverLoc = 0;

  static inline constexpr MsgType MESSAGE_TYPE =
      MsgType::JOIN_AGREEMENT_REQUEST;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_xuid("hostxuid", &hostXuid) &&
           lobbyMsg.package_string("hostname", hostName) &&
           lobbyMsg.package_uint64("reservationkey", &reservationKey) &&
           lobbyMsg.package_glob("secid", &secId, sizeof(secId)) &&
           lobbyMsg.package_glob("seckey", &secKey, sizeof(secKey)) &&
           lobbyMsg.package_glob("serializedadr", serializedAdr.addrBuff,
                                 sizeof(serializedAdr.addrBuff)) &&
           lobbyMsg.package_int("srclobbytype", &sourceLobbyType) &&
           validate(sourceLobbyType) &&
           lobbyMsg.package_int("destlobbytype", &destinationLobbyType) &&
           validate(destinationLobbyType) &&
           lobbyMsg.package_int("destnetworkmode",
                                &destinationLobbyParams.networkMode) &&
           validate(destinationLobbyParams.networkMode) &&
           lobbyMsg.package_int("destmainmode",
                                &destinationLobbyParams.mainMode) &&
           validate(destinationLobbyParams.mainMode) &&
           lobbyMsg.package_int("nonce", &agreementNonce) &&
           lobbyMsg.package_int("serverloc", &serverLoc);
  }
});
ASSERT_SIZE(Msg_JoinAgreementRequest, 0x88);

// Verified
PACKED(struct Msg_JoinComplete {
  // Field name is serialized as "xuid" in lobby message
  bool shouldCommit = false;

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::JOIN_COMPLETE;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_bool("xuid", &shouldCommit);
  }
});
ASSERT_SIZE(Msg_JoinComplete, 0x1);

// Verified
PACKED(struct Msg_JoinMemberInfo {
  MsgMutableClientInfo mutableClientInfoMsg = {};
  MsgFixedClientInfo fixedClientInfoMsg = {};
  uint64_t reservationKey = 0;
  LobbyType targetLobby = LobbyType::INVALID;
  net::SerializedAdr serializedAdr = {};
  uint8_t _padding512[6];

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::JOIN_MEMBER_INFO;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_glob("serializedadr", serializedAdr.addrBuff, 37) &&
           lobbyMsg.package_uint64("reservationkey", &reservationKey) &&
           lobbyMsg.package_int("lobbytype", &targetLobby) &&
           validate(targetLobby) && fixedClientInfoMsg.package(lobbyMsg) &&
           mutableClientInfoMsg.package(lobbyMsg);
  }
});
ASSERT_SIZE(Msg_JoinMemberInfo, 0x518);

// Verified
PACKED(struct Msg_JoinResponse {
  join::JoinResult response = join::JoinResult::SUCCESS;
  name_t name = {0};
  uint32_t serverLocation = 0;
  LobbyType lobbyType = LobbyType::INVALID;
  LobbyParams lobbyParams = {};
  uint8_t _padding34[4];
  uint64_t reservationKey = 0;

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::JOIN_RESPONSE;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_int("response", &response) &&
           lobbyMsg.package_string("name", name) &&
           lobbyMsg.package_uint("serverlocation", &serverLocation) &&
           lobbyMsg.package_int("lobbytype", &lobbyType) &&
           validate(lobbyType) &&
           lobbyMsg.package_int("networkmode", &lobbyParams.networkMode) &&
           validate(lobbyParams.networkMode) &&
           lobbyMsg.package_int("mainmode", &lobbyParams.mainMode) &&
           validate(lobbyParams.mainMode) &&
           lobbyMsg.package_uint64("reservationkey", &reservationKey);
  }
});
ASSERT_SIZE(Msg_JoinResponse, 0x40);

// Verified
PACKED(struct MsgClientMigrateInfo {
  uint16_t totalLatencyMS = 0;
  uint8_t _padding02[2];
  uint32_t recievedPacketCount = 0;
  float m = 0.0f;
  uint32_t uploadBandwidth = 0;
});
ASSERT_SIZE(MsgClientMigrateInfo, 0x10);

// Verified
PACKED(struct Msg_LobbyClientHeartbeat {
  LobbyType lobbyType = LobbyType::INVALID;
  uint8_t _padding04[4];
  MsgMutableClientInfo mutableClientInfoMsg = {};
  MsgClientMigrateInfo migrateInfo = {};

  static inline constexpr MsgType MESSAGE_TYPE =
      MsgType::LOBBY_CLIENT_HEARTBEAT;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_int("lobbytype", &lobbyType) &&
           validate(lobbyType) &&
           lobbyMsg.package_ushort("totallatencyms",
                                   &migrateInfo.totalLatencyMS) &&
           lobbyMsg.package_uint("recievedpackets",
                                 &migrateInfo.recievedPacketCount) &&
           lobbyMsg.package_float("totalstddev", &migrateInfo.m) &&
           lobbyMsg.package_uint("uploadbandwidth",
                                 &migrateInfo.uploadBandwidth) &&
           mutableClientInfoMsg.package(lobbyMsg);
  }
});
ASSERT_SIZE(Msg_LobbyClientHeartbeat, 0x428);

// Verified
PACKED(struct Msg_LobbyHostDisconnect {
  LobbyType lobbyType = LobbyType::INVALID;
  uint8_t _padding04[4];
  XUID xuid = 0;

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::LOBBY_HOST_DISCONNECT;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_int("lobbytype", &lobbyType) &&
           validate(lobbyType) && lobbyMsg.package_xuid("xuid", &xuid);
  }
});
ASSERT_SIZE(Msg_LobbyHostDisconnect, 0x10);

// Verified
PACKED(struct Msg_LobbyHostLeaveWithParty {
  LobbyType lobbyType = LobbyType::INVALID;

  static inline constexpr MsgType MESSAGE_TYPE =
      MsgType::LOBBY_HOST_LEAVE_WITH_PARTY;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_int("lobbytype", &lobbyType) && validate(lobbyType);
  }
});
ASSERT_SIZE(Msg_LobbyHostLeaveWithParty, 0x4);

// Verified
PACKED(struct Msg_LobbyIngameMigrateNewHost {
  LobbyType lobbyType = LobbyType::INVALID;
  uint8_t _padding04[4];
  XUID hostXuid = 0;

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::MIGRATE_NEW_HOST;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_int("lobbytype", &lobbyType) &&
           validate(lobbyType) && lobbyMsg.package_xuid("hostxuid", &hostXuid);
  }
});
ASSERT_SIZE(Msg_LobbyIngameMigrateNewHost, 0x10);

// Verified
PACKED(struct Msg_LobbyMigrateAnnounceHost {
  LobbyType lobbyType = LobbyType::INVALID;
  int32_t timeout = 0;

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::MIGRATE_ANNOUNCE_HOST;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_int("lobbytype", &lobbyType) &&
           validate(lobbyType) && lobbyMsg.package_int("timeout", &timeout);
  }
});
ASSERT_SIZE(Msg_LobbyMigrateAnnounceHost, 0x8);

// Verified
PACKED(struct Msg_LobbyMigrateStart {
  LobbyType lobbyType = LobbyType::INVALID;
  uint8_t _padding04[4];
  XUID migrateTo = 0;

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::MIGRATE_START;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_int("lobbytype", &lobbyType) &&
           validate(lobbyType) &&
           lobbyMsg.package_xuid("migrateTo", &migrateTo);
  }
});
ASSERT_SIZE(Msg_LobbyMigrateStart, 0x10);

// Verified
PACKED(struct Msg_PeerToPeerInfo {
  LobbyType lobbyType = LobbyType::INVALID;
  int32_t connectivityBits = 0;
  XUID clientXuid = 0;

  static inline constexpr MsgType MESSAGE_TYPE = MsgType::PEER_TO_PEER_INFO;

  inline bool package(LobbyMsg & lobbyMsg) {
    return lobbyMsg.package_int("lobbytype", &lobbyType) &&
           validate(lobbyType) &&
           lobbyMsg.package_int("connectbit", &connectivityBits) &&
           lobbyMsg.package_xuid("xuid", &clientXuid);
  }
});
ASSERT_SIZE(Msg_PeerToPeerInfo, 0x10);

} // namespace msg
} // namespace lobby
} // namespace game
