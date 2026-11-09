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

constexpr bool validate(const itemGroup_t group) {
  return group >= itemGroup_t::SMG && group < itemGroup_t::COUNT;
}

[[nodiscard]] inline constexpr const char *serialize(const itemGroup_t group) {
  switch (group) {
  case itemGroup_t::SMG:
    return "ITEM_GROUP_SMG";
  case itemGroup_t::ASSAULT:
    return "ITEM_GROUP_ASSAULT";
  case itemGroup_t::CQB:
    return "ITEM_GROUP_CQB";
  case itemGroup_t::LMG:
    return "ITEM_GROUP_LMG";
  case itemGroup_t::SNIPER:
    return "ITEM_GROUP_SNIPER";
  case itemGroup_t::PISTOL:
    return "ITEM_GROUP_PISTOL";
  case itemGroup_t::LAUNCHER:
    return "ITEM_GROUP_LAUNCHER";
  case itemGroup_t::KNIFE:
    return "ITEM_GROUP_KNIFE";
  case itemGroup_t::SPECIAL:
    return "ITEM_GROUP_SPECIAL";
  case itemGroup_t::SHOTGUN:
    return "ITEM_GROUP_SHOTGUN";
  case itemGroup_t::EXPLOSIVE:
    return "ITEM_GROUP_EXPLOSIVE";
  case itemGroup_t::GRENADE:
    return "ITEM_GROUP_GRENADE";
  case itemGroup_t::MASTERKEY:
    return "ITEM_GROUP_MASTERKEY";
  case itemGroup_t::GRENADELAUNCHER:
    return "ITEM_GROUP_GRENADELAUNCHER";
  case itemGroup_t::FLAMETHROWER:
    return "ITEM_GROUP_FLAMETHROWER";
  case itemGroup_t::SPECIALTY:
    return "ITEM_GROUP_SPECIALTY";
  case itemGroup_t::SPECIALGRENADE:
    return "ITEM_GROUP_SPECIALGRENADE";
  case itemGroup_t::MISCWEAPON:
    return "ITEM_GROUP_MISCWEAPON";
  case itemGroup_t::FEATURE:
    return "ITEM_GROUP_FEATURE";
  case itemGroup_t::BONUSCARD:
    return "ITEM_GROUP_BONUSCARD";
  case itemGroup_t::CYBERCOM_TACRIG:
    return "ITEM_GROUP_CYBERCOM_TACRIG";
  case itemGroup_t::KILLSTREAK:
    return "ITEM_GROUP_KILLSTREAK";
  case itemGroup_t::HERO:
    return "ITEM_GROUP_HERO";
  case itemGroup_t::BUBBLEGUM:
    return "ITEM_GROUP_BUBBLEGUM";
  case itemGroup_t::BUBBLEGUM_CONSUMABLE:
    return "ITEM_GROUP_BUBBLEGUM_CONSUMABLE";
  case itemGroup_t::CYBERCOM_CORE:
    return "ITEM_GROUP_CYBERCOM_CORE";
  case itemGroup_t::CYBERCOM_ABILITIES:
    return "ITEM_GROUP_CYBERCOM_ABILITIES";
  case itemGroup_t::COUNT:
    return "ITEM_GROUP_COUNT";
  case itemGroup_t::INVALID:
    return "ITEM_GROUP_INVALID";
  default:
    return "ITEM_GROUP_UNKNOWN";
  }
}

enum class VoteType : uint32_t {
  INVALID = 0x0,
  BAN = 0x1,
  PROTECT = 0x2,
  COUNT = 0x3,
};
IMPL_ENUM_OPERATORS(VoteType);

constexpr bool validate(const VoteType type) {
  return type >= VoteType::BAN && type < VoteType::COUNT;
}

[[nodiscard]] inline constexpr const char *serialize(const VoteType type) {
  switch (type) {
  case VoteType::INVALID:
    return "VOTE_TYPE_INVALID";
  case VoteType::BAN:
    return "VOTE_TYPE_BAN";
  case VoteType::PROTECT:
    return "VOTE_TYPE_PROTECT";
  case VoteType::COUNT:
    return "VOTE_TYPE_COUNT";
  default:
    return "VOTE_TYPE_UNKNOWN";
  }
}

enum class VoteItemType : uint32_t {
  INVALID = 0x0,
  ITEM = 0x1,
  ATTACHMENT = 0x2,
  ITEM_ATTACHMENT = 0x3,
  ATTACHMENT_GROUP = 0x4,
  COUNT = 0x5,
};
IMPL_ENUM_OPERATORS(VoteItemType);

constexpr bool validate(const VoteItemType type) {
  return type >= VoteItemType::ITEM && type < VoteItemType::COUNT;
}

[[nodiscard]] inline constexpr const char *serialize(const VoteItemType type) {
  switch (type) {
  case VoteItemType::INVALID:
    return "VOTE_ITEM_TYPE_INVALID";
  case VoteItemType::ITEM:
    return "VOTE_ITEM_TYPE_ITEM";
  case VoteItemType::ATTACHMENT:
    return "VOTE_ITEM_TYPE_ATTACHMENT";
  case VoteItemType::ITEM_ATTACHMENT:
    return "VOTE_ITEM_TYPE_ITEM_ATTACHMENT";
  case VoteItemType::ATTACHMENT_GROUP:
    return "VOTE_ITEM_TYPE_ATTACHMENT_GROUP";
  case VoteItemType::COUNT:
    return "VOTE_ITEM_TYPE_COUNT";
  default:
    return "VOTE_ITEM_TYPE_UNKNOWN";
  }
}

enum class GameServerStatus : uint32_t {
  IDLE = 0x0,
  PRELOADING = 0x1,
  STARTING_SERVER = 0x2,
  MIGRATING = 0x3,
  RUNNING = 0x4,
  RUNNING_DEMO = 0x5,
};
IMPL_ENUM_OPERATORS(GameServerStatus);

[[nodiscard]] inline constexpr const char *
serialize(const GameServerStatus status) {
  switch (status) {
  case GameServerStatus::IDLE:
    return "GAME_SERVER_STATUS_IDLE";
  case GameServerStatus::PRELOADING:
    return "GAME_SERVER_STATUS_PRELOADING";
  case GameServerStatus::STARTING_SERVER:
    return "GAME_SERVER_STATUS_STARTING_SERVER";
  case GameServerStatus::MIGRATING:
    return "GAME_SERVER_STATUS_MIGRATING";
  case GameServerStatus::RUNNING:
    return "GAME_SERVER_STATUS_RUNNING";
  case GameServerStatus::RUNNING_DEMO:
    return "GAME_SERVER_STATUS_RUNNING_DEMO";
  default:
    return "GAME_SERVER_STATUS_UNKNOWN";
  }
}

constexpr bool validate(const GameServerStatus status) {
  return status >= GameServerStatus::IDLE &&
         status <= GameServerStatus::RUNNING_DEMO;
}

enum class heroLoadoutTypes_e : int32_t {
  INVALID = -1,
  FIRST = 0x0,
  WEAPON = 0x0,
  GADGET = 0x1,
  COUNT = 0x2,
};
IMPL_ENUM_OPERATORS(heroLoadoutTypes_e);

constexpr bool validate(const heroLoadoutTypes_e type) {
  return type >= heroLoadoutTypes_e::WEAPON && type < heroLoadoutTypes_e::COUNT;
}

[[nodiscard]] inline constexpr const char *
serialize(const heroLoadoutTypes_e type) {
  switch (type) {
  case heroLoadoutTypes_e::INVALID:
    return "HERO_LOADOUT_INVALID";
  case heroLoadoutTypes_e::WEAPON:
    return "HERO_LOADOUT_WEAPON";
  case heroLoadoutTypes_e::GADGET:
    return "HERO_LOADOUT_GADGET";
  case heroLoadoutTypes_e::COUNT:
    return "HERO_LOADOUT_COUNT";
  default:
    return "HERO_LOADOUT_UNKNOWN";
  }
}

static constexpr auto MAX_ITEM_VOTE_COUNT = 216;

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

constexpr bool validate(const LobbyPregameState state) {
  return state >= LobbyPregameState::IDLE && state < LobbyPregameState::COUNT;
}

[[nodiscard]] inline constexpr const char *
serialize(const LobbyPregameState state) {
  switch (state) {
  case LobbyPregameState::IDLE:
    return "LOBBY_PREGAME_STATE_IDLE";
  case LobbyPregameState::ITEM_VOTE:
    return "LOBBY_PREGAME_STATE_ITEM_VOTE";
  case LobbyPregameState::CHARACTER_DRAFT:
    return "LOBBY_PREGAME_STATE_CHARACTER_DRAFT";
  case LobbyPregameState::CAC_EDIT:
    return "LOBBY_PREGAME_STATE_CAC_EDIT";
  case LobbyPregameState::SCORESTREAK_EDIT:
    return "LOBBY_PREGAME_STATE_SCORESTREAK_EDIT";
  case LobbyPregameState::LAUNCH_GAME:
    return "LOBBY_PREGAME_STATE_LAUNCH_GAME";
  case LobbyPregameState::COUNT:
    return "LOBBY_PREGAME_STATE_COUNT";
  default:
    return "LOBBY_PREGAME_STATE_UNKNOWN";
  }
}

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

constexpr bool validate(const LobbyClientPregameState state) {
  return state >= LobbyClientPregameState::IDLE &&
         state < LobbyClientPregameState::COUNT;
}

[[nodiscard]] inline constexpr const char *
serialize(const LobbyClientPregameState state) {
  switch (state) {
  case LobbyClientPregameState::IDLE:
    return "LOBBY_CLIENT_PREGAME_IDLE";
  case LobbyClientPregameState::SELECTING:
    return "LOBBY_CLIENT_PREGAME_SELECTING";
  case LobbyClientPregameState::SELECTED:
    return "LOBBY_CLIENT_PREGAME_SELECTED";
  case LobbyClientPregameState::SELECTION_FORCED:
    return "LOBBY_CLIENT_PREGAME_SELECTION_FORCED";
  case LobbyClientPregameState::SELECTION_COMPLETE:
    return "LOBBY_CLIENT_PREGAME_SELECTION_COMPLETE";
  case LobbyClientPregameState::RESET:
    return "LOBBY_CLIENT_PREGAME_RESET";
  case LobbyClientPregameState::END:
    return "LOBBY_CLIENT_PREGAME_END";
  case LobbyClientPregameState::COUNT:
    return "LOBBY_CLIENT_PREGAME_COUNT";
  default:
    return "LOBBY_CLIENT_PREGAME_UNKNOWN";
  }
}

IMPL_ENUM_OPERATORS(LobbyClientPregameState);

namespace session {
PACKED(struct SessionInfo {
  bool inSession = false;
  uint8_t _padding01[3];
  net::netadr_t netAdr = {};
  uint8_t _padding14[4];
  time_t lastMessageSentToPeer = 0;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"inSession\": {}, \"netAdr\": {}, "
                       "\"lastMessageSentToPeer\": {} }}",
                       ::serialize(inSession), netAdr.serialize(),
                       lastMessageSentToPeer);
  }
});

struct MatchInfo {
  uint32_t score = 0;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"score\": {} }}", score);
  }
};

PACKED(struct VoiceChatInfo {
  int32_t lastCaughtTalking = 0;
  int32_t connectivityBits = 0;
  bool isGameRestricted = false;
  uint8_t _padding09[3];

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"lastCaughtTalking\": {}, \"connectivityBits\": "
                       "{}, \"isGameRestricted\": {} }}",
                       lastCaughtTalking, connectivityBits,
                       ::serialize(isGameRestricted));
  }
});

struct SessionClient {
  XUID xuid = 0;                                // Verified
  active::ActiveClient *activeClient = nullptr; // Verified
  LobbyClientType clientType = LobbyClientType::INVALID;
  MatchInfo matchInfo = {};
  VoiceChatInfo voiceInfo = {};
  uint32_t joinOrder = 0;
  uint64_t joinNonce = 0;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"xuid\": {:016X}, \"activeClient\": {}, \"clientType\": {}, "
        "\"matchInfo\": {}, \"voiceInfo\": {}, \"joinOrder\": {}, "
        "\"joinNonce\": {} }}",
        xuid, ::serialize(activeClient), lobby::serialize(clientType),
        matchInfo.serialize(), voiceInfo.serialize(), joinOrder, joinNonce);
  }
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

[[nodiscard]] inline constexpr const char *
serialize(const SessionStatus status) {
  switch (status) {
  case SessionStatus::INVALID:
    return "SESSION_STATUS_INVALID";
  case SessionStatus::IDLE:
    return "SESSION_STATUS_IDLE";
  case SessionStatus::JOIN:
    return "SESSION_STATUS_JOIN";
  case SessionStatus::MERGE:
    return "SESSION_STATUS_MERGE";
  case SessionStatus::MIGRATE:
    return "SESSION_STATUS_MIGRATE";
  case SessionStatus::PAUSED:
    return "SESSION_STATUS_PAUSED";
  case SessionStatus::SESSION_STATUS_ERROR:
    return "SESSION_STATUS_ERROR";
  case SessionStatus::COUNT:
    return "SESSION_STATUS_COUNT";
  default:
    return "SESSION_STATUS_UNKNOWN";
  }
}

constexpr bool validate(const SessionStatus status) {
  return status >= SessionStatus::IDLE && status < SessionStatus::COUNT;
}

enum class SessionActive : int32_t {
  INACTIVE = 0x0,
  KEEP_ALIVE = 0x1,
  ACTIVE = 0x2,
};
IMPL_ENUM_OPERATORS(SessionActive);

[[nodiscard]] inline constexpr const char *
serialize(const SessionActive active) {
  switch (active) {
  case SessionActive::INACTIVE:
    return "SESSION_ACTIVE_INACTIVE";
  case SessionActive::KEEP_ALIVE:
    return "SESSION_ACTIVE_KEEP_ALIVE";
  case SessionActive::ACTIVE:
    return "SESSION_ACTIVE_ACTIVE";
  default:
    return "SESSION_ACTIVE_UNKNOWN";
  }
}

enum class SessionDirty : int32_t {
  STATE = 0x1,
  UI = 0x2,
  CLIENT_LIST = 0x4,
};
IMPL_ENUM_OPERATORS(SessionDirty);

[[nodiscard]] inline constexpr const char *serialize(const SessionDirty dirty) {
  switch (dirty) {
  case SessionDirty::STATE:
    return "SESSION_DIRTY_STATE";
  case SessionDirty::UI:
    return "SESSION_DIRTY_UI";
  case SessionDirty::CLIENT_LIST:
    return "SESSION_DIRTY_CLIENT_LIST";
  default:
    return "SESSION_DIRTY_UNKNOWN";
  }
}

PACKED(struct SessionHost {
  net::HostInfo info = {}; // Verified
  ClientNum_t maxClients = ClientNum_t::INVALID_CLIENT_INDEX;
  uint8_t _padding7C[4];

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"info\": {}, \"maxClients\": {} }}",
                       info.serialize(), game::serialize(maxClients));
  }
});
ASSERT_SIZE(SessionHost, 0x88);

struct SessionLeader {
  XUID xuid = 0;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"xuid\": {:016X} }}", xuid);
  }
};

PACKED(struct SessionMigrateData {
  uint8_t indexBits = 0;
  uint8_t _padding[3];
  int32_t startTime = 0;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"indexBits\": {}, \"startTime\": {} }}",
                       static_cast<unsigned int>(indexBits), startTime);
  }
});

// Verified
PACKED(struct CustomGameDetails {
  str64_t name = {0};
  qboolean isOfficial = qfalse;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": {}, \"isOfficial\": {} }}",
                       ::serialize(name), ::serialize(isOfficial));
  }
});
ASSERT_SIZE(CustomGameDetails, 0x44);

// Verified
struct LobbyCustomData {
  CustomGameDetails customGameDetails = {};

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"customGameDetails\": {} }}",
                       customGameDetails.serialize());
  }
};

// Verified
PACKED(struct LobbyTheaterData {
  name_t demoName = {0};
  uint64_t fileId = 0;
  int32_t fileSize = 0;
  bool isUserFile = false;
  bool isLiveStream = false;
  uint8_t _padding2E[2];
  uint64_t authorXUID = 0;
  eModes mainMode = eModes::INVALID;
  name_t demoMode = {0};
  name_t fileCategory = {0};
  int32_t downloadPercent = 0;
  bool readyForPlayback = false;
  bool retryDownload = false;
  uint8_t _padding82[2];
  int32_t lastHeartBeatTime = 0;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"demoName\": {}, \"fileId\": {}, \"fileSize\": {}, "
                       "\"isUserFile\": {}, "
                       "\"isLiveStream\": {}, \"authorXUID\": {:016X}, "
                       "\"mainMode\": {}, \"demoMode\": {}, "
                       "\"fileCategory\": {}, \"downloadPercent\": {}, "
                       "\"readyForPlayback\": {}, "
                       "\"retryDownload\": {}, \"lastHeartBeatTime\": {} }}",
                       ::serialize(demoName), fileId, fileSize,
                       ::serialize(isUserFile), ::serialize(isLiveStream),
                       authorXUID, game::serialize(mainMode),
                       ::serialize(demoMode), ::serialize(fileCategory),
                       downloadPercent, ::serialize(readyForPlayback),
                       ::serialize(retryDownload), lastHeartBeatTime);
  }
});
ASSERT_SIZE(LobbyTheaterData, 0x88);

// Verified
PACKED(struct LobbyPublicData {
  int32_t playlistId = 0;
  int32_t playlistCurrentEntry = 0;
  uint8_t playlistLastEntries[8] = {0};
  uint8_t playlistNext = 0;
  uint8_t playlistPrev = 0;
  uint8_t playlistPrevCount = 0;
  uint8_t _padding13[1];

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"playlistId\": {}, \"playlistCurrentEntry\": {}, "
                       "\"playlistLastEntries\": {}, "
                       "\"playlistNext\": {}, \"playlistPrev\": {}, "
                       "\"playlistPrevCount\": {} }}",
                       playlistId, playlistCurrentEntry,
                       ::serialize(playlistLastEntries), playlistNext,
                       playlistPrev, playlistPrevCount);
  }
});
ASSERT_SIZE(LobbyPublicData, 0x14);

struct CharacterSelection {
  uint32_t characterIndex = 0;
  heroLoadoutTypes_e loadoutType = heroLoadoutTypes_e::INVALID;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format(R"({{ "characterIndex": {}, "loadoutType": {} }})",
                       characterIndex, lobby::serialize(loadoutType));
  }
};

// Verified
PACKED(struct GameClientData {
  team_t team = team_t::TEAM_BAD;
  LobbyMapVote mapVote = LobbyMapVote::INVALID;
  bool readyUp = false;
  uint8_t _padding09[3];
  LobbyClientPregameState pregameState = LobbyClientPregameState::IDLE;
  int32_t pregamePosition = 0;
  bool pregameEditComplete = false;
  uint8_t _padding15[3];
  uint32_t itemVoteCount = 0;
  CharacterSelection characterSelection = {};

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"team\": {}, \"mapVote\": {}, \"readyUp\": {}, "
                       "\"pregameState\": {}, "
                       "\"pregamePosition\": {}, \"pregameEditComplete\": {}, "
                       "\"itemVoteCount\": {}, "
                       "\"characterSelection\": {} }}",
                       game::serialize(team), game::lobby::serialize(mapVote),
                       ::serialize(readyUp), lobby::serialize(pregameState),
                       pregamePosition, ::serialize(pregameEditComplete),
                       itemVoteCount, characterSelection.serialize());
  }
});
// Verified
ASSERT_SIZE(GameClientData, 0x24);

PACKED(struct ItemVote {
  XUID clientXuid = 0;
  VoteItemType itemType = VoteItemType::INVALID;
  uint32_t itemIndex = 0;
  itemGroup_t itemGroup = itemGroup_t::INVALID;
  weapon::eAttachment attachment = weapon::eAttachment::NONE;
  VoteType voteType = VoteType::INVALID;
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
  uint32_t pregameVoteCount = 0;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"clientXuid\": {:016X}, \"itemType\": {}, \"itemIndex\": {}, "
        "\"itemGroup\": {}, "
        "\"attachment\": {}, \"voteType\": {}, \"pregameVoteCount\": {} }}",
        clientXuid, lobby::serialize(itemType), itemIndex,
        lobby::serialize(itemGroup), weapon::serialize(attachment),
        lobby::serialize(voteType), pregameVoteCount);
  }
});

PACKED(struct LobbyGameData {
  ControllerIndex_t controllerIndex =
      ControllerIndex_t::INVALID_CONTROLLER_PORT;
  GameServerStatus serverStatus = GameServerStatus::IDLE;
  int32_t launchNonce = 0;
  uint8_t _padding0C[4];
  uint128_t matchHash = {};
  bool preLoaded = false;
  name_t gameType = {0};
  name_t mapName = {0};
  uint8_t _padding61[3];
  info::LobbyUGCInfo ugcInfo = {};
  int32_t status = 0;
  int32_t statusValue = 0;
  LobbyPregameState pregameState = LobbyPregameState::IDLE;
  uint32_t pregameVoteCount = 0;
  ItemVote pregameVotes[216] = {{}};
  LobbyClientPool<GameClientData> client = {{}};
  LobbyPublicData publicData = {}; // Verified
  LobbyCustomData customData = {}; // Verified
  // TODO: the data structure is correct, but why is there a separate field for
  // this? This is duplicative of publicData, and does not exist in BO4 or BO3
  // alpha engines.
  LobbyPublicData playlistData = {}; // Verified
  uint8_t _padding1E8C[4];
  LobbyTheaterData theaterData = {}; //  Verified

  IMPL_SLICE_METHODS(pregameVotes, pregameVoteCount, PregameVotes);

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"controllerIndex\": {}, \"serverStatus\": {}, \"launchNonce\": "
        "{}, "
        "\"preLoaded\": {}, \"gameType\": {}, \"mapName\": {}, \"ugcInfo\": "
        "{}, "
        "\"status\": {}, \"statusValue\": {}, \"pregameState\": {}, "
        "\"pregameVoteCount\": {}, "
        "\"pregameVotes\": {}, \"client\": {}, \"publicData\": {}, "
        "\"customData\": {}, "
        "\"playlistData\": {}, \"theaterData\": {} }}",
        game::serialize(controllerIndex), lobby::serialize(serverStatus),
        launchNonce, ::serialize(preLoaded), ::serialize(gameType),
        ::serialize(mapName), ugcInfo.serialize(), status, statusValue,
        lobby::serialize(pregameState), pregameVoteCount,
        ::serialize(getPregameVotes()), ::serialize(client),
        publicData.serialize(), customData.serialize(),
        playlistData.serialize(), theaterData.serialize());
  }
});
ASSERT_SIZE(LobbyGameData, 0x1F18);

// Verified
struct LobbyPrivateData {
  int32_t playlistId = 0;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"playlistId\": {} }}", playlistId);
  }
};

PACKED(struct LobbyData {
  // This actually acts more like bitflags representing lobby session state -
  // `dirty` is a misleading struct field name.
  uint8_t dirty = 0; // Verified
  uint8_t _padding01[3];
  eGameModes gameMode = eGameModes::INVALID; // Verified
  XUID platformSessionID = 0;                // Verified
  const LobbyType *type = nullptr;
  const LobbyMode *mode = nullptr;    // Verified
  LobbyPrivateData lobbyPrivate = {}; // Verified
  uint8_t _padding24[4];
  LobbyGameData lobbyGame = {}; // Verified

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"dirty\": {:X}, \"gameMode\": {}, \"platformSessionID\": {:016X}, "
        "\"type\": {}, \"mode\": {}, \"lobbyPrivate\": {}, "
        "\"lobbyGame\": {} }}",
        dirty, game::serialize(gameMode), platformSessionID, ::serialize(type),
        ::serialize(mode), lobbyPrivate.serialize(), lobbyGame.serialize());
  }
});

PACKED(struct LobbySession {
  LobbyModule module = LobbyModule::INVALID;     // Verified
  LobbyType type = LobbyType::INVALID;           // Verified
  LobbyMode mode = LobbyMode::INVALID;           // Verified
  SessionStatus status = SessionStatus::INVALID; // Verified
  void *_unknown10 = nullptr;
  const char *name = nullptr;                     // Verified
  str32_t key = {0};                              // Verified
  SessionActive active = SessionActive::INACTIVE; // Verified
  qboolean dirty = qfalse;                        // Verified
  uint64_t flags = 0;                             // Verified
  bool sessionAdvertised = false;                 // Verified
  bool qosListenEnabled = false;
  uint8_t _padding52[6];
  active::ActiveList *activeList = nullptr; // Verified
  SessionHost host = {};                    // Verified
  SessionLeader leader = {};
  uint32_t joinCount = 0;
  uint8_t _paddingF4[4];
  LobbyClientPool<SessionClient> client = {{}}; // Verified
  bool inRecovery = false;                      // Verified
  uint8_t _padding459[3];
  SessionMigrateData migrateData = {};
  uint8_t _padding464[4];
  LobbyData lobbyData = {}; // Verified
  info::LobbyUGCInfo ugcInfo = {};
  uint8_t _padding23CC[4];

  IMPL_SLICE_METHODS(client, joinCount, Client);

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"module\": {}, \"type\": {}, \"mode\": {}, \"status\": {}, "
        "\"unknown10\": {}, \"name\": {}, \"key\": {}, \"active\": {}, "
        "\"dirty\": {}, "
        "\"flags\": {:016X}, \"sessionAdvertised\": {}, \"qosListenEnabled\": "
        "{}, "
        "\"activeList\": {}, \"host\": {}, \"leader\": {}, \"joinCount\": "
        "{}, "
        "\"client\": {}, \"inRecovery\": {}, \"migrateData\": {}, "
        "\"lobbyData\": {}, "
        "\"ugcInfo\": {} }}",
        lobby::serialize(module), lobby::serialize(type),
        lobby::serialize(mode), session::serialize(status),
        ::serialize(_unknown10), ::serialize(name), ::serialize(key),
        session::serialize(active), ::serialize(dirty), flags,
        ::serialize(sessionAdvertised), ::serialize(qosListenEnabled),
        ::serialize(activeList), host.serialize(), leader.serialize(),
        joinCount, ::serialize(getClient()), ::serialize(inRecovery),
        migrateData.serialize(), lobbyData.serialize(), ugcInfo.serialize());
  }
});
ASSERT_OFFSET(LobbySession, inRecovery, 0x458);
ASSERT_OFFSET(LobbySession, client, 0xF8);
ASSERT_OFFSET(LobbySession, host, 0x60);
ASSERT_SIZE(LobbySession, 0x23D0);

struct PlatformSessionData {
  str64_t sessionId = {0};
  str64_t sessionInfo = {0};

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format(R"({{ "sessionId": {}, "sessionInfo": {} }})",
                       ::serialize(sessionId), ::serialize(sessionInfo));
  }
};

// Verified
PACKED(struct GameData {
  GameServerStatus serverStatus = GameServerStatus::IDLE;
  int32_t launchNonce = 0;
  uint128_t matchHash = {};
  eGameModes gameMode = eGameModes::INVALID;
  name_t gameType = {0};
  name_t mapName = {0};
  info::LobbyUGCInfo ugcInfo = {};
  int32_t status = 0;
  int32_t statusValue = 0;
  LobbyPregameState pregameState = LobbyPregameState::IDLE;
  uint32_t pregameVoteCount = 0;
  ItemVote pregameVotes[MAX_ITEM_VOTE_COUNT] = {{}};
  int32_t playlistId = 0;
  int32_t playlistCurrentEntry = 0;
  uint8_t playlistLastEntries[8] = {0};
  uint8_t playlistNext = 0;
  uint8_t playlistPrev = 0;
  uint8_t playlistPrevCount = 0;
  uint8_t _padding1BA3[1];
  LobbyClientPool<GameClientData> client = {{}};
  name_t cpQueuedLevel = {0};
  bool loadingMovieSkipped = false;
  name_t demoName = {0};
  uint8_t _padding1E6D[3];
  uint64_t fileId = 0;
  int32_t fileSize = 0;
  bool isUserFile = false;
  bool isLiveStream = false;
  uint8_t _padding1E7E[2];
  XUID authorXUID = 0;
  eModes mainMode = eModes::INVALID;
  name_t demoMode = {0};
  name_t fileCategory = {0};
  CustomGameDetails customGameDetails = {};

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"serverStatus\": {}, \"launchNonce\": {}, \"gameMode\": {}, "
        "\"gameType\": {}, "
        "\"mapName\": {}, \"ugcInfo\": {}, \"status\": {}, \"statusValue\": "
        "{}, \"pregameState\": {}, "
        "\"pregameVoteCount\": {}, \"pregameVotes\": {}, \"playlistId\": {}, "
        "\"playlistCurrentEntry\": {}, "
        "\"playlistLastEntries\": {}, \"playlistNext\": {}, \"playlistPrev\": "
        "{}, \"playlistPrevCount\": {}, "
        "\"client\": {}, \"cpQueuedLevel\": {}, \"loadingMovieSkipped\": {}, "
        "\"demoName\": {}, "
        "\"fileId\": {}, \"fileSize\": {}, \"isUserFile\": {}, "
        "\"isLiveStream\": {}, \"authorXUID\": {:016X}, "
        "\"mainMode\": {}, \"demoMode\": {}, \"fileCategory\": {}, "
        "\"customGameDetails\": {} }}",
        lobby::serialize(serverStatus), launchNonce, game::serialize(gameMode),
        ::serialize(gameType), ::serialize(mapName), ugcInfo.serialize(),
        status, statusValue, lobby::serialize(pregameState), pregameVoteCount,
        ::serialize(pregameVotes), playlistId, playlistCurrentEntry,
        ::serialize(playlistLastEntries), playlistNext, playlistPrev,
        playlistPrevCount, ::serialize(client), ::serialize(cpQueuedLevel),
        ::serialize(loadingMovieSkipped), ::serialize(demoName), fileId,
        fileSize, ::serialize(isUserFile), ::serialize(isLiveStream),
        authorXUID, game::serialize(mainMode), ::serialize(demoMode),
        ::serialize(fileCategory), customGameDetails.serialize());
  }
});
// Verified
ASSERT_SIZE(GameData, 0x1F10);

typedef array<uint8_t, 0xC000> gametypesettings_t;

enum class ServerStatus : uint32_t {
  PRESENT = 0x0,
  DELAYED = 0x1,
  MISSING = 0x2,
};

[[nodiscard]] inline constexpr const char *
serialize(const ServerStatus status) {
  switch (status) {
  case ServerStatus::PRESENT:
    return "SERVER_STATUS_PRESENT";
  case ServerStatus::DELAYED:
    return "SERVER_STATUS_DELAYED";
  case ServerStatus::MISSING:
    return "SERVER_STATUS_MISSING";
  default:
    return "SERVER_STATUS_UNKNOWN";
  }
}

struct PeerToPeerInfo {
  int32_t connectivityBits = 0;
  LobbyClientPool<msg::NetChanMsg> netMsg = {{}};

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format(R"({{ "connectivityBits": {:X}, "netMsg": {} }})",
                       connectivityBits, ::serialize(netMsg));
  }
};

struct SharedVoiceSessionData {
  uint32_t voiceOutPort = 0;
  uint32_t microphonePort = 0;
  uint32_t headsetPort = 0;
  bool microphoneAttached = false;
  uint32_t lastCaughtTalking = 0;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"voiceOutPort\": {}, \"microphonePort\": {}, \"headsetPort\": {}, "
        "\"microphoneAttached\": {}, \"lastCaughtTalking\": {} }}",
        voiceOutPort, microphonePort, headsetPort,
        ::serialize(microphoneAttached), lastCaughtTalking);
  }
};

struct LobbyPeerResults {
  uint32_t receivedPacketCount = 0;
  uint32_t totalLatencyMS = 0;
  float m = 0.0f;
  float s = 0.0f;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format(
        R"({{ "receivedPacketCount": {}, "totalLatencyMS": {}, "m": {}, "s": {} }})",
        receivedPacketCount, totalLatencyMS, m, s);
  }
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

[[nodiscard]] inline constexpr const char *
serialize(const LobbyMigrateClientState state) {
  switch (state) {
  case LobbyMigrateClientState::CLIENT_IDLE:
    return "MIGRATE_CLIENT_IDLE";
  case LobbyMigrateClientState::CLIENT_SENDING_PACKETS:
    return "MIGRATE_CLIENT_SENDING_PACKETS";
  case LobbyMigrateClientState::WAITING_FOR_RESPONSES:
    return "MIGRATE_CLIENT_WAITING_FOR_RESPONSES";
  case LobbyMigrateClientState::ACTIVE:
    return "MIGRATE_CLIENT_ACTIVE";
  case LobbyMigrateClientState::BECOMING_HOST:
    return "MIGRATE_CLIENT_BECOMING_HOST";
  case LobbyMigrateClientState::CREATING_HOST:
    return "MIGRATE_CLIENT_CREATING_HOST";
  case LobbyMigrateClientState::COUNT:
    return "MIGRATE_CLIENT_COUNT";
  default:
    return "MIGRATE_CLIENT_UNKNOWN";
  }
}

struct LobbyMigrateClientData {
  LobbyClientPool<LobbyPeerResults> peerResults = {{}};
  LobbyClientPool<XUID> nominees = {0};
  LobbyMigrateClientState clientState = LobbyMigrateClientState::CLIENT_IDLE;
  uint32_t sentPacketCount = 0;
  int32_t lastPacketSendMs = 0;
  int32_t currentNominee = 0;
  int32_t migrateStartMs = 0;
  int32_t lasthostTimeMS = 0;
  int32_t hostStartMS = 0;
  int32_t lastCalcTime = 0;
  int32_t nextTestPacketSendMS = 0;
  XUID peerXor = 0;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"peerResults\": {}, \"nominees\": {}, "
                       "\"clientState\": {}, \"sentPacketCount\": {}, "
                       "\"lastPacketSendMs\": {}, \"currentNominee\": {}, "
                       "\"migrateStartMs\": {}, \"lasthostTimeMS\": {}, "
                       "\"hostStartMS\": {}, \"lastCalcTime\": {}, "
                       "\"nextTestPacketSendMS\": {}, \"peerXor\": {:016X} }}",
                       ::serialize(peerResults), ::serialize(nominees),
                       session::serialize(clientState), sentPacketCount,
                       lastPacketSendMs, currentNominee, migrateStartMs,
                       lasthostTimeMS, hostStartMS, lastCalcTime,
                       nextTestPacketSendMS, peerXor);
  }
};

PACKED(struct ClientSession {
  LobbySession lobbySession = {}; // Verified
  uint32_t stateNum = 0;
  int32_t lastHostMsgTimestamp = 0;                  // Verified
  ServerStatus serverStatus = ServerStatus::MISSING; // Verified
  uint8_t _padding23DC[4];
  PeerToPeerInfo peerToPeerInfo = {};
  LocalClientPool<msg::NetChanMsg> channel = {{}};
  LocalClientPool<msg::NetChanMsg> clientContentMsg = {{}};
  LocalClientPool<msg::NetChanMsg> modifiedStatsMsgCfg = {{}}; // Verified
  LobbyMigrateClientData clientMigrateData = {};
  LocalClientPool<bool> voiceCommunicationActive = {false};
  LocalClientPool<bool> paintshopDataDirty = {false};
  uint8_t _padding2A4C[4];
  SharedVoiceSessionData *platformVoiceData = nullptr;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"lobbySession\": {}, \"stateNum\": {}, \"lastHostMsgTimestamp\": "
        "{}, "
        "\"serverStatus\": {}, \"peerToPeerInfo\": {}, \"channel\": {}, "
        "\"clientContentMsg\": {}, "
        "\"modifiedStatsMsgCfg\": {}, \"clientMigrateData\": {}, "
        "\"voiceCommunicationActive\": {}, "
        "\"paintshopDataDirty\": {}, \"hasPlatformVoiceData\": {} }}",
        lobbySession.serialize(), stateNum, lastHostMsgTimestamp,
        session::serialize(serverStatus), peerToPeerInfo.serialize(),
        ::serialize(channel), ::serialize(clientContentMsg),
        ::serialize(modifiedStatsMsgCfg), clientMigrateData.serialize(),
        ::serialize(voiceCommunicationActive), ::serialize(paintshopDataDirty),
        ::serialize(platformVoiceData != nullptr));
  }
});

ASSERT_OFFSET(ClientSession, lobbySession, 0x0);
ASSERT_OFFSET(ClientSession, lastHostMsgTimestamp, 0x23D4);
ASSERT_OFFSET(ClientSession, serverStatus, 0x23D8);
// Verified
ASSERT_SIZE(ClientSession, 0x2A58);

} // namespace session
} // namespace lobby
} // namespace game
