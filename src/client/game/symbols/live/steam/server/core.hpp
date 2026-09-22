#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace live {
namespace steam {
namespace server {
WEAK symbol<LiveUserAuthData *(LiveSteamServer *server, CSteamID steamId)>
    LiveSteamServer_GetAuthDataById{0x141EA88B0, 0x141EB5340, 0x140475020};
WEAK symbol<void(LiveSteamServer *server)>
    LiveSteamServer_EndAllClientAuthSessions{0x141EA8EC0, 0x141EB5950, 0x0};
WEAK symbol<void(LiveSteamServer *server)>
    LiveSteamServer_SteamServersDisconnected_Handle{0x141EAAA00, 0x141EB7490,
                                                    0x0};
} // namespace server
} // namespace steam
} // namespace live
} // namespace game
