#pragma once

#include "game/symbols/sym_include.hpp"

namespace game {
namespace lobby {
namespace types {

WEAK symbol<eNetworkModes(LobbyNetworkMode networkMode)>
    LobbyType_NetworkModeToENetworkMode{0x141EDF1E0, 0x141EEBB50, 0x14049CDC0};
WEAK symbol<const char *(clientplatform_t clientPlatform)>
    LobbyTypes_GetClientPlatformName{0x141EDF230, 0x141EEBBA0, 0x14049CE10};
WEAK symbol<const char *(LobbyMainMode lobbyMainMode)>
    LobbyTypes_GetLobbyMainModeName{0x141EDF290, 0x141EEBC00, 0x14049CE40};
WEAK symbol<const char *(LobbyMode lobbyMode)> LobbyTypes_GetLobbyModeName{
    0x141EDF2C0, 0x0, 0x0};
WEAK symbol<const char *(LobbyModule lobbyModule)>
    LobbyTypes_GetLobbyModuleName{0x141EDF2E0, 0x141EEBC50, 0x0};
WEAK symbol<const char *(LobbyNetworkMode lobbyNetworkMode)>
    LobbyTypes_GetLobbyNetworkModeName{0x141EDF300, 0x141EEBC70, 0x0};
WEAK symbol<const char *(LobbyType lobbyType)> LobbyTypes_GetLobbyTypeName{
    0x141EDF320, 0x141EEBC90, 0x14049CED0};
WEAK symbol<LobbyMainMode()> LobbyTypes_GetMainMode{0x141EDF340, 0x141EEBCB0,
                                                    0x0};
WEAK symbol<const char *(msg::MsgType msgType)> LobbyTypes_GetMsgTypeName{
    0x141EDF3A0, 0x141EEBD10, 0x0};
WEAK symbol<int64_t()> LobbyTypes_Init{0x141EDF3F0, 0x141EEBD60, 0x0};
WEAK symbol<eModes(LobbyMainMode mainMode)> LobbyTypes_MainModeToESessionMode{
    0x141EDF440, 0x141EEBDB0, 0x14049CFF0};

} // namespace types
} // namespace lobby
} // namespace game
