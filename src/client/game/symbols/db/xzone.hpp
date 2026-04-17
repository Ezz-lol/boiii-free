#pragma once
#ifdef GAME_HPP

#include "../macros.hpp"
#include <cstdint>

namespace game {
namespace db {
namespace xzone {

WEAK symbol<bool> g_zoneInited{0x0, 0x14690202C};
WEAK symbol<uint32_t> g_zoneCount{0x14941097C, 0x14699D21C};
WEAK symbol<uint32_t> g_zoneIndex{0x0, 0x1469BB268};
WEAK symbol<ZonePool<XZoneName>> g_zoneNames{0x14998FB80, 0x146E83270};
WEAK symbol<ZonePool<XZone>> g_zones{0x0, 0x146E84AD0};
WEAK symbol<ZonePool<char[65]>> g_zoneNameList{0x0, 0x146E876D0};
WEAK symbol<ZonePool<XZoneInfoInternal>> g_zoneInfo{0x149995020, 0x146E88770};
WEAK symbol<uint32_t> g_zoneInfoCount{0x149996620, 0x146E88714};

} // namespace xzone
} // namespace db
} // namespace game
#endif