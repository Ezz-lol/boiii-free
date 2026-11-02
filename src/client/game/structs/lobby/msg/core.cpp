#include <std_include.hpp>

#include "game/game.hpp"
#include "utils/string.hpp"

namespace game {
namespace lobby {
namespace msg {

bool LobbyMsg::package_int(const std::string_view &key, int32_t *val) {
  switch (packageType) {
  case PackageType::READ:
    return read_int(key, val);
  case PackageType::WRITE:
    return write_int(key, *val);
  case PackageType::NONE:
    return false;
  }
}

bool LobbyMsg::package_bool(const std::string_view &key, bool *val) {
  switch (packageType) {
  case PackageType::READ:
    return read_bool(key, val);
  case PackageType::WRITE:
    return write_uchar(key, static_cast<uint8_t>(*val));
  case PackageType::NONE:
    return false;
  }
}

bool LobbyMsg::package_char(const std::string_view &key, char *val) {

  switch (packageType) {
  case PackageType::READ:
    return read_char(key, val);
  case PackageType::WRITE:
    return write_char(key, *val);
  case PackageType::NONE:
    return false;
  }
}

bool LobbyMsg::package_glob(const std::string_view &key, void *val,
                            size_t numBytes) {
  switch (packageType) {
  case PackageType::READ:
    return read_glob(key, val, numBytes);
  case PackageType::WRITE:
    return write_glob(key, val, numBytes);
  case PackageType::NONE:
    return false;
  }
}

bool LobbyMsg::package_uint(const std::string_view &key, uint32_t *val) {
  switch (packageType) {
  case PackageType::READ:
    return read_uint(key, val);
  case PackageType::WRITE:
    return write_uint(key, *val);
  case PackageType::NONE:
    return false;
  }
}

bool LobbyMsg::package_xuid(const std::string_view &key, XUID *val) {
  switch (packageType) {
  case PackageType::READ:
    return read_xuid(key, val);
  case PackageType::WRITE:
    return write_xuid(key, *val);
  case PackageType::NONE:
    return false;
  }
}

bool LobbyMsg::package_float(const std::string_view &key, float *val) {
  switch (packageType) {
  case PackageType::READ:
    return read_float(key, val);
  case PackageType::WRITE:
    return write_float(key, *val);
  case PackageType::NONE:
    return false;
  }
}

bool LobbyMsg::package_short(const std::string_view &key, int16_t *val) {
  switch (packageType) {
  case PackageType::READ:
    return read_short(key, val);
  case PackageType::WRITE:
    return write_short(key, *val);
  case PackageType::NONE:
    return false;
  }
}

bool LobbyMsg::package_uchar(const std::string_view &key, uint8_t *val) {
  switch (packageType) {
  case PackageType::READ:
    return read_uchar(key, val);
  case PackageType::WRITE:
    return write_uchar(key, *val);
  case PackageType::NONE:
    return false;
  }
}

bool LobbyMsg::package_string(const std::string_view &key, char *val,
                              size_t maxChar) {
  if (packageType == PackageType::NONE) {
    return false;
  }
  if (packageType != PackageType::WRITE) {
    return read_string(key, val, maxChar);
  }
  if (std::strlen(val) > maxChar) {
    return false;
  }
  return write_string(key, val);
}

bool LobbyMsg::package_uint64(const std::string_view &key, uint64_t *val) {
  switch (packageType) {
  case PackageType::READ:
    return read_uint64(key, val);
  case PackageType::WRITE:
    return write_uint64(key, *val);
  case PackageType::NONE:
    return false;
  }
}

bool LobbyMsg::package_ushort(const std::string_view &key, uint16_t *val) {
  switch (packageType) {
  case PackageType::READ:
    return read_ushort(key, val);
  case PackageType::WRITE:
    return write_ushort(key, *val);
  case PackageType::NONE:
    return false;
  }
}

bool LobbyMsg::package_element(bool addElement) {

  switch (packageType) {
  case PackageType::NONE:
    return false;
  case PackageType::WRITE:
    if (!addElement) {
      msg.write_byte(static_cast<uint8_t>(LobbyMsgElementType::ARRAY_END));
      encodeFlags.clear();
      return false;
    }
    msg.write_byte(static_cast<uint8_t>(LobbyMsgElementType::ARRAY_ELEMENT));
    return !msg.overflowed;
  case PackageType::READ:
    if (read_elementType() != LobbyMsgElementType::INT32) {
      encodeFlags.clear();
      return false;
    }
    return true;
  default:
    return !msg.overflowed;
  }
}

bool LobbyMsg::package_arrayStart(const std::string_view &key) {
  switch (packageType) {
  case PackageType::NONE:
    return false;
  case PackageType::WRITE:
    msg.write_byte(static_cast<uint8_t>(LobbyMsgElementType::ARRAY_BEGIN));
    if (key.size() <= LOBBYMSG_MAX_KEY_LENGTH && encodeFlags.encodeKey) {
      msg.write_string(key);
    }
    encodeFlags.isArray = true;
    return !msg.overflowed;
    break;
  case PackageType::READ:
    read_arrayBegin(key);
    encodeFlags.isArray = true;
    return !msg.overflowed;
  }
}

bool LobbyMsg::prep_readMsg() {
  lobbyMsgKey_t keyBuf;

  packageType = PackageType::READ;
  encodeFlags.clear();
  const LobbyMsgElementType msgtypeElemType =
      static_cast<LobbyMsgElementType>(msg.read_byte());
  if (msgtypeElemType != LobbyMsgElementType::UINT8) {
    msg.overflowed = qtrue;
  }
  if (encodeFlags.encodeKey) {
    msg.read_string(keyBuf, LOBBYMSG_MAX_KEY_LENGTH);
    if (!utils::string::iequal("msgtype", keyBuf)) {
      msg.overflowed = qtrue;
    }
  }

  msgType = static_cast<MsgType>(msg.read_byte());

  bool result = false;
  if (!msg.overflowed) {
    const LobbyMsgElementType msgnameElemType =
        static_cast<LobbyMsgElementType>(msg.read_byte());
    if (msgnameElemType != LobbyMsgElementType::STRING) {
      msg.overflowed = qtrue;
    }
    if (encodeFlags.encodeKey) {
      msg.read_string(keyBuf, LOBBYMSG_MAX_KEY_LENGTH);
      if (!utils::string::iequal("msgname", keyBuf)) {
        msg.overflowed = qtrue;
      }
    }
    msgName_t msgname;
    msg.read_string(msgname, MAX_MSG_NAME_STRING_LENGTH);
    if (!msg.overflowed) {
      result = true;
    }
  }
  return result;
}

bool LobbyMsg::prep_readMsg(net::msg::msg_t *m) {
  msg = *m;
  return prep_readMsg();
}

bool LobbyMsg::prep_readMsg(uint8_t *data, size_t length) {
  msg.init(data, length);
  msg.cursize = static_cast<int32_t>(length);
  return prep_readMsg();
}

bool LobbyMsg::prep_writeMsg(uint8_t *data, size_t length, MsgType type) {
  msg.init(data, length);
  if (msg.cursize == 0 && msg.bit == 0) {
    packageType = PackageType::WRITE;
    msgType = type;
    encodeFlags.clear();
    if (write_uchar("msgtype", static_cast<uint16_t>(msgType)) &&
        write_string("msgname", lobbyMsgName[+msgType])) {
      return true;
    }
  }
  return false;
}

bool LobbyMsg::read_char(const std::string_view &expectedKey, char *val) {
  const LobbyMsgElementType type =
      static_cast<LobbyMsgElementType>(msg.read_byte());
  if (type != LobbyMsgElementType::INT8) {
    msg.overflowed = qtrue;
  }
  if (encodeFlags.encodeKey) {
    lobbyMsgKey_t key;
    msg.read_string(key, LOBBYMSG_MAX_KEY_LENGTH);
    if (!utils::string::iequal(expectedKey, key)) {
      msg.overflowed = qtrue;
    }
  }
  *val = static_cast<char>(msg.read_byte());
  return !msg.overflowed;
}

bool LobbyMsg::read_int(const std::string_view &expectedKey, int32_t *val) {
  const LobbyMsgElementType type =
      static_cast<LobbyMsgElementType>(msg.read_byte());
  if (type != LobbyMsgElementType::INT32) {
    msg.overflowed = qtrue;
  }
  if (encodeFlags.encodeKey) {
    lobbyMsgKey_t key;
    msg.read_string(key, LOBBYMSG_MAX_KEY_LENGTH);
    if (!utils::string::iequal(expectedKey, key)) {
      msg.overflowed = qtrue;
    }
  }
  *val = msg.read_long();
  return !msg.overflowed;
}

bool LobbyMsg::read_bool(const std::string_view &expectedKey, bool *val) {
  const LobbyMsgElementType type =
      static_cast<LobbyMsgElementType>(msg.read_byte());
  if (type != LobbyMsgElementType::UINT8) {
    msg.overflowed = qtrue;
  }
  if (encodeFlags.encodeKey) {
    lobbyMsgKey_t key;
    msg.read_string(key, LOBBYMSG_MAX_KEY_LENGTH);
    if (!utils::string::iequal(expectedKey, key)) {
      msg.overflowed = qtrue;
    }
  }
  *val = msg.read_byte() != 0;
  return !msg.overflowed;
}

bool LobbyMsg::read_glob(const std::string_view &expectedKey, void *data,
                         size_t nBytes) {
  const LobbyMsgElementType type =
      static_cast<LobbyMsgElementType>(msg.read_byte());
  if (type != LobbyMsgElementType::GLOB) {
    msg.overflowed = qtrue;
  }
  if (encodeFlags.encodeKey) {
    lobbyMsgKey_t key;
    msg.read_string(key, LOBBYMSG_MAX_KEY_LENGTH);
    if (!utils::string::iequal(expectedKey, key)) {
      msg.overflowed = qtrue;
    }
  }
  if (std::cmp_greater(msg.read_short(), nBytes)) {
    return false;
  }
  msg.read_data(data, nBytes);
  return !msg.overflowed;
}

bool LobbyMsg::read_uint(const std::string_view &expectedKey, uint32_t *val) {
  const LobbyMsgElementType type =
      static_cast<LobbyMsgElementType>(msg.read_byte());
  if (type != LobbyMsgElementType::UINT32) {
    msg.overflowed = qtrue;
  }
  if (encodeFlags.encodeKey) {
    lobbyMsgKey_t key;
    msg.read_string(key, LOBBYMSG_MAX_KEY_LENGTH);
    if (!utils::string::iequal(expectedKey, key)) {
      msg.overflowed = qtrue;
    }
  }
  *val = msg.read_long();
  return !msg.overflowed;
}

bool LobbyMsg::read_xuid(const std::string_view &expectedKey, XUID *val) {
  const LobbyMsgElementType type =
      static_cast<LobbyMsgElementType>(msg.read_byte());
  if (type != LobbyMsgElementType::XUID) {
    msg.overflowed = qtrue;
  }
  if (encodeFlags.encodeKey) {
    lobbyMsgKey_t key;
    msg.read_string(key, LOBBYMSG_MAX_KEY_LENGTH);
    if (!utils::string::iequal(expectedKey, key)) {
      msg.overflowed = qtrue;
    }
  }
  *val = msg.read_int64();
  return !msg.overflowed;
}

bool LobbyMsg::read_float(const std::string_view &expectedKey, float *val) {
  const LobbyMsgElementType type =
      static_cast<LobbyMsgElementType>(msg.read_byte());
  if (type != LobbyMsgElementType::FLOAT) {
    msg.overflowed = qtrue;
  }
  if (encodeFlags.encodeKey) {
    lobbyMsgKey_t key;
    msg.read_string(key, LOBBYMSG_MAX_KEY_LENGTH);
    if (!utils::string::iequal(expectedKey, key)) {
      msg.overflowed = qtrue;
    }
  }
  *val = msg.read_float();
  return !msg.overflowed;
}

bool LobbyMsg::read_short(const std::string_view &expectedKey, int16_t *val) {
  const LobbyMsgElementType type =
      static_cast<LobbyMsgElementType>(msg.read_byte());
  if (type != LobbyMsgElementType::INT16) {
    msg.overflowed = qtrue;
  }
  if (encodeFlags.encodeKey) {
    lobbyMsgKey_t key;
    msg.read_string(key, LOBBYMSG_MAX_KEY_LENGTH);
    if (!utils::string::iequal(expectedKey, key)) {
      msg.overflowed = qtrue;
    }
  }
  *val = msg.read_short();
  return !msg.overflowed;
}

bool LobbyMsg::read_uchar(const std::string_view &expectedKey, uint8_t *val) {
  const LobbyMsgElementType type =
      static_cast<LobbyMsgElementType>(msg.read_byte());
  if (type != LobbyMsgElementType::UINT8) {
    msg.overflowed = qtrue;
  }
  if (encodeFlags.encodeKey) {
    lobbyMsgKey_t key;
    msg.read_string(key, LOBBYMSG_MAX_KEY_LENGTH);
    if (!utils::string::iequal(expectedKey, key)) {
      msg.overflowed = qtrue;
    }
  }
  *val = msg.read_byte();
  return !msg.overflowed;
}

bool LobbyMsg::read_string(const std::string_view &expectedKey, char *outStr,
                           size_t maxChar) {
  const LobbyMsgElementType type =
      static_cast<LobbyMsgElementType>(msg.read_byte());
  if (type != LobbyMsgElementType::STRING) {
    msg.overflowed = qtrue;
  }
  if (encodeFlags.encodeKey) {
    lobbyMsgKey_t key;
    msg.read_string(key, LOBBYMSG_MAX_KEY_LENGTH);
    if (!utils::string::iequal(expectedKey, key)) {
      msg.overflowed = qtrue;
    }
  }
  msg.read_string(outStr, maxChar);
  return !msg.overflowed;
}

bool LobbyMsg::read_uint64(const std::string_view &expectedKey, uint64_t *val) {
  const LobbyMsgElementType type =
      static_cast<LobbyMsgElementType>(msg.read_byte());
  if (type != LobbyMsgElementType::UINT64) {
    msg.overflowed = qtrue;
  }
  if (encodeFlags.encodeKey) {
    lobbyMsgKey_t key;
    msg.read_string(key, LOBBYMSG_MAX_KEY_LENGTH);
    if (!utils::string::iequal(expectedKey, key)) {
      msg.overflowed = qtrue;
    }
  }
  *val = msg.read_int64();
  return !msg.overflowed;
}

bool LobbyMsg::read_ushort(const std::string_view &expectedKey, uint16_t *val) {
  const LobbyMsgElementType type =
      static_cast<LobbyMsgElementType>(msg.read_byte());
  if (type != LobbyMsgElementType::UINT16) {
    msg.overflowed = qtrue;
  }
  if (encodeFlags.encodeKey) {
    lobbyMsgKey_t key;
    msg.read_string(key, LOBBYMSG_MAX_KEY_LENGTH);
    if (!utils::string::iequal(expectedKey, key)) {
      msg.overflowed = qtrue;
    }
  }
  *val = msg.read_short();
  return !msg.overflowed;
}

bool LobbyMsg::read_arrayBegin(const std::string_view &expectedKey) {
  const LobbyMsgElementType type =
      static_cast<LobbyMsgElementType>(msg.read_byte());
  if (type != LobbyMsgElementType::ARRAY_BEGIN) {
    msg.overflowed = qtrue;
  }
  if (encodeFlags.encodeKey) {
    lobbyMsgKey_t key;
    msg.read_string(key, LOBBYMSG_MAX_KEY_LENGTH);
    if (!utils::string::iequal(expectedKey, key)) {
      msg.overflowed = qtrue;
    }
  }
  return !msg.overflowed;
}

LobbyMsgElementType LobbyMsg::read_elementType() {
  const LobbyMsgElementType type =
      static_cast<LobbyMsgElementType>(msg.read_byte());
  switch (type) {
  case LobbyMsgElementType::DEBUG_START:
  case LobbyMsgElementType::DEBUG_END:
    msg.overflowed = qtrue;
    break;
  default:
    break;
  }

  return type;
}

bool LobbyMsg::write_char(const std::string_view &key, char val) {
  if (key.size() > LOBBYMSG_MAX_KEY_LENGTH) {
    return false;
  }
  msg.write_byte(static_cast<uint8_t>(LobbyMsgElementType::INT8));
  if (encodeFlags.encodeKey) {
    msg.write_string(key);
  }
  msg.write_byte(static_cast<int8_t>(val));
  return !msg.overflowed;
}

bool LobbyMsg::write_int(const std::string_view &key, int32_t val) {
  if (key.size() > LOBBYMSG_MAX_KEY_LENGTH) {
    return false;
  }
  msg.write_byte(static_cast<uint8_t>(LobbyMsgElementType::INT32));
  if (encodeFlags.encodeKey) {
    msg.write_string(key);
  }
  msg.write_long(val);
  return !msg.overflowed;
}

bool LobbyMsg::write_glob(const std::string_view &key, void *val,
                          size_t numBytes) {
  msg.write_byte(static_cast<uint8_t>(LobbyMsgElementType::GLOB));
  if (key.size() <= LOBBYMSG_MAX_KEY_LENGTH && encodeFlags.encodeKey) {
    msg.write_string(key);
  }
  msg.write_short(static_cast<int16_t>(numBytes));
  msg.write_data(val, numBytes);
  return !msg.overflowed;
}

bool LobbyMsg::write_uint(const std::string_view &key, uint32_t val) {
  if (key.size() > LOBBYMSG_MAX_KEY_LENGTH) {
    return false;
  }
  msg.write_byte(static_cast<uint8_t>(LobbyMsgElementType::UINT32));
  if (encodeFlags.encodeKey) {
    msg.write_string(key);
  }
  msg.write_long(static_cast<int32_t>(val));
  return !msg.overflowed;
}

bool LobbyMsg::write_xuid(const std::string_view &key, XUID val) {
  if (key.size() > LOBBYMSG_MAX_KEY_LENGTH) {
    return false;
  }
  msg.write_byte(static_cast<uint8_t>(LobbyMsgElementType::XUID));
  if (encodeFlags.encodeKey) {
    msg.write_string(key);
  }
  msg.write_int64(static_cast<int64_t>(val));
  return !msg.overflowed;
}

bool LobbyMsg::write_float(const std::string_view &key, float val) {
  if (key.size() > LOBBYMSG_MAX_KEY_LENGTH) {
    return false;
  }
  msg.write_byte(static_cast<uint8_t>(LobbyMsgElementType::FLOAT));
  if (encodeFlags.encodeKey) {
    msg.write_string(key);
  }
  msg.write_float(val);
  return !msg.overflowed;
}

bool LobbyMsg::write_short(const std::string_view &key, int16_t val) {
  if (key.size() > LOBBYMSG_MAX_KEY_LENGTH) {
    return false;
  }
  msg.write_byte(static_cast<uint8_t>(LobbyMsgElementType::INT16));
  if (encodeFlags.encodeKey) {
    msg.write_string(key);
  }
  msg.write_short(val);
  return !msg.overflowed;
}

bool LobbyMsg::write_uchar(const std::string_view &key, uint8_t val) {
  if (key.size() > LOBBYMSG_MAX_KEY_LENGTH) {
    return false;
  }
  msg.write_byte(static_cast<uint8_t>(LobbyMsgElementType::UINT8));
  if (encodeFlags.encodeKey) {
    msg.write_string(key);
  }
  msg.write_byte(val);
  return !msg.overflowed;
}

bool LobbyMsg::write_string(const std::string_view &key,
                            const std::string_view &val) {
  msg.write_byte(static_cast<uint8_t>(LobbyMsgElementType::STRING));
  if (key.size() <= LOBBYMSG_MAX_KEY_LENGTH && encodeFlags.encodeKey) {
    msg.write_string(key);
  }
  msg.write_string(val);
  return !msg.overflowed;
}

bool LobbyMsg::write_uint64(const std::string_view &key, uint64_t val) {
  if (key.size() > LOBBYMSG_MAX_KEY_LENGTH) {
    return false;
  }
  msg.write_byte(static_cast<uint8_t>(LobbyMsgElementType::UINT64));
  if (encodeFlags.encodeKey) {
    msg.write_string(key);
  }
  msg.write_int64(static_cast<int64_t>(val));
  return !msg.overflowed;
}

bool LobbyMsg::write_ushort(const std::string_view &key, uint16_t val) {
  if (key.size() > LOBBYMSG_MAX_KEY_LENGTH) {
    return false;
  }
  msg.write_byte(static_cast<uint8_t>(LobbyMsgElementType::UINT16));
  if (encodeFlags.encodeKey) {
    msg.write_string(key);
  }
  msg.write_short(static_cast<int16_t>(val));
  return !msg.overflowed;
}
} // namespace msg
} // namespace lobby
} // namespace game
