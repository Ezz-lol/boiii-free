#include <std_include.hpp>

#include "workshop.hpp"
#include <loader/component_loader.hpp>

#include <component/scheduler.hpp>
#include <component/toast.hpp>

#include <utils/io.hpp>
#include <utils/string.hpp>

#include <rapidjson/document.h>

namespace workshop {
namespace {
constexpr std::chrono::seconds CACHE_TTL = 3s;

std::mutex cache_mutex;
std::vector<installed_item> cache;
std::chrono::steady_clock::time_point cache_time{};
bool cache_valid = false;

std::mutex reload_mutex;
std::vector<std::string> finished_in_match;

std::filesystem::path root_for(const std::string_view &kind) {
  return game::get_game_path() / (kind == "Map" ? "usermaps" : "mods");
}

std::optional<rapidjson::Document>
read_workshop_json(const std::filesystem::path &dir) {
  for (const std::filesystem::path &file :
       {dir / "workshop.json", dir / "zone" / "workshop.json"}) {
    std::string json;
    rapidjson::Document doc;
    if (utils::io::read_file(file.string(), &json) &&
        !doc.Parse(json.c_str()).HasParseError() && doc.IsObject()) {
      return doc;
    }
  }
  return std::nullopt;
}

std::string json_string(const rapidjson::Document &doc,
                        std::initializer_list<const char *> keys) {
  for (const char *key : keys) {
    const rapidjson::Document::ConstMemberIterator it = doc.FindMember(key);
    if (it != doc.MemberEnd() && it->value.IsString() &&
        it->value.GetStringLength()) {
      return it->value.GetString();
    }
  }
  return {};
}

std::string detect_kind(const std::filesystem::path &dir) {
  if (const std::optional<rapidjson::Document> doc = read_workshop_json(dir)) {
    const std::string kind =
        normalize_kind(json_string(*doc, {"Type", "type"}));
    if (!kind.empty())
      return kind;
  }

  bool has_map_zone = false;
  std::error_code ec;
  for (const std::filesystem::path &zone_dir : {dir / "zone", dir}) {
    for (const std::filesystem::directory_entry &entry :
         std::filesystem::directory_iterator(zone_dir, ec)) {
      const std::string name =
          utils::string::to_lower(entry.path().filename().string());
      if (name == "mod.ff" || name == "core_mod.ff")
        return "Mod";
      has_map_zone |= entry.path().extension() == ".ff" &&
                      (name.starts_with("zm_") || name.starts_with("mp_") ||
                       name.starts_with("cp_"));
    }
  }
  return has_map_zone ? "Map" : "";
}

void scan_root(const std::string_view &kind,
               std::vector<installed_item> &items) {
  std::error_code ec;
  for (const std::filesystem::directory_entry &entry :
       std::filesystem::directory_iterator(root_for(kind), ec)) {
    if (!entry.is_directory(ec) || entry.path().extension() == ".deleting")
      continue;
    const std::optional<rapidjson::Document> doc =
        read_workshop_json(entry.path());
    if (!doc)
      continue;

    installed_item item{};
    item.folder = entry.path().filename().string();
    item.kind = kind;
    item.path = entry.path().generic_string();
    item.id = json_string(*doc, {"PublisherID", "PublishedFileId"});
    item.title = json_string(*doc, {"Title", "FolderName"});
    item.description = clean_description(json_string(*doc, {"Description"}));
    if (item.id.empty())
      item.id = item.folder;
    if (item.title.empty())
      item.title = item.folder;
    item.file_size = compute_folder_size_bytes(entry.path());
    items.push_back(std::move(item));
  }
}

void invalidate_cache() {
  std::lock_guard lock(cache_mutex);
  cache_valid = false;
}

bool is_loaded_by_game(const installed_item &item) {
  const game::ugc::UGCItem *active = item.kind == "Mod"
                                         ? game::ugc::active_mod.get()
                                         : game::ugc::active_usermap.get();
  return (active->publisherId[0] && (item.id == active->publisherId ||
                                     item.folder == active->publisherId)) ||
         (active->internalName[0] && item.folder == active->internalName);
}

bool is_in_match() {
  return game::com::Com_IsInGame() && !game::com::Com_IsRunningUILevel();
}

void reload_now() {
  invalidate_cache();
  game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0, "userContentReload\n");
}
} // namespace

std::string normalize_kind(const std::string_view &kind) {
  const std::string k = utils::string::to_lower(std::string(kind));
  if (k.starts_with("map") || k.starts_with("usermap") ||
      k.starts_with("custom map"))
    return "Map";
  if (k.starts_with("mod"))
    return "Mod";
  return {};
}

std::vector<installed_item> list_installed() {
  std::lock_guard lock(cache_mutex);
  if (!cache_valid ||
      std::chrono::steady_clock::now() - cache_time > CACHE_TTL) {
    cache.clear();
    scan_root("Map", cache);
    scan_root("Mod", cache);
    cache_time = std::chrono::steady_clock::now();
    cache_valid = true;
  }
  return cache;
}

std::filesystem::path find_installed(const std::string_view &id) {
  if (id.empty())
    return {};
  for (const installed_item &item : list_installed()) {
    if (item.id == id || item.folder == id)
      return item.path;
  }

  std::error_code ec;
  for (const std::filesystem::path &root :
       {root_for("Map"), root_for("Mod"),
        game::get_game_path().parent_path().parent_path() / "workshop" /
            "content" / game::APP_ID_STR}) {
    if (compute_folder_size_bytes(root / id) > 1024)
      return root / id;
  }
  return {};
}

std::filesystem::path install_folder(const std::filesystem::path &download,
                                     const std::string_view &id,
                                     const std::string_view &kind_hint) {
  std::string kind = detect_kind(download);
  if (kind.empty())
    kind = normalize_kind(kind_hint);
  if (kind.empty())
    kind = "Map";

  const std::filesystem::path existing = find_installed(id);
  const bool replaces = existing.parent_path() == root_for("Map") ||
                        existing.parent_path() == root_for("Mod");
  const std::filesystem::path root = root_for(kind);
  const std::filesystem::path target =
      root / (replaces ? existing.filename() : std::filesystem::path(id));

  std::error_code ec;
  if (replaces && existing != target) {
    std::filesystem::remove_all(existing, ec);
  }
  std::filesystem::remove_all(target, ec);
  std::filesystem::create_directories(root, ec);
  std::filesystem::rename(download, target, ec);
  if (ec) {
    ec.clear();
    std::filesystem::copy(download, target,
                          std::filesystem::copy_options::recursive, ec);
    if (ec) {
      printf("[ Workshop ] Could not install %s: %s\n", id.data(),
             ec.message().c_str());
      return {};
    }
    std::filesystem::remove_all(download, ec);
  }

  const std::filesystem::file_time_type::clock::time_point now =
      std::filesystem::file_time_type::clock::now();
  std::filesystem::last_write_time(target, now, ec);
  invalidate_cache();
  return target;
}

bool delete_installed(const std::string_view &id, std::string &error) {
  if (is_any_download_active() &&
      (queue::current_id().empty() || queue::current_id() == id)) {
    error = "Wait for the current download to finish.";
    return false;
  }

  invalidate_cache();
  bool found = false;
  for (const installed_item &item : list_installed()) {
    if (item.id != id && item.folder != id)
      continue;
    found = true;
    if (is_loaded_by_game(item)) {
      error = "Unload the mod/map before deleting it.";
      return false;
    }

    std::error_code ec;
    std::filesystem::path trash = std::filesystem::path(item.path);
    trash += ".deleting";
    std::filesystem::remove_all(trash, ec);
    std::filesystem::rename(item.path, trash, ec);
    if (ec) {
      error = "Files are in use. Restart the game and try again.";
      return false;
    }
    std::filesystem::remove_all(trash, ec);
  }

  if (!found) {
    error = "Item is not installed.";
    return false;
  }
  scheduler::once(reload_now, scheduler::main);
  return true;
}

void request_content_reload(const std::string_view &finished_title) {
  invalidate_cache();
  scheduler::once(
      [finished_title] {
        if (!is_in_match()) {
          reload_now();
          return;
        }
        std::lock_guard lock(reload_mutex);
        finished_in_match.push_back(std::string(finished_title));
      },
      scheduler::main);
}

class installed final : public client_component {
  DEFINE_COMPONENT_NAME("workshop_installed");

public:
  void post_unpack() override {
    scheduler::loop(
        [] {
          if (is_in_match())
            return;
          std::vector<std::string> titles;
          {
            std::lock_guard lock(reload_mutex);
            titles.swap(finished_in_match);
          }
          if (titles.empty())
            return;
          reload_now();
          toast::success("Workshop",
                         titles.size() == 1
                             ? titles[0] + " is now available."
                             : std::to_string(titles.size()) +
                                   " downloaded items are now available.");
        },
        scheduler::main, 2s);
  }
};
} // namespace workshop

REGISTER_COMPONENT(workshop::installed)
