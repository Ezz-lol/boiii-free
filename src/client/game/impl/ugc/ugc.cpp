#include "../../../std_include.hpp"
#include <cstdint>
#include <cstring>

#include <fstream>
#include <cstdio>
#include <algorithm>

#include "rapidjson/document.h"

#include "ugc.hpp"
#include "../../../steam/steam.hpp"
#include "../../../../common/utils/string.hpp"
#include "../../../component/workshop.hpp"

#include <strlcpy.hpp>

namespace game {
namespace ugc {

WorkshopData *UGC_GetModByPublisherId(const char *publisherId) {
  if (publisherId) {
    for (uint32_t modIdx = 0; modIdx < modsPool.count; modIdx++) {
      if (std::strcmp(modsPool.data[modIdx].publisherId, publisherId) == 0) {
        return &modsPool.data[modIdx];
      }
    }
  }
  return nullptr;
}
WorkshopData *UGC_GetUsermapByPublisherId(const char *publisherId) {
  if (publisherId) {
    for (uint32_t usermapIdx = 0; usermapIdx < usermapsPool.count;
         usermapIdx++) {
      if (std::strcmp(usermapsPool.data[usermapIdx].publisherId, publisherId) ==
          0) {
        return &usermapsPool.data[usermapIdx];
      }
    }
  }
  return nullptr;
}

WorkshopData *UGC_GetByPublisherId_Impl(ZoneType zoneType,
                                        const char *publisherId) {
  switch (zoneType) {
  case ZoneType::MOD:
    return UGC_GetModByPublisherId(publisherId);
  case ZoneType::USERMAP:
    return UGC_GetUsermapByPublisherId(publisherId);
  default:
    return nullptr;
  }
}

uint32_t UGC_GetCount_Impl(ZoneType zoneType) {
  switch (zoneType) {
  case ZoneType::MOD:
    return UGC_GetModCount();
  case ZoneType::USERMAP:
    return UGC_GetUsermapCount();
  default:
    return 0;
  }
}

bool UGC_VerifyModVersion(const char *publisherId, uint32_t version) {
  WorkshopData *mod = UGC_GetModByPublisherId(publisherId);
  if (mod) {
    return mod->version == version;
  }

  return false;
}
bool UGC_VerifyUsermapVersion(const char *publisherId, uint32_t version) {
  WorkshopData *usermap = UGC_GetUsermapByPublisherId(publisherId);
  if (usermap) {
    return usermap->version == version;
  }

  return false;
}

bool UGC_VerifyVersion_Impl(ZoneType zoneType, const char *publisherId,
                            uint32_t version) {
  WorkshopData *ugc = UGC_GetByPublisherId_Impl(zoneType, publisherId);
  if (ugc) {
    return ugc->version == version;
  }

  return false;
}

constexpr UGCHash UGC_HASH_NULLPTR = 0;
constexpr UGCHash UGC_HASH_DJB2_INITIAL_SEED = 0x1505;
constexpr UGCHash UGC_HASH_DJB2_CONSTANT = 33;
UGCHash UGC_Hash(const char *str) {
  if (str) {
    UGCHash hash = UGC_HASH_DJB2_INITIAL_SEED;
    for (const char *c = str; *c; c++) {
      hash =
          static_cast<UGCHash>(std::tolower(static_cast<unsigned char>(*c))) +
          hash * UGC_HASH_DJB2_CONSTANT;
    }
    return hash;
  }
  return UGC_HASH_NULLPTR;
}

// Post-invocation patches used in boiii which were originally applied as hooks,
// carried over to our impl
inline void UGC_LoadPool_Patches(ExtendedWorkshopDataPool *pool,
                                 ZoneType zoneType) {

  if (zoneType == ZoneType::MOD) {
    workshop::supplement_mods_from_disk();
  }

  for (uint32_t i = 0; i < pool->count; i++) {
    game::ugc::WorkshopData *ugc = &pool->data[i];

    if (ugc->internalName[0] &&
        (zoneType == ZoneType::USERMAP ||
         std::strcmp(ugc->internalName, "usermaps") != 0) &&
        std::strcmp(ugc->internalName, ugc->title) == 0) {
      workshop::load_workshop_data(ugc);
    }
  }
}

void UGC_LoadPool_Impl(ExtendedWorkshopDataPool *pool, ZoneType zoneType) {

  const char *cwd = sys::Sys_Cwd();
  const char *ugcContentContainerDirname;
  switch (zoneType) {
  case ZoneType::MOD:
    ugcContentContainerDirname = "mods";
    break;
  case ZoneType::USERMAP:
    ugcContentContainerDirname = "usermaps";
    break;
  default:
    ugcContentContainerDirname = "";
    break;
  }

  int32_t numfiles = 0;
  char ugcContentListContainerDir[256];
  snprintf(ugcContentListContainerDir, sizeof(ugcContentListContainerDir),
           "%s/%s", cwd, ugcContentContainerDirname);
  fs::PathList fileList = sys::Sys_ListFiles(ugcContentListContainerDir, "/",
                                             nullptr, &numfiles, 0);

  for (int32_t fileIdx = 0;
       fileIdx < numfiles && pool->count < EXTENDED_WORKSHOP_DATA_POOL_SIZE &&
       fileList[fileIdx] && fileList[fileIdx][0];
       fileIdx++, pool->count++) {
    fs::Path ugcDirname = fileList[fileIdx];
    WorkshopData *entry = &pool->data[pool->count];

    strlcpy(entry->title, ugcDirname, sizeof(entry->title));
    strlcpy(entry->internalName, ugcDirname, sizeof(entry->internalName));
    strlcpy(entry->publisherId, ugcDirname, sizeof(entry->publisherId));
    strlcpy(entry->absolutePathZoneFiles,
            // ORIGINAL:
            // utils::string::va("%s/%s/zone", ugcContentListContainerDir,
            // ugcDirname)

            // PATCH:
            workshop::va_user_content_path(
                "%s/%s/zone", ugcContentListContainerDir, ugcDirname),
            sizeof(entry->absolutePathZoneFiles));
    strlcpy(entry->contentPathToZoneFiles,
            utils::string::va("%s/%s", "mods", ugcDirname),
            sizeof(entry->contentPathToZoneFiles));
    strlcpy(entry->absolutePathContentDirectory, cwd,
            sizeof(entry->absolutePathContentDirectory));
    entry->publisherIdHash = UGC_Hash(ugcDirname);
    entry->version = 1;
    entry->publisherIdInteger = 0;
    entry->type = zoneType;
  }
  fs::FS_FreePathList(fileList);
  if (game::is_client()) {
    for (ControllerIndex_t controllerIndex = game::CONTROLLER_INDEX_0;
         controllerIndex <= game::CONTROLLER_INDEX_COUNT; controllerIndex++) {

      ui::UI_Model_ForceNotify(pool->listModels[controllerIndex]);
    }
  }

  UGC_LoadPool_Patches(pool, zoneType);
}

void UGC_LoadModsPool_Impl() {

  bool is_client = game::is_client();

  if (is_client) {
    for (game::ControllerIndex_t controllerIndex = game::CONTROLLER_INDEX_0;
         controllerIndex != game::CONTROLLER_INDEX_COUNT;
         controllerIndex = static_cast<game::ControllerIndex_t>(
             static_cast<int32_t>(controllerIndex) + 1)) {
      ui::UIModelIndex modelForController =
          ui::UI_Model_GetModelForController(controllerIndex);
      if (modelForController) {
        modsPool.listModels[controllerIndex] =
            ui::UI_Model_CreatePersistentModelFromPath(modelForController,
                                                       "ModsLoadEntry.update");
      }
    }
  }

  clear(&modsPool);

  if (!is_client) {
    steam::PublishedFileId_t downloadedModPublisherId =
        UGC_DownloadedModPublisherId();
    UGC_LoadManifest_Impl(false, true, downloadedModPublisherId);
  }

  UGC_LoadManifests(false, true);
  UGC_LoadPool_Impl(&modsPool, ZoneType::MOD);

  if (is_client) {
    if (ui::UI_Model_GetGlobalModel()) {
      ui::UIModelIndex globalModel = ui::UI_Model_GetGlobalModel();
      ui::UIModelIndex persistentModel =
          ui::UI_Model_CreatePersistentModelFromPath(globalModel,
                                                     "ModsGlobal.update");
      *ugc_last_created_persistent_model = persistentModel;
      if (persistentModel) {
        ui::UI_Model_ForceNotify(persistentModel);
      }
    }
  }
}

void UGC_LoadUsermapsPool_Impl() {

  bool is_client = game::is_client();

  if (is_client) {
    for (game::ControllerIndex_t controllerIndex = game::CONTROLLER_INDEX_0;
         controllerIndex != game::CONTROLLER_INDEX_COUNT;
         controllerIndex = static_cast<game::ControllerIndex_t>(
             static_cast<int32_t>(controllerIndex) + 1)) {
      ui::UIModelIndex modelForController =
          ui::UI_Model_GetModelForController(controllerIndex);
      if (modelForController) {
        usermapsPool.listModels[controllerIndex] =
            ui::UI_Model_CreatePersistentModelFromPath(
                modelForController, "UsermapsEntries.update");
      }
    }
  }

  clear(&usermapsPool);

  if (!is_client) {
    UGC_LoadPlaylistUsermapManifests();
  }

  UGC_LoadManifests(true, false);
  UGC_LoadPool_Impl(&usermapsPool, ZoneType::USERMAP);

  if (is_client) {
    if (ui::UI_Model_GetGlobalModel()) {
      ui::UIModelIndex globalModel = ui::UI_Model_GetGlobalModel();
      ui::UIModelIndex persistentModel =
          ui::UI_Model_CreatePersistentModelFromPath(globalModel,
                                                     "ModsGlobal.update");
      *ugc_last_created_persistent_model = persistentModel;
      if (persistentModel) {
        ui::UI_Model_ForceNotify(persistentModel);
      }
    }
  }
}

void UGC_LoadPools_Impl() {
  UGC_LoadUsermapsPool_Impl();
  UGC_LoadModsPool_Impl();
}

void UGC_LoadModByPublisherId_Impl(LocalClientNum_t localClientNum,
                                   const char *publisherId, bool reloadFS) {
  UGC_LoadPools_Impl();
  WorkshopData genMod{};
  WorkshopData *mod = UGC_GetModByPublisherId(publisherId);
  if (mod == nullptr) {
    if (UGC_DownloadModByPublisherId(publisherId)) {
      return;
    }

    strlcpy(genMod.title, publisherId, sizeof(genMod.title));
    strlcpy(genMod.internalName, publisherId, sizeof(genMod.internalName));
    strlcpy(genMod.publisherId, publisherId, sizeof(genMod.publisherId));
    snprintf(genMod.contentPathToZoneFiles,
             sizeof(genMod.contentPathToZoneFiles), "%s/%s", "mods",
             publisherId);
    strlcpy(genMod.absolutePathContentDirectory, sys::Sys_Cwd(),
            sizeof(genMod.absolutePathContentDirectory));
    strlcpy(genMod.absolutePathZoneFiles,
            // ORIGINAL:
            // utils::string::va("%s/%s/%s/zone", sys::Sys_Cwd(), "mods",
            // publisherId),

            // PATCH:
            workshop::va_mods_path("%s/%s/%s/zone", sys::Sys_Cwd(), "mods",
                                   publisherId),
            sizeof(genMod.absolutePathZoneFiles));
    genMod.version = 1;
    genMod.publisherIdHash = UGC_Hash(publisherId);
    genMod.type = ZoneType::MOD;
    mod = &genMod;
  }
  UGC_LoadMod(localClientNum, mod, reloadFS);
}

void UGC_SetMapPreviewImageByPublisherId_Impl(const char *publisherId) {

  gfx::GfxImage *previewImage =
      db::xasset::DB_FindXAssetHeader(db::xasset::XAssetType::IMAGE,
                                      "img_t7_mod_preview", 1, -1)
          .image;
  if (previewImage) {
    char pathBuf[272];
    WorkshopData *usermap = UGC_GetUsermapByPublisherId(publisherId);
    if (usermap && usermap->absolutePathZoneFiles[0]) {
      snprintf(pathBuf, sizeof(pathBuf), "%s/%s/%s%s",
               usermap->absolutePathZoneFiles, "", "previewimage", ".png");
    } else {
      snprintf(pathBuf, sizeof(pathBuf), "%s/%s/%s/%s/%s%s", sys::Sys_Cwd(),
               "usermaps", publisherId, "", "previewimage", ".png");
    }
    gfx::GfxTexture texture = gfx::Gfx_LoadTextureFromPng(pathBuf);

    bool is_client = game::is_client();
    if (is_client) {
      gfx::Gfx_TexturePool_ReleaseRef(previewImage->texture, 0);
    }
    previewImage->texture.basemap->lpVtbl->Release(
        previewImage->texture.basemap);
    if (texture.basemap == nullptr) {
      texture = (*gfx::loadedGfxImage.get())->texture;
      if (is_client) {
        gfx::Gfx_TexturePool_AddRef(texture, 0);
      }
      texture.basemap->lpVtbl->AddRef(texture.basemap);
    }
    previewImage->texture = texture;
  }
}

void UGC_SetMapLoadingImage_Impl() {

  gfx::GfxImage *loadingImage =
      db::xasset::DB_FindXAssetHeader(db::xasset::XAssetType::IMAGE,
                                      "img_t7_mod_loading", 1, -1)
          .image;
  if (loadingImage && active_usermap->publisherId[0]) {
    char pathBuf[272];
    WorkshopData *usermap =
        UGC_GetUsermapByPublisherId(active_usermap->publisherId);
    if (usermap && usermap->absolutePathZoneFiles[0]) {
      snprintf(pathBuf, sizeof(pathBuf), "%s/%s/%s%s",
               usermap->absolutePathZoneFiles, "", "loadingimage", ".png");
    } else {
      snprintf(pathBuf, sizeof(pathBuf), "%s/%s/%s/%s/%s%s", sys::Sys_Cwd(),
               "usermaps", active_usermap->publisherId, "", "loadingimage",
               ".png");
    }
    gfx::GfxTexture texture = gfx::Gfx_LoadTextureFromPng(pathBuf);
    gfx::Gfx_TexturePool_ReleaseRef(loadingImage->texture, 0);
    loadingImage->texture.basemap->lpVtbl->Release(
        loadingImage->texture.basemap);
    if (texture.basemap == nullptr) {
      texture = (*gfx::loadedGfxImage.get())->texture;
      gfx::Gfx_TexturePool_AddRef(texture, 0);
      texture.basemap->lpVtbl->AddRef(texture.basemap);
    }
    loadingImage->texture = texture;
  }
}

inline void *GetPrimaryHSteamPipe() {
  if (game::is_server()) {
    return ::steam::SteamGameServer_GetHSteamPipe();
  }
  return ::steam::SteamAPI_GetHSteamPipe();
}

class ModsUGCDetailsCallback : steam::CCallbackBase {
public:
  WorkshopData *result;
  // Incomplete
};

class ModsUGCDetailsCallbackResult
    : steam::CCallResult<ModsUGCDetailsCallback,
                         steam::SteamUGCRequestUGCDetailsResult_t> {
public:
  void Set(steam::SteamAPICall_t hApiCall, WorkshopData *result) {
    auto setImpl = reinterpret_cast<void __thiscall (*)(
        ModsUGCDetailsCallbackResult *, steam::SteamAPICall_t hApiCall,
        WorkshopData *result)>(game::select(0x1420D4FC0, 0x1404e11a0));
    setImpl(this, hApiCall, result);
  };
  uint8_t _unknown[8];
};
ASSERT_SIZE(ModsUGCDetailsCallbackResult, 0x30);

void UGC_LoadManifest_Impl(bool usermaps, bool mods,
                           steam::PublishedFileId_t publisherId) {
  if (publisherId && GetPrimaryHSteamPipe()) {

    EngineDependent<steam::cl::SteamInterfaces *, steam::sv::SteamInterfaces *>
        g_steamInterfaces = steam::PrimarySteamInterfaces();
    std::visit(
        [publisherId, usermaps, mods](auto *g_steamInterfaces) {
          steam::ISteamUGC *pSteamUGC = g_steamInterfaces->pSteamUGC;
          if (pSteamUGC == nullptr) {
            InitPrimarySteamInterfaces(g_steamInterfaces);
            pSteamUGC = g_steamInterfaces->pSteamUGC;
            if (pSteamUGC == nullptr) {
              return;
            }
          }

          uint32_t itemState = pSteamUGC->GetItemState(publisherId);
          bool isInstalled = (itemState & steam::k_EItemStateInstalled) != 0;
          bool isReady =
              (itemState & (steam::k_EItemStateNeedsUpdate |
                            steam::k_EItemStateDownloading |
                            steam::k_EItemStateDownloadPending)) == 0;

          if (isInstalled && isReady) {
            char dirPath[260] = {0};
            uint64_t sizeOnDisk = 0;
            uint32_t punTimeStamp = 0;

            if (pSteamUGC->GetItemInstallInfo(publisherId, &sizeOnDisk, dirPath,
                                              sizeof(dirPath), &punTimeStamp)) {
              char jsonPath[260];
              snprintf(jsonPath, sizeof(jsonPath), "%s\\workshop.json",
                       dirPath);

              std::ifstream jsonFile(jsonPath,
                                     std::ios::binary | std::ios::ate);
              if (!jsonFile.is_open()) {
                return;
              }

              std::string jsonContent(
                  (std::istreambuf_iterator<char>(jsonFile)),
                  std::istreambuf_iterator<char>());

              rapidjson::Document doc;
              if (doc.Parse(jsonContent.c_str()).HasParseError()) {
                return;
              }

              const char *typeString =
                  (doc.HasMember("Type") && doc["Type"].IsString())
                      ? doc["Type"].GetString()
                      : "";
              ExtendedWorkshopDataPool *targetPool = nullptr;
              ZoneType zoneType;

              if (strcmp(typeString, "map") == 0) {
                if (usermaps) {
                  targetPool = &usermapsPool;
                  zoneType = ZoneType::USERMAP;
                }
              } else if (strcmp(typeString, "mod") == 0) {
                if (mods) {
                  targetPool = &modsPool;
                  zoneType = ZoneType::MOD;
                }
              }

              if (targetPool &&
                  targetPool->count < EXTENDED_WORKSHOP_DATA_POOL_SIZE) {
                WorkshopData *newUgcEntry =
                    &targetPool->data[targetPool->count];
                targetPool->count++;

                if (doc.HasMember("Title") && doc["Title"].IsString()) {
                  strlcpy(newUgcEntry->title, doc["Title"].GetString(),
                          sizeof(newUgcEntry->title));
                }

                if (doc.HasMember("FolderName") &&
                    doc["FolderName"].IsString()) {
                  strlcpy(newUgcEntry->internalName,
                          doc["FolderName"].GetString(),
                          sizeof(newUgcEntry->internalName));
                }

                if (doc.HasMember("Description") &&
                    doc["Description"].IsString()) {
                  strlcpy(newUgcEntry->description,
                          doc["Description"].GetString(),
                          sizeof(newUgcEntry->description));
                }

                snprintf(newUgcEntry->publisherId,
                         sizeof(newUgcEntry->publisherId), "%llu",
                         sizeof(newUgcEntry->publisherId));

                strlcpy(newUgcEntry->absolutePathZoneFiles, dirPath,
                        sizeof(newUgcEntry->absolutePathZoneFiles));

                const char *appIdPos = strstr(dirPath, APP_ID_STR);
                if (appIdPos) {
                  size_t baseLen = appIdPos - dirPath - 1;
                  size_t maxContentLen =
                      sizeof(newUgcEntry->absolutePathContentDirectory);
                  size_t copyLen = (std::min)(baseLen, maxContentLen);

                  strlcpy(newUgcEntry->absolutePathContentDirectory, dirPath,
                          copyLen);

                  strlcpy(newUgcEntry->contentPathToZoneFiles, appIdPos,
                          sizeof(newUgcEntry->contentPathToZoneFiles));
                }

                newUgcEntry->publisherIdHash =
                    UGC_Hash(newUgcEntry->publisherId);
                newUgcEntry->version = 1;
                newUgcEntry->publisherIdInteger = publisherId;
                newUgcEntry->type = zoneType;

                steam::SteamAPICall_t steamApiCall =
                    pSteamUGC->RequestUGCDetails(publisherId, 60);
                ModsUGCDetailsCallbackResult *callbackResult =
                    new ModsUGCDetailsCallbackResult();

                if (callbackResult) {
                  callbackResult->Set(steamApiCall, newUgcEntry);
                }
              }
            }
          }
        },
        g_steamInterfaces);
  }
}

WorkshopData *UGC_LoadUsermapByPublisherId_Impl(const char *publisherId) {

  WorkshopData *usermap = UGC_GetUsermapByPublisherId(publisherId);
  UGC_SetActiveUsermap(usermap);
  return usermap;
}
} // namespace ugc
} // namespace game