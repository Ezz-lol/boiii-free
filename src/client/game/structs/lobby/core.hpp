#pragma once

#include <game/structs/core.hpp>
#include <game/structs/dw/net/core.hpp>
#include <game/structs/net/net.hpp>

#include <cstdint>
#include <optional>

namespace game {
namespace lobby {

typedef int32_t LobbyUIScreen;
typedef int32_t LobbyLeaderActivity;

enum class LobbyGlobalChannel : uint32_t {
  JOIN = 0x0,
  CLIENT_CONTENT = 0x1,
  COUNT = 0x2,
};
IMPL_ENUM_OPERATORS(LobbyGlobalChannel);

enum class LobbyListComparisonType : int32_t {
  EQUAL_TO_OR_LESS_THAN = -2,
  LESS_THAN = -1,
  EQUAL = 0,
  GREATER_THAN = 1,
  EQUAL_TO_OR_GREATER_THAN = 2,
  NOT_EQUAL = 3,
};
enum class LobbyTaskType : int32_t {
  HOST_CREATE = 0,
  HOST_ADDLOCAL = 1,
  HOST_END = 2,
  HOST_SEARCH = 3,
  HOST_ADVERTISE = 4,
  HOST_GAMERTAGSTOXUIDS = 5,
  CLIENT_END = 6,
  CLIENT_LAUNCH_GAME = 7,
  COUNT = 8,
};
enum class LobbyTaskState : int32_t {
  IDLE = 0,
  HOSTCREATE_PENDING = 1,
  HOSTCREATE_DWNET = 2,
  HOSTCREATE_SESSION = 3,
  HOSTEND_PENDING = 4,
  HOSTEND_ONLINE = 5,
  HOSTEND_DISCONNECT = 6,
  HOSTEND_COOLOFF = 7,
  HOSTEND_SHUTDOWN = 8,
  HOSTSEARCH_PENDING = 9,
  HOSTSEARCH_INPROGRESS = 10,
  HOSTSEARCH_BEGINJOIN = 11,
  HOSTSEARCH_JOINING = 12,
  HOSTADVERTISE_PENDING = 13,
  HOSTADVERTISE_INPROGRESS = 14,
  HOSTPLATFORMSESSIONCREATE = 15,
  HOSTPLATFORMSESSIONUPDATE = 16,
  HOSTPLATFORMSESSIONINVITE = 17,
  HOSTPLATFORMSESSIONGETSESSIONINFO = 18,
  HOSTPLATFORMSESSIONGETINVITEINFO = 19,
  HOSTPLATFORMSESSIONLEAVE = 20,
  HOSTPLATFORMSESSIONSEARCH = 21,
  HOSTPLATFORMSESSIONJOIN = 22,
  HOSTPLATFORMSESSIONSETDURANGOJOINRESTRICTION = 23,
  HOSTGAMERTAGSTOXUIDS = 24,
  CLIENTEND_PENDING = 25,
  CLIENTEND_DISCONNECT = 26,
  CLIENTEND_COOLOFF = 27,
  CLIENTEND_SHUTDOWN = 28,
  HOSTLAUNCH_INIT = 29,
  HOSTLAUNCH_PUMP = 30,
  HOSTLAUNCH_PRELOAD = 31,
  HOSTLAUNCH_GO = 32,
  CLIENTLAUNCH_INIT = 33,
  CLIENTLAUNCH_PUMP = 34,
  CLIENTLAUNCH_GO = 35,
  SUCCESS = 36,
  FAILURE = 37,
};

enum class LobbyType : int32_t {
  INVALID = -1,
  PRIVATE = 0,
  GAME = 1,
  COUNT = 2,
  FIRST = 0,
  LAST = 1,
};
constexpr bool validate(const LobbyType lobbyType) {
  return lobbyType >= LobbyType::PRIVATE && lobbyType < LobbyType::COUNT;
}
IMPL_ENUM_OPERATORS(LobbyType);

constexpr const char *serialize(const LobbyType lobbyType) {
  switch (lobbyType) {
  case LobbyType::PRIVATE:
    return "LOBBYTYPE_PRIVATE";
  case LobbyType::GAME:
    return "LOBBYTYPE_GAME";
  default:
    return "LOBBYTYPE_INVALID";
  }
}

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

constexpr bool validate(const LobbyNetworkMode networkMode) {
  return networkMode > LobbyNetworkMode::UNKNOWN &&
         networkMode < LobbyNetworkMode::COUNT;
}

enum class LobbyMainMode : int32_t {
  INVALID = -1,
  CP = 0,
  MP = 1,
  ZM = 2,
  COUNT = 3,
};
IMPL_ENUM_OPERATORS(LobbyMainMode);

constexpr bool validate(const LobbyMainMode mainMode) {
  return mainMode >= LobbyMainMode::CP && mainMode < LobbyMainMode::COUNT;
}

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

constexpr bool validate(const LobbyMode mode) {
  return mode >= LobbyMode::PUBLIC && mode < LobbyMode::COUNT;
}

enum class PartyPrivacy : int32_t {
  OPEN = 0,
  FRIENDS_ONLY = 1,
  INVITE_ONLY = 2,
  CLOSED = 3,
  COUNT = 4,
};
IMPL_ENUM_OPERATORS(PartyPrivacy);

constexpr bool validate(const PartyPrivacy privacy) {
  return privacy >= PartyPrivacy::OPEN && privacy < PartyPrivacy::COUNT;
}

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

constexpr bool validate(const LobbyMapVote vote) {
  return vote >= LobbyMapVote::NONE && vote <= LobbyMapVote::RANDOM;
}

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

constexpr bool validate(const LobbyDisconnectClient disc) {
  return disc >= LobbyDisconnectClient::DROP &&
         disc <= LobbyDisconnectClient::MISSING_CONTENT;
}
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

// Verified
struct LobbyParams {
  LobbyNetworkMode networkMode = LobbyNetworkMode::INVALID;
  LobbyMainMode mainMode = LobbyMainMode::INVALID;
};

// Verified
PACKED(struct JoinHost {
  net::HostInfo info;
  LobbyType lobbyType;
  LobbyParams lobbyParams;
  uint8_t _padding8C[4];
  uint64_t reservationKey;
  int32_t retryTime;
  int32_t retryCount;
});
ASSERT_SIZE(JoinHost, 0xA0);

struct AgreementStatus {
  XUID xuid = 0;
  name_t name = {0};
  bool responded = false;
  bool agrees = false;
  int32_t startTime = 0;
  int32_t responseTime = 0;
};

// Verified
struct Agreement {
  int32_t nonce = 0;
  LobbyClientPool<AgreementStatus> status = {{}};
  int32_t requestCount = 0;
  int32_t responseCount = 0;
  int32_t agreeCount = 0;
};
ASSERT_SIZE(Agreement, 0x408);

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

typedef uint64_t LobbyID;

// Verified
PACKED(struct ClientProgression {
  uint8_t rank = 0;
  uint8_t prestige = 0;
  uint16_t paragonRank = 0;
  uint8_t paragonIconId = 0;
  uint8_t _padding05[1];
});
ASSERT_SIZE(ClientProgression, 0x6);

template <typename T> using BubbleGumBuffLobbyArray = array<T, 5>;

// Verified
struct BubbleGumBuffsLobbyClientInfo {
  BubbleGumBuffLobbyArray<uint8_t> bubbleGumBuffs = {0};
  BubbleGumBuffLobbyArray<uint8_t> bubbleGumBuffConsumableCount = {0};
};
ASSERT_SIZE(BubbleGumBuffsLobbyClientInfo, 0xA);

// Verified
struct CharacterCustomizationLobbyClientInfo {
  uint8_t selectedCharacterType = 0;
  uint8_t selectedCharacterMode = 0;
  uint8_t selectedHeadType = 0;
  uint8_t selectedLoadoutSlot = 0;
  uint8_t selectedCharacterItem[2] = {0};
  uint8_t selectedCharacterItemColors[2][3] = {{0}};
};
ASSERT_SIZE(CharacterCustomizationLobbyClientInfo, 0xC);

// Verified
PACKED(struct Variant {
  char variantName[17] = {0};
  uint8_t _padding11[3];
  int32_t attachment[8] = {0};
  uint8_t attachmentVariant[8] = {0};
  uint8_t camoIndex = 0;
  uint8_t _padding3D[3];
  int32_t paintjobSlot = 0;
  int32_t paintjobIndex = 0;
  int32_t weaponIndex = 0;
  int32_t variantIndex = 0;
  int32_t sortIndex = 0;
});
ASSERT_SIZE(Variant, 0x54);

// Verified
struct CollectibleInfo {
  bool isSet = false;
  name_t mapName = {0};
  uint8_t collectibleIndex = 0;
};
ASSERT_SIZE(CollectibleInfo, 0x22);

// Verified
PACKED(struct MutableClientInfo {
  LobbyID lobbyID = 0;
  char clantag[5] = {0};
  uint8_t _padding0D[1];
  ClientProgression progression[3] = {{}};
  BubbleGumBuffsLobbyClientInfo bubbleGumInfo = {};
  uint8_t _padding2A[2];
  int32_t arenaPoints = 0;
  float arenaSkill = 0.0f;
  float arenaVariance = 0.0f;
  CharacterCustomizationLobbyClientInfo heroInfo = {};
  Variant showcaseWeapon = {};
  CollectibleInfo collectibles[9] = {{}};
  uint8_t _padding1CA[2];
  // TODO: define flags as bit-packed union
  uint32_t flags = 0;
  BGEmblemBackgroundID backgroundId = 0;
  uint32_t musicUnlocks[3] = {0};
  float skillRating = 0.0f;
  float skillVariance = 0.0f;
  int8_t defaultEmblemIndex = 0;
  int8_t easterEggBits = 0;
  uint16_t unlockedMedals = 0;
  uint32_t profileNonce = 0;
  uint8_t ddlBuff[512] = {0};
  uint16_t ddlBuffSize = 0;
  uint8_t _padding3F2[2];
  union {
    uint32_t probationEndTime[2];
    struct {
      // Serialized as "pprobation" in lobby message packet
      uint32_t publicProbationEndTime = 0;
      // Serialized as "aprobation" in lobby message packet
      uint32_t arenaProbationEndTime = 0;
    };
  };
  bool chunkStatuses[3] = {false};
  bool isStarterPack = false;
  ContentFlags dlcBits = {};
  uint32_t doubleXPGroupMask = 0;
  bool isSplitscreenClient = false;
  bool isInPlatformParty = false;
  bool isInFrontend = false;
  uint8_t _padding40B[1];
  connstate_t connectionState = connstate_t::DISCONNECTED;

  static inline constexpr auto MAX_DDL_BUFF_SIZE = 0x200;

  IMPL_SLICE_METHODS(
      ddlBuff, std::min<uint16_t>(ddlBuffSize, MAX_DDL_BUFF_SIZE), DdlBuff);
});
ASSERT_SIZE(MutableClientInfo, 0x410);

struct SharedDataCenterInfo {
  uint16_t shortId = 0;
  uint8_t rtt[10] = {0};
};
ASSERT_SIZE(SharedDataCenterInfo, 0xC);

// Verified
PACKED(struct FixedClientInfo {
  XUID xuid = 0;
  name_t gamertag = {0};
  dw::net::bdCommonAddrRef commonAdr = {};
  int32_t qport = 0;
  uint8_t latencyBand = 0;
  bool isGuest = false;
  uint8_t _padding36[2];
  int32_t granted = 0;
  int32_t consumed = 0;
  int32_t date = 0;
  int32_t daily = 0;
  uint8_t connectionType = 0;
  uint8_t _padding49[1];
  SharedDataCenterInfo datacenter[10] = {{}};
  uint16_t countryCode = 0;
  float latitude = 0.0f;
  float longitude = 0.0f;
  bool experimentActive = false;
  uint8_t _paddingCD[3];
});
ASSERT_SIZE(FixedClientInfo, 0xD0);

// Verified
PACKED(struct LANClientInfo {
  XUID xuid = 0;
  name_t gamertag = {0};
});
ASSERT_SIZE(LANClientInfo, 0x28);

template <typename T> union LobbyModulePool {
  typedef uint8_t index_t;

  T pool[+LobbyModule::COUNT] = {{}};

  struct {
    T host;
    T client;
  };

  constexpr void assert_range([[maybe_unused]] index_t index) const {
    assert(index < +LobbyModule::COUNT &&
           "index to LobbyModulePool must satisfy range "
           "LobbyModule::HOST <= "
           "index < LobbyModule::COUNT");
  }
  template <IntegralLike<index_t> Index>
  constexpr const T &get(Index index_arg) const noexcept {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return pool[index];
  }
  template <IntegralLike<index_t> Index>
  constexpr T &get(Index index_arg) noexcept {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return pool[index];
  }

  template <IntegralLike<index_t> Index>
  constexpr const T &operator[](Index index) const noexcept {
    return get(index);
  }
  template <IntegralLike<index_t> Index>
  constexpr T &operator[](Index index) noexcept {
    return get(index);
  }

  operator const T *() const noexcept { return pool; }
  operator T *() noexcept { return pool; }

  operator const void *() const noexcept { return pool; }
  operator void *() noexcept { return pool; }

  operator const array<T, +LobbyModule::COUNT> &() const noexcept {
    return pool;
  }

  operator array<T, +LobbyModule::COUNT> &() noexcept { return pool; }

  operator std::span<const T, +LobbyModule::COUNT>() const noexcept {
    return std::span<const T, +LobbyModule::COUNT>(pool);
  }

  operator std::span<T, +LobbyModule::COUNT>() noexcept {
    return std::span<T, +LobbyModule::COUNT>(pool);
  }

  static constexpr auto size() noexcept { return +LobbyModule::COUNT; }
};
} // namespace lobby
} // namespace game
