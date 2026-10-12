#pragma once

#include <game/structs/core.hpp>
#include <game/structs/dw/net.hpp>
#include <game/structs/net/net.hpp>

#include <cstdint>
#include <optional>

namespace game {
namespace lobby {

enum class ClientContentTransferState : uint32_t {
  INACTIVE = 0x0,
  DIRTY = 0x1,
  SENDING = 0x2,
  SENT = 0x3,
};

enum class ClientContentFragmentTransferState : uint32_t {
  INACTIVE = 0x0,
  SENT = 0x1,
  ACKNOWLEDGED = 0x2,
};

enum class ClientContentFragmentDataType : uint32_t {
  NONE = 0x0,
  PAINTSHOP = 0x1,
  LOADOUT = 0x2,
  DONE = 0x3,
};

enum class LobbyType : int32_t {
  INVALID = -1,
  PRIVATE = 0,
  GAME = 1,
  COUNT = 2,
  FIRST = 0,
  LAST = 1,
};
IMPL_ENUM_OPERATORS(LobbyType);

#ifndef NDEBUG
inline constexpr const char *serialize(const LobbyType lobbyType) {
  switch (lobbyType) {
  case LobbyType::PRIVATE:
    return "LOBBYTYPE_PRIVATE";
  case LobbyType::GAME:
    return "LOBBYTYPE_GAME";
  default:
    return "LOBBYTYPE_INVALID";
  }
}
#endif

enum class LobbyClientType : int32_t {
  INVALID = -1,
  ALL = 0,
  LOCAL = 1,
  REMOTE = 2,
  SPLITSCREEN_ALL = 3,
  SPLITSCREEN_REMOTE = 4,
  GUEST = 5,
};
IMPL_ENUM_OPERATORS(LobbyClientType);

enum class LobbyNetworkMode : int32_t {
  INVALID = -1,
  UNKNOWN = 0,
  LAN = 1,
  LIVE = 2,
  COUNT = 3,
  LOCAL = 1,
};
IMPL_ENUM_OPERATORS(LobbyNetworkMode);

enum class LobbyMainMode : int32_t {
  INVALID = -1,
  CP = 0,
  MP = 1,
  ZM = 2,
  COUNT = 3,
};
IMPL_ENUM_OPERATORS(LobbyMainMode);

enum class LobbyModule : int32_t {
  INVALID = -1,
  HOST = 0x0,
  CLIENT = 0x1,
  COUNT = 0x2,
  FIRST = 0x0,
  LAST = 0x1,
  PEER_TO_PEER = 0x3,
};
IMPL_ENUM_OPERATORS(LobbyModule);

enum class LobbyMode : int32_t {
  INVALID = -1,
  PUBLIC = 0,
  CUSTOM = 1,
  THEATER = 2,
  ARENA = 3,
  FREERUN = 4,
  COUNT = 5,
};
IMPL_ENUM_OPERATORS(LobbyMode);

enum class PartyPrivacy : int32_t {
  OPEN = 0,
  FRIENDS_ONLY = 1,
  INVITE_ONLY = 2,
  CLOSED = 3,
  COUNT = 4,
};
IMPL_ENUM_OPERATORS(PartyPrivacy);

enum class LobbyJoinable : int32_t {
  NO_NOT_IN_LOBBY = 0,
  YES = 1,
  YES_FRIENDS_ONLY = 2,
  NO_PARTY_PRIVACY_INVITE_ONLY = 3,
  NO_PARTY_PRIVACY_CLOSED = 4,
  NO_PARTY_FULL = 5,
  NO_GAME_FULL = 6,
  NO_PLAYING_LOCAL = 7,
  NO_GAME_PAUSED = 8,
  NO = 9,
  NO_YOU_NEED_DLC = 10,
  COUNT = 11,
};
IMPL_ENUM_OPERATORS(LobbyJoinable);

enum class LobbyHostType : int32_t {
  INVALID = -1,
  P2P = 0,
  DEDICATED = 1,
  COUNT = 2,
};
IMPL_ENUM_OPERATORS(LobbyHostType);
enum class LobbyMapVote : int32_t {
  INVALID = -1,
  NONE = 0,
  NEXT = 1,
  PREVIOUS = 2,
  RANDOM = 3,
};
IMPL_ENUM_OPERATORS(LobbyMapVote);
enum class LobbyProcessComplete : int32_t {
  INVALID = -1,
  SUCCESS = 0,
  FAILURE = 1,
  LOBBY_PROCESS_COMPLETE_ERROR = 2,
};
IMPL_ENUM_OPERATORS(LobbyProcessComplete);
enum class LobbyDisconnectClient : int32_t {
  INVALID = -1,
  DROP = 0x0,
  KICK = 0x1,
  BADDLC = 0x2,
  KICK_PARTY = 0x3,
  HOSTRELOAD = 0x4,
  NOPARTYCHAT = 0x5,
  LOBBY_DISCONNECT_CLIENT_ERROR = 0x6,
  MISSING_CONTENT = 0x7,
};
IMPL_ENUM_OPERATORS(LobbyDisconnectClient);

enum class MutableClientFlag : uint32_t {
  HEADSET_PRESENT = 0x0,
  MATURE_CONTENT = 0x1,
  IS_ONBOARDING = 0x2,
  COUNT = 0x3,
};
IMPL_ENUM_OPERATORS(MutableClientFlag);

constexpr ClientNum_t MIN_PLAYERS = CLIENT_INDEX_0;
constexpr ClientNum_t MAX_PLAYERS = ClientNum_t::CLIENT_INDEX_COUNT;
template <typename T> using LobbyClientPool = array<T, MAX_PLAYERS>;
template <typename T>
using LobbyClientOptionalPool = array<std::optional<T>, MAX_PLAYERS>;
template <typename T> using AtomicLobbyClientPool = atomicarray<T, MAX_PLAYERS>;
template <typename T>
using AtomicLobbyClientOptionalPool =
    atomicarray<std::optional<T>, MAX_PLAYERS>;

struct LobbyParams {
  LobbyNetworkMode networkMode;
  LobbyMainMode mainMode;
};

struct JoinHost {
  net::HostInfo info;
  LobbyType lobbyType;
  LobbyParams lobbyParams;
  uint64_t reservationKey;
  int32_t retryTime;
  int32_t retryCount;
};

struct AgreementStatus {
  XUID xuid;
  name_t name;
  bool responded;
  bool agrees;
  int32_t startTime;
  int32_t responseTime;
};

struct Agreement {
  int32_t nonce;
  LobbyClientPool<AgreementStatus> status;
  int32_t requestCount;
  int32_t responseCount;
  int32_t agreeCount;
};

struct ServerInfo {
  uint16_t m_usConnectionPort;
  uint16_t m_usQueryPort;
  uint32_t m_unIP;
  int32_t m_nPing;
  uint8_t unk[0x22];
  name_t mapname;
  char description[64];
  char gamemode[16];
  name_t modname;
  int32_t playerCount;
  int32_t maxPlayers;
  int32_t unk2;
  int32_t unk3;
  int32_t unk4;
  bool dedicated;
  bool ranked;
  bool hardcore;
  bool zombies;
  char servername[64];
  char tags[128];
  int32_t unk5;
  int32_t unk6;
};

// =====================================================================
// LobbyMsg struct - matches in-memory layout at the point of inspection.
// game::net::msg::msg_t is 0x38 bytes, followed by msgType at offset 0x38.
// =====================================================================
struct LobbyMsg {
  game::net::msg::msg_t msg; // 0x00 - 0x37
  int32_t msgType;           // 0x38
  char encodeFlags;          // 0x3C
  int32_t packageType;       // 0x40
};

typedef uint64_t LobbyID;

PACKED(struct ClientProgression {
  uint8_t rank;
  uint8_t prestige;
  uint16_t paragonRank;
  uint8_t paragonIconId;
  uint8_t _padding05[1];
});

struct BubbleGumBuffsLobbyClientInfo {
  uint8_t bubbleGumBuffs[5];
  uint8_t bubbleGumBuffConsumableCount[5];
};

struct CharacterCustomizationLobbyClientInfo {
  uint8_t selectedCharacterType;
  uint8_t selectedCharacterMode;
  uint8_t selectedHeadType;
  uint8_t selectedLoadoutSlot;
  uint8_t selectedCharacterItem[2];
  uint8_t selectedCharacterItemColors[2][3];
};

PACKED(struct Variant {
  char variantName[17];
  uint8_t _padding11[3];
  int32_t attachment[8];
  uint8_t attachmentVariant[8];
  uint8_t camoIndex;
  uint8_t _padding3D[3];
  int32_t paintjobSlot;
  int32_t paintjobIndex;
  int32_t weaponIndex;
  int32_t variantIndex;
  int32_t sortIndex;
});

struct CollectibleInfo {
  bool isSet;
  name_t mapName;
  uint8_t collectibleIndex;
};

PACKED(struct MutableClientInfo {
  LobbyID lobbyID;
  char clantag[5];
  uint8_t _padding0D[1];
  ClientProgression progression[3];
  BubbleGumBuffsLobbyClientInfo bubbleGumInfo;
  uint8_t _padding2A[2];
  int32_t arenaPoints;
  float arenaSkill;
  float arenaVariance;
  CharacterCustomizationLobbyClientInfo heroInfo;
  Variant showcaseWeapon;
  CollectibleInfo collectibles[9];
  uint8_t _padding1CA[2];
  uint32_t flags;
  BGEmblemBackgroundID backgroundId;
  uint8_t _padding1D2[2];
  uint32_t musicUnlocks[3];
  float skillRating;
  float skillVariance;
  int8_t defaultEmblemIndex;
  int8_t easterEggBits;
  uint16_t unlockedMedals;
  uint32_t profileNonce;
  uint8_t ddlBuff[512];
  uint16_t ddlBuffSize;
  uint8_t _padding3F2[2];
  uint32_t probationEndTime[2];
  uint8_t chunkStatuses[3];
  bool isStarterPack;
  ContentFlags dlcBits;
  uint32_t doubleXPGroupMask;
  bool isSplitscreenClient;
  bool isInPlatformParty;
  bool isInFrontend;
  uint8_t _padding40B[1];
  connstate_t connectionState;
});
ASSERT_SIZE(MutableClientInfo, 0x410);

struct SharedDataCenterInfo {
  uint16_t shortId;
  uint8_t rtt[10];
};

PACKED(struct FixedClientInfo {
  XUID xuid;
  char gamertag[32];
  dw::net::bdCommonAddrRef commonAdr;
  int32_t qport;
  uint8_t latencyBand;
  uint8_t _padding35[3];
  int granted;
  int consumed;
  int date;
  int daily;
  uint8_t connectionType;
  uint8_t _padding49[1];
  SharedDataCenterInfo datacenter[10];
  uint16_t countryCode;
  float latitude;
  float longitude;
  uint8_t _paddingCC[4];
});

} // namespace lobby
} // namespace game
