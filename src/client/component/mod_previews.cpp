#include <std_include.hpp>

#include "launcher/launcher.hpp"
#include "mod_previews.hpp"
#include "scheduler.hpp"

#include <component/lua/lua_state.hpp>
#include <game/game.hpp>
#include <loader/component_loader.hpp>

#include <utils/http.hpp>
#include <utils/image.hpp>
#include <utils/io.hpp>

#pragma warning(push)
#pragma warning(disable : 4996)
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_resize2.h>
#include <stb_image_write.h>
#pragma warning(pop)

namespace mod_previews {
namespace {
using namespace game::lua;
using namespace game::lua::hks;
using game::db::xasset::XAssetType;

constexpr std::string_view IMAGE_PREFIX = "mod_preview_";
constexpr int32_t PREVIEW_WIDTH = 640;

enum class preview_state { downloading, downloaded, ready, missing };

struct preview {
  preview_state state = preview_state::downloading;
  std::string image_name;
  game::gfx::GfxImage *image = nullptr;
};

std::mutex previews_mutex;
std::unordered_map<std::string, std::unique_ptr<preview>> previews;

std::filesystem::path cached_preview_path(const std::string &ugc_name) {
  return game::get_appdata_path() / "cache" / "mod_previews" /
         (ugc_name + ".png");
}

std::optional<std::string> fetch_preview_data(const std::string &ugc_name) {
  std::string data;
  if (utils::io::read_file(
          (game::get_game_path() / "mods" / ugc_name / "previewimage.png")
              .string(),
          &data)) {
    return data;
  }
  if (ugc_name.empty() || !std::ranges::all_of(ugc_name, ::isdigit)) {
    return std::nullopt;
  }
  const std::string url = launcher::get_steam_workshop_preview_url(ugc_name);
  if (url.empty()) {
    return std::nullopt;
  }
  return utils::http::get_data(url);
}

bool cache_preview(const std::string &ugc_name,
                   const std::filesystem::path &path) {
  const std::optional<std::string> data = fetch_preview_data(ugc_name);
  if (!data) {
    return false;
  }
  try {
    const utils::image::image decoded = utils::image::load_image(*data);
    const int32_t width = static_cast<int32_t>(decoded.width);
    const int32_t height = static_cast<int32_t>(decoded.height);
    const int32_t scaled_width = std::min(width, PREVIEW_WIDTH);
    const int32_t scaled_height = std::max(1, height * scaled_width / width);
    std::string scaled(static_cast<size_t>(scaled_width) * scaled_height * 4,
                       '\0');
    if (!stbir_resize_uint8_srgb(
            reinterpret_cast<const uint8_t *>(decoded.data.data()), width,
            height, 0, reinterpret_cast<uint8_t *>(scaled.data()), scaled_width,
            scaled_height, 0, STBIR_RGBA)) {
      return false;
    }
    std::filesystem::create_directories(path.parent_path());
    return stbi_write_png(path.string().c_str(), scaled_width, scaled_height, 4,
                          scaled.data(), scaled_width * 4) != 0;
  } catch (const std::exception &) {
    return false;
  }
}

void request_download(const std::string &ugc_name) {
  std::unique_ptr<preview> &entry = previews[ugc_name];
  if (entry) {
    return;
  }
  entry = std::make_unique<preview>();
  entry->image_name = std::string(IMAGE_PREFIX) + ugc_name;
  scheduler::once(
      [ugc_name] {
        const std::filesystem::path path = cached_preview_path(ugc_name);
        std::error_code error;
        const bool cached = std::filesystem::exists(path, error) ||
                            cache_preview(ugc_name, path);
        std::scoped_lock lock(previews_mutex);
        previews[ugc_name]->state =
            cached ? preview_state::downloaded : preview_state::missing;
      },
      scheduler::pipeline::async);
}

void load_texture(const std::string &ugc_name, preview &entry) {
  const game::gfx::GfxImage *base =
      game::db::xasset::DB_FindXAssetHeader(XAssetType::IMAGE,
                                            "img_t7_mod_preview", false, 0)
          .image;
  if (!base) {
    return;
  }
  const game::gfx::GfxTexture texture = game::gfx::Gfx_LoadTextureFromPng(
      cached_preview_path(ugc_name).string().c_str());
  if (!texture.basemap || !game::nonnull(game::db::xasset::pool::s_assetPools
                                             ->pools[+XAssetType::IMAGE]
                                             .freeHead)) {
    entry.state = preview_state::missing;
    return;
  }
  entry.image = static_cast<game::gfx::GfxImage *>(
      game::db::xasset::pool::DB_AssetPoolAlloc(XAssetType::IMAGE));
  *entry.image = *base;
  entry.image->texture = texture;
  entry.image->name = entry.image_name.c_str();
  entry.state = preview_state::ready;
}

void request_installed_mods() {
  std::scoped_lock lock(previews_mutex);
  for (uint32_t i = 0; i < game::ugc::modsPool.count; i++) {
    request_download(game::ugc::modsPool.data[i].publisherId);
  }
}

luaReturnCount_e get_preview_image(lua_State *luaVM) {
  const char *ugc_name = lua_tostring(luaVM, 1);
  if (!ugc_name || !ugc_name[0]) {
    lua_pushnil(luaVM);
    return luaReturnCount_e::ONE;
  }

  std::scoped_lock lock(previews_mutex);
  request_download(ugc_name);
  preview &entry = *previews[ugc_name];
  if (entry.state == preview_state::downloaded) {
    load_texture(ugc_name, entry);
  }
  if (entry.state == preview_state::ready) {
    lua_pushstring(luaVM, entry.image_name);
  } else {
    lua_pushnil(luaVM);
  }
  return luaReturnCount_e::ONE;
}

luaReturnCount_e delete_mod(lua_State *luaVM) {
  const char *ugc_name = lua_tostring(luaVM, 1);
  std::string error = "This mod could not be found.";
  if (game::ugc::UGC_ActiveMod_Loaded() || game::com::Com_IsInGame()) {
    error = "Unload mods before deleting one.";
  } else if (ugc_name) {
    for (uint32_t i = 0; i < game::ugc::modsPool.count; i++) {
      const game::ugc::WorkshopData &mod = game::ugc::modsPool.data[i];
      if (std::string_view(mod.publisherId) != ugc_name) {
        continue;
      }
      std::filesystem::path folder =
          std::string_view(mod.absolutePathZoneFiles);
      if (folder.filename() == "zone") {
        folder = folder.parent_path();
      }
      if (launcher::workshop_remove_by_path(folder.string(), error)) {
        lua_pushnil(luaVM);
        return luaReturnCount_e::ONE;
      }
      break;
    }
  }
  lua_pushstring(luaVM, error);
  return luaReturnCount_e::ONE;
}

} // namespace

game::gfx::GfxImage *find_image(const char *name) {
  if (!name || !std::string_view(name).starts_with(IMAGE_PREFIX)) {
    return nullptr;
  }
  std::scoped_lock lock(previews_mutex);
  const auto entry = previews.find(
      std::string(std::string_view(name).substr(IMAGE_PREFIX.size())));
  if (entry == previews.end() || entry->second->state != preview_state::ready) {
    return nullptr;
  }
  return entry->second->image;
}

class component final : public client_component {
  DEFINE_COMPONENT_NAME("mod_previews");

public:
  void post_unpack() override {
    static constexpr const luaL_Reg ModsMenuLibrary[] = {
        lua_state::luaL_LoggedReg<"ModsMenu", "GetPreview",
                                  get_preview_image>(),
        lua_state::luaL_LoggedReg<"ModsMenu", "Delete", delete_mod>(),
        {nullptr, nullptr},
    };
    lua_state::register_library("ModsMenu", ModsMenuLibrary);
    scheduler::loop(request_installed_mods, scheduler::pipeline::main, 1s);
  }
};
} // namespace mod_previews

REGISTER_COMPONENT(mod_previews::component)
