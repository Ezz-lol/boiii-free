#pragma once

#include <cstdint>
#include "core.hpp"

namespace game {
namespace cl {

enum class CLSwitchState : uint32_t {
  CL_SWITCH_STATE_IDLE = 0x0,
  CL_SWITCH_WAIT_FOR_CLIENTS_TO_CONNECT = 0x1,
  CL_SWITCH_STATE_LOAD_FASTFILE = 0x2,
  CL_SWITCH_STATE_PRIME_STREAMER = 0x3,
  CL_SWITCH_STATE_SWITCH_MAPS = 0x4,
};

enum CLSwitchMsg : uint32_t {
  CL_SWITCH_MSG_LOAD_MAP = 0x0,
  CL_SWITCH_MSG_SWITCH_MAPS = 0x1,
  CL_SWITCH_MSG_IDLE = 0x2,
};

#pragma pack(push, 1)
// sizeof=0x10
struct CLSwitchMessage {
  CLSwitchMsg id;
  uint8_t _padding04[4];
  union {
    str64_t *str;
    int64_t integer;
    uint64_t unsignedInteger;
    float real;
  } p;
};
static_assert(sizeof(CLSwitchMessage) == 0x10,
              "CLSwitchMessage size must be 0x10 bytes");
#pragma pack(pop)

// sizeof=0x7B0
#pragma pack(push, 1)
struct CLMapSwitch {
  CLSwitchState state;
  int32_t switchCount;
  str128_t prevConfigString[3];
  str128_t preloadMap;
  str128_t loadingMovie;
  PMemStack preloadPMemStack;
  bool preload;
  uint8_t _padding28D[3];
  int32_t preloadReserveFrameCount;
  uint32_t preloadReservePagesTaken;
  uint32_t preloadReservePagesNeeded;
  cinematic_id loadingMovieId;
  CLSwitchMessage messages[16];
  int messageCount;
  str64_t strArgs[16];
  int strCount;
  int timeFastfileLoadStarted;
  int timeTextureStreamingStarted;
};
static_assert(sizeof(CLMapSwitch) == 0x7B0,
              "CLMapSwitch size must be 0x7B0 bytes");
#pragma pack(pop)
} // namespace cl
} // namespace game