#pragma once

#include "compiled_includes.hpp"
#include <cstring>
#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>

namespace zombies::coop::t7 {
inline constexpr std::string_view server_companion = "scripts/zm/eroded_coop_fields.gsc";
inline constexpr std::string_view client_companion = "scripts/zm/eroded_coop_fields.csc";
inline constexpr std::string_view companion_include = "scripts/zm/eroded_coop_fields";
inline constexpr std::size_t retention_limit = 32 * 1024 * 1024;

inline bool supported_map_name(std::string_view map) noexcept {
  if (map.empty() || map.size() > 64) return false;
  for (const char c : map)
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-')) return false;
  return true;
}
inline bool is_schema_anchor(std::string_view asset, std::string_view map) {
  const auto name = canonical_script(asset);
  if (!name) return false;
  if (*name == "scripts/zm/gametypes/_clientids" && asset.ends_with(".gsc")) return true;
  if (*name == "scripts/zm/_zm_score" && asset.ends_with(".csc")) return true;
  return supported_map_name(map) && canonical_script("scripts/zm/" + std::string(map)) == name &&
      (asset.ends_with(".gsc") || asset.ends_with(".csc"));
}

// A native linker may mutate its returned object. Each lookup receives fresh
// backing; exposed objects are never reused, erased or cleared by VM shutdown.
// The bounded lifetime is deliberate: exhaustion requires a process restart.
template <class Asset> class schema_asset_store {
  struct entry {
    Asset asset{};
    std::string name;
    std::vector<std::uint8_t> bytes;
    entry(std::string_view path, std::vector<std::uint8_t> data)
        : name(path), bytes(std::move(data)) {
      static_assert(std::is_trivially_copyable_v<Asset>);
      // CB names the 24-byte asset RawFile and declares a 32-bit length. Its
      // SCRIPTPARSETREE consumer uses the same length slot as Ezz's size_t;
      // explicitly clear padding so the upper half cannot contain heap data.
      std::memset(&asset, 0, sizeof(asset));
      asset.name = name.c_str();
      asset.len = static_cast<decltype(asset.len)>(bytes.size());
      asset.buffer = reinterpret_cast<decltype(asset.buffer)>(bytes.data());
    }
  };
  std::mutex mutex_;
  std::string server_, client_;
  std::array<std::unique_ptr<entry>, 4096> exposed_{};
  std::size_t count_ = 0;
  std::size_t retained_ = sizeof(exposed_);
  std::size_t limit_;

  static bool valid_payload(std::string_view bytes, std::string_view expected) {
    const auto object = std::span(reinterpret_cast<const std::uint8_t *>(bytes.data()), bytes.size());
    const auto result = clone_with_include(object, "scripts/zm/__schema_validate").result;
    if (result != include_result::inserted && result != include_result::already_present) return false;
    const auto filename = bounded_name(object, read32(object, 0x34));
    return filename && *filename == expected;
  }
public:
  explicit schema_asset_store(std::size_t limit = retention_limit) : limit_(limit) {}

  bool pin(std::string_view server, std::string_view client, std::string& error) {
    if (!valid_payload(server, server_companion) || !valid_payload(client, client_companion)) {
      error = "Companion is not a raw T7 PC object with its fixed logical name.";
      return false;
    }
    std::scoped_lock lock(mutex_);
    if (!server_.empty() || !client_.empty()) {
      if (server_ == server && client_ == client) return true;
      error = "Companion payloads changed after native pinning; restart required.";
      return false;
    }
    if (retained_ > limit_ || server.size() > limit_ - retained_ ||
        client.size() > limit_ - retained_ - server.size()) {
      error = "Companion payloads exceed the retention budget.";
      return false;
    }
    std::string server_copy(server), client_copy(client);
    const auto charge = server_copy.capacity() + client_copy.capacity() + 2;
    if (charge > limit_ - retained_) {
      error = "Companion payloads exceed the retention budget.";
      return false;
    }
    server_.swap(server_copy); client_.swap(client_copy);
    retained_ += charge;
    return true;
  }

  Asset* retain(std::string_view name, std::vector<std::uint8_t> bytes, std::string& error) {
    auto fresh = std::make_unique<entry>(name, std::move(bytes));
    const auto charge = sizeof(entry) + fresh->name.capacity() + 1 + fresh->bytes.capacity();
    std::scoped_lock lock(mutex_);
    if (count_ >= exposed_.size() || charge > limit_ || retained_ > limit_ - charge) {
      error = "Experimental schema asset retention limit reached; restart required.";
      return nullptr;
    }
    auto* result = &fresh->asset;
    exposed_[count_++] = std::move(fresh);
    retained_ += charge;
    return result;
  }

  std::pair<std::size_t, std::size_t> retained() {
    std::scoped_lock lock(mutex_);
    return {count_, retained_};
  }

  Asset* companion(std::string_view name, std::string& error) {
    std::vector<std::uint8_t> bytes;
    {
      std::scoped_lock lock(mutex_);
      const auto& payload = name == server_companion ? server_ : client_;
      if ((name != server_companion && name != client_companion) || payload.empty()) {
        error = "Validated companion payload has not been pinned.";
        return nullptr;
      }
      bytes.assign(payload.begin(), payload.end());
    }
    return retain(name, std::move(bytes), error);
  }
};
} // namespace zombies::coop::t7
