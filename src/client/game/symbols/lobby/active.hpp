#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace active {
WEAK symbol<void(ActiveClient *activeClient,
                 const MutableClientInfo *mutableClientInfo)>
    LobbyActiveList_SetClientInfo{0x141EFE8E0};
WEAK symbol<void(ActiveClient *activeClient, ContentFlags dlcBits)>
    LobbyActiveList_SetDLCBits{0x141EFEB50};
WEAK symbol<ContentFlags(ActiveClient *activeClient)>
    LobbyActiveList_GetDLCBits{0x141EFD850};
WEAK symbol<bool(ActiveClient *activeClient)> LobbyActiveList_IsStarterPack{
    0x142153A80};
} // namespace active
} // namespace lobby
} // namespace game