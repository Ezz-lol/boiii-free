#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace live {
namespace steam {
namespace friends {
namespace overlay {
WEAK symbol<void(CSteamID xuid)> LiveSteam_Friend_Overlay_ShowFriendByID{
    0x141EA44F0, 0x141EB0F80, 0x0};
} // namespace overlay
} // namespace friends
} // namespace steam
} // namespace live
} // namespace game