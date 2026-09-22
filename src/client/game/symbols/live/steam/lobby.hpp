#pragma once

#include <cstdint>

#include <game/symbols/sym_include.hpp>

namespace game {
namespace live {
namespace steam {
namespace lobby {

WEAK symbol<void()> LiveSteamLobby_Pump{0x141EA62A0, 0x141EB2D30, 0x0};

WEAK symbol<void(::game::steam::CCallbackBase *callback, CSteamID *steamId)>
    LiveSteamLobby_GameLobbyJoinRequested_Handle{0x141EA7120, 0x141EB3BB0, 0x0};

WEAK symbol<const char *(::game::steam::CCallbackBase *callback,
                         uint32_t _unused, const char *pchKey)>
    LiveSteamLobby_GetLobbyData_WithCallback{0x141EA5A30, 0x141EB24C0, 0x0};
WEAK symbol<bool(::game::steam::CCallbackBase *callback, uint32_t _unused,
                 int32_t iLobbyData, const char *pchKey,
                 int32_t cchKeyBufferSize, const char *pchValue,
                 int32_t cchValueBufferSize)>
    LiveSteamLobby_GetLobbyDataByIndex_WithCallback{0x141EA5B50, 0x141EB25E0,
                                                    0x0};
WEAK symbol<int32_t(::game::steam::CCallbackBase *callback, uint32_t _unused)>
    LiveSteamLobby_GetLobbyDataCount_WithCallback{0x141EA5C90, 0x141EB2720,
                                                  0x0};

WEAK symbol<bool(CSteamID *lobbyId)> LiveSteamLobby_GetAndValidateID{
    0x141EA6680, 0x141EB3110, 0x0};
WEAK symbol<bool(XUID joinId)> LiveSteam_Lobby_RequestJoin{0x141EA61D0,
                                                           0x141EB2C60, 0x0};
} // namespace lobby
} // namespace steam
} // namespace live
} // namespace game
