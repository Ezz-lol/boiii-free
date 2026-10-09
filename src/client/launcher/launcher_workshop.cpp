#include <std_include.hpp>

#include <game/game.hpp>

#include <component/workshop/workshop.hpp>

#include <launcher/html/html_frame.hpp>
#include <launcher/launcher_workshop.hpp>

#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <mutex>
#include <regex>
#include <set>
#include <thread>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <utils/finally.hpp>
#include <utils/http.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

namespace launcher::workshop {
// Convert UTF-8 std::string to a CComVariant with proper wide-string encoding
CComVariant utf8_variant(const std::string &utf8_str) {
  if (utf8_str.empty())
    return CComVariant(L"");
  const int wide_len =
      MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(),
                          static_cast<int>(utf8_str.size()), nullptr, 0);
  if (wide_len <= 0)
    return CComVariant(utf8_str.c_str()); // fallback
  std::wstring wide(static_cast<size_t>(wide_len), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(),
                      static_cast<int>(utf8_str.size()), &wide[0], wide_len);
  return CComVariant(wide.c_str());
}

std::string format_duration(const int64_t total_seconds) {
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%02lld:%02lld:%02lld", total_seconds / 3600,
           (total_seconds % 3600) / 60, total_seconds % 60);
  return buffer;
}

void try_refresh_workshop_content() {
  try {
    game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0, "userContentReload\n");
    printf("Workshop items refreshed in-game.\n");
  } catch (...) {
    // Game not running yet, nothing to refresh
  }
}

namespace {
constexpr const char *WORKSHOP_STATUS_IDLE = "";

std::mutex workshop_status_mutex;
std::string workshop_status_message = WORKSHOP_STATUS_IDLE;
double workshop_progress_percent = 0.0;
std::string workshop_progress_details = "";
std::string workshop_download_folder = "";

std::atomic<bool> workshop_cancel_requested{false};
std::atomic<bool> workshop_paused{false};

struct workshop_browse_state {
  std::string items_json = "[]";
  std::string mode = "browse";
  std::string query;
  std::string source = "none";
  std::string error;
  bool loading = false;
  bool complete = false;
  uint64_t request_token = 0;
};

std::mutex workshop_browse_mutex;
workshop_browse_state workshop_browse_state_data{};
std::atomic<bool> workshop_browse_loading{false};
std::atomic<uint64_t> workshop_browse_request_token{0};

struct workshop_search_cache_entry {
  std::string items_json = "[]";
  std::chrono::steady_clock::time_point cached_at{};
};

constexpr auto workshop_search_cache_ttl = std::chrono::minutes(10);
constexpr size_t workshop_search_cache_max_entries = 24;
constexpr int workshop_browse_item_batch_delay_ms = 100;
constexpr int workshop_search_item_batch_delay_ms = 0;

std::mutex workshop_search_cache_mutex;
std::map<std::string, workshop_search_cache_entry> workshop_search_cache{};

bool is_current_browse_request(const uint64_t token) {
  return workshop_browse_request_token.load() == token;
}

bool is_active_browse_request(const uint64_t token) {
  return workshop_browse_loading.load() && is_current_browse_request(token);
}

void begin_browse_request(const uint64_t token, const std::string &mode,
                          const std::string &query,
                          const std::string &seed_items_json = "[]",
                          const std::string &source = "none") {
  workshop_browse_loading = true;

  std::lock_guard lock(workshop_browse_mutex);
  workshop_browse_state_data.items_json =
      seed_items_json.empty() ? "[]" : seed_items_json;
  workshop_browse_state_data.mode = mode;
  workshop_browse_state_data.query = query;
  workshop_browse_state_data.source = source;
  workshop_browse_state_data.error.clear();
  workshop_browse_state_data.loading = true;
  workshop_browse_state_data.complete = false;
  workshop_browse_state_data.request_token = token;
}

void set_browse_request_items(const uint64_t token, std::string items_json,
                              const std::string &source) {
  if (!is_current_browse_request(token)) {
    return;
  }

  std::lock_guard lock(workshop_browse_mutex);
  if (workshop_browse_state_data.request_token != token) {
    return;
  }

  workshop_browse_state_data.items_json =
      items_json.empty() ? "[]" : std::move(items_json);
  workshop_browse_state_data.source = source;
  workshop_browse_state_data.error.clear();
}

void set_browse_request_error(const uint64_t token, std::string error,
                              const std::string &source,
                              const bool replace_items = false) {
  if (!is_current_browse_request(token)) {
    return;
  }

  std::lock_guard lock(workshop_browse_mutex);
  if (workshop_browse_state_data.request_token != token) {
    return;
  }

  if (replace_items) {
    workshop_browse_state_data.items_json = "[]";
  }

  workshop_browse_state_data.source = source;
  workshop_browse_state_data.error = std::move(error);
}

void finish_browse_request(const uint64_t token) {
  if (is_current_browse_request(token)) {
    workshop_browse_loading = false;
  }

  std::lock_guard lock(workshop_browse_mutex);
  if (workshop_browse_state_data.request_token == token) {
    workshop_browse_state_data.loading = false;
    workshop_browse_state_data.complete = true;
  }
}

std::string normalize_workshop_search_query(std::string query) {
  utils::string::trim(query);
  return utils::string::to_lower(std::move(query));
}

bool try_get_cached_workshop_search(const std::string &query,
                                    std::string &items_json) {
  const auto normalized_query = normalize_workshop_search_query(query);
  if (normalized_query.empty()) {
    return false;
  }

  std::lock_guard lock(workshop_search_cache_mutex);

  const auto it = workshop_search_cache.find(normalized_query);
  if (it == workshop_search_cache.end()) {
    return false;
  }

  const auto now = std::chrono::steady_clock::now();
  if (now - it->second.cached_at > workshop_search_cache_ttl) {
    workshop_search_cache.erase(it);
    return false;
  }

  items_json = it->second.items_json;
  return !items_json.empty();
}

void store_cached_workshop_search(const std::string &query,
                                  const std::string &items_json) {
  const auto normalized_query = normalize_workshop_search_query(query);
  if (normalized_query.empty() || items_json.empty()) {
    return;
  }

  std::lock_guard lock(workshop_search_cache_mutex);

  const auto now = std::chrono::steady_clock::now();

  for (auto it = workshop_search_cache.begin();
       it != workshop_search_cache.end();) {
    if (now - it->second.cached_at > workshop_search_cache_ttl) {
      it = workshop_search_cache.erase(it);
    } else {
      ++it;
    }
  }

  if (workshop_search_cache.size() >= workshop_search_cache_max_entries) {
    auto oldest_it = workshop_search_cache.begin();
    for (auto it = workshop_search_cache.begin();
         it != workshop_search_cache.end(); ++it) {
      if (it->second.cached_at < oldest_it->second.cached_at) {
        oldest_it = it;
      }
    }

    if (oldest_it != workshop_search_cache.end()) {
      workshop_search_cache.erase(oldest_it);
    }
  }

  workshop_search_cache[normalized_query] = {items_json, now};
}

void save_workshop_backup(const std::string &json_data);

std::string extract_workshop_id(const std::string &input) {
  std::string s = input;
  utils::string::trim(s);
  bool all_digits = true;
  for (char c : s) {
    if (!std::isdigit(static_cast<unsigned char>(c))) {
      all_digits = false;
      break;
    }
  }
  if (all_digits && !s.empty())
    return s;
  auto pos = s.find("id=");
  if (pos != std::string::npos) {
    pos += 3;
    auto end = s.find_first_not_of("0123456789", pos);
    if (end == std::string::npos)
      end = s.size();
    if (end > pos)
      return s.substr(pos, end - pos);
  }
  size_t last_num_start = std::string::npos;
  for (size_t i = 0; i < s.size(); ++i) {
    if (std::isdigit(static_cast<unsigned char>(s[i]))) {
      if (last_num_start == std::string::npos ||
          (i > 0 && !std::isdigit(static_cast<unsigned char>(s[i - 1]))))
        last_num_start = i;
    }
  }
  if (last_num_start != std::string::npos) {
    size_t end = last_num_start;
    while (end < s.size() && std::isdigit(static_cast<unsigned char>(s[end])))
      ++end;
    return s.substr(last_num_start, end - last_num_start);
  }
  return {};
}

void set_workshop_status(const std::string &msg, double progress = -1.0,
                         const std::string &details = "") {
  std::lock_guard lock(workshop_status_mutex);
  workshop_status_message = msg;
  if (progress >= 0.0)
    workshop_progress_percent = progress;
  workshop_progress_details = details;
}

void reset_workshop_status() {
  std::lock_guard lock(workshop_status_mutex);
  workshop_status_message = WORKSHOP_STATUS_IDLE;
  workshop_progress_percent = 0.0;
  workshop_progress_details.clear();
  workshop_download_folder.clear();
}

std::string html_decode(const std::string &input) {
  std::string result = input;
  std::regex re("&#(\\d+);");
  std::smatch m;
  std::string::const_iterator searchStart(result.cbegin());
  std::string output;
  while (std::regex_search(searchStart, result.cend(), m, re)) {
    output.append(searchStart, m[0].first);
    int code = std::stoi(m[1].str());
    if (code >= 32 && code < 127) {
      output.push_back(static_cast<char>(code));
    } else {
      output.append(m[0].str());
    }
    searchStart = m[0].second;
  }
  output.append(searchStart, result.cend());
  result = output;

  size_t pos = 0;
  while ((pos = result.find("&quot;", pos)) != std::string::npos) {
    result.replace(pos, 6, "\"");
    pos += 1;
  }
  pos = 0;
  while ((pos = result.find("&amp;", pos)) != std::string::npos) {
    result.replace(pos, 5, "&");
    pos += 1;
  }
  pos = 0;
  while ((pos = result.find("&lt;", pos)) != std::string::npos) {
    result.replace(pos, 4, "<");
    pos += 1;
  }
  pos = 0;
  while ((pos = result.find("&gt;", pos)) != std::string::npos) {
    result.replace(pos, 4, ">");
    pos += 1;
  }
  return result;
}

std::string get_workshop_backup_path() {
  char cwd[MAX_PATH];
  GetCurrentDirectoryA(sizeof(cwd), cwd);
  return std::string(cwd) + "\\workshop_cache.json";
}

std::string load_workshop_backup() {
  try {
    const auto backup_path = get_workshop_backup_path();
    if (!utils::io::file_exists(backup_path)) {
      return {};
    }

    const auto content = utils::io::read_file(backup_path);
    if (!content.empty()) {
      rapidjson::Document doc;
      if (!doc.Parse(content.c_str()).HasParseError() && doc.IsArray()) {
        return content;
      }
    }
  } catch (...) {
  }
  return {};
}

void save_workshop_backup(const std::string &json_data) {
  try {
    const auto backup_path = get_workshop_backup_path();
    utils::io::write_file(backup_path, json_data);
  } catch (...) {
  }
}

std::string extract_image_url_from_description(const std::string &description) {
  try {
    std::smatch m;
    std::regex url_regex(
        R"((https?://[^\s\"\'<>\)]+\.(?:jpg|png|jpeg|gif|webp)))",
        std::regex::icase);
    if (std::regex_search(description, m, url_regex)) {
      return m[1].str();
    }
  } catch (...) {
  }
  return "";
}

std::string url_encode(const std::string &value) {
  std::string result;
  result.reserve(value.size() * 3);
  for (unsigned char c : value) {
    if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
      result += static_cast<char>(c);
    else {
      char buf[4];
      std::snprintf(buf, sizeof(buf), "%%%02X", c);
      result += buf;
    }
  }
  return result;
}

size_t count_substring_occurrences(const std::string &haystack,
                                   const std::string &needle) {
  if (needle.empty()) {
    return 0;
  }

  size_t count = 0;
  size_t pos = 0;
  while ((pos = haystack.find(needle, pos)) != std::string::npos) {
    ++count;
    pos += needle.size();
  }

  return count;
}

std::vector<std::pair<std::string, int>>
scrape_ids_and_ratings(const std::string &html) {
  std::vector<std::pair<std::string, int>> items;
  std::regex result_link_re(
      R"WORKSHOP(<a href="https://steamcommunity\.com/sharedfiles/filedetails/\?id=(\d+)"[^>]*>\s*<img )WORKSHOP",
      std::regex::icase);
  std::regex generic_link_re(
      R"(https://steamcommunity\.com/sharedfiles/filedetails/\?id=(\d+))",
      std::regex::icase);
  std::regex legacy_id_re(R"(sharedfile_(\d+))", std::regex::icase);
  std::regex legacy_star_re(R"((\d)-star)");

  struct id_pos {
    std::string id;
    size_t pos;
  };

  const auto collect_matches = [&](const std::regex &id_re) {
    std::vector<id_pos> matches{};
    std::set<std::string> page_seen{};

    auto begin = std::sregex_iterator(html.begin(), html.end(), id_re);
    auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
      std::string id = (*it)[1].str();
      if (!id.empty() && !page_seen.count(id)) {
        page_seen.insert(id);
        matches.push_back({id, static_cast<size_t>(it->position())});
      }
    }

    return matches;
  };

  auto matches = collect_matches(result_link_re);
  if (matches.empty()) {
    matches = collect_matches(generic_link_re);
  }
  if (matches.empty()) {
    matches = collect_matches(legacy_id_re);
  }

  for (size_t i = 0; i < matches.size(); ++i) {
    const size_t start = matches[i].pos;
    const size_t block_end = (i + 1 < matches.size())
                                 ? matches[i + 1].pos
                                 : std::min(start + 8000, html.size());
    const std::string block = html.substr(start, block_end - start);

    int stars = static_cast<int>(std::min<size_t>(
        5, count_substring_occurrences(block, "SVGIcon_Star_Filled")));

    if (stars == 0) {
      std::smatch star_match;
      if (std::regex_search(block, star_match, legacy_star_re)) {
        stars = std::stoi(star_match[1].str());
      }
    }

    items.push_back({matches[i].id, stars});
  }

  return items;
}

struct workshop_scrape_result {
  bool success = false;
  std::vector<std::string> ids{};
  std::map<std::string, int> ratings{};
  std::string error{};
};

struct workshop_fetch_result {
  bool success = false;
  std::string items_json = "[]";
  std::string error{};
};

std::string
serialize_workshop_item_objects(const std::vector<std::string> &item_objects) {
  if (item_objects.empty()) {
    return "[]";
  }

  size_t total_size = 2;
  for (const auto &item_json : item_objects) {
    total_size += item_json.size() + 1;
  }

  std::string result;
  result.reserve(total_size);
  result.push_back('[');

  for (size_t i = 0; i < item_objects.size(); ++i) {
    if (i > 0) {
      result.push_back(',');
    }

    result += item_objects[i];
  }

  result.push_back(']');
  return result;
}

utils::http::headers build_workshop_headers() {
  utils::http::headers h{};
  h["User-Agent"] =
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36";
  h["Accept"] = "text/html";
  h["Accept-Language"] = "en-US,en;q=0.9";
  h["Referer"] = "https://steamcommunity.com/app/311210/workshop/";
  return h;
}

workshop_scrape_result
scrape_workshop_listing_pages(const std::function<std::string(int)> &build_url,
                              const int max_pages, const int page_delay_ms,
                              const uint64_t request_token) {
  workshop_scrape_result result{};
  auto headers = build_workshop_headers();
  std::set<std::string> seen_ids{};
  bool saw_response = false;

  for (int page = 1;
       page <= max_pages && is_active_browse_request(request_token); ++page) {
    try {
      const auto resp = utils::http::get_data(build_url(page), headers, {}, 5);
      if (!resp || resp->empty()) {
        if (!saw_response && result.error.empty()) {
          result.error = "Steam workshop returned an empty response.";
        }
        break;
      }

      saw_response = true;

      const auto page_results = scrape_ids_and_ratings(*resp);
      if (page_results.empty()) {
        break;
      }

      for (const auto &[id, rating] : page_results) {
        if (id.empty() || seen_ids.count(id)) {
          continue;
        }

        seen_ids.insert(id);
        result.ids.push_back(id);
        result.ratings[id] = rating;
      }

      if (page < max_pages && page_delay_ms > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(page_delay_ms));
      }
    } catch (...) {
      if (!saw_response && result.error.empty()) {
        result.error = "Steam workshop request failed.";
      }
      break;
    }
  }

  result.success = saw_response;
  if (!result.success && result.error.empty()) {
    result.error = "Steam workshop did not return any data.";
  }

  return result;
}

workshop_fetch_result
build_workshop_items_json(const std::vector<std::string> &ids,
                          const std::map<std::string, int> &item_ratings,
                          const uint64_t request_token,
                          const int batch_delay_ms) {
  workshop_fetch_result result{};
  std::vector<std::string> item_objects{};
  constexpr size_t batch_size = 20;

  for (size_t i = 0; i < ids.size() && is_active_browse_request(request_token);
       i += batch_size) {
    const std::vector<std::string> batch(
        ids.begin() + i, ids.begin() + std::min(i + batch_size, ids.size()));
    for (const auto &item : ::workshop::get_details(batch)) {
      const std::string description =
          item.description.size() > 2000
              ? item.description.substr(0, 2000) + "..."
              : item.description;
      const std::string image_url =
          item.preview_url.empty()
              ? extract_image_url_from_description(description)
              : item.preview_url;
      const auto rating = item_ratings.find(item.id);

      rapidjson::StringBuffer item_buf;
      rapidjson::Writer<rapidjson::StringBuffer> w(item_buf);
      w.StartObject();
      w.Key("id");
      w.String(item.id.c_str());
      w.Key("title");
      w.String(html_decode(item.title).c_str());
      w.Key("description");
      w.String(description.c_str());
      w.Key("imageUrl");
      w.String(image_url.c_str());
      w.Key("starRating");
      w.Int(rating != item_ratings.end() ? rating->second : 0);
      w.Key("subs");
      w.Int64(item.subs);
      w.Key("favorites");
      w.Int64(item.favorites);
      w.Key("file_size");
      w.Uint64(item.file_size);
      w.Key("tags");
      w.StartArray();
      for (const auto &tag : item.tags)
        w.String(tag.c_str());
      w.EndArray();
      w.EndObject();
      item_objects.emplace_back(item_buf.GetString(), item_buf.GetSize());
    }

    if (!item_objects.empty()) {
      result.items_json = serialize_workshop_item_objects(item_objects);
      set_browse_request_items(request_token, result.items_json, "network");
    }
    if (i + batch_size < ids.size() && batch_delay_ms > 0)
      std::this_thread::sleep_for(std::chrono::milliseconds(batch_delay_ms));
  }

  if (item_objects.empty() && !ids.empty()) {
    result.error = "Steam workshop item details could not be loaded.";
    return result;
  }
  result.success = true;
  result.items_json = serialize_workshop_item_objects(item_objects);
  return result;
}

workshop_fetch_result search_workshop_by_name(const std::string &search_text,
                                              const uint64_t request_token) {
  try {
    const std::string encoded_query = url_encode(search_text);
    const auto listing = scrape_workshop_listing_pages(
        [&](const int page) {
          return "https://steamcommunity.com/workshop/browse/"
                 "?appid=311210&searchtext=" +
                 encoded_query +
                 "&browsesort=textsearch&section=readytouseitems&"
                 "actualsort=textsearch&p=" +
                 std::to_string(page);
        },
        10, 0, request_token);

    if (!listing.success) {
      return {false, "[]",
              listing.error.empty() ? "Unable to load workshop search results."
                                    : listing.error};
    }

    return build_workshop_items_json(listing.ids, listing.ratings,
                                     request_token,
                                     workshop_search_item_batch_delay_ms);
  } catch (...) {
    return {false, "[]", "Unexpected workshop search error."};
  }
}

void workshop_search_fetch_thread(const std::string &search_text,
                                  const uint64_t request_token) {
  const auto result = search_workshop_by_name(search_text, request_token);

  if (is_current_browse_request(request_token)) {
    if (result.success) {
      store_cached_workshop_search(search_text, result.items_json);
      set_browse_request_items(request_token, result.items_json, "network");
    } else {
      set_browse_request_error(request_token,
                               result.error.empty()
                                   ? "Unable to load workshop search results."
                                   : result.error,
                               "error", true);
    }
  }

  finish_browse_request(request_token);
}

workshop_fetch_result fetch_all_workshop_items(const uint64_t request_token) {
  try {
    const auto listing = scrape_workshop_listing_pages(
        [&](const int page) {
          return "https://steamcommunity.com/workshop/browse/"
                 "?appid=311210&browsesort=mostrecent&section="
                 "readytouseitems&actualsort=mostrecent&p=" +
                 std::to_string(page);
        },
        20, 200, request_token);

    if (!listing.success) {
      return {false, "[]",
              listing.error.empty() ? "Unable to load workshop browse results."
                                    : listing.error};
    }

    return build_workshop_items_json(listing.ids, listing.ratings,
                                     request_token,
                                     workshop_browse_item_batch_delay_ms);
  } catch (...) {
    return {false, "[]", "Unexpected workshop browse error."};
  }
}

void workshop_browse_fetch_thread(int /*page_num*/,
                                  const uint64_t request_token) {
  const auto result = fetch_all_workshop_items(request_token);

  if (is_current_browse_request(request_token)) {
    if (result.success) {
      if (!result.items_json.empty() && result.items_json != "[]") {
        save_workshop_backup(result.items_json);
      }
      set_browse_request_items(request_token, result.items_json, "network");
    } else {
      bool has_seed_items = false;
      {
        std::lock_guard lock(workshop_browse_mutex);
        has_seed_items =
            workshop_browse_state_data.request_token == request_token &&
            workshop_browse_state_data.items_json != "[]";
      }

      if (has_seed_items) {
        set_browse_request_error(request_token, "", "cache-fallback");
      } else {
        set_browse_request_error(request_token,
                                 result.error.empty()
                                     ? "Unable to load workshop browse results."
                                     : result.error,
                                 "error", true);
      }
    }
  }

  finish_browse_request(request_token);
}

void workshop_download_thread(const std::string &workshop_id,
                              const bool update_existing) {
  if (::workshop::downloading_workshop_item) {
    set_workshop_status("Error: An in-game download is already in progress.",
                        0.0,
                        "Wait for the current in-game download to finish "
                        "before starting a new one from the launcher.");
    return;
  }

  ::workshop::launcher_downloading = true;
  const auto reset_downloading =
      utils::finally([]() { ::workshop::launcher_downloading = false; });
  reset_workshop_status();
  workshop_paused = false;
  set_workshop_status("Initializing...", -1.0, "Workshop ID: " + workshop_id);

  const auto existing = ::workshop::find_installed(workshop_id);
  if (!existing.empty() && !update_existing) {
    set_workshop_status("Already installed.", 100.0,
                        "This workshop item is already installed at:\n" +
                            existing.string() +
                            "\nRemove it first if you want to reinstall.");
    return;
  }

  const auto info = ::workshop::get_steam_workshop_info(workshop_id);
  const std::string title =
      info.title.empty() ? "Workshop #" + workshop_id : info.title;
  const auto started = std::chrono::steady_clock::now();
  {
    std::lock_guard lock(workshop_status_mutex);
    workshop_download_folder =
        (game::get_game_path() / "workshop_downloads" / workshop_id).string();
  }

  ::workshop::download_job job{
      workshop_id, {}, &workshop_cancel_requested, &workshop_paused, {}};
  job.report = [&](const ::workshop::download_status &status) {
    if (workshop_paused.load()) {
      set_workshop_status("Paused - " + title, -1.0,
                          "Download paused. Click Resume to continue.");
      return;
    }
    std::string details;
    if (status.total) {
      details = ::workshop::human_readable_size(status.downloaded) + " / " +
                ::workshop::human_readable_size(status.total);
      if (status.eta >= 0) {
        details += " | " +
                   ::workshop::human_readable_size(
                       static_cast<uint64_t>(status.speed)) +
                   "/s | ETA: " + format_duration(status.eta);
      }
      details += " | ";
    }
    details += "Elapsed: " +
               format_duration(std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::steady_clock::now() - started)
                                   .count());
    set_workshop_status(
        status.message.empty() ? "Downloading " + title + "..."
                               : status.message,
        status.total
            ? std::min(99.0, 100.0 * static_cast<double>(status.downloaded) /
                                 static_cast<double>(status.total))
            : -1.0,
        details);
  };

  std::filesystem::path installed;
  const auto result = ::workshop::download_item(job, &installed);
  if (result == ::workshop::download_result::success) {
    set_workshop_status(
        "Done! Download complete.", 100.0,
        title + " is ready to use (" +
            ::workshop::human_readable_size(
                ::workshop::compute_folder_size_bytes(installed)) +
            ").");
    try_refresh_workshop_content();
  } else if (result == ::workshop::download_result::cancelled) {
    set_workshop_status("Canceled.", 0.0, "");
  } else {
    set_workshop_status("Error: Workshop download failed.", 0.0,
                        ::workshop::describe(result));
  }
}
} // namespace

std::map<std::string, workshop_item_meta>
batch_get_workshop_meta(const std::vector<std::string> &ids) {
  std::map<std::string, workshop_item_meta> result;
  for (const auto &item : ::workshop::get_details(ids)) {
    result[item.id] = {item.time_updated,
                       item.file_size,
                       item.description,
                       item.preview_url,
                       0,
                       item.subs,
                       item.favorites};
  }
  return result;
}

CComVariant start_download(const std::vector<html_argument> &params,
                           const bool update) {
  if (params.empty() || !params[0].is_string())
    return CComVariant("Error: no ID");
  const std::string id = extract_workshop_id(params[0].get_string());
  if (id.empty())
    return CComVariant("Error: Invalid Workshop ID or link.");
  if (::workshop::downloading_workshop_item)
    return CComVariant("Error: An in-game download is already in progress. "
                       "Wait for it to finish.");
  if (::workshop::launcher_downloading.load() &&
      !workshop_cancel_requested.load())
    return CComVariant("Error: A launcher download is already in progress.");
  std::thread([id, update] {
    while (::workshop::launcher_downloading.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    workshop_cancel_requested = false;
    reset_workshop_status();
    workshop_download_thread(id, update);
  }).detach();
  return CComVariant(update ? "Update started" : "Download started");
}

std::mutex size_lookup_mutex;
std::map<std::string, std::optional<uint64_t>> size_lookup;

void register_callbacks(html_frame *frame) {
  frame->register_callback(
      "workshopGetSize",
      [](const std::vector<html_argument> &params) -> CComVariant {
        if (params.empty() || !params[0].is_string())
          return CComVariant("0");
        const std::string id = extract_workshop_id(params[0].get_string());
        if (id.empty())
          return CComVariant("0");
        std::lock_guard lock(size_lookup_mutex);
        const auto [entry, added] = size_lookup.try_emplace(id);
        if (added) {
          std::thread([id] {
            const uint64_t size =
                ::workshop::scrape_workshop_file_size_bytes(id);
            std::lock_guard lock(size_lookup_mutex);
            size_lookup[id] = size;
          }).detach();
        }
        return entry->second
                   ? CComVariant(std::to_string(*entry->second).c_str())
                   : CComVariant("");
      });

  frame->register_callback(
      "workshopGetStatus",
      [](const std::vector<html_argument> & /*params*/) -> CComVariant {
        std::lock_guard lock(workshop_status_mutex);
        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> w(buf);
        w.StartObject();
        w.Key("message");
        w.String(workshop_status_message.c_str());
        w.Key("progress");
        w.Double(workshop_progress_percent);
        w.Key("details");
        w.String(workshop_progress_details.c_str());
        w.Key("downloadFolder");
        w.String(workshop_download_folder.c_str());
        w.Key("paused");
        w.Bool(workshop_paused.load());
        w.EndObject();
        return utf8_variant(std::string(buf.GetString(), buf.GetSize()));
      });

  frame->register_callback(
      "workshopCheckInstalled",
      [](const std::vector<html_argument> &params) -> CComVariant {
        if (params.empty() || !params[0].is_string())
          return CComVariant("");
        auto id = extract_workshop_id(params[0].get_string());
        if (id.empty())
          return CComVariant("");
        return CComVariant(::workshop::find_installed(id).string().c_str());
      });

  frame->register_callback(
      "workshopDownload",
      [](const std::vector<html_argument> &params) -> CComVariant {
        return start_download(params, false);
      });

  frame->register_callback(
      "workshopUpdate",
      [](const std::vector<html_argument> &params) -> CComVariant {
        return start_download(params, true);
      });

  frame->register_callback(
      "workshopCancelDownload",
      [](const std::vector<html_argument> & /*params*/) -> CComVariant {
        workshop_cancel_requested = true;
        workshop_paused = false;
        reset_workshop_status();
        return CComVariant("Cancel requested");
      });

  frame->register_callback(
      "workshopPauseDownload",
      [](const std::vector<html_argument> & /*params*/) -> CComVariant {
        workshop_paused = true;
        return CComVariant("Pause requested");
      });

  frame->register_callback(
      "workshopResumeDownload",
      [](const std::vector<html_argument> & /*params*/) -> CComVariant {
        workshop_paused = false;
        return CComVariant("Resume requested");
      });

  frame->register_callback(
      "workshopIsPaused",
      [](const std::vector<html_argument> & /*params*/) -> CComVariant {
        return CComVariant(workshop_paused.load() ? "true" : "false");
      });

  frame->register_callback(
      "workshopBrowse",
      [](const std::vector<html_argument> &params) -> CComVariant {
        int page = 1;
        if (!params.empty() && params[0].is_string()) {
          try {
            page = std::stoi(params[0].get_string());
          } catch (...) {
            page = 1;
          }
        }
        if (page < 1)
          page = 1;

        const auto request_token =
            workshop_browse_request_token.fetch_add(1) + 1;
        const auto cached_items = load_workshop_backup();
        begin_browse_request(request_token, "browse", "", cached_items,
                             cached_items.empty() ? "none" : "cache");
        std::thread(workshop_browse_fetch_thread, page, request_token).detach();
        return CComVariant("Fetching...");
      });

  frame->register_callback(
      "workshopSearch",
      [](const std::vector<html_argument> &params) -> CComVariant {
        std::string query;
        if (!params.empty() && params[0].is_string())
          query = params[0].get_string();
        utils::string::trim(query);
        if (query.empty())
          return CComVariant("Error: empty search query");

        const auto request_token =
            workshop_browse_request_token.fetch_add(1) + 1;

        std::string cached_items;
        if (try_get_cached_workshop_search(query, cached_items)) {
          begin_browse_request(request_token, "search", query, cached_items,
                               "search-cache");
          finish_browse_request(request_token);
          return CComVariant("Loaded from cache");
        }

        begin_browse_request(request_token, "search", query);
        std::thread(workshop_search_fetch_thread, query, request_token)
            .detach();
        return CComVariant("Searching...");
      });

  frame->register_callback(
      "workshopGetBrowseState",
      [](const std::vector<html_argument> & /*params*/) -> CComVariant {
        std::lock_guard lock(workshop_browse_mutex);

        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> w(buf);
        w.StartObject();
        w.Key("mode");
        w.String(workshop_browse_state_data.mode.c_str());
        w.Key("query");
        w.String(workshop_browse_state_data.query.c_str());
        w.Key("source");
        w.String(workshop_browse_state_data.source.c_str());
        w.Key("error");
        w.String(workshop_browse_state_data.error.c_str());
        w.Key("loading");
        w.Bool(workshop_browse_state_data.loading);
        w.Key("complete");
        w.Bool(workshop_browse_state_data.complete);
        w.Key("requestToken");
        w.Uint64(workshop_browse_state_data.request_token);
        w.Key("items");

        rapidjson::Document items_doc;
        if (!items_doc.Parse(workshop_browse_state_data.items_json.c_str())
                 .HasParseError() &&
            items_doc.IsArray()) {
          items_doc.Accept(w);
        } else {
          w.StartArray();
          w.EndArray();
        }

        w.EndObject();
        return utf8_variant(std::string(buf.GetString(), buf.GetSize()));
      });

  frame->register_callback(
      "workshopGetBrowseData",
      [](const std::vector<html_argument> & /*params*/) -> CComVariant {
        std::lock_guard lock(workshop_browse_mutex);
        return utf8_variant(workshop_browse_state_data.items_json);
      });

  frame->register_callback(
      "workshopIsBrowseLoading",
      [](const std::vector<html_argument> & /*params*/) -> CComVariant {
        return CComVariant(workshop_browse_loading.load() ? "true" : "false");
      });
}
} // namespace launcher::workshop
