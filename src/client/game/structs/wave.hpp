#pragma once

#include <cstdint>

#include <windows.h>

#include <mmeapi.h>

#include <dsound.h>

#include "game/structs/quake/core.hpp"
#include "structs/array.hpp"
#include <game/structs/macros.hpp>
namespace game {
namespace wave {

// Verified
struct WaveInLineInfo {
  qboolean isValid = qfalse;
  uint32_t mixerID = 0;
  uint32_t dstWaveInLineID = 0;
  int32_t oldRecLevel = 0;
  int32_t oldMuteState = 0;
};
ASSERT_SIZE(WaveInLineInfo, 0x14);

// Verified
inline constexpr auto WAVE_IN_LINE_INFO_POOL_SIZE = 8;
typedef Pool<WaveInLineInfo, WAVE_IN_LINE_INFO_POOL_SIZE, uint8_t>
    WaveInLineInfoPool;

inline constexpr auto EXTENDED_WAVE_IN_LINE_INFO_POOL_SIZE = 0xFFFF;
typedef Pool<WaveInLineInfo, EXTENDED_WAVE_IN_LINE_INFO_POOL_SIZE, uint8_t>
    ExtendedWaveInLineInfoPool;
} // namespace wave
} // namespace game
