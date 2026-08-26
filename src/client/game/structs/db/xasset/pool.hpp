#pragma once

#include <game/structs/db/xasset/core.hpp>
#include <game/structs/db/xasset/world.hpp>
#include <game/structs/db/xasset/font.hpp>
#include <game/structs/db/xasset/ttf.hpp>
#include <game/structs/bg/cache.hpp>
#include <game/structs/cm.hpp>
namespace game {

namespace ddl {
struct DDLDef;
}

namespace vehicle {
struct VehicleDef;
struct VehicleFxDef;
struct VehicleSoundDef;
} // namespace vehicle

namespace db {
namespace xasset {
namespace pool {
union XAssetEntryPoolEntry;
union XAssetEntryPoolEntry {
  XAssetEntry entry;
  XAssetEntryPoolEntry *next;
};
ASSERT_SIZE(XAssetEntryPoolEntry, 0x20);
constexpr std::size_t XASSET_ENTRY_POOL_LENGTH = 0x263ff;

#pragma pack(push, 1)
struct XAssetEntryPool {
  XAssetEntryPoolEntry pool[XASSET_ENTRY_POOL_LENGTH];
};

#pragma pack(pop)

#pragma pack(push, 1)
struct TypedXAssetPools {
  XAssetPool physpreset;
  XAssetPool physconstraints;
  XAssetPool destructibledef;
  XAssetPool xanimparts;
  XAssetPool xmodel;
  TypedXAssetPool<XModelMesh> xmodelmesh;
  XAssetPool material;
  XAssetPool compute_shader_set;
  XAssetPool technique_set;
  TypedXAssetPool<gfx::GfxImage> image;
  TypedXAssetPool<snd::SndBank> sound;
  TypedXAssetPool<snd::SndPatch> sound_patch;
  TypedXAssetPool<cm::clipMap_t> clipmap;
  TypedXAssetPool<world::ComWorld> comworld;
  TypedXAssetPool<world::GameWorld> gameworld;
  XAssetPool map_ents;
  TypedXAssetPool<world::GfxWorld> gfxworld;
  XAssetPool light_def;
  XAssetPool lensflare_def;
  XAssetPool ui_map;
  TypedXAssetPool<font::Font> font;
  TypedXAssetPool<font::FontIcon> fonticon;
  TypedXAssetPool<LocalizeEntry> localize_entry;
  XAssetPool weapon;
  XAssetPool weapondef;
  XAssetPool weapon_variant;
  XAssetPool weapon_full;
  XAssetPool cgmedia;
  XAssetPool playersounds;
  XAssetPool playerfx;
  XAssetPool sharedweaponsounds;
  XAssetPool attachment;
  XAssetPool attachment_unique;
  XAssetPool weapon_camo;
  TypedXAssetPool<CustomizationTable> customization_table;
  TypedXAssetPool<CustomizationTable_FEImages> customization_table_fe_images;
  TypedXAssetPool<CustomizationColorInfo> customization_table_color;
  TypedXAssetPool<snd::SndDriverGlobals> snddriver_globals;
  XAssetPool fx;
  XAssetPool tagfx;
  XAssetPool new_lensflare_def;
  XAssetPool impact_fx;
  XAssetPool impact_sound;
  XAssetPool player_character;
  XAssetPool aitype;
  XAssetPool character;
  XAssetPool xmodelalias;
  TypedXAssetPool<RawFile> rawfile;
  XAssetPool stringtable;
  XAssetPool structured_table;
  XAssetPool leaderboard;
  TypedXAssetPool<ddl::DDLDef> ddl;
  XAssetPool glasses;
  XAssetPool texturelist;
  TypedXAssetPool<scr::ScriptParseTree> scriptparsetree;
  XAssetPool keyvaluepairs;
  TypedXAssetPool<vehicle::VehicleDef> vehicledef;
  XAssetPool addon_map_ents;
  XAssetPool tracer;
  XAssetPool slug;
  XAssetPool surfacefx_table;
  XAssetPool surfacesounddef;
  XAssetPool footstep_table;
  XAssetPool entityfximpacts;
  XAssetPool entitysoundimpacts;
  XAssetPool zbarrier;
  TypedXAssetPool<vehicle::VehicleFxDef> vehiclefxdef;
  TypedXAssetPool<vehicle::VehicleSoundDef> vehiclesounddef;
  XAssetPool typeinfo;
  XAssetPool scriptbundle;
  XAssetPool scriptbundlelist;
  XAssetPool rumble;
  XAssetPool bulletpenetration;
  XAssetPool locdmgtable;
  XAssetPool aimtable;
  XAssetPool animselectortableset;
  XAssetPool animmappingtable;
  XAssetPool animstatemachine;
  XAssetPool behaviortree;
  XAssetPool behaviorstatemachine;
  TypedXAssetPool<ttf::TTFDef> ttf;
  XAssetPool sanim;
  XAssetPool light_description;
  XAssetPool shellshock;
  XAssetPool xcam;
  TypedXAssetPool<game::bg::cache::BGCacheInfo> bg_cache;
  XAssetPool texture_combo;
  XAssetPool flametable;
  XAssetPool bitfield;
  XAssetPool attachment_cosmetic_variant;
  TypedXAssetPool<maptable::MapTable> maptable;
  XAssetPool maptable_loading_images;
  XAssetPool medal;
  XAssetPool medaltable;
  XAssetPool objective;
  XAssetPool objective_list;
  XAssetPool umbra_tome;
  TypedXAssetPool<gfx::GfxUmbraTome> navmesh;
  XAssetPool navvolume;
  XAssetPool binaryhtml;
  XAssetPool laser;
  XAssetPool beam;
  XAssetPool streamer_hint;
};

#pragma pack(pop)

#pragma pack(push, 1)

union XAssetPools {
  XAssetPool pools[+XAssetType::COUNT];
  TypedXAssetPools typed;

  inline bool contains(uintptr_t ptr) const noexcept {
    const uintptr_t this_ptr = reinterpret_cast<uintptr_t>(this);
    return ptr >= this_ptr && ptr < (this_ptr + sizeof(XAssetPools));
  }

  template <typename P> inline bool contains(const P *ptr) const noexcept {
    return contains(reinterpret_cast<uintptr_t>(ptr));
  }
};
static_assert(sizeof(XAssetPools) ==
                  sizeof(XAssetPool) * static_cast<int>(XAssetType::COUNT),
              "sizeof(XAssetPools) must be sizeof(XAssetPool) * COUNT");
ASSERT_SIZE(XAssetPools, sizeof(TypedXAssetPools));
#pragma pack(pop)
} // namespace pool
} // namespace xasset
} // namespace db
} // namespace game