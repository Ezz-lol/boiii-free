#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace ugc {
WEAK symbol<BuiltinWorkshopDataPool> builtinModsPool{0x15678D170, 0x14933EAE0};
extern ExtendedWorkshopDataPool modsPool;

WEAK symbol<BuiltinWorkshopDataPool> builtinUsermapsPool{0x1567B3580,
                                                         0x149364EE8};
extern ExtendedWorkshopDataPool usermapsPool;

WEAK symbol<bool()> UGC_ActiveMod_Loaded{0x1420D5020};
WEAK symbol<const char *()> UGC_ActiveMod_PublisherId{0x1420D7680, 0x1404E3230};
WEAK symbol<void(LocalClientNum_t localClientNum, WorkshopData *mod,
                 bool reloadFS)>
    UGC_LoadMod{0x1420D6B50, 0x1404E29E0};
WEAK symbol<void(LocalClientNum_t localClientNum, const char *publisherId,
                 bool reloadFS)>
    UGC_LoadModByPublisherId{0x1420D6930, 0x1404E27C0};
WEAK symbol<bool(LocalClientNum_t localClientNum)> UGC_ModLoad_TryComplete{
    0x1420D6DE0};

WEAK symbol<WorkshopData> active_usermap{0x1567D99A0, 0x14938B2F0};
WEAK symbol<ActiveMod> active_mod{0x15678CCA0, 0x14933E610};

WEAK symbol<void()> reloadUserContent{0x1420D66C0, 0x1404E25C0};

WEAK symbol<WorkshopData *(ZoneType zoneType, const char *publisherId)>
    UGC_GetByPublisherId{0x1420D6120, 0x1404E21D0};
WEAK symbol<uint32_t(ZoneType zoneType)> UGC_GetCount{0x1420D6360, 0x1404E2290};
WEAK symbol<bool(ZoneType zoneType, const char *publisherId, uint32_t version)>
    UGC_VerifyVersion{0x1420D6380, 0x1404E22B0};

// Function is stubbed on client - it immediately returns 0. On server, it is
// implemented and should work.
WEAK symbol<bool(const char *publisherId)> UGC_DownloadModByPublisherId{
    0x141EB6F40, 0x140476D40};
WEAK symbol<void(bool isUsermap, bool allowMods,
                 steam::PublishedFileId_t publisherId)>
    UGC_LoadManifest{0x1420D50A0, 0x1404E1280};
WEAK symbol<void(bool usermaps, bool mods)> UGC_LoadManifests{0x1420D6840,
                                                              0x1404E26D0};

WEAK symbol<void()> UGC_LoadPools{0x1420D66C0, 0x1404E25C0};
WEAK symbol<void()> UGC_LoadUsermapsPool{0x1420D6790, 0x1404E2690};
WEAK symbol<void()> UGC_LoadModsPool{0x1420D66E0, 0x1404E2640};
WEAK symbol<void(ExtendedWorkshopDataPool *pool, ZoneType zoneType)>
    UGC_LoadPool{0x1420D6430, 0x1404E2360};
WEAK symbol<void(const char *publisherId)> UGC_SetMapPreviewImageByPublisherId{
    0x1420D73A0, 0x1404E2FB0};
WEAK symbol<WorkshopData *(const char *publisherId)>
    UGC_LoadUsermapByPublisherId{0x1420D5700, 0x1404E18B0};
WEAK symbol<void(WorkshopData *usermap)> UGC_SetActiveUsermap{0x1420D5560,
                                                              0x1404E1740};

// Server only.
WEAK symbol<steam::PublishedFileId_t()> UGC_DownloadedModPublisherId{
    0x0, 0x140476DD0};
WEAK symbol<void()> UGC_LoadPlaylistUsermapManifests{0x0, 0x14055CB80};

// Client only.
WEAK symbol<void()> UGC_SetMapLoadingImage{0x1420D71B0};
WEAK symbol<ui::UIModelIndex> ugc_last_created_persistent_model{0x1567D9990};

} // namespace ugc
} // namespace game

