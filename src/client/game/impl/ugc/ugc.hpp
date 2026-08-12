#pragma once
#include <game/game.hpp>

namespace game {
namespace ugc {

WorkshopData *UGC_GetModByPublisherId(const char *publisherId);
WorkshopData *UGC_GetUsermapByPublisherId(const char *publisherId);
WorkshopData *UGC_GetByPublisherId_Impl(ZoneType zoneType,
                                        const char *publisherId);
inline uint32_t UGC_GetModCount() { return modsPool.count; }
inline uint32_t UGC_GetUsermapCount() { return usermapsPool.count; }
uint32_t UGC_GetCount_Impl(ZoneType zoneType);
bool UGC_VerifyModVersion(const char *publisherId, uint32_t version);
bool UGC_VerifyUsermapVersion(const char *publisherId, uint32_t version);
bool UGC_VerifyVersion_Impl(ZoneType zoneType, const char *publisherId,
                            uint32_t version);
UGCHash UGC_Hash(const char *str);
void UGC_LoadPool_Impl(ExtendedWorkshopDataPool *pool, ZoneType zoneType);
void UGC_LoadModsPool_Impl();
void UGC_LoadUsermapsPool_Impl();
void UGC_LoadPools_Impl();
void UGC_LoadModByPublisherId_Impl(LocalClientNum_t localClientNum,
                                   const char *publisherId, bool reloadFS);
void UGC_SetMapPreviewImageByPublisherId_Impl(const char *publisherId);
void UGC_SetMapLoadingImage_Impl();
void UGC_LoadManifest_Impl(bool usermaps, bool mods,
                           steam::PublishedFileId_t publisherId);
WorkshopData *UGC_LoadUsermapByPublisherId_Impl(const char *publisherId);

int32_t UGC_ZoneSourcePath_Impl(const char *name, const char *extension,
                                int32_t size, char *buf, ZoneType zoneType,
                                const char *publisherId);

template <const size_t N>
inline int32_t UGC_ZoneSourcePath_Impl(const char *name, const char *extension,
                                       char (&buf)[N], ZoneType zoneType,
                                       const char *publisherId) {
  return UGC_ZoneSourcePath_Impl(name, extension, N, buf, zoneType,
                                 publisherId);
}
} // namespace ugc
} // namespace game