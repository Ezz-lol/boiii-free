#include <std_include.hpp>

#include <game/game.hpp>

#include <component/steamcmd.hpp>
#include <component/workshop.hpp>

#include <launcher/html/html_frame.hpp>
#include <launcher/launcher_workshop.hpp>

#include <atomic>
#include <chrono>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <map>
#include <mutex>
#include <regex>
#include <set>
#include <sstream>
#include <thread>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <utils/compression.hpp>
#include <utils/finally.hpp>
#include <utils/http.hpp>
#include <utils/io.hpp>
#include <utils/nt.hpp>
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

std::string human_readable_size(uint64_t bytes);
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

constexpr const char *STEAM_WORKSHOP_API =
    "https://api.steampowered.com/ISteamRemoteStorage/GetPublishedFileDetails/"
    "v1/";
constexpr int BO3_APP_ID = 311210;

uint64_t parse_human_size_to_bytes(const std::string &text) {
  if (text.empty())
    return 0;

  double value = 0.0;
  std::string unit;
  std::istringstream ss(text);
  if (!(ss >> value))
    return 0;

  if (!(ss >> unit)) {
    // If no unit, assume bytes
    return static_cast<uint64_t>(value);
  }

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
  return static_cast<uint64_t>(bytes);
}

uint64_t scrape_workshop_file_size_bytes(const std::string &workshop_id) {
  utils::http::headers headers;
  headers["User-Agent"] =
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36";
  headers["Accept-Language"] = "en-US,en;q=0.9";
  const std::optional<std::string> html = utils::http::get_data(
      "https://steamcommunity.com/sharedfiles/filedetails/?id=" + workshop_id,
      headers, {}, 2);
  if (!html) {
    return 0;
  }
  constexpr std::string_view marker = "detailsStatRight\">";
  const size_t start = html->find(marker);
  if (start == std::string::npos) {
    return 0;
  }
  std::string text = html->substr(start + marker.size(),
                                  html->find('<', start + marker.size()) -
                                      start - marker.size());
  std::erase(text, ',');
  return parse_human_size_to_bytes(text);
}

struct workshop_info {
  uint64_t file_size = 0;
  std::string title;
};

workshop_info get_steam_workshop_info(const std::string &workshop_id) {
  workshop_info info{};
  if (workshop_id.empty())
    return info;
  try {
    std::string body = "itemcount=1&publishedfileids[0]=" + workshop_id;
    std::optional<std::string> resp =
        utils::http::post_data(STEAM_WORKSHOP_API, body, 10);
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
    const rapidjson::Value &first = details_it->value[0];
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
        info.file_size = static_cast<uint64_t>(size_it->value.GetInt64());
      else if (size_it->value.IsUint())
        info.file_size = size_it->value.GetUint();
      else if (size_it->value.IsInt())
        info.file_size = static_cast<uint64_t>(size_it->value.GetInt());
      else if (size_it->value.IsString())
        info.file_size = static_cast<uint64_t>(
            std::strtoull(size_it->value.GetString(), nullptr, 10));
    }

    return info;
  } catch (...) {
    return info;
  }
}

uint64_t compute_folder_size_bytes(const std::filesystem::path &folder) {
  std::error_code ec;
  if (!std::filesystem::exists(folder, ec))
    return 0;
  uint64_t total = 0;
  for (const auto &entry :
       std::filesystem::recursive_directory_iterator(folder, ec)) {
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

bool copy_directory_recursive_with_progress(
    const std::filesystem::path &from, const std::filesystem::path &to,
    const uint64_t total_bytes,
    const std::function<void(uint64_t)> &on_bytes_copied) {
  std::error_code ec;
  if (!std::filesystem::exists(from, ec))
    return false;
  std::filesystem::create_directories(to, ec);
  if (ec) {
    if (ec == std::errc::no_space_on_device) {
      set_workshop_status(
          "Error: Disk full - cannot create destination folder.", 0.0,
          "Free up disk space and try again.");
    }
    return false;
  }

  uint64_t copied_bytes = 0;
  auto last_update = std::chrono::steady_clock::now();
  for (const auto &entry :
       std::filesystem::recursive_directory_iterator(from, ec)) {
    if (ec)
      return false;
    if (workshop_cancel_requested.load())
      return false;

    const std::filesystem::path rel =
        std::filesystem::relative(entry.path(), from, ec);
    if (ec)
      return false;
    const auto dest_path = to / rel;

    if (entry.is_directory(ec)) {
      std::filesystem::create_directories(dest_path, ec);
      if (ec)
        return false;
      continue;
    }
    if (entry.is_regular_file(ec)) {
      std::filesystem::create_directories(dest_path.parent_path(), ec);
      if (ec)
        return false;

      const auto file_size =
          static_cast<uint64_t>(std::filesystem::file_size(entry.path(), ec));
      if (ec)
        return false;
      std::filesystem::copy_file(
          entry.path(), dest_path,
          std::filesystem::copy_options::overwrite_existing, ec);
      if (ec) {
        if (ec == std::errc::no_space_on_device) {
          set_workshop_status("Error: Disk full during file copy.", 0.0,
                              "Ran out of disk space while copying files. Free "
                              "up space and try again.");
        }
        return false;
      }

      copied_bytes += file_size;
      const auto now = std::chrono::steady_clock::now();
      if (on_bytes_copied &&
          (now - last_update) > std::chrono::milliseconds(250)) {
        last_update = now;
        on_bytes_copied(std::min(copied_bytes, total_bytes));
      }
    }
  }
  if (on_bytes_copied)
    on_bytes_copied(std::min(copied_bytes, total_bytes));
  return true;
}

std::string human_readable_size(uint64_t bytes) {
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

int64_t parse_json_int64(const rapidjson::Value &v) {
  if (v.IsInt64())
    return v.GetInt64();
  if (v.IsInt())
    return v.GetInt();
  if (v.IsUint64())
    return static_cast<int64_t>(v.GetUint64());
  if (v.IsUint())
    return v.GetUint();
  if (v.IsString())
    return static_cast<int64_t>(std::atoll(v.GetString()));
  return 0;
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

  if (ids.empty()) {
    result.success = true;
    return result;
  }

  bool wrote_any_item = false;
  const size_t batch_size = 20;
  std::vector<std::string> item_objects{};
  item_objects.reserve(ids.size());

  for (size_t i = 0; i < ids.size() && is_active_browse_request(request_token);
       i += batch_size) {
    try {
      const size_t batch_end = std::min(i + batch_size, ids.size());
      const int count = static_cast<int>(batch_end - i);
      bool batch_added_item = false;

      std::string body = "itemcount=" + std::to_string(count);
      for (int j = 0; j < count; ++j) {
        body += "&publishedfileids[" + std::to_string(j) + "]=" + ids[i + j];
      }

      const auto api_resp =
          utils::http::post_data(STEAM_WORKSHOP_API, body, 15);
      if (!api_resp || api_resp->empty()) {
        continue;
      }

      rapidjson::Document doc;
      if (doc.Parse(api_resp->c_str()).HasParseError() || !doc.IsObject()) {
        continue;
      }

      const auto resp_it = doc.FindMember("response");
      if (resp_it == doc.MemberEnd() || !resp_it->value.IsObject()) {
        continue;
      }

      const auto details_it = resp_it->value.FindMember("publishedfiledetails");
      if (details_it == resp_it->value.MemberEnd() ||
          !details_it->value.IsArray()) {
        continue;
      }

      for (auto &item : details_it->value.GetArray()) {
        if (!item.IsObject()) {
          continue;
        }

        const auto app_it = item.FindMember("consumer_app_id");
        if (app_it != item.MemberEnd()) {
          int app_id = 0;
          if (app_it->value.IsInt()) {
            app_id = app_it->value.GetInt();
          } else if (app_it->value.IsInt64()) {
            app_id = static_cast<int>(app_it->value.GetInt64());
          } else if (app_it->value.IsString()) {
            app_id = std::atoi(app_it->value.GetString());
          }

          if (app_id != 0 && app_id != BO3_APP_ID) {
            continue;
          }
        }

        std::string id{};
        std::string title{};
        std::string description{};
        std::string image_url{};

        const auto id_it = item.FindMember("publishedfileid");
        if (id_it != item.MemberEnd() && id_it->value.IsString()) {
          id = id_it->value.GetString();
        }
        if (id.empty()) {
          continue;
        }

        const auto title_it = item.FindMember("title");
        if (title_it != item.MemberEnd() && title_it->value.IsString()) {
          title = html_decode(title_it->value.GetString());
        }

        const auto desc_it = item.FindMember("description");
        if (desc_it != item.MemberEnd() && desc_it->value.IsString()) {
          description = desc_it->value.GetString();
          if (description.size() > 2000) {
            description = description.substr(0, 2000) + "...";
          }
        }

        const auto img_it = item.FindMember("preview_url");
        if (img_it != item.MemberEnd() && img_it->value.IsString()) {
          image_url = img_it->value.GetString();
        }
        if (image_url.empty()) {
          image_url = extract_image_url_from_description(description);
        }

        int64_t subs = 0;
        int64_t favorites = 0;

        const auto subs_it = item.FindMember("lifetime_subscriptions");
        if (subs_it != item.MemberEnd()) {
          subs = parse_json_int64(subs_it->value);
        } else {
          const auto alt_subs_it = item.FindMember("subscriptions");
          if (alt_subs_it != item.MemberEnd()) {
            subs = parse_json_int64(alt_subs_it->value);
          }
        }

        const auto fav_it = item.FindMember("lifetime_favorited");
        if (fav_it != item.MemberEnd()) {
          favorites = parse_json_int64(fav_it->value);
        } else {
          const auto alt_fav_it = item.FindMember("favorited");
          if (alt_fav_it != item.MemberEnd()) {
            favorites = parse_json_int64(alt_fav_it->value);
          }
        }

        uint64_t file_size = 0;
        const auto fs_it = item.FindMember("file_size");
        if (fs_it != item.MemberEnd()) {
          if (fs_it->value.IsUint64()) {
            file_size = fs_it->value.GetUint64();
          } else if (fs_it->value.IsUint()) {
            file_size = fs_it->value.GetUint();
          } else if (fs_it->value.IsString()) {
            file_size = std::strtoull(fs_it->value.GetString(), nullptr, 10);
          }
        }

        const int star_rating =
            item_ratings.count(id) ? item_ratings.at(id) : 0;

        rapidjson::StringBuffer item_buf;
        rapidjson::Writer<rapidjson::StringBuffer> item_writer(item_buf);
        item_writer.StartObject();
        item_writer.Key("id");
        item_writer.String(id.c_str());
        item_writer.Key("title");
        item_writer.String(title.c_str());
        item_writer.Key("description");
        item_writer.String(description.c_str());
        item_writer.Key("imageUrl");
        item_writer.String(image_url.c_str());
        item_writer.Key("starRating");
        item_writer.Int(star_rating);
        item_writer.Key("subs");
        item_writer.Int64(subs);
        item_writer.Key("favorites");
        item_writer.Int64(favorites);
        item_writer.Key("file_size");
        item_writer.Uint64(file_size);
        item_writer.Key("tags");
        item_writer.StartArray();
        const auto tags_it = item.FindMember("tags");
        if (tags_it != item.MemberEnd() && tags_it->value.IsArray()) {
          for (const rapidjson::Value &tag : tags_it->value.GetArray()) {
            const auto name_it = tag.FindMember("tag");
            if (name_it != tag.MemberEnd() && name_it->value.IsString()) {
              item_writer.String(name_it->value.GetString());
            }
          }
        }
        item_writer.EndArray();
        item_writer.EndObject();

        item_objects.emplace_back(item_buf.GetString(), item_buf.GetSize());

        wrote_any_item = true;
        batch_added_item = true;
      }

      if (batch_added_item) {
        result.items_json = serialize_workshop_item_objects(item_objects);
        set_browse_request_items(request_token, result.items_json, "network");
      }

      if (i + batch_size < ids.size() && batch_delay_ms > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(batch_delay_ms));
      }
    } catch (...) {
    }
  }

  if (!wrote_any_item) {
    result.error = "Steam workshop item details could not be loaded.";
    return result;
  }

  result.success = true;
  if (result.items_json.empty()) {
    result.items_json = serialize_workshop_item_objects(item_objects);
  }
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

bool has_zone_content(const std::filesystem::path &dir) {
  std::error_code ec;
  if (!std::filesystem::exists(dir, ec))
    return false;
  return compute_folder_size_bytes(dir) > 1024;
}

std::filesystem::path
get_steam_workshop_content_path(const std::filesystem::path &game_path) {
  const std::filesystem::path steamapps = game_path.parent_path().parent_path();
  const std::filesystem::path ws_path =
      steamapps / "workshop" / "content" / game::APP_ID_STR;
  std::error_code ec;
  if (std::filesystem::exists(ws_path, ec))
    return ws_path;
  return {};
}

bool check_steam_ws_folder(const std::filesystem::path &steam_ws,
                           const std::string &workshop_id) {
  if (steam_ws.empty())
    return false;
  std::error_code ec;
  auto item_dir = steam_ws / workshop_id;
  if (!std::filesystem::exists(item_dir, ec))
    return false;
  if (has_zone_content(item_dir / "zone"))
    return true;
  for (const auto &sub : std::filesystem::directory_iterator(item_dir, ec)) {
    if (sub.is_directory(ec) && has_zone_content(sub.path() / "zone"))
      return true;
  }
  return false;
}

std::string find_installed_workshop_item(const std::filesystem::path &game_path,
                                         const std::string &workshop_id) {
  std::error_code ec;
  const char *parent_dirs[] = {"mods", "usermaps"};

  for (const auto &parent_name : parent_dirs) {
    std::filesystem::path candidate =
        game_path / parent_name / workshop_id / "zone";
    if (has_zone_content(candidate))
      return candidate.parent_path().string();
  }

  auto steam_ws = get_steam_workshop_content_path(game_path);
  if (check_steam_ws_folder(steam_ws, workshop_id))
    return (steam_ws / workshop_id).string();

  for (const auto &parent_name : parent_dirs) {
    std::filesystem::path parent = game_path / parent_name;
    if (!std::filesystem::exists(parent, ec))
      continue;
    for (const auto &entry : std::filesystem::directory_iterator(parent, ec)) {
      if (!entry.is_directory(ec))
        continue;
      std::filesystem::path zone_dir = entry.path() / "zone";
      if (!std::filesystem::exists(zone_dir, ec))
        continue;

      std::filesystem::path wsjson = zone_dir / "workshop.json";
      if (!std::filesystem::exists(wsjson, ec))
        continue;
      std::string json_str;
      if (!utils::io::read_file(wsjson.string(), &json_str) || json_str.empty())
        continue;
      rapidjson::Document doc;
      if (doc.Parse(json_str.c_str()).HasParseError() || !doc.IsObject())
        continue;

      auto pfid = doc.FindMember("PublishedFileId");
      if (pfid != doc.MemberEnd() && pfid->value.IsString()) {
        if (std::string(pfid->value.GetString()) == workshop_id) {
          if (has_zone_content(zone_dir))
            return entry.path().string();
        }
      }
      auto pubid = doc.FindMember("PublisherID");
      if (pubid != doc.MemberEnd() && pubid->value.IsString()) {
        if (std::string(pubid->value.GetString()) == workshop_id) {
          if (has_zone_content(zone_dir))
            return entry.path().string();
        }
      }
    }
  }

  try {
    std::string body = "itemcount=1&publishedfileids[0]=" + workshop_id;
    auto api_resp = utils::http::post_data(STEAM_WORKSHOP_API, body, 8);
    if (api_resp && !api_resp->empty()) {
      rapidjson::Document api_doc;
      if (!api_doc.Parse(api_resp->c_str()).HasParseError() &&
          api_doc.IsObject()) {
        auto ri = api_doc.FindMember("response");
        if (ri != api_doc.MemberEnd() && ri->value.IsObject()) {
          auto di = ri->value.FindMember("publishedfiledetails");
          if (di != ri->value.MemberEnd() && di->value.IsArray() &&
              !di->value.Empty()) {
            const auto &item = di->value[0];

            std::string mod_type = "mod";
            auto ti = item.FindMember("tags");
            if (ti != item.MemberEnd() && ti->value.IsArray()) {
              for (auto it = ti->value.Begin(); it != ti->value.End(); ++it) {
                if (it->IsObject()) {
                  auto tag_it = it->FindMember("tag");
                  if (tag_it != it->MemberEnd() && tag_it->value.IsString()) {
                    std::string t = tag_it->value.GetString();
                    for (auto &c : t)
                      c = static_cast<char>(
                          std::tolower(static_cast<unsigned char>(c)));
                    if (t == "map" || t == "maps" ||
                        t.find("map") != std::string::npos) {
                      mod_type = "map";
                      break;
                    }
                  }
                }
              }
            }

            std::string folder_name = workshop_id;

            auto title_it = item.FindMember("title");
            if (title_it != item.MemberEnd() && title_it->value.IsString()) {
              std::string title = title_it->value.GetString();
              std::string safe_name;
              for (char c : title) {
                if (std::isalnum(static_cast<unsigned char>(c)) || c == ' ' ||
                    c == '-' || c == '_')
                  safe_name += c;
              }
              while (!safe_name.empty() && safe_name.back() == ' ')
                safe_name.pop_back();
              while (!safe_name.empty() && safe_name.front() == ' ')
                safe_name.erase(safe_name.begin());
              for (auto &c : safe_name) {
                if (c == ' ')
                  c = '_';
              }
              if (safe_name.length() > 60)
                safe_name = safe_name.substr(0, 60);
              if (!safe_name.empty()) {
                folder_name = safe_name;
              }
            }

            std::vector<std::string> names_to_check = {folder_name};
            if (folder_name != workshop_id)
              names_to_check.push_back(workshop_id);

            for (const auto &name : names_to_check) {
              for (const auto &parent_name : parent_dirs) {
                std::filesystem::path candidate =
                    game_path / parent_name / name / "zone";
                if (has_zone_content(candidate))
                  return candidate.parent_path().string();
              }
            }
          }
        }
      }
    }
  } catch (...) {
  }

  return "";
}

void workshop_download_thread(std::string workshop_id,
                              const bool update_existing) {
  try {
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
    workshop_cancel_requested = false;
    workshop_paused = false;
    set_workshop_status("Initializing...", -1.0, "Workshop ID: " + workshop_id);

    const auto game_path = game::get_game_path();
    std::filesystem::path existing_install;

    {
      auto existing = find_installed_workshop_item(game_path, workshop_id);
      if (!existing.empty() && !update_existing) {
        set_workshop_status("Already installed.", 100.0,
                            "This workshop item is already installed at:\n" +
                                existing +
                                "\nRemove it first if you want to reinstall.");
        return;
      }
      if (!existing.empty()) {
        existing_install = existing;
        set_workshop_status("Preparing update...", -1.0,
                            "Updating: " + existing_install.string());
      }
    }

    std::future<workshop_info> ws_info_future =
        std::async(std::launch::async, get_steam_workshop_info, workshop_id);

    const bool wine = utils::nt::is_wine();
    std::filesystem::path steamcmd_dir = game_path / "steamcmd";

    {
      std::string error;
      if (!steamcmd::ensure_installed(
              steamcmd_dir, wine,
              [](const std::string &status) {
                set_workshop_status(status, -1.0,
                                    "First-time setup, this may take a minute");
              },
              error)) {
        set_workshop_status("Error: SteamCMD is not ready.", 0.0, error);
        return;
      }
    }

    const workshop_info ws_info = ws_info_future.get();
    const uint64_t expected_size = ws_info.file_size;
    const std::string workshop_title =
        ws_info.title.empty() ? ("Workshop #" + workshop_id) : ws_info.title;

    if (expected_size > 0) {
      ULARGE_INTEGER free_bytes_available{};
      ULARGE_INTEGER total_bytes{};
      ULARGE_INTEGER total_free_bytes{};
      const uint64_t required_space =
          expected_size * 2 + (512ULL * 1024 * 1024);
      if (GetDiskFreeSpaceExW(game_path.c_str(), &free_bytes_available,
                              &total_bytes, &total_free_bytes)) {
        if (free_bytes_available.QuadPart < required_space) {
          set_workshop_status(
              "Error: Not enough disk space.", 0.0,
              "Need ~" + human_readable_size(required_space) +
                  " free, but only " +
                  human_readable_size(free_bytes_available.QuadPart) +
                  " available. Free up space and try again.");
          return;
        }
      }
    }

    const std::vector<std::filesystem::path> roots =
        steamcmd::workshop_roots(steamcmd_dir, wine);
    std::filesystem::path content_path =
        roots[0] / "content" / game::APP_ID_STR / workshop_id;
    const std::filesystem::path download_path =
        roots[0] / "downloads" / game::APP_ID_STR / workshop_id;
    const std::filesystem::path alt_content_path =
        roots[1] / "content" / game::APP_ID_STR / workshop_id;
    const std::filesystem::path alt_download_path =
        roots[1] / "downloads" / game::APP_ID_STR / workshop_id;

    steamcmd::clear_downloads(steamcmd_dir);

    std::string cmd_args = "+login anonymous +workshop_download_item 311210 " +
                           workshop_id + " +quit";

    constexpr int MAX_ATTEMPTS = 20;
    constexpr int FAIL_THRESHOLD = 5;
    int attempt = 0;
    int fast_fail_count = 0;

    const auto download_start_time = std::chrono::steady_clock::now();

    while (!std::filesystem::exists(content_path) &&
           !std::filesystem::exists(alt_content_path) &&
           !workshop_cancel_requested.load()) {
      attempt++;
      if (attempt > MAX_ATTEMPTS) {
        int response = MessageBoxW(
            nullptr,
            L"Download has used all retry attempts without completing.\n\n"
            L"Do you want to continue downloading?\n"
            L"(Your progress will be preserved)",
            L"Retry Limit Reached", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST);

        if (response == IDYES) {
          attempt = 1;
          fast_fail_count = 0;
          continue;
        } else {
          set_workshop_status("Download stopped by user after " +
                                  std::to_string(MAX_ATTEMPTS) + " attempts.",
                              0.0, "You can retry the download at any time.");
          return;
        }
      }

      if (fast_fail_count >= FAIL_THRESHOLD) {
        set_workshop_status("Resetting SteamCMD...", -1.0,
                            "Too many quick failures (" +
                                std::to_string(fast_fail_count) +
                                "), resetting and retrying");
        std::error_code ec;
        for (const auto &dir_name : {"steamapps", "dumps", "logs", "depotcache",
                                     "appcache", "userdata"}) {
          auto dir = steamcmd_dir / dir_name;
          if (std::filesystem::exists(dir, ec)) {
            std::filesystem::remove_all(dir, ec);
          }
        }
        fast_fail_count = 0;
        std::this_thread::sleep_for(std::chrono::seconds(2));
      }

      std::string attempt_str =
          (attempt > 1) ? (" (attempt " + std::to_string(attempt) + ")") : "";
      set_workshop_status(
          "Starting download: " + workshop_title + attempt_str, -1.0,
          expected_size > 0
              ? ("File size: " + human_readable_size(expected_size))
              : "Workshop ID: " + workshop_id);

      steamcmd::process steamcmd(steamcmd_dir, wine);
      if (!steamcmd.start(cmd_args)) {
        set_workshop_status("Error: Failed to start SteamCMD.", 0.0,
                            "Attempt " + std::to_string(attempt));
        std::this_thread::sleep_for(std::chrono::seconds(2));
        continue;
      }

      const auto attempt_start = std::chrono::steady_clock::now();
      steamcmd::progress tracker(steamcmd_dir);

      for (;;) {
        if (workshop_cancel_requested.load()) {
          steamcmd.terminate();
          break;
        }

        if (workshop_paused.load()) {
          set_workshop_status("Paused - " + workshop_title, -1.0,
                              "Download paused. Click Resume to continue.");
          steamcmd.terminate();
          while (workshop_paused.load() && !workshop_cancel_requested.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
          }
          break;
        }

        const bool exited = steamcmd.wait(std::chrono::seconds(1));
        tracker.update(steamcmd);

        const std::string elapsed = format_duration(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - download_start_time)
                .count());
        const std::string attempt_suffix =
            attempt > 1 ? " | Attempt " + std::to_string(attempt) : "";

        if (tracker.committing()) {
          set_workshop_status("Finishing " + workshop_title + "...", 99.0,
                              "Moving downloaded files into place");
        } else if (tracker.started()) {
          std::string details = human_readable_size(tracker.installed_bytes()) +
                                " / " +
                                human_readable_size(tracker.install_total());
          if (tracker.eta_seconds() >= 0) {
            details +=
                " | " +
                human_readable_size(static_cast<uint64_t>(tracker.speed())) +
                "/s | ETA: " + format_duration(tracker.eta_seconds());
          }
          details += " | Elapsed: " + elapsed + attempt_suffix;
          set_workshop_status("Downloading " + workshop_title + "...",
                              std::min(99.0, tracker.fraction() * 100.0),
                              details);
          std::lock_guard lock(workshop_status_mutex);
          workshop_download_folder = download_path.string();
        } else {
          set_workshop_status(
              "Waiting for SteamCMD..." +
                  (attempt > 1 ? " (attempt " + std::to_string(attempt) + ")"
                               : std::string{}),
              0.0, "Elapsed: " + elapsed);
        }

        if (exited) {
          break;
        }
      }

      const uint32_t exit_code = steamcmd.exit_code();

      if (workshop_cancel_requested.load()) {
        // Clean up partial download files
        std::error_code cleanup_ec;
        std::filesystem::remove_all(download_path, cleanup_ec);
        std::filesystem::remove_all(content_path, cleanup_ec);
        std::filesystem::remove_all(alt_download_path, cleanup_ec);
        std::filesystem::remove_all(alt_content_path, cleanup_ec);
        set_workshop_status("Canceled.", 0.0, "");
        return;
      }

      // If paused, don't count this as a failed attempt - just loop back
      if (workshop_paused.load()) {
        attempt--; // Don't consume an attempt for pause
        continue;
      }

      if (std::filesystem::exists(content_path)) {
        const auto content_size = compute_folder_size_bytes(content_path);
        if (content_size > 1024) {
          break;
        }

        std::error_code rm_ec;
        std::filesystem::remove_all(content_path, rm_ec);
      }

      {
        std::error_code ec;
        if (std::filesystem::exists(alt_content_path, ec) &&
            compute_folder_size_bytes(alt_content_path) > 1024) {
          content_path = alt_content_path;
          break;
        }
      }

      if (steamcmd.missing_libraries()) {
        set_workshop_status(
            "Error: SteamCMD needs 32-bit libraries.", 0.0,
            "Install them, then try again:\n"
            "Debian/Ubuntu: sudo dpkg --add-architecture i386 && sudo apt "
            "install lib32gcc-s1\n"
            "Fedora: sudo dnf install glibc.i686 libgcc.i686\n"
            "Arch: sudo pacman -S lib32-gcc-libs");
        return;
      }

      auto attempt_elapsed = std::chrono::steady_clock::now() - attempt_start;
      auto attempt_sec =
          std::chrono::duration_cast<std::chrono::seconds>(attempt_elapsed)
              .count();
      if (attempt_sec < 15) {
        fast_fail_count++;
      }

      set_workshop_status(
          "SteamCMD exited without completing" +
              std::string(attempt < MAX_ATTEMPTS ? ", retrying..." : ""),
          0.0,
          "Exit code: " + std::to_string(exit_code) + " | Attempt " +
              std::to_string(attempt) + "/" + std::to_string(MAX_ATTEMPTS) +
              " | Quick fails: " + std::to_string(fast_fail_count));

      std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    if (workshop_cancel_requested.load()) {
      // Clean up partial download files
      std::error_code cleanup_ec;
      std::filesystem::remove_all(download_path, cleanup_ec);
      std::filesystem::remove_all(content_path, cleanup_ec);
      std::filesystem::remove_all(alt_download_path, cleanup_ec);
      std::filesystem::remove_all(alt_content_path, cleanup_ec);
      set_workshop_status("Canceled.", 0.0, "");
      return;
    }

    set_workshop_status("Verifying download...", 99.9, "");
    if (!std::filesystem::exists(content_path)) {
      std::error_code ec;
      if (std::filesystem::exists(alt_content_path, ec) &&
          compute_folder_size_bytes(alt_content_path) > 1024) {
        content_path = alt_content_path;
      } else {
        set_workshop_status(
            "Error: Download failed - content folder not found.", 0.0,
            "SteamCMD did not download to the expected path.\n"
            "Checked: " +
                content_path.string() +
                "\n"
                "    and: " +
                alt_content_path.string());
        return;
      }
    }
    {
      const auto dl_size = compute_folder_size_bytes(content_path);
      if (dl_size < 1024) {
        set_workshop_status(
            "Error: Download is empty or incomplete.", 0.0,
            "SteamCMD created the folder but no files were downloaded (" +
                human_readable_size(dl_size) +
                ").\n"
                "Path: " +
                content_path.string() +
                "\n"
                "Try again or check your internet connection.");
        return;
      }
    }

    std::string mod_type = "mod";
    std::string folder_name = workshop_id;

    std::string json_path = (content_path / "workshop.json").string();
    std::string json_str;
    bool has_workshop_json = false;
    if (utils::io::read_file(json_path, &json_str) && !json_str.empty()) {
      rapidjson::Document doc;
      if (!doc.Parse(json_str.c_str()).HasParseError() && doc.IsObject()) {
        if (doc.HasMember("Type") && doc["Type"].IsString()) {
          mod_type = doc["Type"].GetString();
        }
        if (doc.HasMember("FolderName") && doc["FolderName"].IsString()) {
          std::string fn = doc["FolderName"].GetString();
          if (!fn.empty()) {
            folder_name = fn;
          }
        }
        has_workshop_json = true;
      }
    }

    for (auto &c : mod_type)
      c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (mod_type != "mod" && mod_type != "map") {
      mod_type = "mod";
    }

    if (!has_workshop_json) {
      try {
        std::string body = "itemcount=1&publishedfileids[0]=" + workshop_id;
        auto api_resp = utils::http::post_data(STEAM_WORKSHOP_API, body, 10);
        if (api_resp && !api_resp->empty()) {
          rapidjson::Document api_doc;
          if (!api_doc.Parse(api_resp->c_str()).HasParseError() &&
              api_doc.IsObject()) {
            auto ri = api_doc.FindMember("response");
            if (ri != api_doc.MemberEnd() && ri->value.IsObject()) {
              auto di = ri->value.FindMember("publishedfiledetails");
              if (di != ri->value.MemberEnd() && di->value.IsArray() &&
                  !di->value.Empty()) {
                const auto &item = di->value[0];
                auto ti = item.FindMember("tags");
                if (ti != item.MemberEnd() && ti->value.IsArray()) {
                  for (auto it = ti->value.Begin(); it != ti->value.End();
                       ++it) {
                    if (it->IsObject()) {
                      auto tag_it = it->FindMember("tag");
                      if (tag_it != it->MemberEnd() &&
                          tag_it->value.IsString()) {
                        std::string t = tag_it->value.GetString();
                        for (auto &c : t)
                          c = static_cast<char>(
                              std::tolower(static_cast<unsigned char>(c)));
                        if (t == "map" || t == "maps" ||
                            t.find("map") != std::string::npos) {
                          mod_type = "map";
                          break;
                        }
                      }
                    }
                  }
                }

                if (mod_type != "map") {
                  auto title_it = item.FindMember("title");
                  if (title_it != item.MemberEnd() &&
                      title_it->value.IsString()) {
                    std::string title = title_it->value.GetString();
                    std::string safe_name;
                    for (char c : title) {
                      if (std::isalnum(static_cast<unsigned char>(c)) ||
                          c == ' ' || c == '-' || c == '_') {
                        safe_name += c;
                      }
                    }
                    while (!safe_name.empty() && safe_name.back() == ' ')
                      safe_name.pop_back();
                    while (!safe_name.empty() && safe_name.front() == ' ')
                      safe_name.erase(safe_name.begin());
                    for (auto &c : safe_name) {
                      if (c == ' ')
                        c = '_';
                    }
                    if (safe_name.length() > 60)
                      safe_name = safe_name.substr(0, 60);
                    if (!safe_name.empty()) {
                      folder_name = safe_name;
                    }
                  }
                }
              }
            }
          }
        }
      } catch (...) {
      }
    }

    set_workshop_status(existing_install.empty() ? "Installing files..."
                                                 : "Installing update...",
                        99.9,
                        "Type: " + mod_type + " | Folder: " + workshop_id);

    std::filesystem::path dest_parent =
        (mod_type == "mod") ? (game_path / "mods") : (game_path / "usermaps");
    std::filesystem::path dest;
    if (existing_install.empty()) {
      dest = dest_parent / workshop_id / "zone";
    } else if (has_zone_content(existing_install / "zone")) {
      dest = existing_install / "zone";
    } else {
      std::error_code existing_ec;
      for (const auto &entry :
           std::filesystem::directory_iterator(existing_install, existing_ec)) {
        if (entry.is_directory(existing_ec) &&
            has_zone_content(entry.path() / "zone")) {
          dest = entry.path() / "zone";
          break;
        }
      }
      if (dest.empty())
        dest = existing_install / "zone";
    }
    std::error_code ec;
    std::filesystem::create_directories(dest, ec);
    if (ec) {
      set_workshop_status("Error: Cannot create destination folder.", 0.0,
                          "Failed to create: " + dest.string() + "\n" +
                              ec.message());
      return;
    }

    const uint64_t install_total = compute_folder_size_bytes(content_path);
    set_workshop_status("Copying files...", 99.0,
                        install_total > 0
                            ? ("0 / " + human_readable_size(install_total))
                            : std::string("Preparing..."));
    const bool copied = copy_directory_recursive_with_progress(
        content_path, dest, install_total, [&](uint64_t copied_bytes) {
          if (install_total > 0) {
            double p = 99.0 + (static_cast<double>(copied_bytes) /
                               static_cast<double>(install_total)) *
                                  0.99;
            if (p > 99.99)
              p = 99.99;
            set_workshop_status("Copying files...", p,
                                human_readable_size(copied_bytes) + " / " +
                                    human_readable_size(install_total));
          } else {
            set_workshop_status("Copying files...", 99.5,
                                human_readable_size(copied_bytes));
          }
        });
    if (!copied) {
      if (workshop_cancel_requested.load()) {
        set_workshop_status("Canceled.", 0.0, "");
        return;
      }
      set_workshop_status("Error: Failed to copy downloaded files.", 0.0,
                          "Source: " + content_path.string() +
                              "\n"
                              "Destination: " +
                              dest.string());
      return;
    }

    {
      const auto dest_size = compute_folder_size_bytes(dest);
      if (dest_size < 1024) {
        set_workshop_status(
            "Error: Copy completed but destination is empty.", 0.0,
            "Source had files but nothing was copied to: " + dest.string() +
                "\n"
                "Source size: " +
                human_readable_size(install_total));
        return;
      }
    }

    {
      std::error_code cleanup_ec;
      if (std::filesystem::exists(content_path, cleanup_ec))
        std::filesystem::remove_all(content_path, cleanup_ec);
      if (std::filesystem::exists(download_path, cleanup_ec))
        std::filesystem::remove_all(download_path, cleanup_ec);
      if (std::filesystem::exists(alt_content_path, cleanup_ec))
        std::filesystem::remove_all(alt_content_path, cleanup_ec);
      if (std::filesystem::exists(alt_download_path, cleanup_ec))
        std::filesystem::remove_all(alt_download_path, cleanup_ec);
      std::filesystem::last_write_time(
          existing_install.empty() ? dest_parent / workshop_id
                                   : existing_install,
          std::filesystem::file_time_type::clock::now(), cleanup_ec);
    }

    try {
      std::filesystem::path ws_json_path = dest / "workshop.json";
      std::string ws_json_str;
      rapidjson::Document ws_doc;
      bool need_write = false;

      if (utils::io::read_file(ws_json_path.string(), &ws_json_str) &&
          !ws_json_str.empty()) {
        if (!ws_doc.Parse(ws_json_str.c_str()).HasParseError() &&
            ws_doc.IsObject()) {
          auto pfid = ws_doc.FindMember("PublishedFileId");
          if (pfid == ws_doc.MemberEnd() || !pfid->value.IsString() ||
              std::string(pfid->value.GetString()) != workshop_id) {
            if (pfid != ws_doc.MemberEnd()) {
              pfid->value.SetString(
                  workshop_id.c_str(),
                  static_cast<rapidjson::SizeType>(workshop_id.size()),
                  ws_doc.GetAllocator());
            } else {
              ws_doc.AddMember(
                  rapidjson::Value("PublishedFileId", ws_doc.GetAllocator()),
                  rapidjson::Value(
                      workshop_id.c_str(),
                      static_cast<rapidjson::SizeType>(workshop_id.size()),
                      ws_doc.GetAllocator()),
                  ws_doc.GetAllocator());
            }
            need_write = true;
          }
        } else {
          ws_doc.SetObject();
          ws_doc.AddMember(
              rapidjson::Value("PublishedFileId", ws_doc.GetAllocator()),
              rapidjson::Value(
                  workshop_id.c_str(),
                  static_cast<rapidjson::SizeType>(workshop_id.size()),
                  ws_doc.GetAllocator()),
              ws_doc.GetAllocator());
          need_write = true;
        }
      } else {
        ws_doc.SetObject();
        ws_doc.AddMember(
            rapidjson::Value("PublishedFileId", ws_doc.GetAllocator()),
            rapidjson::Value(
                workshop_id.c_str(),
                static_cast<rapidjson::SizeType>(workshop_id.size()),
                ws_doc.GetAllocator()),
            ws_doc.GetAllocator());
        need_write = true;
      }

      if (need_write) {
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        ws_doc.Accept(writer);
        utils::io::write_file(ws_json_path.string(),
                              std::string(sb.GetString(), sb.GetSize()), false);
      }
    } catch (...) {
    }

    {
      bool has_zone_files = false;
      std::error_code verify_ec;
      for (const auto &entry :
           std::filesystem::recursive_directory_iterator(dest, verify_ec)) {
        if (verify_ec)
          break;
        if (!entry.is_regular_file(verify_ec))
          continue;
        const auto ext = entry.path().extension().string();
        if (ext == ".ff" || ext == ".sabl" || ext == ".sabs" || ext == ".pak" ||
            ext == ".xpak") {
          has_zone_files = true;
          break;
        }
      }
      if (!has_zone_files) {
        uint64_t dest_size = compute_folder_size_bytes(dest);
        if (dest_size < 1024) {
          set_workshop_status("Error: No game files found in download.", 0.0,
                              "The workshop item downloaded but contained no "
                              "usable zone files.\n"
                              "Installed to: " +
                                  dest.string() +
                                  "\n"
                                  "This item may be corrupt or incompatible.");
          return;
        }
        set_workshop_status(
            "Done! Download complete.", 100.0,
            folder_name + " (" + human_readable_size(dest_size) +
                ") \u2014 no .ff files found, may need manual setup.");
        try_refresh_workshop_content();
        return;
      }
    }

    {
      uint64_t final_size = compute_folder_size_bytes(dest);
      set_workshop_status("Done! Download complete.", 100.0,
                          folder_name + " is ready to use (" +
                              human_readable_size(final_size) + ").");
      try_refresh_workshop_content();
    }
  } catch (const std::filesystem::filesystem_error &fse) {
    std::string detail = fse.what();
    if (detail.find("space") != std::string::npos ||
        detail.find("disk") != std::string::npos ||
        fse.code() == std::errc::no_space_on_device) {
      set_workshop_status(
          "Error: Disk full.", 0.0,
          "Ran out of disk space. Free up space and try again.");
    } else {
      set_workshop_status("Error: File system error.", 0.0, detail);
    }
  } catch (const std::exception &ex) {
    set_workshop_status("Error: Workshop download failed.", 0.0, ex.what());
  } catch (...) {
    set_workshop_status("Error: Workshop download crashed.", 0.0, "");
  }
}
} // namespace

std::map<std::string, uint64_t>
batch_get_time_updated(const std::vector<std::string> &ids) {
  std::map<std::string, uint64_t> result;
  if (ids.empty())
    return result;
  try {
    std::string body = "itemcount=" + std::to_string(ids.size());
    for (size_t i = 0; i < ids.size(); i++)
      body += "&publishedfileids[" + std::to_string(i) + "]=" + ids[i];

    auto resp = utils::http::post_data(
        "https://api.steampowered.com/ISteamRemoteStorage/"
        "GetPublishedFileDetails/v1/",
        body, 15);
    if (!resp || resp->empty())
      return result;

    rapidjson::Document doc;
    if (doc.Parse(resp->c_str()).HasParseError() || !doc.IsObject())
      return result;
    auto resp_it = doc.FindMember("response");
    if (resp_it == doc.MemberEnd() || !resp_it->value.IsObject())
      return result;
    auto details_it = resp_it->value.FindMember("publishedfiledetails");
    if (details_it == resp_it->value.MemberEnd() ||
        !details_it->value.IsArray())
      return result;

    for (rapidjson::SizeType i = 0; i < details_it->value.Size(); i++) {
      const auto &item = details_it->value[i];
      if (!item.IsObject())
        continue;
      std::string item_id;
      auto id_it = item.FindMember("publishedfileid");
      if (id_it != item.MemberEnd() && id_it->value.IsString())
        item_id = id_it->value.GetString();
      if (item_id.empty())
        continue;

      auto tu_it = item.FindMember("time_updated");
      if (tu_it != item.MemberEnd()) {
        uint64_t ts = 0;
        if (tu_it->value.IsUint64())
          ts = tu_it->value.GetUint64();
        else if (tu_it->value.IsInt64())
          ts = static_cast<uint64_t>(tu_it->value.GetInt64());
        else if (tu_it->value.IsUint())
          ts = tu_it->value.GetUint();
        else if (tu_it->value.IsInt())
          ts = static_cast<uint64_t>(tu_it->value.GetInt());
        if (ts > 0)
          result[item_id] = ts;
      }
    }
  } catch (...) {
  }
  return result;
}

std::map<std::string, workshop_item_meta>
batch_get_workshop_meta(const std::vector<std::string> &ids) {
  std::map<std::string, workshop_item_meta> result;
  if (ids.empty())
    return result;
  try {
    std::string body = "itemcount=" + std::to_string(ids.size());
    for (size_t i = 0; i < ids.size(); i++)
      body += "&publishedfileids[" + std::to_string(i) + "]=" + ids[i];

    auto resp = utils::http::post_data(
        "https://api.steampowered.com/ISteamRemoteStorage/"
        "GetPublishedFileDetails/v1/",
        body, 15);
    if (!resp || resp->empty())
      return result;

    rapidjson::Document doc;
    if (doc.Parse(resp->c_str()).HasParseError() || !doc.IsObject())
      return result;
    auto resp_it = doc.FindMember("response");
    if (resp_it == doc.MemberEnd() || !resp_it->value.IsObject())
      return result;
    auto details_it = resp_it->value.FindMember("publishedfiledetails");
    if (details_it == resp_it->value.MemberEnd() ||
        !details_it->value.IsArray())
      return result;

    for (rapidjson::SizeType i = 0; i < details_it->value.Size(); i++) {
      const auto &item = details_it->value[i];
      if (!item.IsObject())
        continue;
      std::string item_id;
      auto id_it = item.FindMember("publishedfileid");
      if (id_it != item.MemberEnd() && id_it->value.IsString())
        item_id = id_it->value.GetString();
      if (item_id.empty())
        continue;

      workshop_item_meta meta{};

      auto tu_it = item.FindMember("time_updated");
      if (tu_it != item.MemberEnd()) {
        if (tu_it->value.IsUint64())
          meta.time_updated = tu_it->value.GetUint64();
        else if (tu_it->value.IsUint())
          meta.time_updated = tu_it->value.GetUint();
      }

      auto fs_it = item.FindMember("file_size");
      if (fs_it != item.MemberEnd()) {
        if (fs_it->value.IsUint64())
          meta.file_size = fs_it->value.GetUint64();
        else if (fs_it->value.IsUint())
          meta.file_size = fs_it->value.GetUint();
        else if (fs_it->value.IsString())
          meta.file_size = std::strtoull(fs_it->value.GetString(), nullptr, 10);
      }

      auto desc_it = item.FindMember("description");
      if (desc_it != item.MemberEnd() && desc_it->value.IsString())
        meta.description = desc_it->value.GetString();

      auto prev_it = item.FindMember("preview_url");
      if (prev_it != item.MemberEnd() && prev_it->value.IsString())
        meta.preview_url = prev_it->value.GetString();

      auto subs_it = item.FindMember("subscriptions");
      if (subs_it != item.MemberEnd()) {
        if (subs_it->value.IsInt64())
          meta.subs = subs_it->value.GetInt64();
        else if (subs_it->value.IsInt())
          meta.subs = subs_it->value.GetInt();
      }

      auto fav_it = item.FindMember("favorited");
      if (fav_it != item.MemberEnd()) {
        if (fav_it->value.IsInt64())
          meta.favorites = fav_it->value.GetInt64();
        else if (fav_it->value.IsInt())
          meta.favorites = fav_it->value.GetInt();
      }

      result[item_id] = std::move(meta);
    }
  } catch (...) {
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
            const uint64_t size = scrape_workshop_file_size_bytes(id);
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
        const auto game_path = game::get_game_path();
        auto existing = find_installed_workshop_item(game_path, id);
        return CComVariant(existing.c_str());
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
