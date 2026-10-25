#pragma once

#include <game/structs/core.hpp>
#include <game/structs/macros.hpp>

#include <cstdint>

namespace game {
namespace live {
namespace pcache {
enum pcacheComponent_t : int32_t {
  PCACHE_COMPONENT_PUBLIC_PROFILE = 0x0,
  PCACHE_COMPONENT_PLAYER_EMBLEM = 0x1,
  PCACHE_COMPONENT_LEAGUE_EMBLEM = 0x2,
  PCACHE_COMPONENT_PLAYER_PRESENCE = 0x3,
  PCACHE_COMPONENT_PLAYER_PRIVACY = 0x4,
  PCACHE_NUM_COMPONENTS = 0x5,
};

enum PCacheComponentState : uint32_t {
  PCACHE_STATE_VALID = 0x1,
  // Fetched (or rendered, for emblems) on the next PCache_Update
  PCACHE_STATE_STALE = 0x2,
  PCACHE_STATE_TOUCHED = 0x8,
  // Demonware had nothing for this player
  PCACHE_STATE_NO_DATA = 0x10,
};

// Header of the cached player profile, emblem, presence and privacy entries
struct PCacheComponent {
  XUID xuid;
  ControllerIndex_t controllerIndex;
  pcacheComponent_t type;
  uint32_t updateTime;
  uint32_t touchTime;
  uint32_t state;
  uint8_t _padding1C[4];
};

ASSERT_SIZE(PCacheComponent, 0x20);
ASSERT_OFFSET(PCacheComponent, controllerIndex, 0x8);
ASSERT_OFFSET(PCacheComponent, type, 0xC);
ASSERT_OFFSET(PCacheComponent, updateTime, 0x10);
ASSERT_OFFSET(PCacheComponent, touchTime, 0x14);
ASSERT_OFFSET(PCacheComponent, state, 0x18);
} // namespace pcache
} // namespace live
} // namespace game
