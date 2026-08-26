#include <std_include.hpp>
#include <loader/component_loader.hpp>
#include <game/game.hpp>
#include <game/impl/db/db.hpp>

#include <utils/hook.hpp>
#include <utils/io.hpp>

#include <rapidjson/document.h>

using namespace game::db::xasset;

namespace asset_limits {
namespace {
struct pool_config {
  XAssetType type;
  const char *setting_key;
  uint32_t default_size;
};

inline constexpr pool_config pool_configs[] = {
    {XAssetType::PHYSPRESET, "ap_physpreset", 0x113},
    {XAssetType::PHYSCONSTRAINTS, "ap_physconstraints", 0x80},
    {XAssetType::DESTRUCTIBLEDEF, "ap_destructibledef", 0x80},
    {XAssetType::XANIMPARTS, "ap_xanim", 0x6270},
    {XAssetType::XMODEL, "ap_xmodel", 0x2C00},
    /*
      Limit is hard-coded in `DB_GetMeshPoolSize` (inlined at all callsites),
      and underlying pool static allocation in engine is used directly in some
      functions.
      Cannot be extended solely by asset pool item count extension.
    */
    {XAssetType::XMODELMESH, "ap_xmodelmesh", 0x8C00},
    {XAssetType::MATERIAL, "ap_material", 0x5800},
    {XAssetType::COMPUTE_SHADER_SET, "ap_computeshaderset", 0x100},
    {XAssetType::TECHNIQUE_SET, "ap_techset", 0x400},
    /*
      Limit is hard-coded in `DB_GetImagePoolSize`, and underlying pool static
      allocation in engine is used directly in some functions.
      Cannot be extended solely by asset pool item count extension.
    */
    {XAssetType::IMAGE, "ap_image", 0xC000},
    {XAssetType::SOUND, "ap_sound", 0x20},
    {XAssetType::SOUND_PATCH, "ap_sound_patch", 0x10},
    {XAssetType::CLIPMAP, "ap_col_map", 0x2},
    {XAssetType::COMWORLD, "ap_com_map", 0x2},
    {XAssetType::GAMEWORLD, "ap_game_map", 0x2},
    {XAssetType::MAP_ENTS, "ap_map_ents", 0x2},
    {XAssetType::GFXWORLD, "ap_gfx_map", 0x2},
    {XAssetType::LIGHT_DEF, "ap_lightdef", 0x20},
    {XAssetType::LENSFLARE_DEF, "ap_lensflaredef", 0x46},
    {XAssetType::UI_MAP, "ap_ui_map", 0x00},
    {XAssetType::FONT, "ap_font", 0x10},
    {XAssetType::FONTICON, "ap_fonticon", 0x10},
    {XAssetType::LOCALIZE_ENTRY, "ap_localize", 0x6400},
    {XAssetType::WEAPON, "ap_weapon", 0x600},
    {XAssetType::WEAPONDEF, "ap_weapondef", 0x00},
    {XAssetType::WEAPON_VARIANT, "ap_weaponvariant", 0x00},
    {XAssetType::WEAPON_FULL, "ap_weaponfull", 0x00},
    {XAssetType::CGMEDIA, "ap_cgmediatable", 0x5},
    {XAssetType::PLAYERSOUNDS, "ap_playersoundstable", 0x10},
    {XAssetType::PLAYERFX, "ap_playerfxtable", 0x10},
    {XAssetType::SHAREDWEAPONSOUNDS, "ap_sharedweaponsounds", 0x40},
    {XAssetType::ATTACHMENT, "ap_attachment", 0x80},
    {XAssetType::ATTACHMENT_UNIQUE, "ap_attachmentunique", 0x864},
    {XAssetType::WEAPON_CAMO, "ap_weaponcamo", 0x200},
    {XAssetType::CUSTOMIZATION_TABLE, "ap_customizationtable", 0x8},
    {XAssetType::CUSTOMIZATION_TABLE_FE_IMAGES,
     "ap_customizationtable_feimages", 0x8},
    {XAssetType::CUSTOMIZATION_TABLE_COLOR, "ap_customizationtablecolor",
     0x400},
    {XAssetType::SNDDRIVER_GLOBALS, "ap_snddriverglobals", 0x1},
    {XAssetType::FX, "ap_fx", 0x7D0},
    {XAssetType::TAGFX, "ap_tagfx", 0x40},
    {XAssetType::NEW_LENSFLARE_DEF, "ap_klf", 0x46},
    {XAssetType::IMPACT_FX, "ap_impactsfxtable", 0x100},
    {XAssetType::IMPACT_SOUND, "ap_impactsoundstable", 0x40},
    {XAssetType::PLAYER_CHARACTER, "ap_player_character", 0x8},
    {XAssetType::AITYPE, "ap_aitype", 0x60},
    {XAssetType::CHARACTER, "ap_character", 0x96},
    {XAssetType::XMODELALIAS, "ap_xmodelalias", 0x30},
    {XAssetType::RAWFILE, "ap_rawfile", 0x1388},
    {XAssetType::STRINGTABLE, "ap_stringtable", 0xDC},
    {XAssetType::STRUCTURED_TABLE, "ap_structuredtable", 0x69},
    {XAssetType::LEADERBOARD, "ap_leaderboarddef", 0x100},
    {XAssetType::DDL, "ap_ddl", 0x40},
    {XAssetType::GLASSES, "ap_glasses", 0x2},
    {XAssetType::TEXTURELIST, "ap_texturelist", 0x8},
    {XAssetType::SCRIPTPARSETREE, "ap_scriptparsetree", 0x47E},
    {XAssetType::KEYVALUEPAIRS, "ap_keyvaluepairs", 0x40},
    {XAssetType::VEHICLEDEF, "ap_vehicle", 0x40},
    {XAssetType::ADDON_MAP_ENTS, "ap_addon_map_ents", 0x1},
    {XAssetType::TRACER, "ap_tracer", 0x64},
    {XAssetType::SLUG, "ap_slug", 0x5},
    {XAssetType::SURFACEFX_TABLE, "ap_surfacefxtable", 0x40},
    {XAssetType::SURFACESOUNDDEF, "ap_surfacesounddef", 0x100},
    {XAssetType::FOOTSTEP_TABLE, "ap_footsteptable", 0x20},
    {XAssetType::ENTITYFXIMPACTS, "ap_entityfximpacts", 0x100},
    {XAssetType::ENTITYSOUNDIMPACTS, "ap_entitysoundimpacts", 0x100},
    {XAssetType::ZBARRIER, "ap_zbarrier", 0x10},
    {XAssetType::VEHICLEFXDEF, "ap_vehiclefxdef", 0x20},
    {XAssetType::VEHICLESOUNDDEF, "ap_vehiclesounddef", 0x20},
    {XAssetType::TYPEINFO, "ap_typeinfo", 0x00},
    {XAssetType::SCRIPTBUNDLE, "ap_scriptbundle", 0x400},
    {XAssetType::SCRIPTBUNDLELIST, "ap_scriptbundlelist", 0x40},
    {XAssetType::RUMBLE, "ap_rumble", 0x118},
    {XAssetType::BULLETPENETRATION, "ap_bulletpenetration", 0x1},
    {XAssetType::LOCDMGTABLE, "ap_locdmgtable", 0x1},
    {XAssetType::AIMTABLE, "ap_aimtable", 0xC},
    {XAssetType::ANIMSELECTORTABLESET, "ap_animselectortable", 0x40},
    {XAssetType::ANIMMAPPINGTABLE, "ap_animmappingtable", 0x40},
    {XAssetType::ANIMSTATEMACHINE, "ap_animstatemachine", 0x40},
    {XAssetType::BEHAVIORTREE, "ap_behaviortree", 0x40},
    {XAssetType::BEHAVIORSTATEMACHINE, "ap_behaviorstatemachine", 0x80},
    {XAssetType::TTF, "ap_ttf", 0x30},
    {XAssetType::SANIM, "ap_sanim", 0x400},
    {XAssetType::LIGHT_DESCRIPTION, "ap_lightdescription", 0x226},
    {XAssetType::SHELLSHOCK, "ap_shellshock", 0x40},
    {XAssetType::XCAM, "ap_xcam", 0x214},
    {XAssetType::BG_CACHE, "ap_bgcache", 0x20},
    {XAssetType::TEXTURE_COMBO, "ap_texturecombo", 0x10},
    {XAssetType::FLAMETABLE, "ap_flametable", 0x10},
    {XAssetType::BITFIELD, "ap_bitfield", 0x34},
    {XAssetType::ATTACHMENT_COSMETIC_VARIANT, "ap_attachmentcosmeticvariant",
     0x280},
    {XAssetType::MAPTABLE, "ap_maptable", 0x19},
    {XAssetType::MAPTABLE_LOADING_IMAGES, "ap_maptableloadingimages", 0x19},
    {XAssetType::MEDAL, "ap_medal", 0x300},
    {XAssetType::MEDALTABLE, "ap_medaltable", 0x20},
    {XAssetType::OBJECTIVE, "ap_objective", 0x100},
    {XAssetType::OBJECTIVE_LIST, "ap_objectivelist", 0x40},
    {XAssetType::UMBRA_TOME, "ap_umbra_tome", 0x00},
    {XAssetType::NAVMESH, "ap_navmesh", 0x2},
    {XAssetType::NAVVOLUME, "ap_navvolume", 0x2},
    {XAssetType::BINARYHTML, "ap_binaryhtml", 0x800},
    {XAssetType::LASER, "ap_laser", 0x32},
    {XAssetType::BEAM, "ap_beam", 0x32},
    {XAssetType::STREAMER_HINT, "ap_streamerhint", 0x32},
};

rapidjson::Document load_settings_doc() {
  rapidjson::Document doc;
  const std::filesystem::path path = std::filesystem::path("boiii_players") /
                                     "user" / "launcher_settings.json";
  std::string data;
  if (utils::io::read_file(path.string(), &data) && !data.empty()) {
    if (doc.Parse(data.c_str()).HasParseError() || !doc.IsObject()) {
      doc.SetObject();
    }
  } else {
    doc.SetObject();
  }
  return doc;
}

std::string get_setting(const rapidjson::Document &doc, const char *key) {
  auto it = doc.FindMember(key);
  if (it != doc.MemberEnd() && it->value.IsString()) {
    return it->value.GetString();
  }
  return {};
}

bool is_enabled(const rapidjson::Document &doc) {
  // Check master enable flag
  const std::string val = get_setting(doc, "asset_limits_enabled");
  if (!val.empty() && val != "1")
    return false;

  // Check disable_asset_pools flag (inverted)
  const std::string disable_val = get_setting(doc, "disable_asset_pools");
  if (disable_val == "1")
    return false;

  return true;
}

unsigned int get_pool_size(const rapidjson::Document &doc,
                           const pool_config &cfg) {
  const std::string val = get_setting(doc, cfg.setting_key);
  if (!val.empty()) {
    try {
      const uint32_t parsed = std::stoul(val);
      if (parsed <= std::numeric_limits<int32_t>::max()) {
        return parsed;
      }
    } catch (...) {
    }
  }
  return cfg.default_size;
}

void apply_asset_limits() {
  const rapidjson::Document doc = load_settings_doc();

  // Ensure asset pool initialized and not already extended
  for (const pool_config &cfg : pool_configs) {
    if (static_cast<uint32_t>(pool::s_assetPools->pools[+cfg.type].itemCount) !=
        cfg.default_size) {
      return;
    }
  }

  if (!is_enabled(doc)) {
    printf("Asset pool expansion disabled by user settings\n");
    return;
  }

  for (const pool_config &cfg : pool_configs) {
    const uint32_t size = get_pool_size(doc, cfg);
    reallocate_asset_pool(cfg.type, size);
  }
}

utils::hook::detour DB_AssetPoolInit_hook;
void DB_AssetPoolInit_stub() {
  DB_AssetPoolInit_hook.invoke();
  apply_asset_limits();
  DB_InitBSPGlobals_Impl();
}
} // namespace

class component final : public generic_component {
public:
  void post_unpack() override {
    DB_AssetPoolInit_hook.create(game::db::DB_AssetPoolInit.get(),
                                 DB_AssetPoolInit_stub);
  }
};
} // namespace asset_limits

REGISTER_COMPONENT(asset_limits::component)