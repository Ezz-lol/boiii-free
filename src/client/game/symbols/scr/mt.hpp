#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace scr {
namespace mt {
WEAK symbol<scrMemTreeGlob_t> gScrMemTreeGlob{0x144D5BE80, 0x14225C380};
WEAK symbol<scrMemTreePub_t> gScrMemTreePub{0x1450DC2D0, 0x14225C208};
WEAK symbol<GScrMemoryTreeServerNodePool> gScrMemTreeServerNodes{0x144D5C280,
                                                                 0x14225C880};
WEAK symbol<GScrMemoryTreeServerHeadPool> gScrMemTreeServerHead{0x1450DC280,
                                                                0x14225C310};
} // namespace mt
} // namespace scr
} // namespace game