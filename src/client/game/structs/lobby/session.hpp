#pragma once

#include <game/structs/macros.hpp>
#include <game/structs/net/core.hpp>
#include <game/structs/weapon.hpp>
#include "core.hpp"
#include <cstdint>

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
  XUID xuid;
  active::ActiveClient *activeClient;
  LobbyClientType clientType;
  MatchInfo matchInfo;
  VoiceChatInfo voiceInfo;
  uint32_t joinOrder;
  uint64_t joinNonce;
};

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

PACKED(struct HostInfo {
  XUID xuid;
  char name[32];
  net::netadr_t netAdr;
  net::SerializedAdr serializedAdr;
  net::bdSecurityID secId;
  net::bdSecurityKey secKey;
  uint8_t _padding76[2];
});

PACKED(struct SessionHost {
  HostInfo info;
  int32_t maxClients;
  uint8_t _padding7C[4];
});

struct SessionLeader {
  XUID xuid;
};

PACKED(struct SessionMigrateData {
  uint8_t indexBits;
  uint8_t _padding[3];
  int32_t startTime;
});

struct CustomGameDetails {
  char name[64];
  bool isOfficial;
};

struct LobbyCustomData {
  CustomGameDetails customGameDetails;
};

PACKED(struct LobbyTheaterData {
  char demoName[32];
  uint64_t fileId;
  int32_t fileSize;
  bool isUserFile;
  bool isLiveStream;
  uint8_t _padding2E[2];
  uint64_t authorXUID;
  enum eModes mainMode;
  char demoMode[32];
  char fileCategory[32];
  int32_t downloadPercent;
  bool readyForPlayback;
  bool retryDownload;
  uint8_t _padding82[2];
  int32_t lastHeartBeatTime;
});

PACKED(struct LobbyPublicData {
  int32_t playlistId;
  int32_t playlistCurrentEntry;
  uint8_t playlistLastEntries[8];
  uint8_t playlistNext;
  uint8_t playlistPrev;
  uint8_t playlistPrevCount;
  uint8_t _padding13[1];
});

struct CharacterSelection {
  uint32_t characterIndex;
  heroLoadoutTypes_e loadoutType;
};

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

PACKED(struct ItemVote {
  XUID clientXuid;
  VoteItemType itemType;
  uint32_t itemIndex;
  itemGroup_t itemGroup;
  weapon::eAttachment attachment;
  VoteType voteType;
  uint8_t _padding1C[4];
});

PACKED(struct LobbyGameData {
  ControllerIndex_t controllerIndex;
  GameServerStatus serverStatus;
  int32_t launchNonce;
  uint8_t _padding0C[4];
  uint128_t matchHash;
  bool preLoaded;
  char gameType[32];
  char mapName[32];
  uint8_t _padding61[3];
  int32_t status;
  int32_t statusValue;
  LobbyPregameState pregameState;
  uint32_t pregameVoteCount;
  uint8_t _padding74[4];
  ItemVote pregameVotes[216];
  GameClientData client[18];
  LobbyPublicData publicData;
  LobbyCustomData customData;
  uint8_t _padding1E55[3];
  LobbyTheaterData theaterData;
});

struct LobbyPrivateData {
  int32_t playlistId;
};

PACKED(struct LobbyData {
  bool dirty;
  uint8_t _padding01[3];
  eGameModes gameMode;
  XUID platformSessionID;
  const LobbyType *type;
  const LobbyMode *mode;
  LobbyPrivateData lobbyPrivate;
  uint8_t _padding24[4];
  LobbyGameData lobbyGame;
});

PACKED(struct LobbySession {
  LobbyModule module;
  LobbyType type;
  LobbyMode mode;
  SessionStatus status;
  const char *name;
  char key[32];
  uint8_t _unknown38[8];
  SessionActive active;
  int dirty;
  uint64_t flags;
  bool sessionAdvertised;
  bool qosListenEnabled;
  uint8_t _padding52[6];
  active::ActiveList *activeList;
  SessionHost host;
  SessionLeader leader;
  uint32_t joinCount;
  uint8_t _paddingEC[4];
  uint64_t _unknown;
  SessionClient client[18];
  bool inRecovery;
  uint8_t _padding459[3];
  SessionMigrateData migrateData;
  uint8_t _padding464[4];
  LobbyData lobbyData;
});
ASSERT_SIZE(LobbySession, 0x2370);

} // namespace session
} // namespace lobby
} // namespace game