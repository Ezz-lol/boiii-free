#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

namespace game {
namespace db {

namespace load {

WEAK symbol<xasset::XAssetEntryPtr[15360]> g_copyInfo{0x0, 0x14699D260};
WEAK symbol<uint32_t> g_copyInfoCount{0x0, 0x146902028};
WEAK symbol<void(xasset::XAssetEntry *newEntry)> DB_DelayedCopyXAsset{
    0x14141FF00, 0x1401D4FE0};

WEAK symbol<DB_LoadData> g_load{0x14940C3B0, 0x1468FD4A0};
WEAK symbol<void(xzone::XZoneInfo *zoneInfo, uint32_t zoneCount, bool sync,
                 bool suppressSync)>
    DB_LoadXAssets{0x1414236A0, 0x1401D8740};
WEAK symbol<xasset::XAssetEntry *(xasset::XAssetEntry *entry,
                                  qboolean allowOverride)>
    DB_LinkXAssetEntry{0x1414228C0, 0x1401D7960};

WEAK symbol<bool(const char *path, DBFile f, xzone::XZoneBuffer *fileBuffer,
                 const char *filename, XBlock *blocks, DB_Interrupt *interrupt,
                 uint8_t *buf, PMemStack side, int flags)>
    DB_LoadXFile{0x1413EF6D0, 0x1401A4920};
WEAK symbol<void(int zoneIndex, bool createDefault, qboolean suppressSync)>
    DB_UnloadXZone{0x141425A70, 0x1401DA6C0};
WEAK symbol<void()> DB_ReleaseXAssets{0x1414247C0, 0x1401D9870};
} // namespace load
} // namespace db
} // namespace game
#endif