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
  unsigned int default_size;
};

static const pool_config pool_configs[] = {
    {XAssetType::PHYSPRESET, "ap_physpreset", 256},
    {XAssetType::DESTRUCTIBLEDEF, "ap_destructibledef", 256},
    {XAssetType::XANIMPARTS, "ap_xanim", 4096},
    {XAssetType::XMODEL, "ap_xmodel", 2048},
    {XAssetType::XMODELMESH, "ap_xmodelmesh", 2048},
    {XAssetType::MATERIAL, "ap_material", 8192},
    {XAssetType::TECHNIQUE_SET, "ap_techset", 2048},
    {XAssetType::IMAGE, "ap_image", 8192},
    {XAssetType::SOUND, "ap_sound", 4096},
    {XAssetType::LIGHT_DEF, "ap_lightdef", 256},
    {XAssetType::LOCALIZE_ENTRY, "ap_localize", 2048},
    {XAssetType::WEAPON, "ap_weapon", 512},
    {XAssetType::ATTACHMENT, "ap_attachment", 512},
    {XAssetType::ATTACHMENT_UNIQUE, "ap_attachunique", 1024},
    {XAssetType::WEAPON_CAMO, "ap_weaponcamo", 256},
    {XAssetType::SHAREDWEAPONSOUNDS, "ap_sharedweaponsounds", 128},
    {XAssetType::FX, "ap_fx", 1024},
    {XAssetType::TAGFX, "ap_tagfx", 256},
    {XAssetType::NEW_LENSFLARE_DEF, "ap_newlensflaredef", 128},
    {XAssetType::AITYPE, "ap_aitype", 256},
    {XAssetType::CHARACTER, "ap_character", 256},
    {XAssetType::RAWFILE, "ap_rawfile", 2048},
    {XAssetType::STRINGTABLE, "ap_stringtable", 512},
    {XAssetType::SCRIPTPARSETREE, "ap_scriptparsetree", 2048},
    {XAssetType::SCRIPTBUNDLE, "ap_scriptbundle", 512},
    {XAssetType::RUMBLE, "ap_rumble", 128},
    {XAssetType::LIGHT_DESCRIPTION, "ap_lightdescription", 1024},
    {XAssetType::SHELLSHOCK, "ap_shellshock", 128},
    {XAssetType::XCAM, "ap_xcam", 256},
    {XAssetType::TRACER, "ap_tracer", 128},
    {XAssetType::VEHICLEDEF, "ap_vehicledef", 128},
    {XAssetType::TTF, "ap_ttf", 64},
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
      if (parsed >= 32 && parsed <= 65536) {
        return parsed;
      }
    } catch (...) {
    }
  }
  return cfg.default_size;
}

void apply_asset_limits() {
  static bool applied = false;
  if (applied)
    return;

  const rapidjson::Document doc = load_settings_doc();

  if (!is_enabled(doc)) {
    printf("Asset pool expansion disabled by user settings\n");
    return;
  }

  for (const pool_config &cfg : pool_configs) {
    const uint32_t size = get_pool_size(doc, cfg);
    reallocate_asset_pool(cfg.type, size);
  }

  applied = true;
}

utils::hook::detour db_init_hook;
void db_init_stub() {
  db_init_hook.invoke();
  apply_asset_limits();
}
} // namespace

class component final : public generic_component {
public:
  void post_unpack() override {
    db_init_hook.create(game::db::DB_Init.get(), db_init_stub);
    apply_asset_limits();
  }
};
} // namespace asset_limits

REGISTER_COMPONENT(asset_limits::component)