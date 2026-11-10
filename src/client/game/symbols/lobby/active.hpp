#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace active {
WEAK symbol<void(ActiveClient *activeClient,
                 const MutableClientInfo *mutableClientInfo)>
    LobbyActiveList_SetClientInfo{0x141EF2160, 0x0};
WEAK symbol<void(ActiveClient *activeClient, ContentFlags dlcBits)>
    LobbyActiveList_SetDLCBits{0x141EF23D0, 0x0};
WEAK symbol<ContentFlags(ActiveClient *activeClient)>
    LobbyActiveList_GetDLCBits{0x141EF10D0, 0x0};
WEAK symbol<bool(ActiveClient *activeClient)> LobbyActiveList_IsStarterPack{0x1420FAFC0, 0x0};
WEAK symbol<clientcontent::ClientContent *(ActiveClient *activeClient)>
    LobbyActiveList_GetClientContent{0x141EF0C40, 0x1404AAE80};
WEAK symbol<bool(ActiveClient *activeClient,
                 clientcontent::ClientContentType contentType, XUID xuid)>
    LobbyActiveList_ClearClientContentForClient{0x141EF0A40, 0x1404AAD00};
WEAK symbol<clientcontent::ClientContentTracking *(
    ActiveClient *activeClient, clientcontent::ClientContentType contentType,
    uint32_t trackingIndex)>
    LobbyActiveList_GetClientContentTrackingByIndex{0x141EF0DE0, 0x1404AB020};
WEAK symbol<clientcontent::ClientContentTracking *(
    ActiveClient *activeClient, clientcontent::ClientContentType contentType)>
    LobbyActiveList_GetClientContentTracking{0x141EF0D50, 0x1404AAF90};
} // namespace active
} // namespace lobby
} // namespace game
