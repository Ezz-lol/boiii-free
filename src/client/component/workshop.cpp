#include <cstring>
#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "workshop.hpp"

#include "game/utils.hpp"
#include "command.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>
#include <utils/http.hpp>
#include <utils/thread.hpp>
#include <utils/flags.hpp>
#include "steamcmd.hpp"
#include "fastdl.hpp"
#include "party.hpp"
#include "scheduler.hpp"
#include "download_overlay.hpp"
#include "toast.hpp"

#include <condition_variable>
#include <mutex>
#include <regex>
#include <unordered_map>
#include <shellapi.h>

using namespace game::db;
using XZoneName = xzone::XZoneName;

namespace workshop {
namespace {
std::thread download_thread{};

utils::hook::detour setup_server_map_hook;
utils::hook::detour load_usermap_hook;

static const std::unordered_map<std::string, std::string> dlc_links = {
    {"zm_zod", "https://forum.ezz.lol/topic/6/bo3-dlc"},
    {"zm_castle", "https://forum.ezz.lol/topic/6/bo3-dlc"},
    {"zm_island", "https://forum.ezz.lol/topic/6/bo3-dlc"},
    {"zm_stalingrad", "https://forum.ezz.lol/topic/6/bo3-dlc"},
    {"zm_genesis", "https://forum.ezz.lol/topic/6/bo3-dlc"},
    {"zm_cosmodrome", "https://forum.ezz.lol/topic/6/bo3-dlc"},
    {"zm_theater", "https://forum.ezz.lol/topic/6/bo3-dlc"},
    {"zm_moon", "https://forum.ezz.lol/topic/6/bo3-dlc"},
    {"zm_prototype", "https://forum.ezz.lol/topic/6/bo3-dlc"},
    {"zm_tomb", "https://forum.ezz.lol/topic/6/bo3-dlc"},
    {"zm_temple", "https://forum.ezz.lol/topic/6/bo3-dlc"},
    {"zm_sumpf", "https://forum.ezz.lol/topic/6/bo3-dlc"},
    {"zm_factory", "https://forum.ezz.lol/topic/6/bo3-dlc"},
    {"zm_asylum", "https://forum.ezz.lol/topic/6/bo3-dlc"}};
std::mutex dlc_mutex;
std::condition_variable dlc_cv;
std::string pending_dlc_map;
std::atomic<bool> dlc_thread_shutdown{false};
std::thread dlc_popup_thread_obj;

void dlc_popup_thread_func() {
  while (true) {
    std::unique_lock lock(dlc_mutex);
    dlc_cv.wait_for(lock, std::chrono::milliseconds(200), [] {
      return dlc_thread_shutdown.load() || !pending_dlc_map.empty();
    });
    if (dlc_thread_shutdown.load())
      break;
    if (pending_dlc_map.empty())
      continue;
    std::string map = std::move(pending_dlc_map);
    pending_dlc_map.clear();
    lock.unlock();

    const auto it = dlc_links.find(map);
    if (it != dlc_links.end()) {
      const std::string link = it->second;
      const std::string map_copy = map;
      scheduler::once(
          [map_copy, link] {
            game::ui::UI_OpenErrorPopupWithMessage(
                0, game::ERROR_UI,
                utils::string::va(
                    "Missing DLC map: %s\n\nOpening download page...\n%s",
                    map_copy.c_str(), link.c_str()));
          },
          scheduler::main);
      ShellExecuteA(nullptr, "open", link.c_str(), nullptr, nullptr,
                    SW_SHOWNORMAL);
    }
  }
}

void queue_dlc_popup(const std::string &mapname) {
  std::lock_guard lock(dlc_mutex);
  pending_dlc_map = mapname;
  dlc_cv.notify_one();
}

bool has_mod(const std::string &pub_id) {
  for (unsigned int i = 0; i < game::ugc::modsPool.count; ++i) {
    const game::ugc::WorkshopData *mod_data = &game::ugc::modsPool.data[i];
    if (mod_data->publisherId == pub_id || mod_data->folderName == pub_id) {
      return true;
    }
  }

  return false;
}

std::string resolve_mod_workshop_id(const std::string &mod_name) {
  for (unsigned int i = 0; i < game::ugc::modsPool.count; ++i) {
    const game::ugc::WorkshopData *mod_data = &game::ugc::modsPool.data[i];
    if (mod_data->folderName == mod_name &&
        utils::string::is_numeric(mod_data->publisherId)) {
      return mod_data->publisherId;
    }
  }

  std::error_code ec;
  std::filesystem::path mods_dir("mods");
  if (std::filesystem::exists(mods_dir, ec)) {
    for (const auto &entry :
         std::filesystem::directory_iterator(mods_dir, ec)) {
      if (!entry.is_directory(ec))
        continue;

      auto ws_json = entry.path() / "zone" / "workshop.json";
      if (!std::filesystem::exists(ws_json, ec))
        continue;

      const auto json_str = utils::io::read_file(ws_json.string());
      if (json_str.empty())
        continue;

      rapidjson::Document doc;
      if (doc.Parse(json_str.c_str()).HasParseError() || !doc.IsObject())
        continue;

      auto folder_it = doc.FindMember("FolderName");
      if (folder_it != doc.MemberEnd() && folder_it->value.IsString()) {
        if (std::string(folder_it->value.GetString()) == mod_name) {
          auto pub_it = doc.FindMember("PublishedFileId");
          if (pub_it != doc.MemberEnd() && pub_it->value.IsString()) {
            std::string pfid = pub_it->value.GetString();
            if (utils::string::is_numeric(pfid.data()))
              return pfid;
          }
          auto pubid_it = doc.FindMember("PublisherID");
          if (pubid_it != doc.MemberEnd() && pubid_it->value.IsString()) {
            std::string pid = pubid_it->value.GetString();
            if (utils::string::is_numeric(pid.data()))
              return pid;
          }
        }
      }
    }
  }

  return {};
}

uint32_t get_xzone_index_by_name(const char *zone_name) {
  XZoneName *g_zoneNames =
      reinterpret_cast<XZoneName *>(xzone::g_zoneNames.get());
  for (uint32_t zoneIdx = 0; zoneIdx < *(xzone::g_zoneCount.get()); zoneIdx++) {
    XZoneName *zoneInfo = &g_zoneNames[zoneIdx];
    if (std::strcmp(zoneInfo->name, zone_name) == 0) {
      return zoneIdx;
    }
  }

  return 0xFFFFFFFF; // Invalid index
}

bool unload_xzone_by_name(const char *zone_name, bool createDefault,
                          bool suppressSync) {
  uint32_t zoneIdx = get_xzone_index_by_name(zone_name);
  if (zoneIdx != 0xFFFFFFFF) {
    load::DB_UnloadXZone(zoneIdx, createDefault, suppressSync ? 1 : 0);
    return true;
  }
  return false; // Zone not found
}

void clear_loaded_usermap() {
  // Set first byte of each to null, terminating string immediately - sets each
  // to an empty string
  *(game::ugc::usermap_publisher_id.get()) = 0;
  *(game::ugc::usermap_title.get()) = 0;
  *(game::ugc::internal_usermap_id.get()) = 0; // e.g. zm_*
}

void setup_server_map_stub(game::LocalClientNum_t localClientNum,
                           const char *map, const char *gametype) {
  const char *loaded_mod_id = game::ugc::getPublisherIdFromLoadedMod();
  const bool is_usermap =
      utils::string::is_numeric(map) || !get_usermap_publisher_id(map).empty();
  const bool mod_loaded = std::strlen(loaded_mod_id) > 0;
  const bool usermaps_mod_loaded =
      mod_loaded && std::strcmp(loaded_mod_id, "usermaps") == 0;

  if (is_usermap) {

    if (!mod_loaded) {
      game::ugc::loadMod(localClientNum, "usermaps", false);
    }
  } else {
    clear_loaded_usermap();

    if (usermaps_mod_loaded) {
      game::ugc::loadMod(localClientNum, "", false);
    }

    unload_xzone_by_name("zm_levelcommon", false, false);
  }

  setup_server_map_hook.invoke(localClientNum, map, gametype);
}

void load_workshop_data(game::ugc::WorkshopData *item) {
  const auto base_path = item->absolutePathZoneFiles;
  const auto path = utils::string::va("%s/workshop.json", base_path);
  const auto json_str = utils::io::read_file(path);

  if (json_str.empty()) {
    printf("[ Workshop ] workshop.json has not been found in folder:\n%s\n",
           path);
    return;
  }

  rapidjson::Document doc;
  const rapidjson::ParseResult parse_result = doc.Parse(json_str);

  if (parse_result.IsError() || !doc.IsObject()) {
    printf("[ Workshop ] Unable to parse workshop.json from folder:\n%s\n",
           path);
    return;
  }

  if (!doc.HasMember("Title") || !doc.HasMember("Description") ||
      !doc.HasMember("FolderName") || !doc.HasMember("PublisherID")) {
    printf("[ Workshop ] workshop.json is invalid:\n%s\n", path);
    return;
  }

  utils::string::copy(item->title, doc["Title"].GetString());
  utils::string::copy(item->description, doc["Description"].GetString());
  utils::string::copy(item->folderName, doc["FolderName"].GetString());
  utils::string::copy(item->publisherId, doc["PublisherID"].GetString());
  item->publisherIdInteger = std::strtoul(item->publisherId, nullptr, 10);
}

void populate_workshop_paths(game::ugc::WorkshopData *item,
                             const std::filesystem::path &content_folder,
                             const game::ugc::ZoneType type) {
  std::memset(item, 0, sizeof(game::ugc::WorkshopData));

  const std::filesystem::path zone_path = content_folder / "zone";
  const std::filesystem::path relative_zone_path =
      std::filesystem::path(type == game::ugc::ZoneType::MOD ? "mods"
                                                             : "usermaps") /
      content_folder.filename() / "zone";

  utils::string::copy(item->contentPathToZoneFiles,
                      relative_zone_path.generic_string().c_str());
  utils::string::copy(item->absolutePathContentFolder,
                      content_folder.generic_string().c_str());
  utils::string::copy(item->absolutePathZoneFiles,
                      zone_path.generic_string().c_str());
  item->unk = 1;
  item->unk2 = 0;
  item->unk3 = 0;
  item->unk4 = 0;
  item->type = type;
}

void supplement_mods_from_disk() {
  if (game::ugc::modsPool.count != 0) {
    return;
  }

  std::error_code ec;
  const auto mods_dir = std::filesystem::current_path() / "mods";
  if (!std::filesystem::exists(mods_dir, ec)) {
    return;
  }

  unsigned int count = 0;
  for (const auto &entry : std::filesystem::directory_iterator(mods_dir, ec)) {
    if (ec || !entry.is_directory(ec)) {
      continue;
    }

    const auto zone_dir = entry.path() / "zone";
    const auto workshop_json = zone_dir / "workshop.json";
    if (!std::filesystem::exists(zone_dir, ec) ||
        !std::filesystem::exists(workshop_json, ec)) {
      continue;
    }

    game::ugc::WorkshopData *mod_data = &game::ugc::modsPool.data[count];
    populate_workshop_paths(mod_data, entry.path(), game::ugc::ZoneType::MOD);
    load_workshop_data(mod_data);
    ++count;
  }

  if (count) {
    game::ugc::modsPool.count = count;
    printf("[ Workshop ] Supplemented %u mods from disk fallback\n", count);
  }
}

void load_usermap_content_stub(int32_t *usermaps_count, int type) {
  utils::hook::invoke<void>(game::select(0x1420D6430, 0x1404E2360),
                            usermaps_count, type);

  for (unsigned int i = 0; i < game::ugc::usermapsPool.count; ++i) {
    game::ugc::WorkshopData *usermap_data = &game::ugc::usermapsPool.data[i];

    if (std::strcmp(usermap_data->folderName, usermap_data->title) != 0) {
      continue;
    }

    load_workshop_data(usermap_data);
  }
}

void load_mod_content_stub(int32_t *mods_count, int type) {
  utils::hook::invoke<void>(game::select(0x1420D6430, 0x1404E2360), mods_count,
                            type);
  supplement_mods_from_disk();

  for (unsigned int i = 0; i < game::ugc::modsPool.count; ++i) {
    game::ugc::WorkshopData *mod_data = &game::ugc::modsPool.data[i];

    if (std::strcmp(mod_data->folderName, mod_data->title) != 0) {
      continue;
    }

    load_workshop_data(mod_data);
  }
}

game::ugc::WorkshopData *load_usermap_stub(const char *map_arg) {
  std::string pub_id = map_arg;
  if (!utils::string::is_numeric(map_arg)) {
    pub_id = get_usermap_publisher_id(map_arg);
  }

  return load_usermap_hook.invoke<game::ugc::WorkshopData *>(pub_id.data());
}

bool has_workshop_item_stub(int type, const char *map, int a3) {
  std::string pub_id = map;
  if (!utils::string::is_numeric(map)) {
    pub_id = get_usermap_publisher_id(map);
  }

  return utils::hook::invoke<bool>(0x1420D6380_g, type, pub_id.data(), a3);
}

const char *va_mods_path_stub(const char *fmt, const char *root_dir,
                              const char *mods_dir, const char *dir_name) {
  const auto original_path =
      utils::string::va(fmt, root_dir, mods_dir, dir_name);

  if (utils::io::directory_exists(original_path)) {
    return original_path;
  }

  return utils::string::va("%s/%s/%s", root_dir, mods_dir, dir_name);
}

const char *va_user_content_path_stub(const char *fmt, const char *root_dir,
                                      const char *user_content_dir) {
  const auto original_path = utils::string::va(fmt, root_dir, user_content_dir);

  if (utils::io::directory_exists(original_path)) {
    return original_path;
  }

  return utils::string::va("%s/%s", root_dir, user_content_dir);
}
} // namespace

std::string get_mod_resized_name() {
  const std::string loaded_mod_id = game::ugc::getPublisherIdFromLoadedMod();

  if (loaded_mod_id == "usermaps" || loaded_mod_id.empty()) {
    return loaded_mod_id;
  }

  std::string mod_name = loaded_mod_id;

  for (unsigned int i = 0; i < game::ugc::modsPool.count; ++i) {
    const game::ugc::WorkshopData *mod_data = &game::ugc::modsPool.data[i];

    if (mod_data->publisherId == loaded_mod_id) {
      mod_name = mod_data->title;
      break;
    }
  }

  if (mod_name.size() > 31) {
    mod_name.resize(31);
  }

  return mod_name;
}

std::string get_usermap_publisher_id(const std::string &zone_name) {
  for (unsigned int i = 0; i < game::ugc::usermapsPool.count; ++i) {
    const game::ugc::WorkshopData *usermap_data =
        &game::ugc::usermapsPool.data[i];
    if (usermap_data->folderName == zone_name) {
      if (!utils::string::is_numeric(usermap_data->publisherId)) {
        printf("[ Workshop ] WARNING: The publisherId is not numerical. You "
               "might have set your usermap folder incorrectly!\n%s\n",
               usermap_data->absolutePathZoneFiles);
      }

      return usermap_data->publisherId;
    }
  }

  return {};
}

int get_workshop_retry_attempts() {
  const int val = game::get_dvar_int("workshop_retry_attempts");
  if (val < 1)
    return 1;
  if (val > 1000)
    return 1000;
  return val;
}

std::string get_mod_publisher_id() {
  const std::string loaded_mod_id = game::ugc::getPublisherIdFromLoadedMod();

  if (loaded_mod_id == "usermaps" || loaded_mod_id.empty()) {
    return loaded_mod_id;
  }

  if (!utils::string::is_numeric(loaded_mod_id)) {
    printf("[ Workshop ] WARNING: The publisherId: %s, is not numerical you "
           "might have set your mod folder incorrectly!\n",
           loaded_mod_id.data());
  }

  return loaded_mod_id;
}

bool is_dlc_map(const std::string &mapname) {
  return mapname == "zm_zod" || mapname == "zm_castle" ||
         mapname == "zm_island" || mapname == "zm_stalingrad" ||
         mapname == "zm_genesis" || mapname == "zm_cosmodrome" ||
         mapname == "zm_theater" || mapname == "zm_moon" ||
         mapname == "zm_prototype" || mapname == "zm_tomb" ||
         mapname == "zm_temple" || mapname == "zm_sumpf" ||
         mapname == "zm_factory" || mapname == "zm_asylum";
}

std::atomic<bool> downloading_workshop_item{false};
std::atomic<bool> launcher_downloading{false};

bool is_any_download_active() {
  return downloading_workshop_item.load() || launcher_downloading.load() ||
         fastdl::is_downloading();
}

static std::mutex reconnect_mutex;
static std::string pending_mod_reconnect_address;
static std::string pending_download_reconnect_address;

void set_pending_mod_reconnect(const std::string &address) {
  std::lock_guard lock(reconnect_mutex);
  pending_mod_reconnect_address = address;
}

std::string get_pending_mod_reconnect() {
  std::lock_guard lock(reconnect_mutex);
  auto addr = std::move(pending_mod_reconnect_address);
  pending_mod_reconnect_address.clear();
  return addr;
}

void set_pending_download_reconnect(const std::string &address) {
  std::lock_guard lock(reconnect_mutex);
  // Don't overwrite if a download is already active, preserve the original
  // server
  if (pending_download_reconnect_address.empty() || !is_any_download_active()) {
    pending_download_reconnect_address = address;
  }
}

std::string get_pending_download_reconnect() {
  std::lock_guard lock(reconnect_mutex);
  auto addr = std::move(pending_download_reconnect_address);
  pending_download_reconnect_address.clear();
  return addr;
}

std::uint64_t compute_folder_size_bytes(const std::filesystem::path &folder) {
  std::error_code ec;
  if (!std::filesystem::exists(folder, ec))
    return 0;
  std::uint64_t total = 0;
  for (const auto &entry : std::filesystem::recursive_directory_iterator(
           folder, std::filesystem::directory_options::skip_permission_denied,
           ec)) {
    if (ec)
      break;
    if (!entry.is_regular_file(ec))
      continue;
    total += static_cast<std::uint64_t>(
        std::filesystem::file_size(entry.path(), ec));
    if (ec)
      break;
  }
  return total;
}

std::string human_readable_size(std::uint64_t bytes) {
  const char *suffixes[] = {"B", "KB", "MB", "GB", "TB"};
  double value = static_cast<double>(bytes);
  int idx = 0;
  while (value >= 1024.0 && idx < 4) {
    value /= 1024.0;
    ++idx;
  }
  char buf[64]{};
  std::snprintf(buf, sizeof(buf), "%.2f %s", value, suffixes[idx]);
  return buf;
}

std::uint64_t parse_human_size_to_bytes(const std::string &text) {
  std::smatch m;
  std::regex re(R"((\d+(?:\.\d+)?)\s*(B|KB|MB|GB|TB))", std::regex::icase);
  if (!std::regex_search(text, m, re) || m.size() < 3)
    return 0;
  const double value = std::stod(m[1].str());
  std::string unit = m[2].str();
  for (auto &c : unit)
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

  double mul = 1.0;
  if (unit == "KB")
    mul = 1024.0;
  else if (unit == "MB")
    mul = 1024.0 * 1024.0;
  else if (unit == "GB")
    mul = 1024.0 * 1024.0 * 1024.0;
  else if (unit == "TB")
    mul = 1024.0 * 1024.0 * 1024.0 * 1024.0;

  const auto bytes = value * mul;
  if (bytes <= 0.0)
    return 0;
  return static_cast<std::uint64_t>(bytes);
}

std::uint64_t scrape_workshop_file_size_bytes(const std::string &workshop_id) {
  try {
    utils::http::headers h;
    h["User-Agent"] =
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36";
    h["Accept"] = "text/html";
    h["Accept-Language"] = "en-US,en;q=0.9";
    h["Referer"] = "https://steamcommunity.com/app/311210/workshop/";

    const auto url = "https://steamcommunity.com/sharedfiles/filedetails/?id=" +
                     workshop_id + "&searchtext=";
    const auto resp = utils::http::get_data(url, h, {}, 2);
    if (!resp || resp->empty())
      return 0;

    const std::string &html = *resp;

    {
      std::regex re(
          R"(detailsStatRight[^>]*>\s*([\d,\.]+\s*(?:B|KB|MB|GB|TB))\s*<)",
          std::regex::icase);
      std::smatch m;
      if (std::regex_search(html, m, re) && m.size() >= 2) {
        std::string size_text = m[1].str();
        size_text.erase(std::remove(size_text.begin(), size_text.end(), ','),
                        size_text.end());
        const auto bytes = parse_human_size_to_bytes(size_text);
        if (bytes > 0)
          return bytes;
      }
    }
    {
      std::regex re(R"(File\s*Size\s*<\/div>\s*<div[^>]*>([^<]+)<)",
                    std::regex::icase);
      std::smatch m;
      if (std::regex_search(html, m, re) && m.size() >= 2) {
        std::string size_text = m[1].str();
        size_text.erase(std::remove(size_text.begin(), size_text.end(), ','),
                        size_text.end());
        const auto bytes = parse_human_size_to_bytes(size_text);
        if (bytes > 0)
          return bytes;
      }
    }
    {
      std::regex re(R"(File\s*Size[^\d]*(\d+(?:[,.]\d+)?)\s*(B|KB|MB|GB|TB))",
                    std::regex::icase);
      std::smatch m;
      if (std::regex_search(html, m, re) && m.size() >= 3) {
        std::string num = m[1].str();
        num.erase(std::remove(num.begin(), num.end(), ','), num.end());
        const auto bytes = parse_human_size_to_bytes(num + " " + m[2].str());
        if (bytes > 0)
          return bytes;
      }
    }
    return 0;
  } catch (...) {
    return 0;
  }
}

workshop_info get_steam_workshop_info(const std::string &workshop_id) {
  workshop_info info{};
  if (workshop_id.empty())
    return info;
  try {
    const std::string body = "itemcount=1&publishedfileids[0]=" + workshop_id;
    const auto resp = utils::http::post_data(
        "https://api.steampowered.com/ISteamRemoteStorage/"
        "GetPublishedFileDetails/v1/",
        body, 10);
    if (!resp || resp->empty())
      return info;

    rapidjson::Document doc;
    if (doc.Parse(resp->c_str()).HasParseError() || !doc.IsObject())
      return info;
    auto resp_it = doc.FindMember("response");
    if (resp_it == doc.MemberEnd() || !resp_it->value.IsObject())
      return info;
    auto details_it = resp_it->value.FindMember("publishedfiledetails");
    if (details_it == resp_it->value.MemberEnd() ||
        !details_it->value.IsArray() || details_it->value.Empty())
      return info;
    const auto &first = details_it->value[0];
    if (!first.IsObject())
      return info;

    auto title_it = first.FindMember("title");
    if (title_it != first.MemberEnd() && title_it->value.IsString())
      info.title = title_it->value.GetString();

    auto size_it = first.FindMember("file_size");
    if (size_it != first.MemberEnd()) {
      if (size_it->value.IsUint64())
        info.file_size = size_it->value.GetUint64();
      else if (size_it->value.IsInt64())
        info.file_size = static_cast<std::uint64_t>(size_it->value.GetInt64());
      else if (size_it->value.IsUint())
        info.file_size = size_it->value.GetUint();
      else if (size_it->value.IsInt())
        info.file_size = static_cast<std::uint64_t>(size_it->value.GetInt());
      else if (size_it->value.IsString())
        info.file_size = static_cast<std::uint64_t>(
            std::strtoull(size_it->value.GetString(), nullptr, 10));
      else if (size_it->value.IsDouble())
        info.file_size = static_cast<std::uint64_t>(size_it->value.GetDouble());
      else if (size_it->value.IsNumber())
        info.file_size = static_cast<std::uint64_t>(size_it->value.GetDouble());
    }

    if (info.file_size == 0) {
      const auto scraped = scrape_workshop_file_size_bytes(workshop_id);
      if (scraped > 0)
        info.file_size = scraped;
    }
  } catch (...) {
    const auto scraped = scrape_workshop_file_size_bytes(workshop_id);
    if (scraped > 0)
      info.file_size = scraped;
  }
  return info;
}

bool check_valid_usermap_id(const std::string &mapname,
                            const std::string &pub_id,
                            const std::string &workshop_id,
                            const std::string &base_url) {
  if (!DB_FileExists(mapname.data(), 0) && pub_id.empty()) {
    if (is_dlc_map(mapname.data())) {
      queue_dlc_popup(mapname);
      return false;
    }

    if (downloading_workshop_item.load() || launcher_downloading.load() ||
        fastdl::is_downloading()) {
      scheduler::once(
          [] {
            game::ui::UI_OpenErrorPopupWithMessage(
                0, game::ERROR_UI,
                "You are already downloading a map in the background. You can "
                "download only one item at a time.");
          },
          scheduler::main);
      return false;
    }

    if (!base_url.empty()) {
      fastdl::download_context context{};
      context.mapname = mapname;
      context.pub_id = workshop_id.empty() ? mapname : workshop_id;
      context.map_path = "./usermaps/" + mapname;
      context.base_url = base_url;
      context.success_callback = []() {
        scheduler::once([] { game::ugc::reloadUserContent(); },
                        scheduler::main);
      };
      printf("[ Workshop ] Server has FastDL, attempting download for %s from "
             "%s\n",
             mapname.data(), base_url.data());
      fastdl::start_map_download(context);
      return false;
    }

    if (utils::string::is_numeric(mapname.data())) {
      const std::string id_copy = mapname;
      const auto ws_info = get_steam_workshop_info(id_copy);
      std::string confirm_msg =
          utils::string::va("Usermap '%s' was not found.\n", id_copy.c_str());
      if (!ws_info.title.empty())
        confirm_msg += "Title: " + ws_info.title + "\n";
      if (ws_info.file_size > 0)
        confirm_msg += "Size: " + human_readable_size(ws_info.file_size) + "\n";
      confirm_msg += "\nDo you want to download it from the Steam Workshop?";
      download_overlay::show_confirmation(
          "Download Map?", confirm_msg, [id_copy] {
            download_thread = utils::thread::create_named_thread(
                "workshop_download", steamcmd::initialize_download, id_copy,
                std::string("Map"));
            download_thread.detach();
          });
    } else if (!workshop_id.empty() &&
               utils::string::is_numeric(workshop_id.data())) {
      const std::string id_copy = workshop_id;
      const std::string name_copy = mapname;
      const auto ws_info = get_steam_workshop_info(id_copy);
      std::string confirm_msg =
          utils::string::va("Usermap '%s' was not found.\n", name_copy.c_str());
      if (!ws_info.title.empty())
        confirm_msg += "Title: " + ws_info.title + "\n";
      if (ws_info.file_size > 0)
        confirm_msg += "Size: " + human_readable_size(ws_info.file_size) + "\n";
      confirm_msg += "\nDo you want to download it from the Steam Workshop?";
      download_overlay::show_confirmation(
          "Download Map?", confirm_msg, [id_copy] {
            download_thread = utils::thread::create_named_thread(
                "workshop_download", steamcmd::initialize_download, id_copy,
                std::string("Map"));
            download_thread.detach();
          });
    } else {
      const std::string name_copy = mapname;
      scheduler::once(
          [name_copy] {
            game::ui::UI_OpenErrorPopupWithMessage(
                0, game::ERROR_UI,
                utils::string::va(
                    "Missing usermap: %s\n\nThis server did not provide FastDL "
                    "and did not set workshop_id.\n\nSubscribe on Steam "
                    "Workshop, or ask the server to set sv_wwwBaseURL or "
                    "workshop_id.",
                    name_copy.c_str()));
          },
          scheduler::main);
    }
    return false;
  }
  return true;
}

bool check_valid_mod_id(const std::string &mod,
                        const std::string &workshop_id) {
  if (mod.empty() || mod == "usermaps") {
    return true;
  }

  if (!has_mod(mod)) {
    if (downloading_workshop_item.load() || launcher_downloading.load()) {
      scheduler::once(
          [] {
            game::ui::UI_OpenErrorPopupWithMessage(
                0, game::ERROR_UI,
                "You are already downloading a mod in the background. You can "
                "download only one item at a time.");
          },
          scheduler::main);
      return false;
    }

    if (utils::string::is_numeric(mod.data())) {
      const std::string id_copy = mod;
      const auto ws_info = get_steam_workshop_info(id_copy);
      std::string confirm_msg =
          utils::string::va("Mod '%s' was not found.\n", id_copy.c_str());
      if (!ws_info.title.empty())
        confirm_msg += "Title: " + ws_info.title + "\n";
      if (ws_info.file_size > 0)
        confirm_msg += "Size: " + human_readable_size(ws_info.file_size) + "\n";
      confirm_msg += "\nDo you want to download it from the Steam Workshop?";
      download_overlay::show_confirmation(
          "Download Mod?", confirm_msg, [id_copy] {
            download_thread = utils::thread::create_named_thread(
                "workshop_download", steamcmd::initialize_download, id_copy,
                std::string("Mod"));
            download_thread.detach();
          });
    } else if (!workshop_id.empty() &&
               utils::string::is_numeric(workshop_id.data())) {
      const std::string id_copy = workshop_id;
      const std::string name_copy = mod;
      const auto ws_info = get_steam_workshop_info(id_copy);
      std::string confirm_msg =
          utils::string::va("Mod '%s' was not found.\n", name_copy.c_str());
      if (!ws_info.title.empty())
        confirm_msg += "Title: " + ws_info.title + "\n";
      if (ws_info.file_size > 0)
        confirm_msg += "Size: " + human_readable_size(ws_info.file_size) + "\n";
      confirm_msg += "\nDo you want to download it from the Steam Workshop?";
      download_overlay::show_confirmation(
          "Download Mod?", confirm_msg, [id_copy] {
            download_thread = utils::thread::create_named_thread(
                "workshop_download", steamcmd::initialize_download, id_copy,
                std::string("Mod"));
            download_thread.detach();
          });
    } else {
      std::string resolved_id = resolve_mod_workshop_id(mod);
      if (!resolved_id.empty()) {
        const std::string name_copy = mod;
        const auto ws_info = get_steam_workshop_info(resolved_id);
        std::string confirm_msg = utils::string::va(
            "Mod '%s' was not found.\nResolved workshop ID: %s\n",
            name_copy.c_str(), resolved_id.c_str());
        if (!ws_info.title.empty())
          confirm_msg += "Title: " + ws_info.title + "\n";
        if (ws_info.file_size > 0)
          confirm_msg +=
              "Size: " + human_readable_size(ws_info.file_size) + "\n";
        confirm_msg += "\nDo you want to download it now?";
        download_overlay::show_confirmation(
            "Download Mod?", confirm_msg, [resolved_id] {
              download_thread = utils::thread::create_named_thread(
                  "workshop_download", steamcmd::initialize_download,
                  resolved_id, std::string("Mod"));
              download_thread.detach();
            });
      } else {
        const std::string name_copy = mod;
        scheduler::once(
            [name_copy] {
              game::ui::UI_OpenErrorPopupWithMessage(
                  0, game::ERROR_UI,
                  utils::string::va(
                      "Could not download: folder name is not numeric and "
                      "'workshop_id' dvar is empty.\nMod: %s\nSet workshop_id "
                      "or subscribe on Steam Workshop.",
                      name_copy.c_str()));
            },
            scheduler::main);
      }
    }
    return false;
  }

  return true;
}

bool mod_load_requires_fs_reinitialization(const std::string &mod_name) {
  return !mod_name.empty() && mod_name != "usermaps";
}

bool mod_switch_requires_fs_reinitialization(const std::string &current_mod,
                                             const std::string &new_mod) {
  return mod_load_requires_fs_reinitialization(current_mod) ||
         mod_load_requires_fs_reinitialization(new_mod);
}

void wait_for_mod_load() {
  game::ugc::ModLoadState *mod_load_state = game::ugc::mod_load_state.get();

  while (*mod_load_state == game::ugc::ModLoadState::LOADING) {
    std::this_thread::sleep_for(100ms);
  }
}

void setup_same_mod_as_host(game::LocalClientNum_t localClientNum,
                            const std::string &usermap,
                            const std::string &mod) {
  const std::string loaded_mod = game::ugc::getPublisherIdFromLoadedMod();
  if (loaded_mod != mod) {
    if (!usermap.empty() || !mod.empty()) {
      bool fs_reinit_required =
          mod_switch_requires_fs_reinitialization(loaded_mod, mod);
      game::ugc::loadMod(localClientNum, mod.data(), fs_reinit_required);
      if (fs_reinit_required) {
        wait_for_mod_load();
      }
    } else if (game::ugc::isModLoaded()) {
      bool fs_reinit_required =
          mod_switch_requires_fs_reinitialization(loaded_mod, "");
      game::ugc::loadMod(localClientNum, "", fs_reinit_required);
      if (fs_reinit_required) {
        wait_for_mod_load();
      }
    }
  }
}

static std::mutex reconnect_guard_mutex;
static std::string last_auto_reconnect_target;

void com_error_missing_map_stub(const char *file, int line, int code,
                                const char *fmt, ...) {
  const auto target = party::get_connect_host();
  if (target.type != game::net::NA_BAD) {
    const auto addr_str =
        utils::string::va("%i.%i.%i.%i:%hu", target.ipv4.a, target.ipv4.b,
                          target.ipv4.c, target.ipv4.d, target.port);

    {
      std::lock_guard lock(reconnect_guard_mutex);
      if (last_auto_reconnect_target == addr_str) {
        last_auto_reconnect_target.clear();
        game::com::Com_Error_(file, line, code, "%s", "Missing map!");
        return;
      }
      last_auto_reconnect_target = addr_str;
    }

    const std::string addr_copy(addr_str);
    printf("[ Workshop ] Missing map/mod detected, reconnecting to %s for "
           "download\n",
           addr_copy.c_str());

    scheduler::once(
        [addr_copy] {
          game::cbuf::Cbuf_AddText(
              0, utils::string::va("connect %s\n", addr_copy.c_str()));
        },
        scheduler::main, 3s);

    game::com::Com_Error_(file, line, code, "%s",
                          "Missing map! Reconnecting to download...");
    return;
  }

  game::com::Com_Error_(file, line, code, "%s", "Missing map!");
}

inline constexpr uint32_t
rip_relative_displacement(const uintptr_t instructionEndAddr,
                          const uintptr_t addr) {
  const uintptr_t displacement = addr - instructionEndAddr;
  return static_cast<uint32_t>(displacement);
}

void patch_rip_relative_ptr(uintptr_t instrOffset, uint32_t instrPtrArgOffset,
                            uint32_t instrLen, const auto *newAddr) {
  uintptr_t relocatedInstrOffset = reinterpret_cast<uintptr_t>(
      game::relocate(static_cast<size_t>(instrOffset)));
  uintptr_t instrPtrArgAddr = relocatedInstrOffset + instrPtrArgOffset;
  uintptr_t instrEndAddr = relocatedInstrOffset + instrLen;
  uintptr_t newAddrInt = reinterpret_cast<uintptr_t>(newAddr);
  uint32_t newAddrRipRelative =
      rip_relative_displacement(instrEndAddr, newAddrInt);
  utils::hook::set<uint32_t>(instrPtrArgAddr, newAddrRipRelative);
}

void *memset_extended_workshop_data_pool(
    const game::ugc::ExtendedWorkshopDataPool *pool, int32_t val) {
  return memset(const_cast<void *>(static_cast<const void *>(pool)), val,
                game::ugc::EXTENDED_WORKSHOP_DATA_POOL_STRUCT_SIZE);
}

void *memset_extended_mods_pool(
    [[maybe_unused]] game::ugc::BuiltinWorkshopDataPool *pool, int32_t val,
    [[maybe_unused]] size_t len) {
  return memset_extended_workshop_data_pool(game::ugc::modsPoolPtr, val);
}

void *memset_extended_usermaps_pool(
    [[maybe_unused]] game::ugc::BuiltinWorkshopDataPool *pool, int32_t val,
    [[maybe_unused]] size_t len) {
  return memset_extended_workshop_data_pool(game::ugc::usermapsPoolPtr, val);
}

inline void extend_usermaps_pool() {
  // lea     rcx, usermapsPool
  patch_rip_relative_ptr(game::select(0x1420D52BF, 0x1404E149F), 3, 7,
                         game::ugc::usermapsPoolPtr);

  //  cmp     cs:usermapsPool.count, ebx
  patch_rip_relative_ptr(game::select(0x1420D5725, 0x1404E18D5), 2, 6,
                         game::ugc::usermapsPoolCountPtr);
  // lea     rbp, usermapsPool.data.publisherId
  patch_rip_relative_ptr(game::select(0x1420D572D, 0x1404E18DD), 3, 7,
                         game::ugc::usermapsPoolDataPublisherIdPtr);
  // cmp     ebx, cs:usermapsPool.count
  patch_rip_relative_ptr(game::select(0x1420D575B, 0x1404E190B), 2, 6,
                         game::ugc::usermapsPoolCountPtr);
  // lea     rax, usermapsPool.data
  patch_rip_relative_ptr(game::select(0x1420D5768, 0x1404E1918), 3, 7,
                         game::ugc::usermapsPoolDataPtr);

  // lea     rdi, usermapsPool
  patch_rip_relative_ptr(game::select(0x1420D6155, 0x1404E2205), 3, 7,
                         game::ugc::usermapsPoolPtr);

  // Lua function - client-specific
  if (game::is_client()) {
    // mov     ebx, cs:usermapsPool.count
    patch_rip_relative_ptr(0x1420D6207, 2, 6, game::ugc::usermapsPoolCountPtr);
    // lea     r14, usermapsPool
    patch_rip_relative_ptr(0x1420D624F, 3, 7, game::ugc::usermapsPoolPtr);
  }

  // cmovz   eax, cs:usermapsPool.count
  patch_rip_relative_ptr(game::select(0x1420D6371, 0x1404E22A1), 3, 7,
                         game::ugc::usermapsPoolCountPtr);

  // lea     rsi, usermapsPool
  patch_rip_relative_ptr(game::select(0x1420D63AB, 0x1404E22DB), 3, 7,
                         game::ugc::usermapsPoolPtr);

  // Server-specific function - resets both mod and usermap data
  if (game::is_server()) {
    utils::hook::call(0x1404E25D3_g, memset_extended_usermaps_pool);
    // lea     rcx, usermapsPool
    patch_rip_relative_ptr(0x1404E25E6, 3, 7, game::ugc::usermapsPoolPtr);
  }

  // Set map loading image - client-specific
  if (game::is_client()) {
    // cmp     cs:usermapsPool.count, ebx
    patch_rip_relative_ptr(0x1420D720D, 2, 6, game::ugc::usermapsPoolCountPtr);
    // lea     rbp, usermapsPool.data.publisherId
    patch_rip_relative_ptr(0x1420D7224, 3, 7,
                           game::ugc::usermapsPoolDataPublisherIdPtr);
    // cmp     ebx, cs:usermapsPool.count
    patch_rip_relative_ptr(0x1420D724F, 2, 6, game::ugc::usermapsPoolCountPtr);
    // lea     rcx, usermapsPool.data
    patch_rip_relative_ptr(0x1420D7333, 3, 7, game::ugc::usermapsPoolDataPtr);
  }

  utils::hook::call(game::select(0x1420D67DB_g, 0x1404E26A3_g),
                    memset_extended_usermaps_pool);
  // lea     rcx, usermapsPool
  patch_rip_relative_ptr(game::select(0x1420D67E9, 0x1404E26B6), 3, 7,
                         game::ugc::usermapsPoolPtr);

  // Set map preview image
  {
    // cmp     cs:usermapsPool.count, ebx
    patch_rip_relative_ptr(game::select(0x1420D7401, 0x1404E3011), 2, 6,
                           game::ugc::usermapsPoolCountPtr);
    // lea     rbp, usermapsPool.data.publisherId
    patch_rip_relative_ptr(game::select(0x1420D7409, 0x1404E3019), 3, 7,
                           game::ugc::usermapsPoolDataPublisherIdPtr);
    // cmp     ebx, cs:usermapsPool.count
    patch_rip_relative_ptr(game::select(0x1420D742F, 0x1404E303F), 2, 6,
                           game::ugc::usermapsPoolCountPtr);
    // lea     rcx, usermapsPool.data
    patch_rip_relative_ptr(game::select(0x1420D7513, 0x1404E310B), 3, 7,
                           game::ugc::usermapsPoolDataPtr);
  }
}

inline void extend_mods_pool() {
  // lea     rcx, modsPool.data
  patch_rip_relative_ptr(game::select(0x1420D5315, 0x1404E14F5), 3, 7,
                         game::ugc::modsPoolDataPtr);

  // cmp     cs:modsPool.count, ebx
  patch_rip_relative_ptr(game::select(0x1420D5A17, 0x1404E1BC7), 2, 6,
                         game::ugc::modsPoolCountPtr);
  // lea     rbp, modsPool.data.publisherId
  patch_rip_relative_ptr(game::select(0x1420D5A2A, 0x1404E1BDA), 3, 7,
                         game::ugc::modsPoolDataPublisherIdPtr);
  // cmp     ebx, cs:modsPool.count
  patch_rip_relative_ptr(game::select(0x1420D5A5F, 0x1404E1C0F), 2, 6,
                         game::ugc::modsPoolCountPtr);
  // lea     rcx, modsPool.data
  patch_rip_relative_ptr(game::select(0x1420D5AED, 0x1404E1C9D), 3, 7,
                         game::ugc::modsPoolDataPtr);

  // lea     rdi, modsPool
  patch_rip_relative_ptr(game::select(0x1420D614C, 0x1404E21FC), 3, 7,
                         game::ugc::modsPoolPtr);

  // Lua function - client-specific
  if (game::is_client()) {
    // mov     ebx, cs:modsPool.count
    patch_rip_relative_ptr(0x1420D61F6, 2, 6, game::ugc::modsPoolCountPtr);
    // lea     r14, modsPool
    patch_rip_relative_ptr(0x1420D6244, 3, 7, game::ugc::modsPoolPtr);
  }

  // mov     eax, cs:modsPool.count
  patch_rip_relative_ptr(game::select(0x1420D6365, 0x1404E2295), 2, 6,
                         game::ugc::modsPoolCountPtr);

  // lea     rsi, modsPool
  patch_rip_relative_ptr(game::select(0x1420D63A2, 0x1404E22D2), 3, 7,
                         game::ugc::modsPoolPtr);

  // Server-specific function - resets both mod and usermap data
  if (game::is_server()) {
    utils::hook::call(0x1404E2606_g, memset_extended_mods_pool);
    // lea     rcx, modsPool
    patch_rip_relative_ptr(0x1404E2625, 3, 7, game::ugc::modsPoolPtr);
  }

  utils::hook::call(game::select(0x1420D672B_g, 0x1404E2653_g),
                    memset_extended_mods_pool);
  // lea     rcx, modsPool
  patch_rip_relative_ptr(game::select(0x1420D6739, 0x1404E2672), 3, 7,
                         game::ugc::modsPoolPtr);

  // cmp     cs:modsPool.count, esi
  patch_rip_relative_ptr(game::select(0x1420D6975, 0x1404E2800), 2, 6,
                         game::ugc::modsPoolCountPtr);
  // lea     r15, modsPool.data.publisherId
  patch_rip_relative_ptr(game::select(0x1420D697F, 0x1404E280A), 3, 7,
                         game::ugc::modsPoolDataPublisherIdPtr);
  // cmp     edi, cs:modsPool.count
  patch_rip_relative_ptr(game::select(0x1420D69AB, 0x1404E283B), 2, 6,
                         game::ugc::modsPoolCountPtr);
  // lea     rax, modsPool.data
  patch_rip_relative_ptr(game::select(0x1420D69B8, 0x1404E2848), 3, 7,
                         game::ugc::modsPoolDataPtr);
}

inline void extend_immediate_pool_length_checks() {
  // cmp     rcx, 80h -> cmp     rcx, 2000h
  utils::hook::set<uint32_t>(game::select(0x1420D6506_g, 0x1404E2436_g),
                             game::ugc::EXTENDED_WORKSHOP_DATA_POOL_SIZE);
  // cmp     rdx, 80h -> cmp     rdx, 2000h
  utils::hook::set<uint32_t>(game::select(0x1420D5325_g, 0x1404E1505_g),
                             game::ugc::EXTENDED_WORKSHOP_DATA_POOL_SIZE);
}

void extend_ugc_pools() {
  extend_usermaps_pool();
  extend_mods_pool();
  extend_immediate_pool_length_checks();
}

class component final : public generic_component {
public:
  void post_unpack() override {

    extend_ugc_pools();

    [[maybe_unused]] const auto *dvar_retry = game::register_dvar_int(
        "workshop_retry_attempts", 30, 1, 1000, game::DVAR_ARCHIVE,
        "Number of connection retry attempts for workshop downloads (default "
        "15, increase for slow connections)");
    [[maybe_unused]] const auto *dvar_timeout = game::register_dvar_int(
        "workshop_timeout", 300, 60, 3600, game::DVAR_ARCHIVE,
        "Download timeout in seconds for workshop items (reserved for future "
        "use)");

    dlc_popup_thread_obj = std::thread(dlc_popup_thread_func);

    command::add("userContentReload", [](const command::params &params) {
      game::ugc::reloadUserContent();
      if (!game::is_server())
        toast::info("Workshop", "User content reloaded");
    });
    command::add("workshop_config", [](const command::params &params) {
      printf(
          "[ Workshop ] workshop_retry_attempts: %d (set in game or config)\n",
          get_workshop_retry_attempts());
      printf("[ Workshop ] workshop_timeout: %d\n",
             game::get_dvar_int("workshop_timeout"));
    });
    command::add("workshop_download", [](const command::params &params) {
      if (params.size() < 2) {
        printf("[ Workshop ] Usage: workshop_download <id> [Map|Mod]\n");
        return;
      }
      const std::string id = params.get(1);
      std::string type_str = params.size() >= 3 ? params.get(2) : "Map";
      if (id.empty())
        return;
      if (is_any_download_active()) {
        game::ui::UI_OpenErrorPopupWithMessage(
            0, game::ERROR_UI,
            "A download is already in progress. Wait for it to finish.");
        return;
      }
      if (type_str != "Map" && type_str != "Mod")
        type_str = "Map";
      printf("[ Workshop ] Starting download: %s (%s)\n", id.c_str(),
             type_str.c_str());
      if (!game::is_server())
        toast::show("Workshop",
                    utils::string::va("Downloading %s: %s", type_str.c_str(),
                                      id.c_str()),
                    "t7_icon_menu_options_download");
      download_thread = utils::thread::create_named_thread(
          "workshop_download", steamcmd::initialize_download, id, type_str);
      download_thread.detach();
    });

    utils::hook::call(game::select(0x1420D6AA6, 0x1404E2936),
                      va_mods_path_stub);
    utils::hook::call(game::select(0x1420D6577, 0x1404E24A7),
                      va_user_content_path_stub);

    load_usermap_hook.create(game::select(0x1420D5700, 0x1404E18B0),
                             load_usermap_stub);
    utils::hook::call(game::select(0x1420D67F5, 0x1404E25F2),
                      load_usermap_content_stub);

    if (game::is_server()) {
      utils::hook::jump(0x1404E2635_g, load_mod_content_stub);
      return;
    }

    utils::hook::call(0x1420D6745_g, load_mod_content_stub);
    utils::hook::call(0x14135CD84_g, has_workshop_item_stub);
    setup_server_map_hook.create(*game::cl::CL_SetupForNewServerMap,
                                 setup_server_map_stub);

    if (game::is_client()) {
      utils::hook::call(0x14135CDA1_g, com_error_missing_map_stub);
    }
  }

  void pre_destroy() override {
    downloading_workshop_item = false;
    dlc_thread_shutdown = true;
    dlc_cv.notify_one();
    if (dlc_popup_thread_obj.joinable())
      dlc_popup_thread_obj.join();
    if (download_thread.joinable())
      download_thread.join();
  }
};
} // namespace workshop

REGISTER_COMPONENT(workshop::component)