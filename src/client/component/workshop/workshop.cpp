#include <std_include.hpp>

#include "workshop.hpp"
#include <loader/component_loader.hpp>

#include <component/command.hpp>
#include <game/utils.hpp>

#include <utils/flags.hpp>
#include <utils/hook.hpp>
#include <utils/http.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

#include <component/download_overlay.hpp>
#include <component/fastdl.hpp>
#include <component/network.hpp>
#include <component/party.hpp>
#include <component/scheduler.hpp>
#include <component/toast.hpp>

#include <game/impl/db/xzone/xzone.hpp>
#include <game/impl/lua/lua.hpp>
#include <game/impl/ugc/ugc.hpp>

#include <condition_variable>
#include <mutex>
#include <regex>
#include <shellapi.h>

#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <frozen/unordered_set.h>

using namespace game::db;
using XZoneName = xzone::XZoneName;

namespace workshop {
game::EngineDependentDvar workshop_timeout;
game::EngineDependentDvar workshop_retry_attempts;

utils::hook::detour CL_SetupForNewServerMap_hook;

inline constexpr std::pair<frozen::string, frozen::string> DLC_LINK_ARRAY[] = {
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
inline constexpr frozen::unordered_map<frozen::string, frozen::string,
                                       std::size(DLC_LINK_ARRAY)>
    DLC_LINKS = frozen::make_unordered_map(DLC_LINK_ARRAY);
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

    if (DLC_LINKS.contains(frozen::string(map.data()))) {
      const char *link = DLC_LINKS.at(frozen::string(map.data())).data();
      const std::string map_copy = map;
      scheduler::once(
          [map_copy, link] {
            game::ui::UI_OpenErrorPopupWithMessage(
                game::LOCAL_CLIENT_0, game::errorCode::UI,
                utils::string::va(
                    "Missing DLC map: %s\n\nOpening download page...\n%s",
                    map_copy.c_str(), link));
          },
          scheduler::main);
      ShellExecuteA(nullptr, "open", link, nullptr, nullptr, SW_SHOWNORMAL);
    }
  }
}

void queue_dlc_popup(const std::string_view &mapname) {
  std::lock_guard lock(dlc_mutex);
  pending_dlc_map = mapname;
  dlc_cv.notify_one();
}

bool has_mod(const std::string_view &pub_id) {
  for (uint32_t i = 0; i < game::ugc::modsPool.count; ++i) {
    const game::ugc::UGCItem *mod_data = &game::ugc::modsPool.data[i];
    if (mod_data->publisherId == pub_id || mod_data->internalName == pub_id) {
      return true;
    }
  }

  return false;
}

bool has_usermap(const std::string_view &internal_id) {
  for (uint32_t i = 0; i < game::ugc::modsPool.count; ++i) {
    const game::ugc::UGCItem *mod_data = &game::ugc::modsPool.data[i];
    if (mod_data->publisherId == internal_id ||
        mod_data->internalName == internal_id) {
      return true;
    }
  }

  return false;
}

std::string resolve_mod_workshop_id(const std::string_view &mod_name) {
  for (uint32_t i = 0; i < game::ugc::modsPool.count; ++i) {
    const game::ugc::UGCItem *mod_data = &game::ugc::modsPool.data[i];
    if (mod_data->internalName == mod_name &&
        utils::string::is_numeric(mod_data->publisherId)) {
      return mod_data->publisherId;
    }
  }

  std::error_code ec;
  std::filesystem::path mods_dir("mods");
  if (std::filesystem::exists(mods_dir, ec)) {
    for (const std::filesystem::directory_entry &entry :
         std::filesystem::directory_iterator(mods_dir, ec)) {
      if (!entry.is_directory(ec))
        continue;

      std::filesystem::path ws_json = entry.path() / "zone" / "workshop.json";
      if (!std::filesystem::exists(ws_json, ec))
        continue;

      const std::string json_str = utils::io::read_file(ws_json.string());
      if (json_str.empty())
        continue;

      rapidjson::Document doc;
      if (doc.Parse(json_str.c_str()).HasParseError() || !doc.IsObject())
        continue;

      rapidjson::Document::MemberIterator folder_it =
          doc.FindMember("FolderName");
      if (folder_it != doc.MemberEnd() && folder_it->value.IsString()) {
        if (std::string(folder_it->value.GetString()) == mod_name) {
          rapidjson::Document::MemberIterator pub_it =
              doc.FindMember("PublishedFileId");
          if (pub_it != doc.MemberEnd() && pub_it->value.IsString()) {
            std::string pfid = pub_it->value.GetString();
            if (utils::string::is_numeric(pfid.data()))
              return pfid;
          }
          rapidjson::Document::MemberIterator pubid_it =
              doc.FindMember("PublisherID");
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
  const uint32_t count =
      std::min<uint32_t>(*xzone::g_zoneCount.get(), xzone::MAX_ZONE_COUNT);
  for (uint32_t zoneIdx = 0; zoneIdx <= count; zoneIdx++) {
    XZoneName *zoneInfo = &g_zoneNames[zoneIdx];
    if (std::strcmp(zoneInfo->name, zone_name) == 0) {
      return zoneIdx;
    }
  }

  return 0xFFFFFFFF; // Invalid index
}

bool is_xzone_loaded(const char *zone_name) {
  const uint32_t zoneIdx = get_xzone_index_by_name(zone_name);
  return zoneIdx != 0xFFFFFFFF &&
         reinterpret_cast<XZoneName *>(xzone::g_zoneNames.get())[zoneIdx]
                 .state == xzone::XZoneState::XZONE_COMPLETE;
}

bool are_xzones_loading() {
  const XZoneName *g_zoneNames =
      reinterpret_cast<XZoneName *>(xzone::g_zoneNames.get());
  const uint32_t count =
      std::min<uint32_t>(*xzone::g_zoneCount.get(), xzone::MAX_ZONE_COUNT);
  for (uint32_t zoneIdx = 0; zoneIdx <= count; zoneIdx++) {
    switch (g_zoneNames[zoneIdx].state) {
    case xzone::XZoneState::XZONE_UNLOADING:
    case xzone::XZoneState::XZONE_LOADING:
    case xzone::XZoneState::XZONE_LOADED:
      return true;
    default:
      break;
    }
  }
  return false;
}

bool unload_xzone_by_name(const char *zone_name, bool createDefault,
                          bool suppressSync) {
  uint32_t zoneIdx = get_xzone_index_by_name(zone_name);
  if (zoneIdx != 0xFFFFFFFF) {
    load::DB_UnloadXZone(zoneIdx, createDefault,
                         suppressSync ? game::qtrue : game::qfalse);
    return true;
  }
  return false; // Zone not found
}

void CL_SetupForNewServerMap_stub(game::LocalClientNum_t localClientNum,
                                  const char *map, const char *gametype) {
  const std::string loaded_mod_id = game::ugc::UGC_ActiveMod_PublisherId();
  const bool is_usermap =
      utils::string::is_numeric(map) ||
      game::ugc::UGC_GetUsermapByInternalName(map).has_value();
  const bool mod_loaded = loaded_mod_id.size() > 0;
  const bool usermaps_mod_loaded = mod_loaded && loaded_mod_id == "usermaps";

  if (is_usermap) {
    if (!mod_loaded) {
      game::ugc::UGC_LoadModByPublisherId_Impl(localClientNum, "usermaps",
                                               false);
    }
  } else {
    if (usermaps_mod_loaded) {
      game::ugc::UGC_LoadModByPublisherId_Impl(localClientNum, "", false);
    }

    unload_xzone_by_name("zm_levelcommon", false, false);
  }

  CL_SetupForNewServerMap_hook.invoke(localClientNum, map, gametype);
}

// Steam descriptions are BBCode. Only known tags are touched, so text like
// "[ZM]" survives
std::string clean_description(const std::string_view &text) {
  static const std::unordered_set<std::string_view> dropped_blocks = {
      "img", "previewyoutube", "video", "previewimg"};
  static const std::unordered_set<std::string_view> line_tags = {
      "h1",    "h2",   "h3",    "hr", "list",    "olist",
      "quote", "code", "table", "tr", "spoiler", "p"};
  static const std::unordered_set<std::string_view> inline_tags = {
      "b", "i",     "u",    "s",    "strike", "noparse", "url",
      "*", "color", "size", "font", "center", "emoticon"};

  const auto lower = [](std::string_view value) {
    std::string out(value);
    for (char &c : out)
      c = static_cast<char>(std::tolower(static_cast<uint8_t>(c)));
    return out;
  };

  const std::string lowered = lower(text);
  std::string out;
  out.reserve(text.size());
  for (size_t i = 0; i < text.size();) {
    const char c = text[i];
    if (c == '\r') {
      ++i;
      continue;
    }
    if (c != '[') {
      out.push_back(c);
      ++i;
      continue;
    }

    const size_t end = text.find(']', i);
    if (end == std::string_view::npos) {
      // A tag cut off at the end of the text
      break;
    }
    std::string_view tag = text.substr(i + 1, end - i - 1);
    const bool closing = tag.starts_with('/');
    if (closing)
      tag.remove_prefix(1);
    const size_t param = tag.find_first_of("= ");
    const std::string name = lower(tag.substr(0, param));

    if (!closing && dropped_blocks.contains(name)) {
      const size_t close = lowered.find("[/" + name + "]", end);
      i = close == std::string::npos ? text.size() : close + name.size() + 3;
      continue;
    }
    if (!closing && name == "url" && param == std::string_view::npos) {
      // A bare link, the text is the URL itself
      const size_t close = lowered.find("[/url]", end);
      i = close == std::string::npos ? text.size() : close + 6;
      continue;
    }
    if (line_tags.contains(name)) {
      out.push_back('\n');
    } else if (name == "*") {
      out += "\n- ";
    } else if (name == "td" || name == "th") {
      out.push_back(' ');
    } else if (!inline_tags.contains(name)) {
      out.push_back('[');
      ++i;
      continue;
    }
    i = end + 1;
  }

  // Bare links and HTML entities
  std::string result;
  result.reserve(out.size());
  for (size_t i = 0; i < out.size();) {
    const std::string_view rest = std::string_view(out).substr(i);
    if (rest.starts_with("http://") || rest.starts_with("https://") ||
        rest.starts_with("www.")) {
      while (i < out.size() && !std::isspace(static_cast<uint8_t>(out[i])))
        ++i;
      continue;
    }
    bool replaced = false;
    for (const auto &[entity, value] :
         {std::pair{"&amp;", "&"}, std::pair{"&quot;", "\""},
          std::pair{"&#39;", "'"}, std::pair{"&apos;", "'"},
          std::pair{"&lt;", "<"}, std::pair{"&gt;", ">"},
          std::pair{"&nbsp;", " "}}) {
      if (rest.starts_with(entity)) {
        result += value;
        i += std::string_view(entity).size();
        replaced = true;
        break;
      }
    }
    if (!replaced)
      result.push_back(out[i++]);
  }

  // The game font only has Latin-1, so map common symbols and drop the rest
  // (emoji and such) instead of showing boxes
  std::string glyphs;
  glyphs.reserve(result.size());
  for (size_t i = 0; i < result.size();) {
    const uint8_t lead = static_cast<uint8_t>(result[i]);
    const size_t length = lead < 0x80   ? 1
                          : lead < 0xE0 ? 2
                          : lead < 0xF0 ? 3
                                        : 4;
    if (length == 1 || i + length > result.size()) {
      glyphs.push_back(result[i++]);
      continue;
    }
    uint32_t code = lead & (0xFF >> (length + 1));
    for (size_t k = 1; k < length; ++k)
      code = (code << 6) | (static_cast<uint8_t>(result[i + k]) & 0x3F);

    if (code < 0x100) {
      glyphs.append(result, i, length);
    } else if (code == 0x2018 || code == 0x2019) {
      glyphs.push_back('\'');
    } else if (code == 0x201C || code == 0x201D) {
      glyphs.push_back('"');
    } else if (code == 0x2013 || code == 0x2014 || code == 0x2022 ||
               code == 0x2606) {
      glyphs.push_back('-');
    } else if (code == 0x2605) {
      glyphs.push_back('*');
    } else if (code == 0x2026) {
      glyphs += "...";
    }
    i += length;
  }
  result = std::move(glyphs);

  // Trim each line and drop empty ones
  std::string cleaned;
  cleaned.reserve(result.size());
  size_t start = 0;
  while (start <= result.size()) {
    size_t stop = result.find('\n', start);
    if (stop == std::string::npos)
      stop = result.size();
    std::string_view line =
        std::string_view(result).substr(start, stop - start);
    while (!line.empty() && std::isspace(static_cast<uint8_t>(line.front())))
      line.remove_prefix(1);
    while (!line.empty() && std::isspace(static_cast<uint8_t>(line.back())))
      line.remove_suffix(1);
    if (!line.empty() && line != "-") {
      if (!cleaned.empty())
        cleaned.push_back('\n');
      // Removed links leave double spaces behind
      for (const char c : line) {
        const bool space = c == ' ' || c == '\t';
        if (!space || cleaned.back() != ' ')
          cleaned.push_back(space ? ' ' : c);
      }
    }
    start = stop + 1;
  }
  return cleaned;
}

void load_workshop_data(game::ugc::UGCItem *item) {
  const char *base_path = item->absolutePathZoneFiles;
  const char *path = utils::string::va("%s/workshop.json", base_path);
  const std::string json_str = utils::io::read_file(path);

  if (json_str.empty()) {
    return;
  }

  rapidjson::Document doc;
  const rapidjson::ParseResult parse_result = doc.Parse(json_str.data());

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
  utils::string::copy(
      item->description,
      clean_description(doc["Description"].GetString()).c_str());
  utils::string::copy(item->internalName, doc["FolderName"].GetString());
  utils::string::copy(item->publisherId, doc["PublisherID"].GetString());
  item->publisherIdInteger = std::strtoull(item->publisherId, nullptr, 10);
  item->publisherIdHash = game::ugc::UGC_Hash(item->publisherId);
}

void populate_workshop_paths(game::ugc::UGCItem *item,
                             const std::filesystem::path &content_folder,
                             const game::ZoneType type) {
  item->clear();

  const std::filesystem::path zone_path = content_folder / "zone";
  const std::filesystem::path relative_zone_path =
      std::filesystem::path(type == game::ZoneType::MOD ? "mods" : "usermaps") /
      content_folder.filename() / "zone";

  utils::string::copy(item->contentPathToZoneFiles,
                      relative_zone_path.generic_string().c_str());
  utils::string::copy(item->absolutePathContentDirectory,
                      content_folder.generic_string().c_str());
  utils::string::copy(item->absolutePathZoneFiles,
                      zone_path.generic_string().c_str());
  item->version = 1;
  item->publisherIdHash = 0;
  item->type = type;
}

void supplement_mods_from_disk() {
  if (game::ugc::modsPool.count != 0) {
    return;
  }

  std::error_code ec;
  const std::filesystem::path mods_dir =
      std::filesystem::current_path() / "mods";
  if (!std::filesystem::exists(mods_dir, ec)) {
    return;
  }

  uint32_t count = 0;
  for (const std::filesystem::directory_entry &entry :
       std::filesystem::directory_iterator(mods_dir, ec)) {
    if (ec || !entry.is_directory(ec)) {
      continue;
    }

    const std::filesystem::path zone_dir = entry.path() / "zone";
    const std::filesystem::path workshop_json = zone_dir / "workshop.json";
    if (!std::filesystem::exists(zone_dir, ec) ||
        !std::filesystem::exists(workshop_json, ec)) {
      continue;
    }

    game::ugc::UGCItem *mod_data = &game::ugc::modsPool.data[count];
    populate_workshop_paths(mod_data, entry.path(), game::ZoneType::MOD);
    load_workshop_data(mod_data);
    ++count;
  }

  if (count) {
    game::ugc::modsPool.count = count;
    printf("[ Workshop ] Supplemented %u mods from disk fallback\n", count);
  }
}

void supplement_usermaps_from_disk() {
  std::error_code ec;
  uint32_t count = game::ugc::usermapsPool.count;
  for (const std::filesystem::directory_entry &entry :
       std::filesystem::directory_iterator(
           std::filesystem::current_path() / "usermaps", ec)) {
    if (count >= game::ugc::EXTENDED_WORKSHOP_DATA_POOL_SIZE)
      break;
    if (!entry.is_directory(ec) ||
        (!std::filesystem::exists(entry.path() / "workshop.json", ec) &&
         !std::filesystem::exists(entry.path() / "zone" / "workshop.json",
                                  ec))) {
      continue;
    }

    const std::string folder = entry.path().filename().string();
    if (std::any_of(game::ugc::usermapsPool.data,
                    game::ugc::usermapsPool.data + count,
                    [&](const game::ugc::UGCItem &existing) {
                      return folder == existing.publisherId ||
                             folder == existing.internalName;
                    })) {
      continue;
    }

    game::ugc::UGCItem *map_data = &game::ugc::usermapsPool.data[count++];
    populate_workshop_paths(map_data, entry.path(), game::ZoneType::USERMAP);
    load_workshop_data(map_data);
  }

  if (count != game::ugc::usermapsPool.count) {
    printf("[ Workshop ] Supplemented %u usermaps from disk fallback\n",
           count - game::ugc::usermapsPool.count);
    game::ugc::usermapsPool.count = count;
  }
}

void supplement_ugc_from_workshop(game::ZoneType zoneType) {
  if (game::ugc::usermapsPool.count >=
      game::ugc::EXTENDED_WORKSHOP_DATA_POOL_SIZE) {
    return;
  }

  std::error_code ec;
  const std::filesystem::path current_dir = std::filesystem::current_path();
  const std::filesystem::path steamapps =
      current_dir.parent_path().parent_path();
  const std::filesystem::path workshop_path =
      steamapps / "workshop" / "content" / game::APP_ID_STR;

  if (!std::filesystem::exists(workshop_path, ec)) {
    return;
  }

  const char *ugc_dirname;
  const char *ugc_type_str;
  game::ugc::ExtendedUGCPool *pool;
  switch (zoneType) {
  case game::ZoneType::USERMAP:
    ugc_dirname = "usermaps";
    ugc_type_str = "map";
    pool = &game::ugc::usermapsPool;
    break;
  case game::ZoneType::MOD:
    ugc_dirname = "mods";
    ugc_type_str = "mod";
    pool = &game::ugc::modsPool;
    break;
  default:
    return;
  }

  uint32_t count = pool->count;
  for (const std::filesystem::directory_entry &entry :
       std::filesystem::directory_iterator(workshop_path, ec)) {
    if (ec || !entry.is_directory(ec)) {
      continue;
    }

    const std::filesystem::path workshop_json = entry.path() / "workshop.json";
    if (!std::filesystem::exists(workshop_json, ec)) {
      continue;
    }

    const std::string json_data = utils::io::read_file(workshop_json.string());

    rapidjson::Document doc;
    const rapidjson::ParseResult parse_result = doc.Parse(json_data.c_str());

    if (parse_result.IsError() || !doc.IsObject()) {
      continue;
    }

    if (!doc.HasMember("Type") || !doc["Type"].IsString() ||
        strcmp(doc["Type"].GetString(), ugc_type_str) != 0) {
      continue;
    }

    game::ugc::UGCItem *ugc_data = &pool->data[count];

    ugc_data->clear();
    utils::string::copy(ugc_data->absolutePathContentDirectory,
                        entry.path().generic_string().c_str());
    utils::string::copy(ugc_data->absolutePathZoneFiles,
                        entry.path().generic_string().c_str());

    const std::filesystem::path relative_path =
        std::filesystem::path(ugc_dirname) / entry.path().filename();
    utils::string::copy(ugc_data->contentPathToZoneFiles,
                        relative_path.generic_string().c_str());

    ugc_data->version = 1;
    ugc_data->type = zoneType;

    if (doc.HasMember("Title") && doc["Title"].IsString()) {
      utils::string::copy(ugc_data->title, doc["Title"].GetString());
    }
    if (doc.HasMember("Description") && doc["Description"].IsString()) {
      utils::string::copy(
          ugc_data->description,
          clean_description(doc["Description"].GetString()).c_str());
    }
    if (doc.HasMember("FolderName") && doc["FolderName"].IsString()) {
      utils::string::copy(ugc_data->internalName,
                          doc["FolderName"].GetString());
    }
    if (doc.HasMember("PublisherID") && doc["PublisherID"].IsString()) {
      utils::string::copy(ugc_data->publisherId,
                          doc["PublisherID"].GetString());
      ugc_data->publisherIdInteger =
          std::strtoull(ugc_data->publisherId, nullptr, 10);
      ugc_data->publisherIdHash = game::ugc::UGC_Hash(ugc_data->publisherId);
    }
    ++count;

    if (count >= game::ugc::EXTENDED_WORKSHOP_DATA_POOL_SIZE) {
      break;
    }
  }

  const uint32_t added = count - pool->count;
  if (added) {
    pool->count = count;
    printf("[ Workshop ] Supplemented %u %s from Steam workshop\n", added,
           ugc_dirname);
  }
}

utils::hook::detour UGC_LoadUsermapByPublisherId_hook;
game::ugc::UGCItem *
UGC_LoadUsermapByPublisherId_HandleInternalName(const char *maybePublisherId) {
  std::string publisherId = maybePublisherId;
  if (!utils::string::is_numeric(maybePublisherId)) {
    const game::ugc::OptionalUGCItemRef item =
        game::ugc::UGC_GetUsermapByInternalName(maybePublisherId);
    if (item.has_value()) {
      publisherId = item->get().publisherId;
    } else {
      // Not a usermap, e.g. a stock map after a custom one. Clear the active
      // usermap as the load does when it finds none, so the previous custom
      // map doesn't stay active
      game::ugc::UGC_SetActiveUsermap(nullptr);
      return nullptr;
    }
  }

  return game::ugc::UGC_LoadUsermapByPublisherId_Impl(publisherId.data());
}

utils::hook::detour UGC_VerifyVersion_hook;
bool UGC_VerifyVersion_HandleInternalName(game::ZoneType type,
                                          const char *maybePublisherId,
                                          uint32_t version) {
  std::string publisherId = maybePublisherId;
  if (!utils::string::is_numeric(maybePublisherId) &&
      type == game::ZoneType::USERMAP) {
    const game::ugc::OptionalUGCItemRef item =
        game::ugc::UGC_GetUsermapByInternalName(maybePublisherId);
    if (item.has_value()) {
      publisherId = item->get().publisherId;
    } else {
      return false;
    }
  }
  return game::ugc::UGC_VerifyVersion_Impl(type, publisherId.c_str(), version);
}

const char *va_mods_path(const char *fmt, const char *root_dir,
                         const char *mods_dir, const char *dir_name) {
  const char *original_path =
      utils::string::va(fmt, root_dir, mods_dir, dir_name);

  if (utils::io::directory_exists(original_path)) {
    return original_path;
  }

  return utils::string::va("%s/%s/%s", root_dir, mods_dir, dir_name);
}

const char *va_user_content_path(const char *fmt, const char *root_dir,
                                 const char *user_content_dir) {
  const char *original_path =
      utils::string::va(fmt, root_dir, user_content_dir);

  if (utils::io::directory_exists(original_path)) {
    return original_path;
  }

  return utils::string::va("%s/%s", root_dir, user_content_dir);
}

std::string get_mod_resized_name() {
  const std::string loaded_mod_id = game::ugc::UGC_ActiveMod_PublisherId();

  if (loaded_mod_id == "usermaps" || loaded_mod_id.empty()) {
    return loaded_mod_id;
  }

  std::string mod_name = loaded_mod_id;

  for (uint32_t i = 0; i < game::ugc::modsPool.count; ++i) {
    const game::ugc::UGCItem *mod_data = &game::ugc::modsPool.data[i];

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

int get_workshop_retry_attempts() {
  if (!workshop_retry_attempts.cl)
    return 30;
  const int val = workshop_retry_attempts.get_int();
  if (val < 1)
    return 1;
  if (val > 1000)
    return 1000;
  return val;
}

std::string get_mod_publisher_id() {
  return game::ugc::UGC_ActiveMod_PublisherId();
}
inline constexpr frozen::string ZM_DLC_MAP_ARRAY[] = {
    "zm_asylum", "zm_castle",  "zm_cosmodrome", "zm_factory",    "zm_genesis",
    "zm_island", "zm_moon",    "zm_prototype",  "zm_stalingrad", "zm_sumpf",
    "zm_temple", "zm_theater", "zm_tomb",       "zm_zod",
};

inline constexpr frozen::unordered_set<frozen::string,
                                       std::size(ZM_DLC_MAP_ARRAY)>
    ZM_DLC_MAPS = frozen::make_unordered_set(ZM_DLC_MAP_ARRAY);
inline constexpr bool is_zm_dlc_map(const std::string_view mapname) {
  return ZM_DLC_MAPS.contains(mapname);
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

void set_pending_mod_reconnect(const std::string_view &address) {
  std::lock_guard lock(reconnect_mutex);
  pending_mod_reconnect_address = address;
}

std::string get_pending_mod_reconnect() {
  std::lock_guard lock(reconnect_mutex);
  std::string addr = std::move(pending_mod_reconnect_address);
  pending_mod_reconnect_address.clear();
  return addr;
}

void set_pending_download_reconnect(const std::string_view &address) {
  std::lock_guard lock(reconnect_mutex);
  // Don't overwrite if a download is already active, preserve the original
  // server
  if (pending_download_reconnect_address.empty() || !is_any_download_active()) {
    pending_download_reconnect_address = address;
  }
}

std::string get_pending_download_reconnect() {
  std::lock_guard lock(reconnect_mutex);
  const std::string addr = std::move(pending_download_reconnect_address);
  pending_download_reconnect_address.clear();
  return addr;
}

uint64_t compute_folder_size_bytes(const std::filesystem::path &folder) {
  std::error_code ec;
  if (!std::filesystem::exists(folder, ec))
    return 0;
  uint64_t total = 0;
  for (const std::filesystem::directory_entry &entry :
       std::filesystem::recursive_directory_iterator(
           folder, std::filesystem::directory_options::skip_permission_denied,
           ec)) {
    if (ec)
      break;
    if (!entry.is_regular_file(ec))
      continue;
    total +=
        static_cast<uint64_t>(std::filesystem::file_size(entry.path(), ec));
    if (ec)
      break;
  }
  return total;
}

std::string human_readable_size(uint64_t bytes) {
  const std::string_view suffixes[] = {"B", "KB", "MB", "GB", "TB"};
  double value = static_cast<double>(bytes);
  int idx = 0;
  while (value >= 1024.0 && idx < 4) {
    value /= 1024.0;
    ++idx;
  }
  return std::format("{:.2f} {}", value, suffixes[idx]);
}

uint64_t parse_human_size_to_bytes(const std::string_view &text) {
  std::smatch m;
  std::regex re(R"((\d+(?:\.\d+)?)\s*(B|KB|MB|GB|TB))", std::regex::icase);
  const std::string text_copy = std::string(text);
  if (!std::regex_search(text_copy, m, re) || m.size() < 3)
    return 0;
  const double value = std::stod(m[1].str());
  std::string unit = m[2].str();
  for (char &c : unit)
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

  const double bytes = value * mul;
  if (bytes <= 0.0)
    return 0;
  return static_cast<uint64_t>(bytes);
}

uint64_t scrape_workshop_file_size_bytes(const std::string_view &workshop_id) {
  utils::http::headers h;
  h["User-Agent"] =
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36";
  h["Accept"] = "text/html";
  h["Referer"] = "https://steamcommunity.com/app/311210/workshop/";
  const std::optional<std::string> resp = utils::http::get_data(
      std::format(
          "https://steamcommunity.com/sharedfiles/filedetails/?id={}&l=english",
          workshop_id),
      h, {}, 10);
  if (!resp)
    return 0;

  const auto column = [&](const std::string_view &cls) {
    std::vector<std::string> out;
    const std::string needle = std::format("class=\"{}\"", cls);
    for (size_t pos = resp->find(needle); pos != std::string::npos;
         pos = resp->find(needle, pos + 1)) {
      const size_t gt = resp->find('>', pos);
      const size_t lt = resp->find('<', gt);
      if (gt == std::string::npos || lt == std::string::npos)
        break;
      std::string text = resp->substr(gt + 1, lt - gt - 1);
      utils::string::trim(text);
      out.push_back(utils::string::to_lower(text));
    }
    return out;
  };

  const std::vector<std::string> labels = column("detailsStatLeft");
  const std::vector<std::string> values = column("detailsStatRight");
  for (size_t i = 0; i < std::min(labels.size(), values.size()); ++i) {
    if (labels[i].find("size") == std::string::npos &&
        labels[i].find("tamanho") == std::string::npos &&
        labels[i].find("taille") == std::string::npos &&
        labels[i].find("tama") == std::string::npos &&
        labels[i].find("gr") == std::string::npos)
      continue;
    std::basic_string<char> text = values[i];
    if (text.find('.') == std::string::npos)
      std::replace(text.begin(), text.end(), ',', '.');
    else
      std::erase(text, ',');
    if (const uint64_t bytes = parse_human_size_to_bytes(text))
      return bytes;
  }
  return 0;
}

std::vector<item_details> get_details(const std::vector<std::string> &ids) {
  std::vector<item_details> result;
  constexpr size_t BATCH = 100;
  for (size_t start = 0; start < ids.size(); start += BATCH) {
    const size_t count = std::min(BATCH, ids.size() - start);
    std::string body = "itemcount=" + std::to_string(count);
    for (size_t i = 0; i < count; ++i)
      body += "&publishedfileids[" + std::to_string(i) + "]=" + ids[start + i];

    const std::optional<std::string> resp = utils::http::post_data(
        "https://api.steampowered.com/ISteamRemoteStorage/"
        "GetPublishedFileDetails/v1/",
        body, 15);
    rapidjson::Document doc;
    if (!resp || doc.Parse(resp->c_str()).HasParseError() || !doc.IsObject() ||
        !doc.HasMember("response") ||
        !doc["response"].HasMember("publishedfiledetails") ||
        !doc["response"]["publishedfiledetails"].IsArray())
      continue;

    for (const rapidjson::Document::Array::ValueType &entry :
         doc["response"]["publishedfiledetails"].GetArray()) {
      const auto string_of = [&](const char *key) {
        const rapidjson::Document::Array::ValueType::ConstMemberIterator it =
            entry.FindMember(key);
        return it != entry.MemberEnd() && it->value.IsString()
                   ? std::string(it->value.GetString())
                   : std::string();
      };
      const auto number_of = [&](const char *key) -> uint64_t {
        const rapidjson::Document::Array::ValueType::ConstMemberIterator it =
            entry.FindMember(key);
        if (it == entry.MemberEnd())
          return 0;
        if (it->value.IsString())
          return std::strtoull(it->value.GetString(), nullptr, 10);
        return it->value.IsNumber()
                   ? static_cast<uint64_t>(it->value.GetDouble())
                   : 0;
      };

      if (entry.IsObject()) {
        const uint64_t app_id = number_of("consumer_app_id");
        if (!app_id || app_id == game::APP_ID) {
          item_details item{};
          item.id = string_of("publishedfileid");
          if (!item.id.empty()) {
            item.title = string_of("title");
            item.description = clean_description(string_of("description"));
            item.preview_url = string_of("preview_url");
            item.file_size = number_of("file_size");
            item.time_updated = number_of("time_updated");
            item.subs = static_cast<std::int64_t>(
                number_of("lifetime_subscriptions")
                    ? number_of("lifetime_subscriptions")
                    : number_of("subscriptions"));
            item.favorites =
                static_cast<std::int64_t>(number_of("lifetime_favorited")
                                              ? number_of("lifetime_favorited")
                                              : number_of("favorited"));

            game::ZoneType type = game::ZoneType::USERMAP;
            if (entry.HasMember("tags") && entry["tags"].IsArray()) {
              for (const rapidjson::GenericValue<rapidjson::UTF8<>> &tag :
                   entry["tags"].GetArray()) {
                if (tag.IsObject() && tag.HasMember("tag") &&
                    tag["tag"].IsString()) {
                  item.tags.emplace_back(tag["tag"].GetString());
                  type = normalize_kind(item.tags.back())
                             .value_or(game::ZoneType::USERMAP);
                }
              }
            }
            item.type = type;
            result.push_back(std::move(item));
          }
        }
      }
    }
  }
  return result;
}

workshop_info get_steam_workshop_info(const std::string_view &workshop_id) {
  workshop_info info{};
  if (workshop_id.empty())
    return info;
  const std::vector<item_details> details =
      get_details({std::string(workshop_id)});
  if (!details.empty()) {
    info.title = details[0].title;
    info.file_size = details[0].file_size;
  }
  if (!info.file_size)
    info.file_size = scrape_workshop_file_size_bytes(workshop_id);
  return info;
}

void offer_download(game::ZoneType zoneType, const std::string_view &name,
                    const std::string_view &workshop_id) {
  const workshop_info info = get_steam_workshop_info(workshop_id);
  std::string message =
      std::format("{}  '{}' was not found.\n", game::toString(zoneType), name);
  if (name != workshop_id)
    message += std::format("Workshop ID: {}\n", workshop_id);
  if (!info.title.empty())
    message += "Title: " + info.title + "\n";
  if (info.file_size > 0)
    message += "Size: " + human_readable_size(info.file_size) + "\n";
  message += "\nDo you want to download it from the Steam Workshop?";
  download_overlay::show_confirmation(
      std::format("Download {}?", game::toString(zoneType)), message,
      [workshop_id, zoneType] { start_download(workshop_id, zoneType); });
}

bool start_fastdl(game::ZoneType zoneType, const std::string_view &name,
                  const std::string_view &workshop_id,
                  const std::string_view &base_uri,
                  std::function<void()> failure_callback = {}) {
  std::string_view base = base_uri;
  while (base.ends_with('/')) {
    base.remove_suffix(1);
  }
  const std::string folder =
      std::format("{}s/{}", game::toString(zoneType), name);
  fastdl::download_context *context = new fastdl::download_context();
  context->item_name = name;
  context->pub_id = workshop_id.empty() ? name : workshop_id;
  context->item_path = folder;
  context->content_tree_uri = std::format("{}/{}", base, folder);
  context->type = zoneType;
  context->failure_callback = {};
  context->success_callback = [title = std::string(name)] {
    scheduler::once(
        [title] {
          game::ugc::UGC_LoadPools_Impl();
          const std::string reconnect = get_pending_download_reconnect();
          if (reconnect.empty()) {
            toast::success("Download finished", title + " is ready.");
            return;
          }
          download_overlay::show_confirmation(
              "Download Complete",
              title + " downloaded successfully!\n\nDo you want to connect to "
                      "the server?",
              [reconnect] {
                game::cbuf::Cbuf_AddText(
                    game::LOCAL_CLIENT_0,
                    utils::string::va("connect %s\n", reconnect.c_str()));
              });
        },
        scheduler::main);
  };
  printf("[ Workshop ] Server has FastDL, attempting download for %s from "
         "%s\n",
         context->item_name.c_str(), context->content_tree_uri.c_str());
  return fastdl::start_download(context);
}

bool is_busy_downloading() {
  if (downloading_workshop_item.load() || launcher_downloading.load() ||
      fastdl::is_downloading()) {
    scheduler::once(
        [] {
          game::ui::UI_OpenErrorPopupWithMessage(
              game::LOCAL_CLIENT_0, game::errorCode::UI,
              "You are already downloading an item in the background. You can "
              "download only one item at a time.");
        },
        scheduler::main);
    return true;
  }
  return false;
}

void report_version_mismatch(game::ZoneType zoneType,
                             const std::string_view &name,
                             const std::string_view &workshop_id,
                             const uint64_t local_built,
                             const uint64_t server_built) {
  const std::string what =
      std::format("{} '{}'", game::toString(zoneType), name);
  if (local_built && server_built > local_built &&
      utils::string::is_numeric(workshop_id.data())) {
    const std::string id(workshop_id);
    download_overlay::show_confirmation(
        std::format("{} update needed", game::toString(zoneType)),
        std::format("This server runs a newer version of the {} than "
                    "yours.\n\nDownload the latest version from the Workshop "
                    "now?",
                    what),
        [id, zoneType] { start_download(id, zoneType); });
    return;
  }

  std::string message;
  if (local_built && server_built > local_built) {
    message = std::format("This server runs a newer version of the {} than "
                          "yours. Get the updated version to join.",
                          what);
  } else if (server_built && local_built > server_built) {
    message = std::format("This server runs an older version of the {} than "
                          "yours. The server owner needs to update it before "
                          "you can join.",
                          what);
  } else {
    message = std::format("Your files for the {} don't match this server's. "
                          "Reinstall it to fix this.",
                          what);
  }
  scheduler::once(
      [message] {
        game::ui::UI_OpenErrorPopupWithMessage(
            game::LOCAL_CLIENT_0, game::errorCode::UI, message.c_str());
      },
      scheduler::main);
}

bool check_valid_usermap_id(const std::string_view &mapname,
                            const std::string_view &pub_id,
                            const std::string_view &workshop_id,
                            const std::string_view &base_uri) {
  if (pub_id.empty() && is_zm_dlc_map(mapname.data()) &&
      !DB_ValidFastFile(mapname.data(), 0)) {
    queue_dlc_popup(mapname);
    return false;
  }

  if (downloading_workshop_item.load() || launcher_downloading.load() ||
      fastdl::is_downloading()) {
    scheduler::once(
        [] {
          game::ui::UI_OpenErrorPopupWithMessage(
              game::LOCAL_CLIENT_0, game::errorCode::UI,
              "You are already downloading a map in the background. You can "
              "download only one item at a time.");
        },
        scheduler::main);
    return false;
  }

  if (!base_uri.empty()) {
    std::function<void()> failure_callback = {};
    if (utils::string::is_numeric(mapname.data())) {
      failure_callback = [mapname] {
        offer_download(game::ZoneType::USERMAP, mapname, mapname);
      };
    } else if (!workshop_id.empty() &&
               utils::string::is_numeric(workshop_id.data())) {
      failure_callback = [mapname, workshop_id] {
        offer_download(game::ZoneType::USERMAP, mapname, workshop_id);
      };
    }
    return !start_fastdl(game::ZoneType::USERMAP, mapname, workshop_id,
                         base_uri, failure_callback);
  }

  if (!has_usermap(mapname.data())) {
    if (utils::string::is_numeric(mapname.data())) {
      offer_download(game::ZoneType::USERMAP, mapname, mapname);
    } else if (!workshop_id.empty() &&
               utils::string::is_numeric(workshop_id.data())) {
      offer_download(game::ZoneType::USERMAP, mapname, workshop_id);
    } else {
      const std::string name_copy = std::string(mapname);
      scheduler::once(
          [name_copy] {
            game::ui::UI_OpenErrorPopupWithMessage(
                game::LOCAL_CLIENT_0, game::errorCode::UI,
                utils::string::va(
                    "Missing usermap: %s\n\nThis server did not provide "
                    "FastDL\n\nSubscribe on Steam "
                    "Workshop, or ask the server to set sv_wwwBaseURL.",
                    name_copy.c_str()));
          },
          scheduler::main);
    }
    return false;
  }

  return true;
}

void offer_mod_from_workshop(const std::string &mod,
                             const std::string &workshop_id) {
  const std::string resolved_id =
      utils::string::is_numeric(mod.data())
          ? mod
          : (!workshop_id.empty() && utils::string::is_numeric(workshop_id)
                 ? workshop_id
                 : resolve_mod_workshop_id(mod));
  if (resolved_id.empty()) {
    scheduler::once(
        [mod] {
          game::ui::UI_OpenErrorPopupWithMessage(
              game::LOCAL_CLIENT_0, game::errorCode::UI,
              utils::string::va(
                  "Could not download mod %s: workshop ID is not provided by "
                  "server and does not have FastDL setup",
                  mod.c_str()));
        },
        scheduler::main);
  } else {
    offer_download(game::ZoneType::MOD, mod, resolved_id);
  }
}

bool check_valid_mod_id(const std::string_view &mod,
                        const std::string_view &workshop_id,
                        const std::string_view &base_uri) {
  if (!mod.empty() && mod != "usermaps") {
    if (!is_busy_downloading()) {
      const std::string mod_copy(mod);
      const std::string id_copy(workshop_id);
      if (base_uri.empty()) {
        if (!has_mod(workshop_id)) {
          offer_mod_from_workshop(mod_copy, id_copy);
          return false;
        }
      } else {
        return !start_fastdl(game::ZoneType::MOD, mod, workshop_id, base_uri,
                             [id_copy, mod_copy]() {
                               offer_mod_from_workshop(mod_copy, id_copy);
                             }) &&
               has_mod(workshop_id);
      }
    }
  }
  return true;
}

bool mod_load_requires_fs_reinitialization(const std::string_view &mod_name) {
  return !mod_name.empty() && mod_name != "usermaps";
}

bool mod_switch_requires_fs_reinitialization(
    const std::string_view &current_mod, const std::string_view &new_mod) {
  return mod_load_requires_fs_reinitialization(current_mod) ||
         mod_load_requires_fs_reinitialization(new_mod);
}

void wait_for_mod_load() {
  while (game::ugc::active_mod->loadState == game::ugc::ModLoadState::LOADING) {
    std::this_thread::sleep_for(100ms);
  }
}

void setup_same_mod_as_host(game::LocalClientNum_t localClientNum,
                            const std::string_view &usermap,
                            const std::string_view &mod, bool force_fs_reinit) {
  const std::string loaded_mod = game::ugc::UGC_ActiveMod_PublisherId();
  // Dedicated servers always report usermaps, even on stock maps. Stock maps
  // don't need it, so keep whatever usermaps state we have instead of
  // reloading
  const std::string_view target =
      mod == "usermaps" && usermap.empty() && loaded_mod != "usermaps"
          ? std::string_view{}
          : mod;
  if (loaded_mod != target) {
    const bool fs_reinit_required =
        force_fs_reinit ||
        mod_switch_requires_fs_reinitialization(loaded_mod, target);
    const std::string target_str{target};
    game::ugc::UGC_LoadModByPublisherId_Impl(localClientNum, target_str.data(),
                                             fs_reinit_required);
    if (fs_reinit_required) {
      wait_for_mod_load();
    }
  }
}

static std::mutex reconnect_guard_mutex;
static std::string last_auto_reconnect_target;

void com_error_missing_map_stub(const char *file, int line,
                                game::errorParm code, const char *fmt, ...) {
  const game::net::netadr_t target = party::get_connected_server();
  if (network::is_connectable_address(target)) {
    const char *addr_str =
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
              game::LOCAL_CLIENT_0,
              utils::string::va("connect %s\n", addr_copy.c_str()));
        },
        scheduler::main, 3s);

    game::com::Com_Error_(file, line, code, "%s",
                          "You don't have this map. Reconnecting to download "
                          "it...");
    return;
  }

  game::com::Com_Error_(file, line, code, "%s", "Missing map!");
}

#ifndef NDEBUG
utils::hook::detour UGC_LoadMod_hook;
void UGC_LoadMod_LogFirst(game::LocalClientNum_t localClientNum,
                          game::ugc::UGCItem *mod, bool reloadFS) {
  const std::string mod_str = mod ? mod->serialize() : "NULL";
  game::trace(
      "UGC_LoadMod called with localClientNum: {}, mod: {}, reloadFS: {}",
      serialize(localClientNum), mod_str.data(), reloadFS ? "true" : "false");

  return UGC_LoadMod_hook.invoke(localClientNum, mod, reloadFS);
}
#endif

/*
  The hooks below re-implement and replace most UGC handling logic.
  They are modified from the base engine implementation to:
    - Increase max entry count of the usermaps and mods pools from 128 to
  8192
    - Handle UGC content stored in the game installation's mods and usermaps
  directories
    - Improve usermap internal ID handling and handle usermap directories
  labelled with internal ID of the usermap, as opposed to its publisher ID.
*/
void extend_ugc_pools() {
  utils::hook::jump(game::db::xzone::DB_CheckModXFile.get(),
                    game::db::xzone::DB_CheckModXFile_Impl);
  utils::hook::jump(game::ugc::UGC_GetByPublisherId.get(),
                    game::ugc::UGC_GetByPublisherId_Impl);
  utils::hook::jump(game::ugc::UGC_GetCount.get(),
                    game::ugc::UGC_GetCount_Impl);
  utils::hook::jump(game::ugc::UGC_VerifyVersion.get(),
                    UGC_VerifyVersion_HandleInternalName);
  utils::hook::jump(game::ugc::UGC_LoadPool.get(),
                    game::ugc::UGC_LoadPool_Impl);
  utils::hook::jump(game::ugc::UGC_LoadModsPool.get(),
                    game::ugc::UGC_LoadModsPool_Impl);
  utils::hook::jump(game::ugc::UGC_LoadUsermapsPool.get(),
                    game::ugc::UGC_LoadUsermapsPool_Impl);
  utils::hook::jump(game::ugc::UGC_LoadPools.get(),
                    game::ugc::UGC_LoadPools_Impl);
  utils::hook::jump(game::ugc::UGC_LoadModByPublisherId.get(),
                    game::ugc::UGC_LoadModByPublisherId_Impl);
  utils::hook::jump(game::ugc::UGC_SetMapPreviewImageByPublisherId.get(),
                    game::ugc::UGC_SetMapPreviewImageByPublisherId_Impl);
  utils::hook::jump(game::ugc::UGC_LoadManifest.get(),
                    game::ugc::UGC_LoadManifest_Impl);
  UGC_LoadUsermapByPublisherId_hook.create(
      game::ugc::UGC_LoadUsermapByPublisherId.get(),
      UGC_LoadUsermapByPublisherId_HandleInternalName);

  if (game::is_client()) {
    utils::hook::jump(game::lua::Mods_Lists_GetInfoEntries_Slice.get(),
                      game::lua::Mods_Lists_GetInfoEntries_Slice_Impl);

    utils::hook::jump(game::ugc::UGC_SetMapLoadingImage.get(),
                      game::ugc::UGC_SetMapLoadingImage_Impl);
  }

#ifndef NDEBUG
  UGC_LoadMod_hook.create(game::ugc::UGC_LoadMod, UGC_LoadMod_LogFirst);
#endif
}

class component final : public generic_component {
  DEFINE_COMPONENT_NAME("workshop");

public:
  void post_unpack() override {
    extend_ugc_pools();

    if (game::is_client()) {
      workshop_retry_attempts = game::register_dvar_int(
          "workshop_retry_attempts", 30, 1, 1000, game::DVAR_ARCHIVE,
          "Number of connection retry attempts for workshop downloads "
          "(default "
          "15, increase for slow connections)");
      workshop_timeout = game::register_dvar_int(
          "workshop_timeout", 300, 60, 3600, game::DVAR_ARCHIVE,
          "Download timeout in seconds for workshop items (reserved for "
          "future "
          "use)");

      dlc_popup_thread_obj = std::thread(dlc_popup_thread_func);

      command::add("userContentReload", [](const command::params &params) {
        game::ugc::UGC_LoadPools_Impl();
        if (!game::is_server())
          toast::info("Workshop", "User content reloaded");
      });
      command::add("workshop_config", [](const command::params &params) {
        printf("[ Workshop ] workshop_retry_attempts: %d (set in game or "
               "config)\n",
               get_workshop_retry_attempts());
        printf("[ Workshop ] workshop_timeout: %d\n",
               workshop_timeout.get_int());
      });
      command::add("workshop_download", [](const command::params &params) {
        if (params.size() < 2) {
          printf("[ Workshop ] Usage: workshop_download <id> [Map|Mod]\n");
          return;
        }
        const std::string id = params.get(1);
        if (!id.empty()) {

          game::ZoneType zoneType =
              params.size() >= 3 &&
                      utils::string::to_lower(params.get(2)).find("mod") !=
                          std::string::npos
                  ? game::ZoneType::MOD
                  : game::ZoneType::USERMAP;
          start_download(id, zoneType);
        }
      });
      command::add("loadmod", [](const command::params &params) {
        if (params.size() > 0) {
          const std::string mod = params.get(1);

          if (mod.empty() || mod == "usermaps") {
            game::ugc::UGC_LoadModByPublisherId_Impl(game::LOCAL_CLIENT_0,
                                                     mod.c_str(), true);
          } else {
            for (size_t i = 0; i < game::ugc::modsPool.count; ++i) {
              const game::ugc::UGCItem *data = &game::ugc::modsPool.data[i];
              if (std::string_view(data->internalName) == mod ||
                  std::string_view(data->publisherId) == mod) {
                return game::ugc::UGC_LoadModByPublisherId_Impl(
                    game::LOCAL_CLIENT_0, data->publisherId, true);
              }
            }
          }
        }
      });

      command::add("printmod", [](const command::params &params) {
        const auto print = [](const std::string_view &msg) -> void {
          fprintf(stdout, "%s\n", msg.data());
          fflush(stdout);

          game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                                game::consoleLabel_e::DEFAULT, "%s\n",
                                msg.data());
          game::trace("[printmod] {}", msg.data());
        };

        if (params.size() > 0) {
          const std::string field = utils::string::to_lower(params.get(1));

          if (field == "publisherid" || field == "publisher_id" ||
              field == "ugcname" || field == "ugc_name") {
            print(game::ugc::active_mod->publisherId);
          } else if (field == "internal_name" || field == "internalname") {
            print(game::ugc::active_mod->internalName);
          } else if (field == "title") {
            print(game::ugc::active_mod->title);
          } else if (field == "description") {
            print(game::ugc::active_mod->description);
          }
        } else {
          print(game::ugc::active_mod->internalName);
        }
      });

      CL_SetupForNewServerMap_hook.create(
          game::cl::CL_SetupForNewServerMap.get(),
          CL_SetupForNewServerMap_stub);

      utils::hook::call(game::cl::CL_SetupForNewServerMap.offset(0x81),
                        com_error_missing_map_stub);
    }
  }

  void pre_destroy() override {
    if (game::is_client()) {
      cancel_download();
      dlc_thread_shutdown = true;
      dlc_cv.notify_one();
      if (dlc_popup_thread_obj.joinable())
        dlc_popup_thread_obj.join();
    }
  }
};
} // namespace workshop

REGISTER_COMPONENT(workshop::component)
