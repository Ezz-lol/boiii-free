#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace db {
namespace xasset {
namespace maptable {

WEAK symbol<MapTable *(const char *mapTableName)> Com_GetMapTable{0x1422A49E0,
                                                                  0x140565080};
}

} // namespace xasset
} // namespace db
} // namespace game