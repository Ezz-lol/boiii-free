#pragma once

#include "compiled_includes.hpp"
#include <cstring>
#include <memory>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace zombies::coop::t7 {
inline constexpr std::string_view server_companion = "scripts/zm/eroded_coop_fields.gsc";
inline constexpr std::string_view client_companion = "scripts/zm/eroded_coop_fields.csc";
inline constexpr std::string_view companion_include = "scripts/zm/eroded_coop_fields";
inline constexpr std::size_t retention_limit = 32 * 1024 * 1024;
inline std::optional<unsigned> fixed_companion_role(std::string_view name) noexcept {
  const auto same_path = [name](std::string_view expected) noexcept {
    if (name.size() != expected.size()) return false;
    for (std::size_t i = 0; i < name.size(); ++i) {
      auto c = name[i];
      if (c == '\\') c = '/';
      if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + ('a' - 'A'));
      if (c != expected[i]) return false;
    }
    return true;
  };
  if (same_path(server_companion)) return 0;
  if (same_path(client_companion)) return 1;
  return std::nullopt;
}
inline std::string canonical_asset(std::string_view name) {
  auto path = canonical_script(name);
  if (!path || name.size() < 4) return {};
  std::string extension(name.substr(name.size() - 4));
  for (auto& c : extension) if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + ('a' - 'A'));
  const bool client = extension == ".csc";
  if (!client && extension != ".gsc") return {};
  return *path + (client ? ".csc" : ".gsc");
}

inline bool supported_map_name(std::string_view map) noexcept {
  if (map.empty() || map.size() > 64) return false;
  for (const char c : map)
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-')) return false;
  return true;
}
inline bool is_schema_anchor(std::string_view asset, std::string_view map) {
  const auto full = canonical_asset(asset);
  const auto name = canonical_script(full);
  if (!name) return false;
  if (*name == "scripts/zm/gametypes/_clientids" && full.ends_with(".gsc")) return true;
  if (*name == "scripts/zm/_zm_score" && full.ends_with(".csc")) return true;
  return supported_map_name(map) && canonical_script("scripts/zm/" + std::string(map)) == name &&
      (full.ends_with(".gsc") || full.ends_with(".csc"));
}

// Native recursive linking needs stable identity within one VM load. Each name
// receives one mutable object per observed generation; new generations receive
// fresh backing copied from an immutable pre-link template. Retired backing is
// never freed while an old VM may still retain a pointer.
template <class Asset> class schema_asset_store {
  struct entry {
    Asset asset{};
    std::string name;
    std::vector<std::uint8_t> bytes;
    std::vector<std::uint8_t> immutable;
    entry(std::string_view path, std::vector<std::uint8_t> data)
        : name(path), bytes(std::move(data)), immutable(bytes) {
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
  std::unordered_map<std::string, Asset*> current_[2];
  std::uint64_t generation_[2]{1, 1};
  std::unique_ptr<entry> emergency_[2];
  std::uint64_t emergency_generation_[2]{};
  bool exhausted_ = false;
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
  struct generation_lease { unsigned role{}; std::uint64_t generation{}; };
  explicit schema_asset_store(std::size_t limit = retention_limit) : limit_(limit) {}

  bool pin(std::string_view server, std::string_view client, std::string& error) {
    if (!valid_payload(server, server_companion) || !valid_payload(client, client_companion)) {
      error = "Companion is not a raw T7 PC object with its fixed logical name.";
      return false;
    }
    std::scoped_lock lock(mutex_);
    if (exhausted_) {
      error = "Schema asset store is permanently exhausted; restart required.";
      return false;
    }
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
    auto emergency_server = std::make_unique<entry>(server_companion,
        std::vector<std::uint8_t>(server.begin(), server.end()));
    auto emergency_client = std::make_unique<entry>(client_companion,
        std::vector<std::uint8_t>(client.begin(), client.end()));
    const auto entry_charge = [](const entry& e) {
      return sizeof(entry) + e.name.capacity() + 1 + e.bytes.capacity() + e.immutable.capacity();
    };
    const auto charge = server_copy.capacity() + client_copy.capacity() + 2 +
        entry_charge(*emergency_server) + entry_charge(*emergency_client);
    if (charge > limit_ - retained_) {
      error = "Companion payloads exceed the retention budget.";
      return false;
    }
    server_.swap(server_copy); client_.swap(client_copy);
    emergency_[0] = std::move(emergency_server); emergency_[1] = std::move(emergency_client);
    retained_ += charge;
    return true;
  }

  static unsigned role(std::string_view name) noexcept { return name.ends_with(".csc") ? 1u : 0u; }
  bool begin_generation(bool client) {
    std::scoped_lock lock(mutex_);
    if (exhausted_) return false;
    const unsigned index = client ? 1u : 0u;
    current_[index].clear();
    ++generation_[index];
    return true;
  }
  generation_lease capture_generation(std::string_view name) {
    const auto index = role(canonical_asset(name));
    std::scoped_lock lock(mutex_);
    return {index, generation_[index]};
  }
  Asset* cached(std::string_view name, std::optional<generation_lease> lease = {}) {
    std::scoped_lock lock(mutex_);
    const auto key = canonical_asset(name);
    const auto index = role(key);
    if (lease && (lease->role != index || lease->generation != generation_[index])) return nullptr;
    const auto& map = current_[index];
    const auto found = map.find(key);
    return found == map.end() ? nullptr : found->second;
  }
  bool exhausted() {
    std::scoped_lock lock(mutex_);
    return exhausted_;
  }
  std::optional<std::vector<std::uint8_t>> template_for(const Asset* asset) {
    std::scoped_lock lock(mutex_);
    for (std::size_t i = 0; i < count_; ++i)
      if (&exposed_[i]->asset == asset) return exposed_[i]->immutable;
    return std::nullopt;
  }
  Asset* retain(std::string_view name, std::vector<std::uint8_t> bytes, std::string& error,
      std::optional<generation_lease> lease = {}) {
    std::scoped_lock lock(mutex_);
    const auto key = canonical_asset(name);
    if (key.empty()) { error = "Invalid compiled asset path."; return nullptr; }
    const auto index = role(key);
    if (lease && (lease->role != index || lease->generation != generation_[index])) {
      error = "Script asset generation changed during lookup; stale work was rejected.";
      return nullptr;
    }
    auto& cache = current_[index];
    if (const auto found = cache.find(key); found != cache.end()) return found->second;
    if (exhausted_) {
      error = "Schema asset store is permanently exhausted; restart required.";
      return nullptr;
    }
    auto fresh = std::make_unique<entry>(key, std::move(bytes));
    const auto charge = sizeof(entry) + fresh->name.capacity() + 1 + fresh->bytes.capacity() +
        fresh->immutable.capacity() + sizeof(std::pair<const std::string, Asset*>) + name.size() + 64;
    if (count_ >= exposed_.size() || charge > limit_ || retained_ > limit_ - charge) {
      error = "Experimental schema asset retention limit reached; restart required.";
      exhausted_ = true;
      return nullptr;
    }
    auto* result = &fresh->asset;
    exposed_[count_++] = std::move(fresh);
    retained_ += charge;
    cache.emplace(key, result);
    return result;
  }

  std::pair<std::size_t, std::size_t> retained() {
    std::scoped_lock lock(mutex_);
    return {count_, retained_};
  }

  Asset* companion(std::string_view name, std::string& error,
      std::optional<generation_lease> captured = {}) {
    const auto lease = captured ? *captured : capture_generation(name);
    if (auto* existing = cached(name, lease)) return existing;
    const auto key = canonical_asset(name);
    std::vector<std::uint8_t> bytes;
    {
      std::scoped_lock lock(mutex_);
      if (lease.role > 1 || lease.role != role(key) || lease.generation != generation_[role(key)]) {
        error = "Script companion generation changed during lookup; stale work was rejected.";
        return nullptr;
      }
      const auto& payload = key == server_companion ? server_ : client_;
      if ((key != server_companion && key != client_companion) || payload.empty()) {
        error = "Validated companion payload has not been pinned.";
        return nullptr;
      }
      const auto index = role(key);
      if (exhausted_ && emergency_generation_[index] == generation_[index])
        return &emergency_[index]->asset;
      bytes.assign(payload.begin(), payload.end());
    }
    return retain(key, std::move(bytes), error, lease);
  }
  // A missing asset is dereferenced by the native resolver. Reserve a valid,
  // never-before-exposed fallback at pin time for the first fatal generation.
  // Admission stays revoked; this permits safe unwinding to the deferred drop.
  Asset* emergency_companion(std::string_view name) {
    // No string/map/vector allocation on the allocation-failure path.
    const auto chosen = fixed_companion_role(name);
    if (!chosen) return nullptr;
    const auto index = *chosen;
    std::scoped_lock lock(mutex_);
    exhausted_ = true; // no later expanded generation may reuse this fallback
    for (const auto& [path, existing] : current_[index])
      if (fixed_companion_role(path) == chosen) return existing; // preserve identity if normalization ran out of memory
    if (!emergency_[index]) return nullptr;
    if (emergency_generation_[index] && emergency_generation_[index] != generation_[index]) return nullptr;
    emergency_generation_[index] = generation_[index];
    return &emergency_[index]->asset;
  }
};
} // namespace zombies::coop::t7
