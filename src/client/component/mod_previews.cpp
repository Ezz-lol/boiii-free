#include <std_include.hpp>

#include "launcher/launcher.hpp"
#include "mod_previews.hpp"
#include "scheduler.hpp"

#include <component/lua/lua_state.hpp>

#include <loader/component_loader.hpp>

#include <utils/http.hpp>
#include <utils/image.hpp>
#include <utils/io.hpp>

#include <game/impl/ugc/ugc.hpp>

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
typedef std::unordered_map<std::string, std::unique_ptr<preview>> previewMap_t;
previewMap_t previews;

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
  if (!ugc_name.empty() && std::ranges::all_of(ugc_name, ::isdigit)) {
    const std::string url = launcher::get_steam_workshop_preview_url(ugc_name);
    if (!url.empty()) {
      return utils::http::get_data(url);
    }
  }
  return std::nullopt;
}

bool cache_preview(const std::string &ugc_name,
                   const std::filesystem::path &path) {
  const std::optional<std::string> data = fetch_preview_data(ugc_name);
  if (data) {
    try {
      const utils::image::image decoded = utils::image::load_image(*data);
      const int32_t width = static_cast<int32_t>(decoded.width);
      const int32_t height = static_cast<int32_t>(decoded.height);
      const int32_t scaled_width = std::min(width, PREVIEW_WIDTH);
      const int32_t scaled_height = std::max(1, height * scaled_width / width);
      std::string scaled(static_cast<size_t>(scaled_width) * scaled_height * 4,
                         '\0');
      if (stbir_resize_uint8_srgb(
              reinterpret_cast<const uint8_t *>(decoded.data.data()), width,
              height, 0, reinterpret_cast<uint8_t *>(scaled.data()),
              scaled_width, scaled_height, 0, STBIR_RGBA)) {
        std::filesystem::create_directories(path.parent_path());
        return stbi_write_png(path.string().c_str(), scaled_width,
                              scaled_height, 4, scaled.data(),
                              scaled_width * 4) != 0;
      }
    } catch (const std::exception &) {
    }
  }

  return false;
}

void request_download(const std::string &ugc_name) {
  std::unique_ptr<preview> &entry = previews[ugc_name];
  if (!entry) {
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
}

void load_texture(const std::string &ugc_name, preview &entry) {
  const game::gfx::GfxImage *base =
      game::db::xasset::DB_FindXAssetHeader(XAssetType::IMAGE,
                                            "img_t7_mod_preview", false, 0)
          .image;
  if (base) {
    const game::gfx::GfxTexture texture = game::gfx::Gfx_LoadTextureFromPng(
        cached_preview_path(ugc_name).string().c_str());
    if (texture.basemap && game::nonnull(game::db::xasset::pool::s_assetPools
                                             ->pools[+XAssetType::IMAGE]
                                             .freeHead)) {
      entry.image = static_cast<game::gfx::GfxImage *>(
          game::db::xasset::pool::DB_AssetPoolAlloc(XAssetType::IMAGE));
      *entry.image = *base;
      entry.image->texture = texture;
      entry.image->name = entry.image_name.c_str();
      entry.state = preview_state::ready;
    } else {
      entry.state = preview_state::missing;
    }
  }
}

void request_installed_mods() {
  std::scoped_lock lock(previews_mutex);
  for (const game::ugc::UGCItem &item : game::ugc::modsPool.iter()) {
    request_download(item.publisherId);
  }
}

luaReturnCount_e get_preview_image(lua_State *luaVM) {
  const char *ugc_name = lua_tostring(luaVM, 1);
  if (ugc_name && ugc_name[0]) {
    std::scoped_lock lock(previews_mutex);
    request_download(ugc_name);
    preview &entry = *previews[ugc_name];
    if (entry.state == preview_state::downloaded) {
      load_texture(ugc_name, entry);
    }
    if (entry.state == preview_state::ready) {
      lua_pushstring(luaVM, entry.image_name);
      return luaReturnCount_e::ONE;
    }
  }

  lua_pushnil(luaVM);
  return luaReturnCount_e::ONE;
}

luaReturnCount_e delete_mod(lua_State *luaVM) {
  const char *ugc_name = lua_tostring(luaVM, 1);
  std::string error = "This mod could not be found.";
  if (game::ugc::UGC_ActiveMod_Loaded() || game::com::Com_IsInGame()) {
    error = "Unload mods before deleting one.";
  } else if (ugc_name) {
    const game::ugc::OptionalUGCItemRef mod =
        game::ugc::UGC_GetModByPublisherId(ugc_name);
    if (mod.has_value()) {
      if (std::string_view(mod->get().publisherId) == ugc_name) {
        std::filesystem::path folder =
            std::string_view(mod->get().absolutePathZoneFiles);
        if (folder.filename() == "zone") {
          folder = folder.parent_path();
        }
        if (launcher::workshop_remove_by_path(folder.string(), error)) {
          lua_pushnil(luaVM);
          return luaReturnCount_e::ONE;
        }
      }
    } else {
      error = "Mod could not be found.";
    }
  }
  lua_pushstring(luaVM, error);
  return luaReturnCount_e::ONE;
}

} // namespace

game::gfx::GfxImage *find_image(const char *name) {
  if (name && std::string_view(name).starts_with(IMAGE_PREFIX)) {
    std::scoped_lock lock(previews_mutex);
    const previewMap_t::iterator entry = previews.find(
        std::string(std::string_view(name).substr(IMAGE_PREFIX.size())));
    if (entry != previews.end() &&
        entry->second->state == preview_state::ready) {
      return entry->second->image;
    }
  }
  return nullptr;
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
