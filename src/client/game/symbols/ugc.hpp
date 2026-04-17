#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "macros.hpp"

namespace game {
namespace ugc {
WEAK symbol<unsigned int> modsCount{0x15678D170, 0x14933EAE0};
WEAK symbol<workshop_data> modsPool{0x15678D178, 0x14933EAE8};

WEAK symbol<unsigned int> usermapsCount{0x1567B3580, 0x149364EE8};
WEAK symbol<workshop_data> usermapsPool{0x1567B3588, 0x149364EF0};

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