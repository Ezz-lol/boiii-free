#include <std_include.hpp>

#include <game/impl/db/db.hpp>
#include <loader/component_loader.hpp>

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

#include <frozen/unordered_map.h>
#include <rapidjson/document.h>

#include "asset_limits.hpp"
#include "script.hpp"

namespace asset_limits {

void apply_list(const std::vector<pool_config> &limits) {
  for (const pool_config &limit : limits) {
    pool::reallocate_asset_pool(limit.type, limit.size);
  }
}

template <typename T> std::optional<T> parse_int(const std::string_view &str) {
  const char *first = str.data();
  const char *last = str.data() + str.size();
  int32_t base = 10;
  if (str.starts_with("0x")) {
    base = 16;
    first = str.data() + 2;
  }

  T result = {};
  auto [ptr, ec] = std::from_chars(first, last, result, base);

  if (ec == std::errc() && ptr == last) {
    return result;
  }
  return std::nullopt;
}

std::vector<pool_config> parse_list(std::string &data) {
  std::vector<pool_config> result;
  data = utils::string::replace(
      utils::string::replace(utils::string::replace(data, "\r\n", "\n"), "\r",
                             ""),
      "\t", " ");
  const std::vector<std::string> lines = utils::string::split(data, '\n');
  for (const std::string &line : lines) {
    // T7Recharged syntax:
    // int_type_value string_type_name default_pool_size extended_pool_size
    // Minimal sntax: int_type_value extended_pool_size
    const std::vector<std::string> parts = utils::string::split(line, ' ');
    if (parts.size() > 1) {
      const std::optional<int32_t> parsed_type_val =
          parse_int<int32_t>(parts[0]);
      if (parsed_type_val.has_value() &&
          validate(static_cast<XAssetType>(parsed_type_val.value()))) {
        const std::optional<uint32_t> parsed_size_val =
            parse_int<uint32_t>(parts[parts.size() - 1]);
        if (parsed_size_val.has_value()) {
          result.push_back(
              {.type = static_cast<XAssetType>(parsed_type_val.value()),
               .size = parsed_size_val.value()});
        }
      }
    }
  }

  return result;
}

namespace {

inline constexpr default_pool_config DEFAULT_POOL_CONFIGS[] = {
    {.type = XAssetType::PHYSPRESET,
     .setting_key = "ap_physpreset",
     .default_size = 0x113},
    {.type = XAssetType::PHYSCONSTRAINTS,
     .setting_key = "ap_physconstraints",
     .default_size = 0x80},
    {.type = XAssetType::DESTRUCTIBLEDEF,
     .setting_key = "ap_destructibledef",
     .default_size = 0x80},
    {.type = XAssetType::XANIMPARTS,
     .setting_key = "ap_xanim",
     .default_size = 0x6270},
    {.type = XAssetType::XMODEL,
     .setting_key = "ap_xmodel",
     .default_size = 0x2C00},
    /*
      Limit is hard-coded in `DB_GetMeshPoolSize` (inlined at all callsites),
      and underlying pool static allocation in engine is used directly in some
      functions.
      Cannot be extended solely by asset pool item count extension.
    */
    {.type = XAssetType::XMODELMESH,
     .setting_key = "ap_xmodelmesh",
     .default_size = 0x8C00},
    {.type = XAssetType::MATERIAL,
     .setting_key = "ap_material",
     .default_size = 0x5800},
    {.type = XAssetType::COMPUTE_SHADER_SET,
     .setting_key = "ap_computeshaderset",
     .default_size = 0x100},
    {.type = XAssetType::TECHNIQUE_SET,
     .setting_key = "ap_techset",
     .default_size = 0x400},
    /*
      Limit is hard-coded in `DB_GetImagePoolSize`, and underlying pool static
      allocation in engine is used directly in some functions.
      Cannot be extended solely by asset pool item count extension.
    */
    {.type = XAssetType::IMAGE,
     .setting_key = "ap_image",
     .default_size = 0xC000},
    {.type = XAssetType::SOUND,
     .setting_key = "ap_sound",
     .default_size = 0x20},
    {.type = XAssetType::SOUND_PATCH,
     .setting_key = "ap_sound_patch",
     .default_size = 0x10},
    {.type = XAssetType::CLIPMAP,
     .setting_key = "ap_col_map",
     .default_size = 0x2},
    {.type = XAssetType::COMWORLD,
     .setting_key = "ap_com_map",
     .default_size = 0x2},
    {.type = XAssetType::GAMEWORLD,
     .setting_key = "ap_game_map",
     .default_size = 0x2},
    {.type = XAssetType::MAP_ENTS,
     .setting_key = "ap_map_ents",
     .default_size = 0x2},
    {.type = XAssetType::GFXWORLD,
     .setting_key = "ap_gfx_map",
     .default_size = 0x2},
    {.type = XAssetType::LIGHT_DEF,
     .setting_key = "ap_lightdef",
     .default_size = 0x20},
    {.type = XAssetType::LENSFLARE_DEF,
     .setting_key = "ap_lensflaredef",
     .default_size = 0x46},
    {.type = XAssetType::UI_MAP,
     .setting_key = "ap_ui_map",
     .default_size = 0x00},
    {.type = XAssetType::FONT, .setting_key = "ap_font", .default_size = 0x10},
    {.type = XAssetType::FONTICON,
     .setting_key = "ap_fonticon",
     .default_size = 0x10},
    {.type = XAssetType::LOCALIZE_ENTRY,
     .setting_key = "ap_localize",
     .default_size = 0x6400},
    {.type = XAssetType::WEAPON,
     .setting_key = "ap_weapon",
     .default_size = 0x600},
    {.type = XAssetType::WEAPONDEF,
     .setting_key = "ap_weapondef",
     .default_size = 0x00},
    {.type = XAssetType::WEAPON_VARIANT,
     .setting_key = "ap_weaponvariant",
     .default_size = 0x00},
    {.type = XAssetType::WEAPON_FULL,
     .setting_key = "ap_weaponfull",
     .default_size = 0x00},
    {.type = XAssetType::CGMEDIA,
     .setting_key = "ap_cgmediatable",
     .default_size = 0x5},
    {.type = XAssetType::PLAYERSOUNDS,
     .setting_key = "ap_playersoundstable",
     .default_size = 0x10},
    {.type = XAssetType::PLAYERFX,
     .setting_key = "ap_playerfxtable",
     .default_size = 0x10},
    {.type = XAssetType::SHAREDWEAPONSOUNDS,
     .setting_key = "ap_sharedweaponsounds",
     .default_size = 0x40},
    {.type = XAssetType::ATTACHMENT,
     .setting_key = "ap_attachment",
     .default_size = 0x80},
    {.type = XAssetType::ATTACHMENT_UNIQUE,
     .setting_key = "ap_attachmentunique",
     .default_size = 0x864},
    {.type = XAssetType::WEAPON_CAMO,
     .setting_key = "ap_weaponcamo",
     .default_size = 0x200},
    {.type = XAssetType::CUSTOMIZATION_TABLE,
     .setting_key = "ap_customizationtable",
     .default_size = 0x8},
    {.type = XAssetType::CUSTOMIZATION_TABLE_FE_IMAGES,
     .setting_key = "ap_customizationtable_feimages",
     .default_size = 0x8},
    {.type = XAssetType::CUSTOMIZATION_TABLE_COLOR,
     .setting_key = "ap_customizationtablecolor",
     .default_size = 0x400},
    {.type = XAssetType::SNDDRIVER_GLOBALS,
     .setting_key = "ap_snddriverglobals",
     .default_size = 0x1},
    {.type = XAssetType::FX, .setting_key = "ap_fx", .default_size = 0x7D0},
    {.type = XAssetType::TAGFX,
     .setting_key = "ap_tagfx",
     .default_size = 0x40},
    {.type = XAssetType::NEW_LENSFLARE_DEF,
     .setting_key = "ap_klf",
     .default_size = 0x46},
    {.type = XAssetType::IMPACT_FX,
     .setting_key = "ap_impactsfxtable",
     .default_size = 0x100},
    {.type = XAssetType::IMPACT_SOUND,
     .setting_key = "ap_impactsoundstable",
     .default_size = 0x40},
    {.type = XAssetType::PLAYER_CHARACTER,
     .setting_key = "ap_player_character",
     .default_size = 0x8},
    {.type = XAssetType::AITYPE,
     .setting_key = "ap_aitype",
     .default_size = 0x60},
    {.type = XAssetType::CHARACTER,
     .setting_key = "ap_character",
     .default_size = 0x96},
    {.type = XAssetType::XMODELALIAS,
     .setting_key = "ap_xmodelalias",
     .default_size = 0x30},
    {.type = XAssetType::RAWFILE,
     .setting_key = "ap_rawfile",
     .default_size = 0x1388},
    {.type = XAssetType::STRINGTABLE,
     .setting_key = "ap_stringtable",
     .default_size = 0xDC},
    {.type = XAssetType::STRUCTURED_TABLE,
     .setting_key = "ap_structuredtable",
     .default_size = 0x69},
    {.type = XAssetType::LEADERBOARD,
     .setting_key = "ap_leaderboarddef",
     .default_size = 0x100},
    {.type = XAssetType::DDL, .setting_key = "ap_ddl", .default_size = 0x40},
    {.type = XAssetType::GLASSES,
     .setting_key = "ap_glasses",
     .default_size = 0x2},
    {.type = XAssetType::TEXTURELIST,
     .setting_key = "ap_texturelist",
     .default_size = 0x8},
    {.type = XAssetType::SCRIPTPARSETREE,
     .setting_key = "ap_scriptparsetree",
     .default_size = 0x47E},
    {.type = XAssetType::KEYVALUEPAIRS,
     .setting_key = "ap_keyvaluepairs",
     .default_size = 0x40},
    {.type = XAssetType::VEHICLEDEF,
     .setting_key = "ap_vehicle",
     .default_size = 0x40},
    {.type = XAssetType::ADDON_MAP_ENTS,
     .setting_key = "ap_addon_map_ents",
     .default_size = 0x1},
    {.type = XAssetType::TRACER,
     .setting_key = "ap_tracer",
     .default_size = 0x64},
    {.type = XAssetType::SLUG, .setting_key = "ap_slug", .default_size = 0x5},
    {.type = XAssetType::SURFACEFX_TABLE,
     .setting_key = "ap_surfacefxtable",
     .default_size = 0x40},
    {.type = XAssetType::SURFACESOUNDDEF,
     .setting_key = "ap_surfacesounddef",
     .default_size = 0x100},
    {.type = XAssetType::FOOTSTEP_TABLE,
     .setting_key = "ap_footsteptable",
     .default_size = 0x20},
    {.type = XAssetType::ENTITYFXIMPACTS,
     .setting_key = "ap_entityfximpacts",
     .default_size = 0x100},
    {.type = XAssetType::ENTITYSOUNDIMPACTS,
     .setting_key = "ap_entitysoundimpacts",
     .default_size = 0x100},
    {.type = XAssetType::ZBARRIER,
     .setting_key = "ap_zbarrier",
     .default_size = 0x10},
    {.type = XAssetType::VEHICLEFXDEF,
     .setting_key = "ap_vehiclefxdef",
     .default_size = 0x20},
    {.type = XAssetType::VEHICLESOUNDDEF,
     .setting_key = "ap_vehiclesounddef",
     .default_size = 0x20},
    {.type = XAssetType::TYPEINFO,
     .setting_key = "ap_typeinfo",
     .default_size = 0x00},
    {.type = XAssetType::SCRIPTBUNDLE,
     .setting_key = "ap_scriptbundle",
     .default_size = 0x400},
    {.type = XAssetType::SCRIPTBUNDLELIST,
     .setting_key = "ap_scriptbundlelist",
     .default_size = 0x40},
    {.type = XAssetType::RUMBLE,
     .setting_key = "ap_rumble",
     .default_size = 0x118},
    {.type = XAssetType::BULLETPENETRATION,
     .setting_key = "ap_bulletpenetration",
     .default_size = 0x1},
    {.type = XAssetType::LOCDMGTABLE,
     .setting_key = "ap_locdmgtable",
     .default_size = 0x1},
    {.type = XAssetType::AIMTABLE,
     .setting_key = "ap_aimtable",
     .default_size = 0xC},
    {.type = XAssetType::ANIMSELECTORTABLESET,
     .setting_key = "ap_animselectortable",
     .default_size = 0x40},
    {.type = XAssetType::ANIMMAPPINGTABLE,
     .setting_key = "ap_animmappingtable",
     .default_size = 0x40},
    {.type = XAssetType::ANIMSTATEMACHINE,
     .setting_key = "ap_animstatemachine",
     .default_size = 0x40},
    {.type = XAssetType::BEHAVIORTREE,
     .setting_key = "ap_behaviortree",
     .default_size = 0x40},
    {.type = XAssetType::BEHAVIORSTATEMACHINE,
     .setting_key = "ap_behaviorstatemachine",
     .default_size = 0x80},
    {.type = XAssetType::TTF, .setting_key = "ap_ttf", .default_size = 0x30},
    {.type = XAssetType::SANIM,
     .setting_key = "ap_sanim",
     .default_size = 0x400},
    {.type = XAssetType::LIGHT_DESCRIPTION,
     .setting_key = "ap_lightdescription",
     .default_size = 0x226},
    {.type = XAssetType::SHELLSHOCK,
     .setting_key = "ap_shellshock",
     .default_size = 0x40},
    {.type = XAssetType::XCAM, .setting_key = "ap_xcam", .default_size = 0x214},
    {.type = XAssetType::BG_CACHE,
     .setting_key = "ap_bgcache",
     .default_size = 0x20},
    {.type = XAssetType::TEXTURE_COMBO,
     .setting_key = "ap_texturecombo",
     .default_size = 0x10},
    {.type = XAssetType::FLAMETABLE,
     .setting_key = "ap_flametable",
     .default_size = 0x10},
    {.type = XAssetType::BITFIELD,
     .setting_key = "ap_bitfield",
     .default_size = 0x34},
    {.type = XAssetType::ATTACHMENT_COSMETIC_VARIANT,
     .setting_key = "ap_attachmentcosmeticvariant",
     .default_size = 0x280},
    {.type = XAssetType::MAPTABLE,
     .setting_key = "ap_maptable",
     .default_size = 0x19},
    {.type = XAssetType::MAPTABLE_LOADING_IMAGES,
     .setting_key = "ap_maptableloadingimages",
     .default_size = 0x19},
    {.type = XAssetType::MEDAL,
     .setting_key = "ap_medal",
     .default_size = 0x300},
    {.type = XAssetType::MEDALTABLE,
     .setting_key = "ap_medaltable",
     .default_size = 0x20},
    {.type = XAssetType::OBJECTIVE,
     .setting_key = "ap_objective",
     .default_size = 0x100},
    {.type = XAssetType::OBJECTIVE_LIST,
     .setting_key = "ap_objectivelist",
     .default_size = 0x40},
    {.type = XAssetType::UMBRA_TOME,
     .setting_key = "ap_umbra_tome",
     .default_size = 0x00},
    {.type = XAssetType::NAVMESH,
     .setting_key = "ap_navmesh",
     .default_size = 0x2},
    {.type = XAssetType::NAVVOLUME,
     .setting_key = "ap_navvolume",
     .default_size = 0x2},
    {.type = XAssetType::BINARYHTML,
     .setting_key = "ap_binaryhtml",
     .default_size = 0x800},
    {.type = XAssetType::LASER,
     .setting_key = "ap_laser",
     .default_size = 0x32},
    {.type = XAssetType::BEAM, .setting_key = "ap_beam", .default_size = 0x32},
    {.type = XAssetType::STREAMER_HINT,
     .setting_key = "ap_streamerhint",
     .default_size = 0x32},
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
  rapidjson::Document::ConstMemberIterator it = doc.FindMember(key);
  return it != doc.MemberEnd() && it->value.IsString() ? it->value.GetString()
                                                       : std::string();
}

bool is_enabled(const rapidjson::Document &doc) {
  // Check master enable flag
  const std::string val = get_setting(doc, "asset_limits_enabled");
  if (val.empty() || val == "1") {
    // Check disable_asset_pools flag (inverted)
    const std::string disable_val = get_setting(doc, "disable_asset_pools");
    if (disable_val != "1") {
      return true;
    }
  }

  return false;
}

uint32_t get_pool_size(const rapidjson::Document &doc,
                       const default_pool_config &cfg) {
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

  if (is_enabled(doc)) {
    for (const default_pool_config &cfg : DEFAULT_POOL_CONFIGS) {
      const uint32_t size = get_pool_size(doc, cfg);
      pool::reallocate_asset_pool(cfg.type, size);
    }
  } else {
    printf("Asset pool expansion disabled by user settings\n");
  }
}

utils::hook::detour DB_AssetPoolInit_hook;
void DB_AssetPoolInit_stub() {
  DB_AssetPoolInit_hook.invoke();
  apply_asset_limits();
  pool::DB_InitBSPGlobals_Impl();

  script::load_rawfiles();
}

utils::hook::detour DB_AssetPoolAlloc_hook;
} // namespace

class component final : public generic_component {
  DEFINE_COMPONENT_NAME("asset_limits");

public:
  void post_unpack() override {
    DB_AssetPoolInit_hook.create(game::db::DB_AssetPoolInit.get(),
                                 DB_AssetPoolInit_stub);
    DB_AssetPoolAlloc_hook.create(
        game::db::xasset::pool::DB_AssetPoolAlloc,
        game::db::xasset::pool::DB_AssetPoolAlloc_Impl);
  }
};
} // namespace asset_limits

REGISTER_COMPONENT(asset_limits::component)
