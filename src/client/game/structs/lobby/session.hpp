#pragma once

#include <cstdint>

#include <game/structs/lobby/core.hpp>
#include <game/structs/lobby/info.hpp>
#include <game/structs/lobby/msg/core.hpp>
#include <game/structs/macros.hpp>
#include <game/structs/net/core.hpp>
#include <game/structs/weapon.hpp>

namespace game {
namespace lobby {

namespace active {
struct ActiveClient;
struct ActiveList;
} // namespace active

enum class itemGroup_t : int32_t {
  SMG = 0x0,
  FIRST = 0x0,
  ASSAULT = 0x1,
  CQB = 0x2,
  LMG = 0x3,
  SNIPER = 0x4,
  PISTOL = 0x5,
  LAUNCHER = 0x6,
  KNIFE = 0x7,
  SPECIAL = 0x8,
  SHOTGUN = 0x9,
  EXPLOSIVE = 0x10,
  GRENADE = 0x11,
  MASTERKEY = 0x12,
  GRENADELAUNCHER = 0x13,
  FLAMETHROWER = 0x14,
  SPECIALTY = 0x15,
  SPECIALGRENADE = 0x16,
  MISCWEAPON = 0x17,
  FEATURE = 0x18,
  BONUSCARD = 0x19,
  CYBERCOM_TACRIG = 0x20,
  GLOBAL_ITEMS_START = 0x21,
  KILLSTREAK = 0x21,
  HERO = 0x22,
  BUBBLEGUM = 0x23,
  BUBBLEGUM_CONSUMABLE = 0x24,
  CYBERCOM_CORE = 0x25,
  CYBERCOM_ABILITIES = 0x26,
  COUNT = 0x27,
  INVALID = -1,
};
IMPL_ENUM_OPERATORS(itemGroup_t);

enum class VoteType : uint32_t {
  INVALID = 0x0,
  BAN = 0x1,
  PROTECT = 0x2,
  COUNT = 0x3,
};
IMPL_ENUM_OPERATORS(VoteType);
enum class VoteItemType : uint32_t {
  INVALID = 0x0,
  ITEM = 0x1,
  ATTACHMENT = 0x2,
  ITEM_ATTACHMENT = 0x3,
  ATTACHMENT_GROUP = 0x4,
  COUNT = 0x5,
};
IMPL_ENUM_OPERATORS(VoteItemType);

enum class GameServerStatus : uint32_t {
  IDLE = 0x0,
  PRELOADING = 0x1,
  STARTING_SERVER = 0x2,
  MIGRATING = 0x3,
  RUNNING = 0x4,
  RUNNING_DEMO = 0x5,
};
IMPL_ENUM_OPERATORS(GameServerStatus);

enum class heroLoadoutTypes_e : int32_t {
  INVALID = -1,
  FIRST = 0x0,
  WEAPON = 0x0,
  GADGET = 0x1,
  COUNT = 0x2,
};
IMPL_ENUM_OPERATORS(heroLoadoutTypes_e);

enum class LobbyPregameState : uint32_t {
  IDLE = 0x0,
  ITEM_VOTE = 0x1,
  CHARACTER_DRAFT = 0x2,
  CAC_EDIT = 0x3,
  SCORESTREAK_EDIT = 0x4,
  LAUNCH_GAME = 0x5,
  COUNT = 0x6,
};
IMPL_ENUM_OPERATORS(LobbyPregameState);

enum class LobbyClientPregameState : uint32_t {
  IDLE = 0x0,
  SELECTING = 0x1,
  SELECTED = 0x2,
  SELECTION_FORCED = 0x3,
  SELECTION_COMPLETE = 0x4,
  RESET = 0x5,
  END = 0x6,
  COUNT = 0x7,
};
IMPL_ENUM_OPERATORS(LobbyClientPregameState);

namespace session {
PACKED(struct SessionInfo {
  bool inSession;
  uint8_t _padding01[3];
  net::netadr_t netAdr;
  uint8_t _padding14[4];
  time_t lastMessageSentToPeer;
});

struct MatchInfo {
  uint32_t score;
};

PACKED(struct VoiceChatInfo {
  int32_t lastCaughtTalking;
  int32_t connectivityBits;
  bool isGameRestricted;
  uint8_t _padding09[3];
});

struct SessionClient {
  XUID xuid;                          // Verified
  active::ActiveClient *activeClient; // Verified
  LobbyClientType clientType;
  MatchInfo matchInfo;
  VoiceChatInfo voiceInfo;
  uint32_t joinOrder;
  uint64_t joinNonce;
};
ASSERT_OFFSET(SessionClient, xuid, 0x0);
// Verified
ASSERT_SIZE(SessionClient, 0x30);

enum class SessionStatus : int32_t {
  INVALID = -1,
  IDLE = 0x0,
  JOIN = 0x1,
  MERGE = 0x2,
  MIGRATE = 0x3,
  PAUSED = 0x4,
  SESSION_STATUS_ERROR = 0x5,
  COUNT = 0x6,
};
IMPL_ENUM_OPERATORS(SessionStatus);

enum class SessionActive : int32_t {
  INACTIVE = 0x0,
  KEEP_ALIVE = 0x1,
  ACTIVE = 0x2,
};
IMPL_ENUM_OPERATORS(SessionActive);
enum class SessionDirty : int32_t {
  STATE = 0x1,
  UI = 0x2,
  CLIENT_LIST = 0x4,
};
IMPL_ENUM_OPERATORS(SessionDirty);

PACKED(struct SessionHost {
  net::HostInfo info; // Verified
  ClientNum_t maxClients;
  uint8_t _padding7C[4];
});
ASSERT_SIZE(SessionHost, 0x88);

struct SessionLeader {
  XUID xuid;
};

PACKED(struct SessionMigrateData {
  uint8_t indexBits;
  uint8_t _padding[3];
  int32_t startTime;
});

// Verified
PACKED(struct CustomGameDetails {
  str64_t name;
  qboolean isOfficial;
});
ASSERT_SIZE(CustomGameDetails, 0x44);

// Verified
struct LobbyCustomData {
  CustomGameDetails customGameDetails;
};

// Verified
PACKED(struct LobbyTheaterData {
  name_t demoName;
  uint64_t fileId;
  int32_t fileSize;
  bool isUserFile;
  bool isLiveStream;
  uint8_t _padding2E[2];
  uint64_t authorXUID;
  eModes mainMode;
  name_t demoMode;
  name_t fileCategory;
  int32_t downloadPercent;
  bool readyForPlayback;
  bool retryDownload;
  uint8_t _padding82[2];
  int32_t lastHeartBeatTime;
});
ASSERT_SIZE(LobbyTheaterData, 0x88);

// Verified
PACKED(struct LobbyPublicData {
  int32_t playlistId;
  int32_t playlistCurrentEntry;
  uint8_t playlistLastEntries[8];
  uint8_t playlistNext;
  uint8_t playlistPrev;
  uint8_t playlistPrevCount;
  uint8_t _padding13[1];
});
ASSERT_SIZE(LobbyPublicData, 0x14);

struct CharacterSelection {
  uint32_t characterIndex;
  heroLoadoutTypes_e loadoutType;
};

// Verified
PACKED(struct GameClientData {
  team_t team;
  LobbyMapVote mapVote;
  bool readyUp;
  uint8_t _padding09[3];
  LobbyClientPregameState pregameState;
  int32_t pregamePosition;
  bool pregameEditComplete;
  uint8_t _padding15[3];
  uint32_t itemVoteCount;
  CharacterSelection characterSelection;
});
// Verified
ASSERT_SIZE(GameClientData, 0x24);

PACKED(struct ItemVote {
  XUID clientXuid;
  VoteItemType itemType;
  uint32_t itemIndex;
  itemGroup_t itemGroup;
  weapon::eAttachment attachment;
  VoteType voteType;
  /*
    Note: this is a new field, used only where
    `ItemVote` is stored in the local `LobbySession`, under
    `lobbySession.lobbyData.lobbyGame.pregameVotes`, for internal comparison to
    incoming packet data that indicates a new item vote count.

    This field is not serialized in `ItemVote` data in any packets, and
    otherwise is not treated as existent. Its addition does not affect the data
    structure size however, as there were previously four bytes
    (`4 == sizeof(uint32_t)`) of padding here.
  */
  uint32_t pregameVoteCount;
});

PACKED(struct LobbyGameData {
  ControllerIndex_t controllerIndex;
  GameServerStatus serverStatus;
  int32_t launchNonce;
  uint8_t _padding0C[4];
  uint128_t matchHash;
  bool preLoaded;
  name_t gameType;
  name_t mapName;
  uint8_t _padding61[3];
  info::LobbyUGCInfo ugcInfo;
  int32_t status;
  int32_t statusValue;
  LobbyPregameState pregameState;
  uint32_t pregameVoteCount;
  ItemVote pregameVotes[216];
  LobbyClientPool<GameClientData> client;
  LobbyPublicData publicData; // Verified
  LobbyCustomData customData; // Verified
  // TODO: the data structure is correct, but why is there a separate field for
  // this? This is duplicative of publicData, and does not exist in BO4 or BO3
  // alpha engines.
  LobbyPublicData playlistData; // Verified
  uint8_t _padding1E8C[4];
  LobbyTheaterData theaterData; //  Verified
});
ASSERT_SIZE(LobbyGameData, 0x1F18);

// Verified
struct LobbyPrivateData {
  int32_t playlistId;
};

PACKED(struct LobbyData {
  // This actually acts more like bitflags representing lobby session state -
  // `dirty` is a misleading struct field name.
  uint8_t dirty; // Verified
  uint8_t _padding01[3];
  eGameModes gameMode;    // Verified
  XUID platformSessionID; // Verified
  const LobbyType *type;
  const LobbyMode *mode;         // Verified
  LobbyPrivateData lobbyPrivate; // Verified
  uint8_t _padding24[4];
  LobbyGameData lobbyGame; // Verified
});

PACKED(struct LobbySession {
  LobbyModule module;   // Verified
  LobbyType type;       // Verified
  LobbyMode mode;       // Verified
  SessionStatus status; // Verified
  void *_unknown10;
  const char *name;       // Verified
  str32_t key;            // Verified
  SessionActive active;   // Verified
  qboolean dirty;         // Verified
  uint64_t flags;         // Verified
  bool sessionAdvertised; // Verified
  bool qosListenEnabled;
  uint8_t _padding52[6];
  active::ActiveList *activeList; // Verified
  SessionHost host;               // Verified
  SessionLeader leader;
  uint32_t joinCount;
  uint8_t _paddingF4[4];
  LobbyClientPool<SessionClient> client; // Verified
  bool inRecovery;                       // Verified
  uint8_t _padding459[3];
  SessionMigrateData migrateData;
  uint8_t _padding464[4];
  LobbyData lobbyData; // Verified
  info::LobbyUGCInfo ugcInfo;
  uint8_t _padding23CC[4];
});
ASSERT_OFFSET(LobbySession, inRecovery, 0x458);
ASSERT_OFFSET(LobbySession, client, 0xF8);
ASSERT_OFFSET(LobbySession, host, 0x60);
ASSERT_SIZE(LobbySession, 0x23D0);

struct PlatformSessionData {
  str64_t sessionId;
  str64_t sessionInfo;
};

// Verified
PACKED(struct GameData {
  GameServerStatus serverStatus;
  int32_t launchNonce;
  uint128_t matchHash;
  eGameModes gameMode;
  name_t gameType;
  name_t mapName;
  info::LobbyUGCInfo ugcInfo;
  int32_t status;
  int32_t statusValue;
  LobbyPregameState pregameState;
  uint32_t pregameVoteCount;
  ItemVote pregameVotes[216];
  int32_t playlistId;
  int32_t playlistCurrentEntry;
  uint8_t playlistLastEntries[8];
  uint8_t playlistNext;
  uint8_t playlistPrev;
  uint8_t playlistPrevCount;
  uint8_t _padding1BA3[1];
  LobbyClientPool<GameClientData> client;
  name_t cpQueuedLevel;
  bool loadingMovieSkipped;
  name_t demoName;
  uint8_t _padding1E6D[3];
  uint64_t fileId;
  int32_t fileSize;
  bool isUserFile;
  bool isLiveStream;
  uint8_t _padding1E7E[2];
  XUID authorXUID;
  eModes mainMode;
  name_t demoMode;
  name_t fileCategory;
  CustomGameDetails customGameDetails;
});
// Verified
ASSERT_SIZE(GameData, 0x1F10);

typedef array<uint8_t, 0xC000> gametypesettings_t;

enum class ServerStatus : uint32_t {
  PRESENT = 0x0,
  DELAYED = 0x1,
  MISSING = 0x2,
};

struct PeerToPeerInfo {
  int32_t connectivityBits;
  msg::NetChanMsg netMsg[18];
};

struct SharedVoiceSessionData {
  uint32_t voiceOutPort;
  uint32_t microphonePort;
  uint32_t headsetPort;
  bool microphoneAttached;
  uint32_t lastCaughtTalking;
};

struct LobbyPeerResults {
  uint32_t receivedPacketCount;
  uint32_t totalLatencyMS;
  float m;
  float s;
};

enum class LobbyMigrateClientState : uint32_t {
  CLIENT_IDLE = 0x0,
  CLIENT_SENDING_PACKETS = 0x1,
  WAITING_FOR_RESPONSES = 0x2,
  ACTIVE = 0x3,
  BECOMING_HOST = 0x4,
  CREATING_HOST = 0x5,
  COUNT = 0x6,
};

struct LobbyMigrateClientData {
  LobbyClientPool<LobbyPeerResults> peerResults;
  LobbyClientPool<XUID> nominees;
  LobbyMigrateClientState clientState;
  uint32_t sentPacketCount;
  int32_t lastPacketSendMs;
  int32_t currentNominee;
  int32_t migrateStartMs;
  int32_t lasthostTimeMS;
  int32_t hostStartMS;
  int32_t lastCalcTime;
  int32_t nextTestPacketSendMS;
  XUID peerXor;
};

PACKED(struct ClientSession {
  LobbySession lobbySession; // Verified
  uint32_t stateNum;
  int32_t lastHostMsgTimestamp; // Verified
  ServerStatus serverStatus;    // Verified
  uint8_t _padding23DC[4];
  PeerToPeerInfo peerToPeerInfo;
  LocalClientPool<msg::NetChanMsg> channel;
  LocalClientPool<msg::NetChanMsg> clientContentMsg;
  LocalClientPool<msg::NetChanMsg> modifiedStatsMsgCfg; // Verified
  LobbyMigrateClientData clientMigrateData;
  LocalClientPool<bool> voiceCommunicationActive;
  LocalClientPool<bool> paintshopDataDirty;
  uint8_t _padding2A4C[4];
  SharedVoiceSessionData *platformVoiceData;
});

ASSERT_OFFSET(ClientSession, lobbySession, 0x0);
ASSERT_OFFSET(ClientSession, lastHostMsgTimestamp, 0x23D4);
ASSERT_OFFSET(ClientSession, serverStatus, 0x23D8);
// Verified
ASSERT_SIZE(ClientSession, 0x2A58);

} // namespace session
} // namespace lobby
} // namespace game
