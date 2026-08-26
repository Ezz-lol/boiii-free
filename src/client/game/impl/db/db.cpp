#include <std_include.hpp>

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "db.hpp"

namespace game {
namespace db {
namespace xasset {
int32_t DB_GetXAssetTypeSize_Impl(XAssetType type) {

  switch (type) {
  case XAssetType::PHYSPRESET:
    return 0x78;
  case XAssetType::PHYSCONSTRAINTS:
    return 0x690;
  case XAssetType::DESTRUCTIBLEDEF:
    return 0x30;
  case XAssetType::XANIMPARTS:
    return 0xF8;
  case XAssetType::XMODEL:
    return 0x188;
  case XAssetType::XMODELMESH:
    return sizeof(XModelMesh);
  case XAssetType::MATERIAL:
    return 0x2A0;
  case XAssetType::COMPUTE_SHADER_SET:
    return 0x18;
  case XAssetType::TECHNIQUE_SET:
    return 0x70;
  case XAssetType::IMAGE:
    return sizeof(gfx::GfxImage);
  case XAssetType::SOUND:
    return sizeof(snd::SndBank);
  case XAssetType::SOUND_PATCH:
    return sizeof(snd::SndPatch);
  case XAssetType::CLIPMAP:
    return sizeof(cm::clipMap_t);
  case XAssetType::COMWORLD:
    return sizeof(world::ComWorld);
  case XAssetType::GAMEWORLD:
    return sizeof(world::GameWorld);
  case XAssetType::MAP_ENTS:
    return 0x48;
  case XAssetType::GFXWORLD:
    return sizeof(world::GfxWorld);
  case XAssetType::LIGHT_DEF:
    return 0x28;
  case XAssetType::LENSFLARE_DEF:
    return 0x218;
  case XAssetType::UI_MAP:
    // Unknown. Pool entry is never populated, and asset type is not handled by
    // DB.
    return -1;
  case XAssetType::FONT:
    return sizeof(font::Font);
  case XAssetType::FONTICON:
    return sizeof(font::FontIcon);
  case XAssetType::LOCALIZE_ENTRY:
    return sizeof(LocalizeEntry);
  case XAssetType::WEAPON:
    return 0x300;
  case XAssetType::WEAPONDEF:
    // Unknown. Pool entry is never populated, and asset type is not handled by
    // DB.
    return -1;
  case XAssetType::WEAPON_VARIANT:
    // Unknown. Pool entry is never populated, and asset type is not handled by
    // DB.
    return -1;
  case XAssetType::WEAPON_FULL:
    // Unknown. Pool entry is never populated, and asset type is not handled by
    // DB.
    return -1;
  case XAssetType::CGMEDIA:
    return 0x420;
  case XAssetType::PLAYERSOUNDS:
    return 0x448;
  case XAssetType::PLAYERFX:
    return 0x98;
  case XAssetType::SHAREDWEAPONSOUNDS:
    return 0xF0;
  case XAssetType::ATTACHMENT:
    return 0x220;
  case XAssetType::ATTACHMENT_UNIQUE:
    return 0x840;
  case XAssetType::WEAPON_CAMO:
    return 0x18;
  case XAssetType::CUSTOMIZATION_TABLE:
    return sizeof(CustomizationTable);
  case XAssetType::CUSTOMIZATION_TABLE_FE_IMAGES:
    return sizeof(CustomizationTable_FEImages);
  case XAssetType::CUSTOMIZATION_TABLE_COLOR:
    return sizeof(CustomizationColorInfo);
  case XAssetType::SNDDRIVER_GLOBALS:
    return sizeof(snd::SndDriverGlobals);
  case XAssetType::FX:
    return 0x90;
  case XAssetType::TAGFX:
    return 0x18;
  case XAssetType::NEW_LENSFLARE_DEF:
    return 0xA8;
  case XAssetType::IMPACT_FX:
    return 0x18;
  case XAssetType::IMPACT_SOUND:
    return 0x20;
  case XAssetType::PLAYER_CHARACTER:
    return 0x90;
  case XAssetType::AITYPE:
    return 0x930;
  case XAssetType::CHARACTER:
    return 0x270;
  case XAssetType::XMODELALIAS:
    return 0x198;
  case XAssetType::RAWFILE:
    return sizeof(RawFile);
  case XAssetType::STRINGTABLE:
    return 0x20;
  case XAssetType::STRUCTURED_TABLE:
    return 0x38;
  case XAssetType::LEADERBOARD:
    return 0x30;
  case XAssetType::DDL:
    return sizeof(ddl::DDLDef);
  case XAssetType::GLASSES:
    return 0x58;
  case XAssetType::TEXTURELIST:
    return 0x48;
  case XAssetType::SCRIPTPARSETREE:
    return sizeof(scr::ScriptParseTree);
  case XAssetType::KEYVALUEPAIRS:
    return 0x18;
  case XAssetType::VEHICLEDEF:
    return sizeof(vehicle::VehicleDef);
  case XAssetType::ADDON_MAP_ENTS:
    return 0x68;
  case XAssetType::TRACER:
    return 0x98;
  case XAssetType::SLUG:
    return 0x18;
  case XAssetType::SURFACEFX_TABLE:
    return 0x148;
  case XAssetType::SURFACESOUNDDEF:
    return 0xA8;
  case XAssetType::FOOTSTEP_TABLE:
    return 0x40;
  case XAssetType::ENTITYFXIMPACTS:
    return 0x218;
  case XAssetType::ENTITYSOUNDIMPACTS:
    return 0x110;
  case XAssetType::ZBARRIER:
    return 0x300;
  case XAssetType::VEHICLEFXDEF:
    return sizeof(vehicle::VehicleFxDef);
  case XAssetType::VEHICLESOUNDDEF:
    return sizeof(vehicle::VehicleSoundDef);
  case XAssetType::TYPEINFO:
    return 0x2010;
  case XAssetType::SCRIPTBUNDLE:
    return 0x30;
  case XAssetType::SCRIPTBUNDLELIST:
    return 0x20;
  case XAssetType::RUMBLE:
    return 0x40;
  case XAssetType::BULLETPENETRATION:
    return 0x1E8;
  case XAssetType::LOCDMGTABLE:
    return 0x60;
  case XAssetType::AIMTABLE:
    return 0x330;
  case XAssetType::ANIMSELECTORTABLESET:
    return 0x18;
  case XAssetType::ANIMMAPPINGTABLE:
    return 0x18;
  case XAssetType::ANIMSTATEMACHINE:
    return 0x38;
  case XAssetType::BEHAVIORTREE:
    return 0x28;
  case XAssetType::BEHAVIORSTATEMACHINE:
    return 0x20;
  case XAssetType::TTF:
    return sizeof(ttf::TTFDef);
  case XAssetType::SANIM:
    return 0x70;
  case XAssetType::LIGHT_DESCRIPTION:
    return 0x240;
  case XAssetType::SHELLSHOCK:
    return 0xB0;
  case XAssetType::XCAM:
    return 0xA8;
  case XAssetType::BG_CACHE:
    return sizeof(bg::cache::BGCacheInfo);
  case XAssetType::TEXTURE_COMBO:
    return 0x80;
  case XAssetType::FLAMETABLE:
    return 0x218;
  case XAssetType::BITFIELD:
    return 0x20;
  case XAssetType::ATTACHMENT_COSMETIC_VARIANT:
    return 0x18;
  case XAssetType::MAPTABLE:
    return 0x28;
  case XAssetType::MAPTABLE_LOADING_IMAGES:
    return 0x18;
  case XAssetType::MEDAL:
    return 0x30;
  case XAssetType::MEDALTABLE:
    return 0x18;
  case XAssetType::OBJECTIVE:
    return 0x70;
  case XAssetType::OBJECTIVE_LIST:
    return 0x18;
  case XAssetType::UMBRA_TOME:
    // Unverified. Pool entry is never populated, and asset type is not handled
    // by DB.
    // Currently also unverified whether this is a `GfxUmbraTome`,
    // `ComUmbraTome`, or some other data structure.
    return sizeof(gfx::GfxUmbraTome);
  case XAssetType::NAVMESH:
    return 0x68;
  case XAssetType::NAVVOLUME:
    return 0x48;
  case XAssetType::BINARYHTML:
    return 0x18;
  case XAssetType::LASER:
    return 0x70;
  case XAssetType::BEAM:
    return 0x2B0;
  case XAssetType::STREAMER_HINT:
    return 0x28;
  default:
    return -1;
  }
}
void reallocate_asset_pool(const XAssetType type, const uint32_t new_size) {
  const int32_t entry_size = DB_GetXAssetTypeSize_Impl(type);
  if (entry_size <= 0) {
    return;
  }
  volatile XAssetPool *pool = &pool::s_assetPools->pools[+type];

  // Skip if pool already meets or exceeds requested size
  if (!pool->isSingleton && pool->itemCount < static_cast<int32_t>(new_size)) {
    fprintf(stdout, "Reallocating asset pool type %d: %d -> %u entries\n",
            +type, pool->itemCount, new_size);
    fflush(stdout);
    void *new_pool = calloc(new_size, entry_size);
    if (new_pool == nullptr) {
      fprintf(stderr, "Failed to allocate asset pool for type %d (size: %u)\n",
              static_cast<int32_t>(type), new_size);
      fflush(stderr);
      return;
    }

    if (pool->pool != nullptr && pool->itemAllocCount > 0) {
      // Copy existing entries
      memcpy(new_pool, pool->pool, pool->itemAllocCount * entry_size);
    }

    // Rebuild free list for new entries
    pool->freeHead = reinterpret_cast<AssetLink *>(
        reinterpret_cast<char *>(new_pool) +
        static_cast<size_t>(entry_size) * pool->itemAllocCount);

    for (int32_t i = pool->itemAllocCount;
         i < static_cast<int32_t>(new_size) - 1; i++) {
      AssetLink *current =
          reinterpret_cast<AssetLink *>(reinterpret_cast<char *>(new_pool) +
                                        static_cast<size_t>(entry_size) * i);
      current->next = reinterpret_cast<AssetLink *>(
          reinterpret_cast<char *>(new_pool) +
          static_cast<size_t>(entry_size) * (i + 1));
    }

    // Last entry points to null
    AssetLink *last = reinterpret_cast<AssetLink *>(
        reinterpret_cast<char *>(new_pool) + entry_size * (new_size - 1));
    last->next = nullptr;

    fprintf(stdout, "Reallocated asset pool type %d: %d -> %u entries\n", +type,
            pool->itemCount, new_size);
    fflush(stdout);

    pool->pool = new_pool;
    pool->itemSize = entry_size;
    pool->itemCount = new_size;
  }
}

// The engine always inlines this function, so we reimplement it here for use
// elsewhere.
__inline_def uint32_t DB_HashForName(const char *name, const XAssetType type) {
  uint32_t hash = static_cast<uint32_t>(type);
  while (*name) {
    char c = *name++;
    if (c == '\\') {
      c = '/';
    }
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    hash = (hash << 16) + (hash << 6) + c - hash;
  }
  return hash;
}

pool::XAssetEntryPoolEntry *
DB_GetAssetEntryPoolEntryByName(const char *name, const XAssetType type) {
  uint32_t hash = DB_HashForName(name, type);
  uint32_t index = hash % pool::XASSET_ENTRY_POOL_LENGTH;
  pool::XAssetEntryPoolEntry *entry =
      reinterpret_cast<pool::XAssetEntryPoolEntry *>(
          &pool::g_assetEntryPool->pool[index]);

  // Find match in collision list
  for (; entry != nullptr &&
         (entry->entry.asset.type != type ||
          strcmp(entry->entry.asset.header.named->name, name) != 0);
       entry = entry->next) {
  }

  return entry;
}

void DB_InitBSPGlobals_Impl() {
  *cm::cm = pool::s_assetPools->typed.clipmap.pool;
  *world::s_world = pool::s_assetPools->typed.gfxworld.pool;
  *world::comWorld = pool::s_assetPools->typed.comworld.pool;
  *world::gameWorld = pool::s_assetPools->typed.gameworld.pool;
  *world::gameWorldCurrent = *world::gameWorld;
}
} // namespace xasset
} // namespace db
} // namespace game