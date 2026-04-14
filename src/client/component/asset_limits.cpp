#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

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
    {XAssetType::ASSET_TYPE_PHYSPRESET, "ap_physpreset", 256},
    {XAssetType::ASSET_TYPE_DESTRUCTIBLEDEF, "ap_destructibledef", 256},
    {XAssetType::ASSET_TYPE_XANIMPARTS, "ap_xanim", 4096},
    {XAssetType::ASSET_TYPE_XMODEL, "ap_xmodel", 2048},
    {XAssetType::ASSET_TYPE_XMODELMESH, "ap_xmodelmesh", 2048},
    {XAssetType::ASSET_TYPE_MATERIAL, "ap_material", 8192},
    {XAssetType::ASSET_TYPE_TECHNIQUE_SET, "ap_techset", 2048},
    {XAssetType::ASSET_TYPE_IMAGE, "ap_image", 8192},
    {XAssetType::ASSET_TYPE_SOUND, "ap_sound", 4096},
    {XAssetType::ASSET_TYPE_LIGHT_DEF, "ap_lightdef", 256},
    {XAssetType::ASSET_TYPE_LOCALIZE_ENTRY, "ap_localize", 2048},
    {XAssetType::ASSET_TYPE_WEAPON, "ap_weapon", 512},
    {XAssetType::ASSET_TYPE_ATTACHMENT, "ap_attachment", 512},
    {XAssetType::ASSET_TYPE_ATTACHMENT_UNIQUE, "ap_attachunique", 1024},
    {XAssetType::ASSET_TYPE_WEAPON_CAMO, "ap_weaponcamo", 256},
    {XAssetType::ASSET_TYPE_SHAREDWEAPONSOUNDS, "ap_sharedweaponsounds", 128},
    {XAssetType::ASSET_TYPE_FX, "ap_fx", 1024},
    {XAssetType::ASSET_TYPE_TAGFX, "ap_tagfx", 256},
    {XAssetType::ASSET_TYPE_NEW_LENSFLARE_DEF, "ap_newlensflaredef", 128},
    {XAssetType::ASSET_TYPE_AITYPE, "ap_aitype", 256},
    {XAssetType::ASSET_TYPE_CHARACTER, "ap_character", 256},
    {XAssetType::ASSET_TYPE_RAWFILE, "ap_rawfile", 2048},
    {XAssetType::ASSET_TYPE_STRINGTABLE, "ap_stringtable", 512},
    {XAssetType::ASSET_TYPE_SCRIPTPARSETREE, "ap_scriptparsetree", 2048},
    {XAssetType::ASSET_TYPE_SCRIPTBUNDLE, "ap_scriptbundle", 512},
    {XAssetType::ASSET_TYPE_RUMBLE, "ap_rumble", 128},
    {XAssetType::ASSET_TYPE_LIGHT_DESCRIPTION, "ap_lightdescription", 1024},
    {XAssetType::ASSET_TYPE_SHELLSHOCK, "ap_shellshock", 128},
    {XAssetType::ASSET_TYPE_XCAM, "ap_xcam", 256},
    {XAssetType::ASSET_TYPE_TRACER, "ap_tracer", 128},
    {XAssetType::ASSET_TYPE_VEHICLEDEF, "ap_vehicledef", 128},
    {XAssetType::ASSET_TYPE_TTF, "ap_ttf", 64},
};

rapidjson::Document load_settings_doc() {
  rapidjson::Document doc;
  const auto path = std::filesystem::path("boiii_players") / "user" /
                    "launcher_settings.json";
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
  const auto val = get_setting(doc, "asset_limits_enabled");
  if (!val.empty() && val != "1")
    return false;

  // Check disable_asset_pools flag (inverted)
  const auto disable_val = get_setting(doc, "disable_asset_pools");
  if (disable_val == "1")
    return false;

  return true;
}

unsigned int get_pool_size(const rapidjson::Document &doc,
                           const pool_config &cfg) {
  const auto val = get_setting(doc, cfg.setting_key);
  if (!val.empty()) {
    try {
      const auto parsed = std::stoul(val);
      if (parsed >= 32 && parsed <= 65536) {
        return static_cast<unsigned int>(parsed);
      }
    } catch (...) {
    }
  }
  return cfg.default_size;
}

void reallocate_asset_pool(const XAssetType type, const unsigned int new_size) {
  if (static_cast<int>(type) < 0 || type >= XAssetType::ASSET_TYPE_COUNT) {
    printf("[AssetLimits] Invalid asset type %d\n", static_cast<int>(type));
    return;
  }

  const auto entry_size = DB_GetXAssetTypeSize(type);
  if (entry_size <= 0) {
    printf("[AssetLimits] Invalid entry size for type %d\n",
           static_cast<int>(type));
    return;
  }

  auto *pool = &DB_XAssetPool[static_cast<int>(type)];

  // Skip if pool already meets or exceeds requested size
  if (pool->itemAllocCount >= static_cast<int>(new_size)) {
    return;
  }

  const auto new_pool = calloc(new_size, entry_size);
  if (!new_pool) {
    printf("Failed to allocate asset pool for type %d (size: %u)\n",
           static_cast<int>(type), new_size);
    return;
  }

  // Copy existing entries
  memcpy(new_pool, pool->pool,
         pool->itemAllocCount * static_cast<size_t>(entry_size));

  // Rebuild free list for new entries
  pool->freeHead = reinterpret_cast<AssetLink *>(
      static_cast<char *>(new_pool) +
      static_cast<size_t>(entry_size) * pool->itemAllocCount);

  for (auto i = pool->itemAllocCount; i < static_cast<int>(new_size) - 1; i++) {
    auto *current = reinterpret_cast<AssetLink *>(
        static_cast<char *>(new_pool) + static_cast<size_t>(entry_size) * i);
    current->next = reinterpret_cast<AssetLink *>(
        static_cast<char *>(new_pool) +
        static_cast<size_t>(entry_size) * (i + 1));
  }

  // Last entry points to null
  auto *last = reinterpret_cast<AssetLink *>(static_cast<char *>(new_pool) +
                                             static_cast<size_t>(entry_size) *
                                                 (new_size - 1));
  last->next = nullptr;

  pool->pool = new_pool;
  pool->itemAllocCount = static_cast<int>(new_size);

  printf("Reallocated asset pool type %d: %d -> %u entries\n",
         static_cast<int>(type), pool->itemCount, new_size);
}

void apply_asset_limits() {
  static bool applied = false;
  if (applied)
    return;

  const auto doc = load_settings_doc();

  if (!is_enabled(doc)) {
    printf("Asset pool expansion disabled by user settings\n");
    return;
  }

  for (const auto &cfg : pool_configs) {
    const auto size = get_pool_size(doc, cfg);
    reallocate_asset_pool(cfg.type, size);
  }

  applied = true;
}

utils::hook::detour com_sessionmode_setmode_hook;

game::eModes com_sessionmode_setmode_stub(game::eModes mode) {
  const auto result = com_sessionmode_setmode_hook.invoke<game::eModes>(mode);
  apply_asset_limits();
  return result;
}
} // namespace

class component final : public client_component {
public:
  void post_unpack() override {
    com_sessionmode_setmode_hook.create(game::Com_SessionMode_SetMode.get(),
                                        com_sessionmode_setmode_stub);
    apply_asset_limits();
  }
};
} // namespace asset_limits

REGISTER_COMPONENT(asset_limits::component)