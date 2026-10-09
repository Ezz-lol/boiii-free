#pragma once

#include <cstdint>

#include <game/structs/core.hpp>
#include <game/structs/net/core.hpp>
namespace game {
namespace lobby {

struct LobbyMsg;

namespace msg {

typedef fastcall_t<void(ControllerIndex_t controllerIndex, net::netadr_t *adr,
                        XUID xuid, LobbyMsg *msg)>
    LobbyMsg_MsgHandleCallback;

enum class MsgType : int32_t {
  NONE = -1,
  INFO_REQUEST = 0,
  INFO_RESPONSE = 1,
  LOBBY_STATE_PRIVATE = 2,
  LOBBY_STATE_GAME = 3,
  LOBBY_STATE_GAMEPUBLIC = 4,
  LOBBY_STATE_GAMECUSTOM = 5,
  LOBBY_STATE_GAMETHEATER = 6,
  LOBBY_HOST_HEARTBEAT = 7,
  LOBBY_HOST_DISCONNECT = 8,
  LOBBY_HOST_DISCONNECT_CLIENT = 9,
  LOBBY_HOST_LEAVE_WITH_PARTY = 0xA,
  LOBBY_CLIENT_HEARTBEAT = 0xB,
  LOBBY_CLIENT_DISCONNECT = 0xC,
  LOBBY_CLIENT_RELIABLE_DATA = 0xD,
  LOBBY_CLIENT_CONTENT = 0xE,
  LOBBY_MODIFIED_STATS = 0xF,
  JOIN_LOBBY = 0x10,
  JOIN_RESPONSE = 0x11,
  JOIN_AGREEMENT_REQUEST = 0x12,
  JOIN_AGREEMENT_RESPONSE = 0x13,
  JOIN_COMPLETE = 0x14,
  JOIN_MEMBER_INFO = 0x15,
  SERVERLIST_INFO = 0x16,
  PEER_TO_PEER_CONNECTIVITY_TEST = 0x17,
  PEER_TO_PEER_INFO = 0x18,
  LOBBY_MIGRATE_TEST = 0x19,
  MIGRATE_ANNOUNCE_HOST = 0x1A,
  MIGRATE_START = 0x1B,
  INGAME_MIGRATE_TO = 0x1C,
  MIGRATE_NEW_HOST = 0x1D,
  VOICE_PACKET = 0x1E,
  VOICE_RELAY_PACKET = 0x1F,
  DEMO_STATE = 0x20,
  COUNT = 0x21,
};
IMPL_ENUM_OPERATORS(MsgType);

struct LobbyMsgHandler {
  MsgType msgType;
  LobbyMsg_MsgHandleCallback *function;
};

#ifndef DEFINE_LOBBYMSGHANDLER_TABLE_METHODS
#define DEFINE_LOBBYMSGHANDLER_TABLE_METHODS()                                 \
  static inline constexpr auto size() noexcept { return COUNT; }               \
                                                                               \
  inline constexpr void assert_range([[maybe_unused]] size_t index) const {    \
    assert(index < COUNT && "index to aligned_array must be < array length");  \
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

  static inline constexpr auto COUNT = 12;
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

  static inline constexpr auto COUNT = 12;
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
  static inline constexpr auto COUNT = 5;
  LobbyMsgHandler handlers[COUNT];

  DEFINE_LOBBYMSGHANDLER_TABLE_METHODS();
};
} // namespace msg
} // namespace lobby
} // namespace game
