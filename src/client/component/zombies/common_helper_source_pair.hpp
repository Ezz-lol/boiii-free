#pragma once
// Source-pair and load-completion guard; no linked-body authentication or new
// wire API. Caller callbacks must sample only already-captured/native atomic
// context; never call this store, the loader registry, compiler or engine
// linker in them.
#include "schema_asset_store.hpp"
#include "common_helper_native_context.hpp"
#include <atomic>
#include <limits>

namespace zombies::coop::common_helper {
using digest = std::array<std::uint8_t, 32>;
using hash_function = digest (*)(std::span<const std::uint8_t>);
constexpr std::string_view logical_name = "custom_scripts/zm/eroded_coop.gsc";
constexpr std::size_t common_limit = 1024 * 1024;
constexpr std::uint32_t helper_name = 3397961268u, helper_space = 1107623254u;
constexpr std::uint32_t script_hash(std::string_view value) noexcept {
  std::uint32_t hash = 0x4b9ace2f;
  for (char c : value)
    hash = (hash ^ static_cast<std::uint8_t>(c)) * 0x1000193;
  return hash * 0x1000193;
}
static_assert(helper_name == script_hash("guarded_powerup_timer"));
static_assert(helper_space == script_hash("eroded_coop"));
enum class family { cb, ezz };
struct descriptor {
  std::uint32_t start{}, end{}, name{}, space{};
  std::uint8_t params{}, flags{};
  bool operator==(const descriptor &) const = default;
};
inline std::uint16_t read16(std::span<const std::uint8_t> bytes,
                            std::size_t at) {
  return static_cast<std::uint16_t>(bytes[at] | bytes[at + 1] << 8);
}
inline digest hex_digest(std::string_view text) {
  digest value{};
  const auto nibble = [](char c) -> unsigned {
    return c >= 'a' ? c - 'a' + 10u : c - '0';
  };
  for (std::size_t i = 0; i < value.size(); ++i)
    value[i] = static_cast<std::uint8_t>((nibble(text[i * 2]) << 4) |
                                         nibble(text[i * 2 + 1]));
  return value;
}
inline const auto source_pin = hex_digest(
    "199cb573e1034ed6299158f43c1e92c523a3287d25673d7912985ccdcbe500b7");
inline const auto cb_compiled_pin = hex_digest(
    "9a1aca430f15a051d66fc7658de8102085610c3b67b065b069a96397b76cd7f2");

inline std::optional<descriptor> inspect(std::span<const std::uint8_t> bytes) {
  if (bytes.size() < t7::object_header_size || bytes.size() > common_limit ||
      !std::equal(t7::object_magic.begin(), t7::object_magic.end(),
                  bytes.begin()))
    return {};
  const auto name = t7::bounded_name(bytes, t7::read32(bytes, 0x34));
  if (!name || *name != logical_name)
    return {};
  const auto code = t7::read32(bytes, 0x14), size = t7::read32(bytes, 0x30);
  const auto table = t7::read32(bytes, 0x20);
  const auto count = read16(bytes, 0x3a);
  if (code < t7::object_header_size || code > bytes.size() || !size ||
      size > bytes.size() - code || table < t7::object_header_size ||
      table > bytes.size() || !count || count > 4096 ||
      count > (bytes.size() - table) / 20)
    return {};
  std::optional<descriptor> chosen;
  std::vector<std::uint32_t> starts;
  for (std::size_t i = 0; i < count; ++i) {
    const auto at = table + i * 20;
    const auto start = t7::read32(bytes, at + 4);
    if (start < code || start >= code + size ||
        std::find(starts.begin(), starts.end(), start) != starts.end())
      return {};
    starts.push_back(start);
    if (t7::read32(bytes, at + 8) == helper_name &&
        t7::read32(bytes, at + 12) == helper_space) {
      if (chosen || bytes[at + 16] != 2 || bytes[at + 17] != 0)
        return {};
      chosen = descriptor{start, code + size, helper_name, helper_space, 2, 0};
    }
  }
  if (!chosen)
    return {};
  for (const auto start : starts)
    if (start > chosen->start && start < chosen->end)
      chosen->end = start;
  return chosen;
}

// INTERNAL trusted loader binding, never supplied by a dvar, packet or file.
// Ezz may create this ONLY from the successful readable-source compiler path,
// attached to the newly allocated tree BEFORE publishing it to loaded_scripts.
// Rawmagic and precompiled branches must leave
// ezz_source_compile_success=false.
template <class Asset> struct binding {
  family source_family{};
  const Asset *loader_tree{};
  const void *loader_buffer{};
  std::size_t length{};
  digest compiled_digest{}, helper_body_digest{}, readable_digest{},
      cleaned_digest{};
  descriptor helper{};
  bool ezz_source_compile_success{};
};

template <class Asset>
std::optional<binding<Asset>>
bind_loader_object(family source_family, const Asset *newly_allocated_tree,
                   std::span<const std::uint8_t> compiled,
                   std::span<const std::uint8_t> readable,
                   std::span<const std::uint8_t> cleaned,
                   bool source_compile_success, hash_function hash) {
  if ((source_family != family::cb && source_family != family::ezz) || !hash ||
      !newly_allocated_tree || !newly_allocated_tree->buffer ||
      static_cast<std::size_t>(newly_allocated_tree->len) != compiled.size())
    return {};
  const auto helper = inspect(compiled);
  if (!helper)
    return {};
  const auto compiled_digest = hash(compiled);
  if (source_family == family::cb && compiled_digest != cb_compiled_pin)
    return {};
  digest raw{}, clean{};
  if (source_family == family::ezz) {
    if (!source_compile_success || readable.empty() ||
        readable.size() > common_limit || cleaned.empty() ||
        cleaned.size() > common_limit)
      return {};
    raw = hash(readable);
    clean = hash(cleaned);
    if (raw != source_pin || clean != source_pin)
      return {};
  }
  const auto *allocated =
      reinterpret_cast<const std::uint8_t *>(newly_allocated_tree->buffer);
  if (hash({allocated, compiled.size()}) != compiled_digest)
    return {};
  return binding<Asset>{
      source_family,
      newly_allocated_tree,
      newly_allocated_tree->buffer,
      compiled.size(),
      compiled_digest,
      hash(compiled.subspan(helper->start, helper->end - helper->start)),
      raw,
      clean,
      *helper,
      source_family == family::ezz && source_compile_success};
}

template <class Asset>
bool selected_winner_matches(const binding<Asset> &bound,
                             const Asset *actual_selected_winner,
                             hash_function hash) {
  if (!hash || !actual_selected_winner ||
      actual_selected_winner != bound.loader_tree ||
      actual_selected_winner->buffer != bound.loader_buffer ||
      static_cast<std::size_t>(actual_selected_winner->len) != bound.length ||
      bound.length > common_limit)
    return false;
  const auto bytes = std::span(
      reinterpret_cast<const std::uint8_t *>(actual_selected_winner->buffer),
      bound.length);
  const auto helper = inspect(bytes);
  return helper && *helper == bound.helper &&
         hash(bytes) == bound.compiled_digest &&
         hash(bytes.subspan(helper->start, helper->end - helper->start)) ==
             bound.helper_body_digest;
}

// INTERNAL seal made while the actual loader mutex/registry/allocator is
// protected. Preregistration later reads only this owned copy, not freed trees.
template <class Asset> struct sealed_binding {
  binding<Asset> bound;
  std::vector<std::uint8_t> bytes;
};
template <class Asset>
std::optional<sealed_binding<Asset>>
seal_selected_winner(const binding<Asset> &bound,
                     const Asset *actual_selected_winner, hash_function hash) {
  if (!selected_winner_matches(bound, actual_selected_winner, hash))
    return {};
  const auto *begin =
      reinterpret_cast<const std::uint8_t *>(actual_selected_winner->buffer);
  sealed_binding<Asset> sealed{bound, {begin, begin + bound.length}};
  if (hash(sealed.bytes) != bound.compiled_digest)
    return {};
  return sealed;
}
template <class Asset>
bool valid_seal(const sealed_binding<Asset> &sealed, hash_function hash) {
  const auto &bound = sealed.bound;
  if (!hash || sealed.bytes.size() != bound.length ||
      bound.length > common_limit ||
      (bound.source_family != family::cb && bound.source_family != family::ezz))
    return false;
  const auto helper = inspect(sealed.bytes);
  if (!helper || *helper != bound.helper ||
      hash(sealed.bytes) != bound.compiled_digest ||
      hash(std::span<const std::uint8_t>(sealed.bytes)
               .subspan(helper->start, helper->end - helper->start)) !=
          bound.helper_body_digest)
    return false;
  if (bound.source_family == family::cb)
    return bound.compiled_digest == cb_compiled_pin;
  return bound.ezz_source_compile_success &&
         bound.readable_digest == source_pin &&
         bound.cleaned_digest == source_pin;
}

template <class Asset> class lifecycle {
public:
  using store_type = t7::schema_asset_store<Asset>;
  using lease = typename store_type::generation_lease;
  struct request {
    lease vm;
    int native_epoch{}, capacity{};
    std::string map;
    native_ticket session;
  };
  struct range {
    std::uintptr_t begin{}, end{};
  };

private:
  struct receipt {
    request identity;
    Asset *common{};
    Asset *powerup{};
    descriptor helper;
    std::vector<range> powerup_protected;
    std::shared_ptr<std::atomic<bool>> revoked;
    bool common_load_returned{};
    bool load_complete{};
  };
  // Lock order: receipt operation -> retained store. Store callbacks use only
  // native atomics and never reenter this lifecycle, loader or engine.
  std::mutex operation_mutex_;
  std::atomic<std::shared_ptr<const receipt>> current_;
  static bool valid(const request &request) {
    return request.vm.role == 0 && request.vm.generation != 0 &&
           request.native_epoch > 0 && request.capacity >= 5 &&
           request.capacity <= 8 && t7::supported_map_name(request.map) &&
           request.session && request.session->epoch == request.native_epoch &&
           request.session->capacity == request.capacity &&
           request.session->map == request.map;
  }
  static bool same(const request &a, const request &b) {
    return a.vm.role == b.vm.role && a.vm.generation == b.vm.generation &&
           a.native_epoch == b.native_epoch && a.capacity == b.capacity &&
           a.map == b.map && a.session == b.session;
  }

public:
  // Called after actual registry scans join and selected winner is sealed under
  // loader protection, BEFORE custom includes/Scr_LoadScript. No live loader
  // object, registry or allocator is read by this function or store callback.
  template <class NativeCurrent>
  Asset *preregister(store_type &store, const request &request,
                     const sealed_binding<Asset> &sealed, hash_function hash,
                     std::string &error, NativeCurrent native_current) {
    const std::lock_guard operation_lock(operation_mutex_);
    if (!valid(request) || !native_current() || !valid_seal(sealed, hash))
      return nullptr;
    const auto &bound = sealed.bound;
    const auto *bytes = sealed.bytes.data();
    auto prepared = std::make_shared<receipt>();
    prepared->identity = request;
    prepared->helper = bound.helper;
    prepared->revoked = std::make_shared<std::atomic<bool>>(false);
    prepared->common =
        store.retain_if(logical_name, {bytes, bytes + bound.length}, error,
                        request.vm, {}, native_current);
    if (!prepared->common)
      return nullptr;
    // retain_if returns an existing name in this lease. Never bless a later
    // differently compiled winner by attaching its descriptor to old backing.
    const auto immutable = store.template_for(prepared->common);
    if (!immutable || hash(*immutable) != bound.compiled_digest)
      return nullptr;
    bool published = false;
    (void)store.if_current(request.vm, [&] {
      if (!native_current())
        return;
      const auto previous = current_.load();
      if (previous && same(previous->identity, request)) {
        // Duplicate loader work cannot erase link state or clear an observed
        // foreign detour. A revoked lease requires a fresh VM/session.
        if (previous->common != prepared->common || previous->revoked->load())
          return;
        published = true;
        return;
      }
      current_.store(prepared);
      published = true;
    });
    return published ? prepared->common : nullptr;
  }
  // The fixed raw metadata worker publishes/retains the transformed stock
  // module separately. Its verified raw factory and foreign-call ABI are NOT
  // supplied here. Only exact factory-owned caller/timer ranges are observed.
  template <class NativeCurrent>
  bool pair_transformed_powerup(store_type &store, const request &request,
                                Asset *transformed,
                                std::vector<range> protected_ranges,
                                NativeCurrent native_current) {
    const std::lock_guard operation_lock(operation_mutex_);
    const auto previous = current_.load();
    if (!previous || !valid(request) || !same(previous->identity, request) ||
        previous->revoked->load())
      return false;
    if (!transformed || !transformed->buffer || transformed->len <= 0 ||
        static_cast<std::size_t>(transformed->len) > 256 * 1024 ||
        protected_ranges.size() != 2)
      return false;
    const auto base = reinterpret_cast<std::uintptr_t>(transformed->buffer);
    const auto size = static_cast<std::size_t>(transformed->len);
    if (base > std::numeric_limits<std::uintptr_t>::max() - size)
      return false;
    for (const auto range : protected_ranges)
      if (range.begin < base || range.begin >= range.end ||
          range.end > base + size)
        return false;
    if (protected_ranges[0].begin < protected_ranges[1].end &&
        protected_ranges[1].begin < protected_ranges[0].end)
      return false;
    auto next = std::make_shared<receipt>(*previous);
    next->powerup = transformed;
    next->powerup_protected = std::move(protected_ranges);
    return store.common_powerup_if(
        request.vm, previous->common, transformed, false,
        [&] {
          return native_current() && current_.load() == previous &&
                 !previous->revoked->load();
        },
        [&] { current_.store(next); });
  }
  // Trusted source/cache/load-completion only: no engine-entry or whole-body
  // authentication claim. Called after the ACTUAL existing custom load loop and
  // pending detours finish; no invented Scr_GetFunctionHandle pointer
  // conversion.
  template <class NativeCurrent>
  bool finish_loader(store_type &store, const request &request,
                     NativeCurrent native_current) {
    const std::lock_guard operation_lock(operation_mutex_);
    const auto previous = current_.load();
    if (!previous || !same(previous->identity, request) ||
        !previous->common_load_returned || !previous->powerup ||
        previous->revoked->load())
      return false;
    auto next = std::make_shared<receipt>(*previous);
    next->load_complete = true;
    return store.common_powerup_if(
        request.vm, previous->common, previous->powerup, false,
        [&] {
          return native_current() && current_.load() == previous &&
                 !previous->revoked->load();
        },
        [&] { current_.store(next); });
  }
  // Called ONLY after the real existing server Scr_LoadScript call for the
  // exact canonical common module returns. Ezz's missing-include early return
  // does not reach this callback. It proves return, not engine entry identity.
  template <class NativeCurrent>
  bool note_common_load_returned(store_type &store, const request &request,
                                 NativeCurrent native_current) {
    const std::lock_guard operation_lock(operation_mutex_);
    const auto previous = current_.load();
    if (!previous || !same(previous->identity, request) ||
        previous->revoked->load())
      return false;
    auto next = std::make_shared<receipt>(*previous);
    next->common_load_returned = true;
    return store.common_only_if(
        request.vm, previous->common,
        [&] {
          return native_current() && current_.load() == previous &&
                 !previous->revoked->load();
        },
        [&] { current_.store(next); });
  }
  template <class NativeCurrent>
  bool ready(store_type &store, const request &request,
             NativeCurrent native_current) {
    const std::lock_guard operation_lock(operation_mutex_);
    const auto observed = current_.load();
    if (!observed || !same(observed->identity, request) ||
        !observed->load_complete || observed->revoked->load())
      return false;
    return store.common_powerup_if(
        request.vm, observed->common, observed->powerup, true,
        [&] {
          return native_current() && current_.load() == observed &&
                 !observed->revoked->load();
        },
        [] {});
  }
  // Exact canonical common lookup and protected detour resolution use the same
  // retained backing. It is available before the dependent powerup is linked,
  // and remains available in the current ending VM. Admission is separate.
  template <class NativeOwned>
  Asset *lookup_common(store_type &store, const request &request,
                       NativeOwned native_owned) {
    const std::lock_guard operation_lock(operation_mutex_);
    const auto observed = current_.load();
    if (!observed || !same(observed->identity, request))
      return nullptr;
    Asset *result = nullptr;
    (void)store.common_only_if(
        request.vm, observed->common,
        [&] { return native_owned() && current_.load() == observed; },
        [&] { result = observed->common; });
    return result;
  }

  template <class NativeOwned>
  Asset *lookup_powerup(store_type &store, const request &request,
                        NativeOwned native_owned) {
    const std::lock_guard operation_lock(operation_mutex_);
    const auto observed = current_.load();
    if (!observed || !same(observed->identity, request) || !observed->powerup)
      return nullptr;
    Asset *result = nullptr;
    (void)store.common_powerup_if(
        request.vm, observed->common, observed->powerup, false,
        [&] { return native_owned() && current_.load() == observed; },
        [&] { result = observed->powerup; });
    return result;
  }
  // Relevant Ezz pending AND runtime replacefunc paths call this before their
  // original detour operation. Unrelated targets do nothing; normal mod detour
  // semantics still run. Failure publication must be native atomic/deferred.
  template <class NativeCurrent, class RevokeAdmission>
  bool observe_detour(store_type &store, std::uintptr_t target,
                      NativeCurrent native_current,
                      RevokeAdmission revoke_admission) {
    const std::lock_guard operation_lock(operation_mutex_);
    const auto observed = current_.load();
    if (!observed)
      return false;
    const auto base =
        reinterpret_cast<std::uintptr_t>(observed->common->buffer);
    if (base >
        std::numeric_limits<std::uintptr_t>::max() - observed->helper.end)
      return false;
    bool relevant = target >= base + observed->helper.start &&
                    target < base + observed->helper.end;
    for (const auto range : observed->powerup_protected)
      relevant = relevant || (range.begin < range.end &&
                              target >= range.begin && target < range.end);
    if (!relevant)
      return false;
    return store.common_only_if(
        observed->identity.vm, observed->common,
        [&] { return native_current() && current_.load() == observed; },
        [&] {
          observed->revoked->store(true);
          revoke_admission(observed->identity.native_epoch);
        });
  }
};
} // namespace zombies::coop::common_helper
