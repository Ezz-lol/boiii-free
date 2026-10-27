#include <std_include.hpp>

#include "workshop.hpp"

#include <game/game.hpp>
#include <game/impl/ugc/ugc.hpp>
#include <game/utils.hpp>

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

#include <chrono>
#include <fstream>
#include <mutex>
#include <unordered_map>

namespace workshop {
namespace {
constexpr size_t SAMPLE_SIZE = 64 * 1024;
constexpr std::chrono::seconds CACHE_TTL{30};

struct cached_content {
  content_info info;
  std::chrono::steady_clock::time_point time;
};

std::mutex cache_mutex;
std::unordered_map<std::string, cached_content> cache;

std::string read_at(std::ifstream &file, const uint64_t offset,
                    const size_t length) {
  std::string data(length, '\0');
  file.seekg(static_cast<std::streamoff>(offset));
  file.read(data.data(), static_cast<std::streamsize>(length));
  data.resize(static_cast<size_t>(file.gcount()));
  return data;
}

content_info compute_content(const std::filesystem::path &folder) {
  std::vector<std::pair<std::string, std::filesystem::path>> files;
  std::error_code ec;
  for (std::filesystem::recursive_directory_iterator it(folder, ec), end;
       !ec && it != end; it.increment(ec)) {
    std::error_code file_ec;
    if (!it->is_regular_file(file_ec) ||
        utils::string::to_lower(it->path().filename().string()) ==
            "workshop.json") {
      continue;
    }
    files.emplace_back(utils::string::to_lower(std::filesystem::relative(
                                                   it->path(), folder, file_ec)
                                                   .generic_string()),
                       it->path());
  }
  if (files.empty()) {
    return {};
  }
  std::ranges::sort(files);

  content_info info{};
  std::string digest;
  for (const auto &[name, path] : files) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
      continue;
    }
    const uint64_t size = static_cast<uint64_t>(file.tellg());
    std::string sample = read_at(file, 0, SAMPLE_SIZE);
    if (size > SAMPLE_SIZE) {
      sample += read_at(file, size - std::min<uint64_t>(size, SAMPLE_SIZE),
                        SAMPLE_SIZE);
    }
    if (sample.size() >= 0x18 && sample.starts_with("TAff")) {
      info.built =
          std::max(info.built,
                   *reinterpret_cast<const uint64_t *>(sample.data() + 0x10));
    }
    digest += name + '\0' + std::to_string(size) + '\0' +
              utils::cryptography::sha1::compute(sample);
  }
  info.hash = utils::cryptography::sha1::compute(digest, true).substr(0, 16);
  return info;
}

content_info zone_content(const char *zone_path) {
  if (!zone_path || !zone_path[0]) {
    return {};
  }
  const std::filesystem::path folder(zone_path);
  std::error_code ec;
  if (!std::filesystem::is_directory(folder, ec) ||
      std::filesystem::equivalent(folder, game::get_game_path() / "zone", ec)) {
    return {};
  }

  const std::string key = folder.generic_string();
  const std::lock_guard lock(cache_mutex);
  const auto now = std::chrono::steady_clock::now();
  const auto found = cache.find(key);
  if (found != cache.end() && now - found->second.time < CACHE_TTL) {
    return found->second.info;
  }
  content_info info = compute_content(folder);
  cache[key] = {info, now};
  return info;
}

const game::ugc::UGCItem *find_mod(const std::string_view &id) {
  for (uint32_t i = 0; i < game::ugc::modsPool.count; ++i) {
    const game::ugc::UGCItem *mod = &game::ugc::modsPool.data[i];
    if (mod->publisherId == id || mod->internalName == id) {
      return mod;
    }
  }
  return nullptr;
}
} // namespace

content_info mod_content(const std::string_view &id) {
  if (id.empty() || id == "usermaps") {
    return {};
  }
  const game::ugc::UGCItem *mod = find_mod(id);
  return mod ? zone_content(mod->absolutePathZoneFiles) : content_info{};
}

content_info usermap_content(const std::string_view &mapname) {
  const game::ugc::OptionalUGCItemRef usermap =
      game::ugc::UGC_GetUsermapByInternalName(mapname);
  return usermap.has_value()
             ? zone_content(usermap->get().absolutePathZoneFiles)
             : content_info{};
}

content_info loaded_mod_content() {
  return mod_content(get_mod_publisher_id());
}

content_info loaded_usermap_content() {
  return zone_content(game::ugc::active_usermap->absolutePathZoneFiles);
}
} // namespace workshop
