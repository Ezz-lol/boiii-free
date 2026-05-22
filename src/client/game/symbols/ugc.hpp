#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "macros.hpp"

namespace game {
namespace ugc {
WEAK symbol<BuiltinWorkshopDataPool> builtinModsPool{0x15678D170, 0x14933EAE0};
static ExtendedWorkshopDataPool modsPool{};
constexpr ExtendedWorkshopDataPool *modsPoolPtr = &modsPool;
constexpr WorkshopData (*modsPoolDataPtr)[EXTENDED_WORKSHOP_DATA_POOL_SIZE] =
    &modsPool.data;
constexpr str32_t *modsPoolDataPublisherIdPtr = &modsPool.data[0].publisherId;
constexpr uint32_t *modsPoolCountPtr = &modsPool.count;

WEAK symbol<BuiltinWorkshopDataPool> builtinUsermapsPool{0x1567B3580,
                                                         0x149364EE8};
static ExtendedWorkshopDataPool usermapsPool{};
constexpr ExtendedWorkshopDataPool *usermapsPoolPtr = &usermapsPool;
constexpr WorkshopData (
    *usermapsPoolDataPtr)[EXTENDED_WORKSHOP_DATA_POOL_SIZE] =
    &usermapsPool.data;
constexpr str32_t *usermapsPoolDataPublisherIdPtr =
    &usermapsPool.data[0].publisherId;
constexpr uint32_t *usermapsPoolCountPtr = &usermapsPool.count;

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

WEAK symbol<ModLoadState> mod_load_state{0x15678D168, 0x14933EAD8};

} // namespace ugc
} // namespace game

#endif