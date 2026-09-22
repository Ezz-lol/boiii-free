#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace live {
namespace steam {
namespace friends {
WEAK symbol<void(CSteamID id)> LiveSteam_Friend_AddByID{0x141EA4430,
                                                        0x141EB0EC0, 0x0};
} // namespace friends
} // namespace steam
} // namespace live
} // namespace game
