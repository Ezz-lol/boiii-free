#pragma once

#include <cstdint>

#include "game/structs/core.hpp"
#include "game/structs/lobby/core.hpp"
#include "game/structs/net/core.hpp"
#include "game/structs/net/msg.hpp"
namespace game {
namespace lobby {

namespace msg {

struct LobbyMsg;
typedef fastcall_t<void(ControllerIndex_t controllerIndex,
                        net::netadr_t *fromAdr, XUID fromXUID,
                        LobbyMsg *lobbyMsg)>
    LobbyMsg_MsgHandleCallback;
typedef LobbyMsg_MsgHandleCallback *LobbyMsg_MsgHandleCallbackPtr;

enum class LobbyMsgElementType : uint32_t {
  INT32 = 0x0,
  UINT32 = 0x1,
  INT16 = 0x2,
  UINT16 = 0x3,
  INT8 = 0x4,
  UINT8 = 0x5,
  INT64 = 0x6,
  UINT64 = 0x7,
  FLOAT = 0x8,
  XUID = 0x9,
  STRING = 0xA,
  GLOB = 0xB,
  ARRAY_BEGIN = 0xC,
  ARRAY_ELEMENT = 0xD,
  ARRAY_END = 0xE,
  DEBUG_START = 0xF,
  DEBUG_END = 0x10,
  COUNT = 0x11,
};

inline constexpr const char *serialize(const LobbyMsgElementType type) {
  switch (type) {
  case LobbyMsgElementType::INT32:
    return "ELEMENT_TYPE_INT32";
  case LobbyMsgElementType::UINT32:
    return "ELEMENT_TYPE_UINT32";
  case LobbyMsgElementType::INT16:
    return "ELEMENT_TYPE_INT16";
  case LobbyMsgElementType::UINT16:
    return "ELEMENT_TYPE_UINT16";
  case LobbyMsgElementType::INT8:
    return "ELEMENT_TYPE_INT8";
  case LobbyMsgElementType::UINT8:
    return "ELEMENT_TYPE_UINT8";
  case LobbyMsgElementType::INT64:
    return "ELEMENT_TYPE_INT64";
  case LobbyMsgElementType::UINT64:
    return "ELEMENT_TYPE_UINT64";
  case LobbyMsgElementType::FLOAT:
    return "ELEMENT_TYPE_FLOAT";
  case LobbyMsgElementType::XUID:
    return "ELEMENT_TYPE_XUID";
  case LobbyMsgElementType::STRING:
    return "ELEMENT_TYPE_STRING";
  case LobbyMsgElementType::GLOB:
    return "ELEMENT_TYPE_GLOB";
  case LobbyMsgElementType::ARRAY_BEGIN:
    return "ELEMENT_TYPE_ARRAY_BEGIN";
  case LobbyMsgElementType::ARRAY_ELEMENT:
    return "ELEMENT_TYPE_ARRAY_ELEMENT";
  case LobbyMsgElementType::ARRAY_END:
    return "ELEMENT_TYPE_ARRAY_END";
  case LobbyMsgElementType::DEBUG_START:
    return "ELEMENT_TYPE_DEBUG_START";
  case LobbyMsgElementType::DEBUG_END:
    return "ELEMENT_TYPE_DEBUG_END";
  case LobbyMsgElementType::COUNT:
    return "ELEMENT_TYPE_COUNT";
  default:
    return "ELEMENT_TYPE_UNKNOWN";
  }
}

enum class MsgType : int32_t {
  NONE = -1,
  INFO_REQUEST = 0,
  INFO_RESPONSE = 1,
  LOBBY_STATE_PRIVATE = 2,
  LOBBY_STATE_GAME = 3,
  // Unused
  LOBBY_STATE_GAMEPUBLIC = 4,
  // Unused
  LOBBY_STATE_GAMECUSTOM = 5,
  // Unused
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

inline constexpr const char *serialize(const MsgType type) {
  switch (type) {
  case MsgType::NONE:
    return "MSG_TYPE_NONE";
  case MsgType::INFO_REQUEST:
    return "MSG_TYPE_INFO_REQUEST";
  case MsgType::INFO_RESPONSE:
    return "MSG_TYPE_INFO_RESPONSE";
  case MsgType::LOBBY_STATE_PRIVATE:
    return "MSG_TYPE_LOBBY_STATE_PRIVATE";
  case MsgType::LOBBY_STATE_GAME:
    return "MSG_TYPE_LOBBY_STATE_GAME";
  case MsgType::LOBBY_STATE_GAMEPUBLIC:
    return "MSG_TYPE_LOBBY_STATE_GAMEPUBLIC";
  case MsgType::LOBBY_STATE_GAMECUSTOM:
    return "MSG_TYPE_LOBBY_STATE_GAMECUSTOM";
  case MsgType::LOBBY_STATE_GAMETHEATER:
    return "MSG_TYPE_LOBBY_STATE_GAMETHEATER";
  case MsgType::LOBBY_HOST_HEARTBEAT:
    return "MSG_TYPE_LOBBY_HOST_HEARTBEAT";
  case MsgType::LOBBY_HOST_DISCONNECT:
    return "MSG_TYPE_LOBBY_HOST_DISCONNECT";
  case MsgType::LOBBY_HOST_DISCONNECT_CLIENT:
    return "MSG_TYPE_LOBBY_HOST_DISCONNECT_CLIENT";
  case MsgType::LOBBY_HOST_LEAVE_WITH_PARTY:
    return "MSG_TYPE_LOBBY_HOST_LEAVE_WITH_PARTY";
  case MsgType::LOBBY_CLIENT_HEARTBEAT:
    return "MSG_TYPE_LOBBY_CLIENT_HEARTBEAT";
  case MsgType::LOBBY_CLIENT_DISCONNECT:
    return "MSG_TYPE_LOBBY_CLIENT_DISCONNECT";
  case MsgType::LOBBY_CLIENT_RELIABLE_DATA:
    return "MSG_TYPE_LOBBY_CLIENT_RELIABLE_DATA";
  case MsgType::LOBBY_CLIENT_CONTENT:
    return "MSG_TYPE_LOBBY_CLIENT_CONTENT";
  case MsgType::LOBBY_MODIFIED_STATS:
    return "MSG_TYPE_LOBBY_MODIFIED_STATS";
  case MsgType::JOIN_LOBBY:
    return "MSG_TYPE_JOIN_LOBBY";
  case MsgType::JOIN_RESPONSE:
    return "MSG_TYPE_JOIN_RESPONSE";
  case MsgType::JOIN_AGREEMENT_REQUEST:
    return "MSG_TYPE_JOIN_AGREEMENT_REQUEST";
  case MsgType::JOIN_AGREEMENT_RESPONSE:
    return "MSG_TYPE_JOIN_AGREEMENT_RESPONSE";
  case MsgType::JOIN_COMPLETE:
    return "MSG_TYPE_JOIN_COMPLETE";
  case MsgType::JOIN_MEMBER_INFO:
    return "MSG_TYPE_JOIN_MEMBER_INFO";
  case MsgType::SERVERLIST_INFO:
    return "MSG_TYPE_SERVERLIST_INFO";
  case MsgType::PEER_TO_PEER_CONNECTIVITY_TEST:
    return "MSG_TYPE_PEER_TO_PEER_CONNECTIVITY_TEST";
  case MsgType::PEER_TO_PEER_INFO:
    return "MSG_TYPE_PEER_TO_PEER_INFO";
  case MsgType::LOBBY_MIGRATE_TEST:
    return "MSG_TYPE_LOBBY_MIGRATE_TEST";
  case MsgType::MIGRATE_ANNOUNCE_HOST:
    return "MSG_TYPE_MIGRATE_ANNOUNCE_HOST";
  case MsgType::MIGRATE_START:
    return "MSG_TYPE_MIGRATE_START";
  case MsgType::INGAME_MIGRATE_TO:
    return "MSG_TYPE_INGAME_MIGRATE_TO";
  case MsgType::MIGRATE_NEW_HOST:
    return "MSG_TYPE_MIGRATE_NEW_HOST";
  case MsgType::VOICE_PACKET:
    return "MSG_TYPE_VOICE_PACKET";
  case MsgType::VOICE_RELAY_PACKET:
    return "MSG_TYPE_VOICE_RELAY_PACKET";
  case MsgType::DEMO_STATE:
    return "MSG_TYPE_DEMO_STATE";
  case MsgType::COUNT:
    return "MSG_TYPE_COUNT";
  default:
    return "MSG_TYPE_UNKNOWN";
  }
}

inline constexpr std::string_view lobbyMsgName[+MsgType::COUNT] = {
    "InfoRequest",
    "InfoResponse",
    "StatePrivate",
    "StateGame",
    "StateGamePublic",
    "StateGameCustom",
    "StateGameTheater",
    "HostHeartbeat",
    "HostDisconnect",
    "HostDisconnectClient",
    "HostLeaveWithParty",
    "ClientHeartbeat",
    "ClientDisconnect",
    "ClientReliableData",
    "ClientContent",
    "ClientStatsModified",
    "JoinLobby",
    "JoinResponse",
    "JoinAgreementRequest",
    "JoinAgreementResponse",
    "JoinComplete",
    "JoinMemberInfo",
    "ServerlistInfo",
    "P2PConnectivityTest",
    "P2PInfo",
    "MigrateBandwidthTest",
    "AnnounceHost",
    "StartMigrate",
    "MigrateHostTo",
    "NewIngameHost",
    "VoicePacket",
    "VoiceRelayPacket",
    "DemoState",
};
inline constexpr auto MAX_MSG_NAME_STRING_LENGTH = 0x20;
typedef char msgName_t[MAX_MSG_NAME_STRING_LENGTH];

enum class PackageType : uint32_t {
  NONE = 0x0,
  WRITE = 0x1,
  READ = 0x2,
};

inline constexpr const char *serialize(const PackageType type) {
  switch (type) {
  case PackageType::NONE:
    return "PACKAGE_TYPE_NONE";
  case PackageType::WRITE:
    return "PACKAGE_TYPE_WRITE";
  case PackageType::READ:
    return "PACKAGE_TYPE_READ";
  default:
    return "PACKAGE_TYPE_UNKNOWN";
  }
}

inline constexpr auto LOBBYMSG_MAX_KEY_LENGTH = 0x10;
typedef char lobbyMsgKey_t[LOBBYMSG_MAX_KEY_LENGTH];

union LobbyMsgEncodeFlags {
  uint8_t packed;
  struct {
    uint8_t isArray : 1;
    uint8_t encodeKey : 1;
    uint8_t : 6;
  };

  alwaysinline constexpr void clear() noexcept { packed = 0; }

  inline std::string serialize() const noexcept {
    return std::format(R"({{ "isArray": {}, "encodeKey": {} }})", isArray,
                       encodeKey);
  }
};

template <typename T>
concept Packageable = requires(T t, LobbyMsg &msg) {
  { t.package(msg) } -> std::same_as<bool>;
};

template <typename T>
concept TypedPacket = requires {
  { T::MESSAGE_TYPE } -> std::same_as<const MsgType &>;
};

template <typename T>
concept Packet = Packageable<T> && TypedPacket<T>;

struct LobbyMsg {
  net::msg::msg_t msg;
  MsgType msgType;
  LobbyMsgEncodeFlags encodeFlags;
  PackageType packageType;

  bool package_int(const std::string_view &key, int32_t *val);
  template <typename T>
    requires(sizeof(T) == sizeof(int32_t) && !std::is_same_v<T, int32_t>)
  bool package_int(const std::string_view &key, T *val) {
    return package_int(key, reinterpret_cast<int32_t *>(val));
  }
  bool package_bool(const std::string_view &key, bool *val);
  bool package_bool(const std::string_view &key, qboolean *val) {
    return package_bool(key, &val->value);
  }
  bool package_char(const std::string_view &key, char *val);
  bool package_glob(const std::string_view &key, void *val, size_t numbytes);
  template <const size_t N>
  alwaysinline bool package_glob(const std::string_view &key,
                                 uint8_t (&arr)[N]) {
    return package_glob(key, reinterpret_cast<void *>(arr), N);
  }
  template <const size_t N>
  alwaysinline bool package_glob(const std::string_view &key,
                                 std::span<uint8_t, N> data) {
    return package_glob(key, reinterpret_cast<void *>(data.data()), N);
  }

  alwaysinline bool package_glob(const std::string_view &key,
                                 std::span<uint8_t> data) {
    return package_glob(key, reinterpret_cast<void *>(data.data()),
                        data.size());
  }

  bool package_uint(const std::string_view &key, uint32_t *val);
  template <typename T>
    requires(sizeof(T) == sizeof(uint32_t) && !std::is_same_v<T, uint32_t>)
  bool package_uint(const std::string_view &key, T *val) {
    return package_uint(key, reinterpret_cast<uint32_t *>(val));
  }
  bool package_xuid(const std::string_view &key, XUID *val);
  bool package_float(const std::string_view &key, float *val);
  bool package_short(const std::string_view &key, int16_t *val);
  bool package_uchar(const std::string_view &key, uint8_t *val);
  bool package_string(const std::string_view &key, char *val, size_t maxChar);
  template <const size_t N>
  alwaysinline bool package_string(const std::string_view &key,
                                   char (&val)[N]) {
    return package_string(key, val, N);
  }
  bool package_uint64(const std::string_view &key, uint64_t *val);
  bool package_ushort(const std::string_view &key, uint16_t *val);
  bool package_element(bool addElement);

  // If we're expecting array to be terminated here, validate termination is
  // received. Otherwise, validate array element has been received.
  bool validated_package_element(bool addElement) {
    return package_element(addElement) == addElement;
  }
  bool package_arrayStart(const std::string_view &key);
  bool prep_readMsg();
  bool prep_readMsg(net::msg::msg_t *msg);
  bool prep_readMsg(uint8_t *data, size_t length);
  bool prep_writeMsg(uint8_t *data, size_t length, MsgType type);
  bool read_char(const std::string_view &expectedKey, char *val);
  bool read_int(const std::string_view &expectedKey, int32_t *val);
  template <typename T>
    requires(sizeof(T) == sizeof(int32_t) && !std::is_same_v<T, int32_t>)
  bool read_int(const std::string_view &key, T *val) {
    return read_int(key, reinterpret_cast<int32_t *>(val));
  }
  bool read_bool(const std::string_view &expectedKey, bool *val);
  bool read_glob(const std::string_view &expectedKey, void *data,
                 size_t nBytes);
  template <const size_t N>
  alwaysinline bool read_glob(const std::string_view &expectedKey,
                              uint8_t (&arr)[N]) {
    return read_glob(expectedKey, reinterpret_cast<void *>(arr), N);
  }
  bool read_uint(const std::string_view &expectedKey, uint32_t *val);
  template <typename T>
    requires(sizeof(T) == sizeof(uint32_t) && !std::is_same_v<T, uint32_t>)
  bool read_uint(const std::string_view &key, T *val) {
    return read_uint(key, reinterpret_cast<uint32_t *>(val));
  }
  bool read_xuid(const std::string_view &expectedKey, XUID *val);
  bool read_float(const std::string_view &expectedKey, float *val);
  bool read_short(const std::string_view &expectedKey, int16_t *val);
  bool read_uchar(const std::string_view &expectedKey, uint8_t *val);
  bool read_string(const std::string_view &expectedKey, char *outStr,
                   size_t maxChar);
  template <const size_t N>
  alwaysinline bool read_string(const std::string_view &key,
                                char (&outStr)[N]) {
    return read_string(key, outStr, N);
  }

  bool read_uint64(const std::string_view &expectedKey, uint64_t *val);
  bool read_ushort(const std::string_view &expectedKey, uint16_t *val);
  bool read_arrayBegin(const std::string_view &expectedKey);
  LobbyMsgElementType read_elementType();
  bool write_char(const std::string_view &key, char val);
  bool write_int(const std::string_view &key, int32_t val);
  template <typename T>
    requires(sizeof(T) == sizeof(int32_t) && !std::is_same_v<T, int32_t>)
  bool write_int(const std::string_view &key, T val) {
    return write_int(key, std::bit_cast<int32_t>(val));
  }
  bool write_glob(const std::string_view &key, void *val, size_t numbytes);
  template <const size_t N>
  alwaysinline bool write_glob(const std::string_view &key, uint8_t (&arr)[N]) {
    return write_glob(key, reinterpret_cast<void *>(arr), N);
  }

  bool write_uint(const std::string_view &key, uint32_t val);
  template <typename T>
    requires(sizeof(T) == sizeof(uint32_t) && !std::is_same_v<T, uint32_t>)
  bool write_uint(const std::string_view &key, T val) {
    return write_uint(key, std::bit_cast<uint32_t>(val));
  }
  bool write_xuid(const std::string_view &key, XUID val);
  bool write_float(const std::string_view &key, float val);
  bool write_short(const std::string_view &key, int16_t val);
  bool write_uchar(const std::string_view &key, uint8_t val);
  alwaysinline bool write_bool(const std::string_view &key, bool val) {
    return write_uchar(key, static_cast<uint8_t>(val));
  }

  bool write_string(const std::string_view &key, const std::string_view &val);
  template <const size_t N>
  alwaysinline bool write_string(const std::string_view &key, char (&val)[N]) {
    return write_string(key, val, N);
  }
  bool write_uint64(const std::string_view &key, uint64_t val);
  bool write_ushort(const std::string_view &key, uint16_t val);

  template <Packageable T> alwaysinline bool package(T &data) {
    return data.package(*this);
  }

  template <Packageable T>
  alwaysinline bool package(T *data, const size_t count) {
    bool result = true;
    for (size_t i = 0; result &= validated_package_element(i < count); ++i) {
      result &= package(data[i]);
    }

    return result;
  }

  template <Packageable T, const size_t N>
  alwaysinline bool package(T (&data)[N]) {
    return package(data, N);
  }

  template <Packageable T> alwaysinline bool package(LobbyModulePool<T> &data) {
    return package(data.pool, data.size());
  }

  template <Packageable T, const size_t N>
  alwaysinline bool package(std::span<T, N> &data) {
    return package(data.data(), N);
  }

  template <Packageable T> alwaysinline bool package(std::span<T> &data) {
    return package(data.data(), data.size());
  }

  template <Packageable T, const size_t N>
  alwaysinline bool package(std::array<T, N> &data) {
    return package(data.data(), N);
  }

  template <Packageable T> alwaysinline bool package(std::vector<T> &data) {
    return package(data.data(), data.size());
  }

  template <Packet P, const size_t N>
  alwaysinline bool write(P &packet, uint8_t (&buf)[N]) {
    return prep_writeMsg(buf, N, P::MESSAGE_TYPE) && package(packet);
  }

  template <Packet P, const size_t N>
  alwaysinline bool read(P &packet, uint8_t (&buf)[N]) {
    return prep_readMsg(buf, N) && package(packet);
  }

  inline std::string serialize() const noexcept {
    return std::format(
        R"({{ "msgType": {}, "encodeFlags": {}, "packageType": {} }})",
        msg::serialize(msgType), encodeFlags.serialize(),
        msg::serialize(packageType));
  }
};

enum class MsgState : uint32_t {
  IDLE = 0x0,
  WAITING_FOR_ACK = 0x1,
};

inline constexpr const char *serialize(const MsgState state) {
  switch (state) {
  case MsgState::IDLE:
    return "MSG_STATE_IDLE";
  case MsgState::WAITING_FOR_ACK:
    return "MSG_STATE_WAITING_FOR_ACK";
  default:
    return "MSG_STATE_UNKNOWN";
  }
}

struct EVENT_HANDLE {
  bool manualReset = false;
  uint8_t _padding1[7] = {0};

  inline std::string serialize() const noexcept {
    return std::format(R"({{ "manualReset": {} }})", ::serialize(manualReset));
  }
};

enum class netchanMessageState_e : uint32_t {
  COMPLETE = 0x0,
  FAILED = 0x1,
  TIMEOUT = 0x2,
};

inline constexpr const char *serialize(const netchanMessageState_e state) {
  switch (state) {
  case netchanMessageState_e::COMPLETE:
    return "NETCHAN_MESSAGE_STATE_COMPLETE";
  case netchanMessageState_e::FAILED:
    return "NETCHAN_MESSAGE_STATE_FAILED";
  case netchanMessageState_e::TIMEOUT:
    return "NETCHAN_MESSAGE_STATE_TIMEOUT";
  default:
    return "NETCHAN_MESSAGE_STATE_UNKNOWN";
  }
}

struct netchanMsgConfig_s {
  EVENT_HANDLE completedEvent = {};
  netchanMessageState_e status = netchanMessageState_e::FAILED;
  int32_t timeoutMS = 0;

  inline std::string serialize() const noexcept {
    return std::format(
        R"({{ "completedEvent": {}, "status": {}, "timeoutMS": {} }})",
        completedEvent.serialize(), msg::serialize(status), timeoutMS);
  }
};

typedef netchanMsgConfig_s netchanMsgConfig_t;

// Verified
struct NetChanMsg {
  int32_t sentTime = 0;
  MsgState msgState = MsgState::IDLE;
  MsgType msgType = MsgType::NONE;
  netchanMsgConfig_t config = {};
  int32_t timeout = 0;
  XUID toXUID = 0;
  XUID clientXUID = 0;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"sentTime\": {}, \"msgState\": {}, \"msgType\": {}, \"config\": "
        "{}, "
        "\"timeout\": {}, \"toXUID\": {:016X}, \"clientXUID\": {:016X} }}",
        sentTime, msg::serialize(msgState), msg::serialize(msgType),
        config.serialize(), timeout, toXUID, clientXUID);
  }
};
ASSERT_SIZE(NetChanMsg, 0x30);

struct LobbyMsgHandler {
  MsgType msgType;
  LobbyMsg_MsgHandleCallbackPtr function;

  inline std::string serialize() const noexcept {
    return std::format(R"({{ "msgType": {}, "function": {} }})",
                       msg::serialize(msgType), ::serialize(function));
  }
};
} // namespace msg
} // namespace lobby
} // namespace game
