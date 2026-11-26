#pragma once

#include "compiled_includes.hpp"
#include <cstring>
#include <memory>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace zombies::coop::t7 {
inline constexpr std::string_view server_companion =
    "scripts/zm/eroded_coop_fields.gsc";
inline constexpr std::string_view client_companion =
    "scripts/zm/eroded_coop_fields.csc";
inline constexpr std::string_view companion_include =
    "scripts/zm/eroded_coop_fields";
inline constexpr std::string_view island_companion =
    "scripts/zm/eroded_coop_island_trials.gsc";
inline constexpr std::string_view island_include =
    "scripts/zm/eroded_coop_island_trials";
inline constexpr std::size_t retention_limit = 32 * 1024 * 1024;
// Payload identity is distinct from VM realm: both server objects share realm0
// generations, while each owns its own canonical emergency asset.
enum class companion_id : unsigned {
  server_fields,
  client_fields,
  island_trials
};
inline constexpr std::size_t companion_count = 3;
inline constexpr std::array<std::string_view, companion_count> companion_names{
    server_companion, client_companion, island_companion};
constexpr unsigned companion_realm(companion_id id) noexcept {
  return id == companion_id::client_fields ? 1u : 0u;
}
inline std::optional<companion_id>
fixed_companion_identity(std::string_view name) noexcept {
  const auto same_path = [name](std::string_view expected) noexcept {
    if (name.size() != expected.size())
      return false;
    for (std::size_t i = 0; i < name.size(); ++i) {
      auto c = name[i];
      if (c == '\\')
        c = '/';
      if (c >= 'A' && c <= 'Z')
        c = static_cast<char>(c + ('a' - 'A'));
      if (c != expected[i])
        return false;
    }
    return true;
  };
  for (unsigned index = 0; index < companion_names.size(); ++index)
    if (same_path(companion_names[index]))
      return static_cast<companion_id>(index);
  return std::nullopt;
}
inline std::optional<unsigned>
fixed_companion_role(std::string_view name) noexcept {
  const auto id = fixed_companion_identity(name);
  return id ? std::optional<unsigned>{companion_realm(*id)} : std::nullopt;
}
inline std::string canonical_asset(std::string_view name) {
  auto path = canonical_script(name);
  if (!path || name.size() < 4)
    return {};
  std::string extension(name.substr(name.size() - 4));
  for (auto &c : extension)
    if (c >= 'A' && c <= 'Z')
      c = static_cast<char>(c + ('a' - 'A'));
  const bool client = extension == ".csc";
  if (!client && extension != ".gsc")
    return {};
  return *path + (client ? ".csc" : ".gsc");
}

inline bool supported_map_name(std::string_view map) noexcept {
  if (map.empty() || map.size() > 64)
    return false;
  for (const char c : map)
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '_' || c == '-'))
      return false;
  return true;
}
inline bool is_schema_anchor(std::string_view asset, std::string_view map) {
  const auto full = canonical_asset(asset);
  const auto name = canonical_script(full);
  if (!name)
    return false;
  if (*name == "scripts/zm/gametypes/_clientids" && full.ends_with(".gsc"))
    return true;
  if (*name == "scripts/zm/_zm_score" && full.ends_with(".csc"))
    return true;
  return supported_map_name(map) &&
         canonical_script("scripts/zm/" + std::string(map)) == name &&
         (full.ends_with(".gsc") || full.ends_with(".csc"));
}
inline bool is_island_server_anchor(std::string_view asset,
                                    std::string_view map) {
  return map == "zm_island" &&
         canonical_asset(asset) == "scripts/zm/zm_island.gsc";
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
    entry(std::string_view path, std::vector<std::uint8_t> data,
          std::optional<std::vector<std::uint8_t>> pristine = {})
        : name(path), bytes(std::move(data)),
          immutable(pristine ? std::move(*pristine) : bytes) {
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
  std::array<std::string, companion_count> payloads_;
  std::unordered_map<std::string, Asset *> current_[2];
  std::uint64_t generation_[2]{1, 1};
  std::unique_ptr<entry> emergency_[companion_count];
  std::uint64_t emergency_generation_[companion_count]{};
  std::uint64_t island_server_generation_{};
  bool exhausted_ = false;
  std::array<std::unique_ptr<entry>, 4096> exposed_{};
  std::size_t count_ = 0;
  std::size_t retained_ = sizeof(exposed_);
  std::size_t limit_;

  static bool valid_payload(std::string_view bytes, std::string_view expected) {
    const auto object = std::span(
        reinterpret_cast<const std::uint8_t *>(bytes.data()), bytes.size());
    const auto result =
        clone_with_include(object, "scripts/zm/__schema_validate").result;
    if (result != include_result::inserted &&
        result != include_result::already_present)
      return false;
    const auto filename = bounded_name(object, read32(object, 0x34));
    return filename && *filename == expected;
  }

public:
  struct generation_lease {
    unsigned role{};
    std::uint64_t generation{};
  };
  explicit schema_asset_store(std::size_t limit = retention_limit)
      : limit_(limit) {}

  bool pin(std::string_view server, std::string_view client,
           std::string_view island, std::string &error) {
    const std::array<std::string_view, companion_count> incoming{server, client,
                                                                 island};
    for (std::size_t index = 0; index < companion_count; ++index) {
      if (!valid_payload(incoming[index], companion_names[index])) {
        error =
            "Companion is not a raw T7 PC object with its fixed logical name.";
        return false;
      }
    }
    std::scoped_lock lock(mutex_);
    if (exhausted_) {
      error = "Schema asset store is permanently exhausted; restart required.";
      return false;
    }
    if (std::any_of(payloads_.begin(), payloads_.end(),
                    [](const auto &value) { return !value.empty(); })) {
      for (std::size_t index = 0; index < companion_count; ++index) {
        if (payloads_[index] != incoming[index]) {
          error = "Companion payloads changed after native pinning; restart "
                  "required.";
          return false;
        }
      }
      return true;
    }
    if (retained_ > limit_) {
      error = "Companion payloads exceed the retention budget.";
      return false;
    }
    auto remaining = limit_ - retained_;
    for (const auto bytes : incoming) {
      if (bytes.size() > remaining) {
        error = "Companion payloads exceed the retention budget.";
        return false;
      }
      remaining -= bytes.size();
    }
    std::array<std::string, companion_count> copies;
    std::array<std::unique_ptr<entry>, companion_count> emergencies;
    std::size_t charge = 0;
    for (std::size_t index = 0; index < companion_count; ++index) {
      copies[index] = incoming[index];
      emergencies[index] = std::make_unique<entry>(
          companion_names[index],
          std::vector<std::uint8_t>(incoming[index].begin(),
                                    incoming[index].end()));
      const auto &reserved = *emergencies[index];
      const auto next = copies[index].capacity() + 1 + sizeof(entry) +
                        reserved.name.capacity() + 1 +
                        reserved.bytes.capacity() +
                        reserved.immutable.capacity();
      if (next > limit_ - retained_ - charge) {
        error = "Companion payloads exceed the retention budget.";
        return false;
      }
      charge += next;
    }
    // Publish only after every identity, allocation and budget check succeeds.
    payloads_.swap(copies);
    for (std::size_t index = 0; index < companion_count; ++index)
      emergency_[index] = std::move(emergencies[index]);
    retained_ += charge;
    return true;
  }

  static unsigned role(std::string_view name) noexcept {
    return name.ends_with(".csc") ? 1u : 0u;
  }
  bool begin_generation(bool client) {
    std::scoped_lock lock(mutex_);
    if (exhausted_)
      return false;
    const unsigned index = client ? 1u : 0u;
    current_[index].clear();
    ++generation_[index];
    if (!client)
      island_server_generation_ = 0;
    return true;
  }
  generation_lease capture_generation(std::string_view name) {
    const auto index = role(canonical_asset(name));
    std::scoped_lock lock(mutex_);
    return {index, generation_[index]};
  }
  // Execute a context-guarded failure action only while this VM lease is
  // current. The callback must not call this store; begin_generation cannot
  // overtake it.
  template <class Action>
  bool if_current(generation_lease lease, Action action) {
    std::scoped_lock lock(mutex_);
    if (lease.role > 1 || lease.generation != generation_[lease.role])
      return false;
    action();
    return true;
  }
  // Only the native transform of a validated Island server anchor may arm this
  // identity. A new SERVER VM revokes it; a CSC generation cannot grant it.
  bool authorize_island_server(generation_lease lease) {
    std::scoped_lock lock(mutex_);
    if (exhausted_ || lease.role != 0u || lease.generation != generation_[0])
      return false;
    island_server_generation_ = generation_[0];
    return true;
  }
  bool island_server_authorized() {
    std::scoped_lock lock(mutex_);
    return island_server_generation_ != 0 &&
           island_server_generation_ == generation_[0];
  }
  Asset *cached(std::string_view name,
                std::optional<generation_lease> lease = {}) {
    std::scoped_lock lock(mutex_);
    const auto key = canonical_asset(name);
    const auto index = role(key);
    if (lease &&
        (lease->role != index || lease->generation != generation_[index]))
      return nullptr;
    const auto &map = current_[index];
    const auto found = map.find(key);
    return found == map.end() ? nullptr : found->second;
  }
  bool exhausted() {
    std::scoped_lock lock(mutex_);
    return exhausted_;
  }
  std::optional<std::vector<std::uint8_t>> template_for(const Asset *asset) {
    std::scoped_lock lock(mutex_);
    for (std::size_t i = 0; i < count_; ++i)
      if (&exposed_[i]->asset == asset)
        return exposed_[i]->immutable;
    return std::nullopt;
  }
  Asset *retain(std::string_view name, std::vector<std::uint8_t> bytes,
                std::string &error, std::optional<generation_lease> lease = {},
                std::optional<std::vector<std::uint8_t>> pristine = {}) {
    return retain_if(name, std::move(bytes), error, lease, std::move(pristine),
                     [] { return true; });
  }
  // The caller's native context is sampled under the generation lock before
  // allocations and again immediately before publication. Guards must not call
  // this store. Optional pristine backing is charged as immutable.capacity().
  template <class Guard>
  Asset *retain_if(std::string_view name, std::vector<std::uint8_t> bytes,
                   std::string &error, std::optional<generation_lease> lease,
                   std::optional<std::vector<std::uint8_t>> pristine,
                   Guard current) {
    std::scoped_lock lock(mutex_);
    const auto key = canonical_asset(name);
    if (key.empty()) {
      error = "Invalid compiled asset path.";
      return nullptr;
    }
    const auto index = role(key);
    if (lease &&
        (lease->role != index || lease->generation != generation_[index])) {
      error = "Script asset generation changed during lookup; stale work was "
              "rejected.";
      return nullptr;
    }
    if (!current()) {
      error = "Native schema context changed during lookup.";
      return nullptr;
    }
    auto &cache = current_[index];
    if (const auto found = cache.find(key); found != cache.end())
      return found->second;
    if (exhausted_) {
      error = "Schema asset store is permanently exhausted; restart required.";
      return nullptr;
    }
    auto fresh =
        std::make_unique<entry>(key, std::move(bytes), std::move(pristine));
    const auto charge = sizeof(entry) + fresh->name.capacity() + 1 +
                        fresh->bytes.capacity() + fresh->immutable.capacity() +
                        sizeof(std::pair<const std::string, Asset *>) +
                        name.size() + 64;
    if (!current()) {
      error = "Native schema context changed before retention.";
      return nullptr;
    }
    if (count_ >= exposed_.size() || charge > limit_ ||
        retained_ > limit_ - charge) {
      error = "Experimental schema asset retention limit reached; restart "
              "required.";
      exhausted_ = true;
      return nullptr;
    }
    // Allocate the cache node before publishing lifetime/count/charge. A node
    // allocation failure must not leave a half-published retained entry.
    const auto inserted = cache.emplace(key, nullptr);
    bool accepted = false;
    try {
      accepted = current();
    } catch (...) {
      cache.erase(inserted.first);
      throw;
    }
    if (!accepted) {
      cache.erase(inserted.first);
      error = "Native schema context changed before publication.";
      return nullptr;
    }
    auto *result = &fresh->asset;
    exposed_[count_++] = std::move(fresh);
    retained_ += charge;
    inserted.first->second = result;
    return result;
  }

  std::pair<std::size_t, std::size_t> retained() {
    std::scoped_lock lock(mutex_);
    return {count_, retained_};
  }

  Asset *companion(std::string_view name, std::string &error,
                   std::optional<generation_lease> captured = {}) {
    const auto identity = fixed_companion_identity(name);
    if (!identity) {
      error = "Unknown fixed companion identity.";
      return nullptr;
    }
    const auto index = static_cast<unsigned>(*identity);
    const auto realm = companion_realm(*identity);
    const auto lease = captured ? *captured : capture_generation(name);
    if (auto *existing = cached(name, lease))
      return existing;
    const auto key = canonical_asset(name);
    std::vector<std::uint8_t> bytes;
    {
      std::scoped_lock lock(mutex_);
      if (lease.role != realm || lease.generation != generation_[realm]) {
        error = "Script companion generation changed during lookup; stale work "
                "was rejected.";
        return nullptr;
      }
      if (*identity == companion_id::island_trials &&
          island_server_generation_ != generation_[0]) {
        error = "Island companion was not authorized by this server VM's map "
                "anchor.";
        return nullptr;
      }
      const auto &payload = payloads_[index];
      if (payload.empty()) {
        error = "Validated companion payload has not been pinned.";
        return nullptr;
      }
      if (exhausted_ && emergency_generation_[index] == generation_[realm])
        return &emergency_[index]->asset;
      bytes.assign(payload.begin(), payload.end());
    }
    return retain(key, std::move(bytes), error, lease);
  }
  // A missing asset is dereferenced by the native resolver. Reserve a valid,
  // never-before-exposed fallback at pin time for the first fatal generation.
  // Admission stays revoked; this permits safe unwinding to the deferred drop.
  Asset *emergency_companion(std::string_view name) {
    // No string/map/vector allocation on the allocation-failure path.
    const auto chosen = fixed_companion_identity(name);
    if (!chosen)
      return nullptr;
    const auto index = static_cast<unsigned>(*chosen);
    const auto realm = companion_realm(*chosen);
    std::scoped_lock lock(mutex_);
    exhausted_ = true; // no later expanded generation may reuse this fallback
    for (const auto &[path, existing] : current_[realm])
      if (fixed_companion_identity(path) == chosen)
        return existing;
    if (!emergency_[index])
      return nullptr;
    if (emergency_generation_[index] &&
        emergency_generation_[index] != generation_[realm])
      return nullptr;
    emergency_generation_[index] = generation_[realm];
    return &emergency_[index]->asset;
  }
};
} // namespace zombies::coop::t7
