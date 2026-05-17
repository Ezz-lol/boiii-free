#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "macros.hpp"

namespace game {
namespace ugc {
WEAK symbol<BuiltinWorkshopDataPool> builtinModsPool{0x15678D170, 0x14933EAE0};
static ExtendedWorkshopDataPool modsPool = {};
static const ExtendedWorkshopDataPool *modsPoolPtr = &modsPool;
static const WorkshopData (*modsPoolDataPtr)[EXTENDED_WORKSHOP_DATA_POOL_SIZE] =
    &modsPool.data;
static const str32_t *modsPoolDataPublisherIdPtr =
    &modsPool.data[0].publisherId;
static const uint32_t *modsPoolCountPtr = &modsPool.count;

WEAK symbol<BuiltinWorkshopDataPool> builtinUsermapsPool{0x1567B3580,
                                                         0x149364EE8};
static ExtendedWorkshopDataPool usermapsPool = {};
static const ExtendedWorkshopDataPool *usermapsPoolPtr = &usermapsPool;
static const WorkshopData (
    *usermapsPoolDataPtr)[EXTENDED_WORKSHOP_DATA_POOL_SIZE] =
    &usermapsPool.data;
static const str32_t *usermapsPoolDataPublisherIdPtr =
    &usermapsPool.data[0].publisherId;
static const uint32_t *usermapsPoolCountPtr = &usermapsPool.count;

WEAK symbol<bool()> isModLoaded{0x1420D5020};
WEAK symbol<const char *()> getPublisherIdFromLoadedMod{0x1420D7680,
                                                        0x1404E3230};
WEAK symbol<void(LocalClientNum_t localClientNum, const char *mod,
                 bool reloadFS)>
    loadMod{0x1420D6930};
WEAK symbol<bool(LocalClientNum_t localClientNum)> isModLoading{0x1420D6DE0};
WEAK symbol<char> usermap_publisher_id{0x1567D9A24};
WEAK symbol<char> usermap_title{0x1567D99A0};
// This should correspond to folder name, with value something like
// cp_* for a campaign map, zm_* for zombies map, or mp_* for multiplayer map
WEAK symbol<char> internal_usermap_id{0x1567D9A04};
WEAK symbol<void()> reloadUserContent{0x1420D66C0, 0x1404E25C0};
} // namespace ugc
} // namespace game

#endif