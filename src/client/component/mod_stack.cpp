#include <std_include.hpp>

#include "mod_stack.hpp"
#include "scheduler.hpp"
#include "workshop/workshop.hpp"

#include <component/lua/lua_state.hpp>
#include <game/game.hpp>
#include <game/impl/ugc/ugc.hpp>
#include <game/utils.hpp>
#include <loader/component_loader.hpp>

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace mod_stack {
namespace {
using namespace game::lua;
using namespace game::lua::hks;
using game::bg::cache::BGCacheInfoDef;
using game::db::xasset::XAssetEntry;
using game::db::xasset::XAssetType;
using game::db::xzone::XZoneInfo;
using game::db::xzone::XZoneName;
using game::db::xzone::XZoneState;

constexpr std::string_view ALIAS_PREFIX = "stack_";
constexpr int32_t CORE_MOD_FLAGS = 0x40;
constexpr int32_t MODE_MOD_FLAGS = 0x100;
constexpr std::array MODES{"mp", "zm", "cp"};

utils::hook::detour zone_path_hook;
utils::hook::detour link_asset_hook;
utils::hook::detour zone_dependency_hook;
utils::hook::detour open_sound_bank_hook;
utils::hook::detour asset_priority_hook;
utils::hook::detour apply_queued_hook;

struct provider {
  std::string id;
  std::filesystem::path folder;
};

struct route {
  std::filesystem::path folder;
  std::string id;
  std::string name;
};

struct precache_source {
  game::bg::cache::BGCacheInfo *cache;
  std::vector<BGCacheInfoDef> defs;
};

std::recursive_mutex mutex;
std::map<std::string, std::filesystem::path, std::greater<>> folders;
std::vector<std::string> loaded;
std::vector<std::string> priorities;
size_t ui_owner = SIZE_MAX;
std::unordered_map<const void *, std::string> sound_owners;
std::map<std::string, std::map<std::string, precache_source>> precaches;
std::deque<std::vector<BGCacheInfoDef>> merged_precaches;
std::deque<std::string> stream_names;
std::unordered_map<const char *, const char *> stream_originals;

struct script_copy {
  std::string zone;
  std::string name;
  std::string data;
};
std::vector<script_copy> script_copies_;
// Best mod priority that shipped either half (.gsc/.csc) of each script
std::unordered_map<std::string, size_t> script_owners;

std::string startup_list;
std::atomic_bool reload_pending;

thread_local std::vector<XZoneInfo> prepared;
thread_local std::vector<std::array<char, 64>> prepared_names;
thread_local std::string sound_owner;

std::string command_line_list() {
  int32_t count = 0;
  LPWSTR *const argv = CommandLineToArgvW(GetCommandLineW(), &count);
  std::string result;
  for (int32_t i = 0; argv && i + 2 < count; ++i) {
    if (_wcsicmp(argv[i], L"+set") == 0 &&
        _wcsicmp(argv[i + 1], L"fs_extramods") == 0) {
      result = utils::string::convert(argv[i + 2]);
    }
  }
  LocalFree(argv);
  return result;
}

std::string main_mod() {
  const std::string id = game::ugc::UGC_ActiveMod_PublisherId();
  return id == "usermaps" ? std::string{} : id;
}

bool is_mod_zone(const std::string_view name) {
  return name == "core_mod" || name == "mp_mod" || name == "zm_mod" ||
         name == "cp_mod";
}

std::string alias(const std::string &id, const std::string_view name) {
  return std::format("{}{}_{}", ALIAS_PREFIX, id, name);
}

bool has_file(const std::filesystem::path &folder, const std::string &file) {
  std::error_code ec;
  return std::filesystem::is_regular_file(folder / file, ec);
}

std::optional<route> find_route(const std::string_view name) {
  std::string_view rest = name;
  std::string_view language;
  if (rest.size() > 3 && rest[2] == '_' && !rest.starts_with(ALIAS_PREFIX)) {
    language = rest.substr(0, 3);
    rest.remove_prefix(3);
  }
  if (!rest.starts_with(ALIAS_PREFIX)) {
    return std::nullopt;
  }
  rest.remove_prefix(ALIAS_PREFIX.size());

  std::scoped_lock lock(mutex);
  for (const auto &[id, folder] : folders) {
    if (rest.size() > id.size() && rest.starts_with(id) &&
        rest[id.size()] == '_') {
      return route{folder, id,
                   std::string(language) +
                       std::string(rest.substr(id.size() + 1))};
    }
  }
  return std::nullopt;
}

std::string zone_name(const XAssetEntry *entry) {
  if (!entry->zoneIndex) {
    return {};
  }
  return reinterpret_cast<XZoneName *>(
             game::db::xzone::g_zoneNames.get())[entry->zoneIndex]
      .name;
}

std::string loading_zone() {
  const char *name = game::db::load::g_load->filename;
  return name ? name : "";
}

bool zone_alive(const std::string &name) {
  // The zone count can trail the highest used slot, so scan the whole table
  const auto *zones =
      reinterpret_cast<XZoneName *>(game::db::xzone::g_zoneNames.get());
  for (uint32_t i = 0; i <= game::db::xzone::MAX_ZONE_COUNT; ++i) {
    const XZoneState state = zones[i].state;
    if (name == zones[i].name && (state == XZoneState::XZONE_LOADING ||
                                  state == XZoneState::XZONE_LOADED ||
                                  state == XZoneState::XZONE_COMPLETE)) {
      return true;
    }
  }
  return false;
}

std::vector<provider> resolve_providers(const std::vector<std::string> &ids) {
  std::vector<provider> result;
  for (const std::string &id : ids) {
    const game::ugc::UGCItem *mod = game::ugc::UGC_UnwrapOptionalUGCItem(
        game::ugc::UGC_GetModByPublisherId(id.c_str()));
    if (mod && mod->absolutePathZoneFiles[0] &&
        alias(id, "en_zm_mod_patch").size() < 60) {
      result.push_back({id, mod->absolutePathZoneFiles});
    }
  }
  return result;
}

int32_t prepare_zones_stub(XZoneInfo *zones, const int32_t count, char *,
                           XZoneInfo *, const int32_t capacity) {
  const std::span<XZoneInfo> input(zones, count);
  const bool mod_load = std::ranges::any_of(input, [](const XZoneInfo &zone) {
    return zone.allocFlags == CORE_MOD_FLAGS;
  });
  std::vector<std::string> ids;
  {
    std::scoped_lock lock(mutex);
    ids = mod_load ? get() : loaded;
  }
  const std::vector<provider> extras = resolve_providers(ids);
  std::deque<std::string> names;
  std::vector<XZoneInfo> plan;

  const auto add_extras = [&](const std::string &zone, const int32_t flags) {
    for (const provider &mod : extras) {
      if (has_file(mod.folder, zone + ".ff")) {
        names.push_back(alias(mod.id, zone));
        plan.push_back({names.back().c_str(), flags, 0, 0, 0, {}});
      }
    }
  };
  const game::ugc::UGCItem *main = game::ugc::UGC_UnwrapOptionalUGCItem(
      game::ugc::UGC_GetModByPublisherId(main_mod().c_str()));
  const auto main_has = [&](const std::string &zone) {
    return !main || has_file(main->absolutePathZoneFiles, zone + ".ff");
  };
  const auto contains = [&](const std::string &name) {
    return std::ranges::any_of(input, [&](const XZoneInfo &zone) {
      return zone.name && name == zone.name;
    });
  };

  for (const XZoneInfo &zone : input) {
    plan.push_back(zone);
    if (zone.name && is_mod_zone(zone.name)) {
      add_extras(zone.name, zone.allocFlags);
    }
    if (!zone.name && zone.allocFlags == CORE_MOD_FLAGS &&
        !main_has("core_mod")) {
      add_extras("core_mod", CORE_MOD_FLAGS);
    }
  }
  for (const char *mode : MODES) {
    if (contains(std::format("{}_common", mode)) &&
        !main_has(std::format("{}_mod", mode))) {
      add_extras(std::format("{}_mod", mode), MODE_MOD_FLAGS);
    }
  }

  {
    std::scoped_lock lock(mutex);
    for (auto &[cache, sources] : precaches) {
      std::erase_if(sources, [&](const auto &source) {
        return std::ranges::any_of(plan, [&](const XZoneInfo &zone) {
          return zone.name && (source.first == zone.name ||
                               (source.first.size() > 3 &&
                                source.first.substr(3) == zone.name));
        });
      });
    }
  }
  if (mod_load) {
    std::scoped_lock lock(mutex);
    loaded = ids;
    priorities.clear();
    script_owners.clear();
    for (const provider &mod : extras) {
      folders[mod.id] = mod.folder;
      priorities.push_back(mod.id);
    }
    ui_owner = SIZE_MAX;
  }
  if (mod_load && main_mod().empty()) {
    scheduler::once([] { set({}); }, scheduler::main);
  }

  const size_t size = std::max<size_t>(capacity, plan.size() * 4);
  prepared.resize(size);
  prepared_names.resize(size);
  return game::db::load::DB_ExpandZoneList(
      plan.data(), static_cast<int32_t>(plan.size()),
      reinterpret_cast<char (*)[64]>(prepared_names.data()), prepared.data(),
      static_cast<int32_t>(size));
}

void schedule_zones_stub(XZoneInfo *, const int32_t count) {
  game::db::load::DB_LoadXZone(prepared.data(), count);
}

int32_t zone_path_stub(const char *name, const char *extension,
                       const int32_t size, char *buf, const game::ZoneType type,
                       const char *publisher) {
  if (name && extension) {
    if (const std::optional<route> target = find_route(name)) {
      return std::snprintf(buf, size, "%s/%s%s",
                           target->folder.generic_string().c_str(),
                           target->name.c_str(), extension);
    }
    if (!sound_owner.empty() && type == game::ZoneType::MOD) {
      std::scoped_lock lock(mutex);
      if (const auto folder = folders.find(sound_owner);
          folder != folders.end()) {
        return std::snprintf(buf, size, "%s/%s%s",
                             folder->second.generic_string().c_str(), name,
                             extension);
      }
    }
  }
  return zone_path_hook.invoke<int32_t>(name, extension, size, buf, type,
                                        publisher);
}

bool zone_dependency_stub(const char *key, const char *zone,
                          const char **cursor) {
  const std::optional<route> target =
      key && zone && cursor && std::string_view(key) == "xpak_read"
          ? find_route(zone)
          : std::nullopt;
  if (!target) {
    return zone_dependency_hook.invoke<bool>(key, zone, cursor);
  }

  {
    std::scoped_lock lock(mutex);
    if (const auto original = stream_originals.find(*cursor);
        original != stream_originals.end()) {
      *cursor = original->second;
    }
  }
  if (!zone_dependency_hook.invoke<bool>(key, target->name.c_str(), cursor)) {
    return false;
  }

  const char *original = *cursor;
  if (!original || !has_file(target->folder, std::string(original) + ".xpak")) {
    return true;
  }

  const std::string name = alias(target->id, original);
  std::scoped_lock lock(mutex);
  const auto existing = std::ranges::find(stream_names, name);
  const std::string &stored = existing != stream_names.end()
                                  ? *existing
                                  : stream_names.emplace_back(name);
  stream_originals[stored.c_str()] = original;
  *cursor = stored.c_str();
  return true;
}

bool open_sound_bank_stub(const bool streamed, void *asset_bank, void *load) {
  {
    std::scoped_lock lock(mutex);
    const auto owner = load ? sound_owners.find(*static_cast<void **>(load))
                            : sound_owners.end();
    sound_owner = owner != sound_owners.end() ? owner->second : std::string{};
  }
  const bool result =
      open_sound_bank_hook.invoke<bool>(streamed, asset_bank, load);
  sound_owner.clear();
  return result;
}

std::optional<size_t> zone_priority(const std::string &zone) {
  std::string_view base = zone;
  if (base.size() > 3 && base[2] == '_') {
    base.remove_prefix(3);
  }
  if (base.ends_with("_patch")) {
    base.remove_suffix(6);
  }
  if (is_mod_zone(base)) {
    return 0;
  }
  const std::optional<route> owner = find_route(zone);
  if (!owner) {
    return std::nullopt;
  }
  std::scoped_lock lock(mutex);
  const auto found = std::ranges::find(priorities, owner->id);
  return found == priorities.end()
             ? std::nullopt
             : std::optional<size_t>(found - priorities.begin() + 1);
}

std::optional<size_t> priority_of(const XAssetEntry *entry) {
  return zone_priority(zone_name(entry));
}

void merge_precache(const XAssetEntry *entry) {
  game::bg::cache::BGCacheInfo *cache = entry->asset.header.bgCache;
  if (!cache || !cache->name || !is_mod_zone(cache->name) ||
      cache->defCount < 0) {
    return;
  }

  // A zone reloaded under the same name frees its old caches, so only keep
  // caches the asset pool still holds
  std::unordered_set<const void *> live{cache};
  for (const auto &item : game::db::xasset::pool::g_assetEntryPool->pool) {
    if (item.entry.inuse && item.entry.asset.type == XAssetType::BG_CACHE) {
      live.insert(item.entry.asset.header.data);
    }
  }

  std::scoped_lock lock(mutex);
  std::map<std::string, precache_source> &sources = precaches[cache->name];
  sources[loading_zone()] = {cache, {cache->def, cache->def + cache->defCount}};
  std::erase_if(sources, [&](const auto &source) {
    return !zone_alive(source.first) || !live.contains(source.second.cache);
  });

  size_t top = ui_owner;
  for (const auto &[zone, source] : sources) {
    top = std::min(top, zone_priority(zone).value_or(SIZE_MAX));
  }
  if (sources.size() < 2 &&
      zone_priority(sources.begin()->first).value_or(SIZE_MAX) == top) {
    return;
  }
  std::map<std::pair<game::bg::cache::BGCacheTypes, std::string>,
           BGCacheInfoDef>
      unique;
  // Owner first so its menus win name clashes; other mods' menus still load.
  for (const bool owner_pass : {true, false}) {
    for (const auto &[zone, source] : sources) {
      if ((zone_priority(zone).value_or(SIZE_MAX) == top) != owner_pass) {
        continue;
      }
      for (const BGCacheInfoDef &def : source.defs) {
        unique.try_emplace({def.type, def.name}, def);
      }
    }
  }
  std::vector<BGCacheInfoDef> &merged = merged_precaches.emplace_back();
  for (const auto &[key, def] : unique) {
    merged.push_back(def);
  }
  for (const auto &[zone, source] : sources) {
    source.cache->def = merged.data();
    source.cache->defCount = static_cast<int32_t>(merged.size());
  }
}

bool is_ui_file(const XAssetEntry *entry) {
  const char *name = entry->asset.type == XAssetType::RAWFILE
                         ? *static_cast<const char **>(entry->asset.header.data)
                         : nullptr;
  return name && (std::string_view(name).starts_with("ui/") ||
                  std::string_view(name).ends_with(".lua"));
}

void track_ui_owner() {
  if (const std::optional<size_t> priority = zone_priority(loading_zone())) {
    std::scoped_lock lock(mutex);
    ui_owner = std::min(ui_owner, *priority);
  }
}

constexpr std::string_view ENTRY_SCRIPTS[] = {
    "scripts/zm/gametypes/_clientids.gsc",
    "scripts/mp/gametypes/_clientids.gsc",
    "scripts/cp/gametypes/_clientids.gsc",
};

bool is_entry_script(const XAssetEntry *entry) {
  const game::scr::ScriptParseTree *tree = entry->asset.header.scriptParseTree;
  return tree && tree->name &&
         std::ranges::find(ENTRY_SCRIPTS, std::string_view(tree->name)) !=
             std::end(ENTRY_SCRIPTS);
}

std::string script_base(const XAssetEntry *entry) {
  const game::scr::ScriptParseTree *tree = entry->asset.header.scriptParseTree;
  const std::string_view name = tree && tree->name ? tree->name : "";
  if (!name.ends_with(".gsc") && !name.ends_with(".csc")) {
    return {};
  }
  return std::string(name.substr(0, name.size() - 4));
}

int32_t asset_priority_stub(const XAssetEntry *candidate,
                            const XAssetEntry *existing) {
  const int32_t original =
      asset_priority_hook.invoke<int32_t>(candidate, existing);
  if (candidate->asset.type != existing->asset.type) {
    return original;
  }
  {
    std::scoped_lock lock(mutex);
    if (priorities.empty()) {
      return original;
    }
  }

  const std::optional<size_t> incoming = priority_of(candidate);
  const std::optional<size_t> active = priority_of(existing);
  if (candidate->asset.type == XAssetType::SCRIPTPARSETREE &&
      is_entry_script(candidate)) {
    // Mods' entry scripts all run as renamed copies (see script_copies), so
    // the game's own copy keeps the original slot
    if (incoming) {
      return 0;
    }
    if (active) {
      return 1;
    }
  }
  if (incoming && candidate->asset.type == XAssetType::SCRIPTPARSETREE) {
    // A script's server and client halves must come from the same mod, or
    // their clientfield registrations disagree and the match disconnects
    std::scoped_lock lock(mutex);
    const auto owner = script_owners.find(script_base(candidate));
    if (owner != script_owners.end() && owner->second < *incoming) {
      return 0;
    }
  }
  if (incoming && active && *incoming != *active) {
    return *incoming < *active;
  }
  return original;
}

void track_script_owner(const XAssetEntry *entry) {
  const std::optional<size_t> priority = zone_priority(loading_zone());
  if (!priority) {
    return;
  }
  std::string base = script_base(entry);
  if (base.empty()) {
    return;
  }
  std::scoped_lock lock(mutex);
  const auto [owner, added] =
      script_owners.try_emplace(std::move(base), *priority);
  if (!added) {
    owner->second = std::min(owner->second, *priority);
  }
}

void capture_script_copy(const XAssetEntry *entry) {
  const game::scr::ScriptParseTree *tree = entry->asset.header.scriptParseTree;
  if (!tree || !tree->name || !tree->buffer || !tree->len ||
      std::ranges::find(ENTRY_SCRIPTS, std::string_view(tree->name)) ==
          std::end(ENTRY_SCRIPTS)) {
    return;
  }
  const std::string zone = loading_zone();
  if (!is_mod_zone(zone) && !find_route(zone)) {
    return;
  }
  std::string data(reinterpret_cast<const char *>(tree->buffer), tree->len);
  std::scoped_lock lock(mutex);
  std::erase_if(script_copies_, [&](const script_copy &copy) {
    return copy.zone == zone && copy.name == tree->name;
  });
  script_copies_.push_back({zone, tree->name, std::move(data)});
}

template <typename T> T *at(std::string &buf, const size_t offset) {
  return offset + sizeof(T) <= buf.size()
             ? reinterpret_cast<T *>(buf.data() + offset)
             : nullptr;
}

uint32_t append_string(std::string &buf, const std::string &text) {
  const auto offset = static_cast<uint32_t>(buf.size());
  buf.append(text);
  buf.push_back('\0');
  return offset;
}

// Moves a compiled script into its own namespace, file name and system name so
// the game treats it as a separate script from the copy it already loaded.
std::optional<std::string> rename_script(std::string buf,
                                         const std::string &new_name,
                                         const std::string &tag) {
  using game::scr::GSC_EXPORT_ITEM;
  using game::scr::GSC_IMPORT_ITEM;
  using game::scr::GSC_OBJ;
  using game::scr::GSC_STRINGTABLE_ITEM;
  using game::scr::ScrVarCanonicalName_t;

  const GSC_OBJ *header = at<GSC_OBJ>(buf, 0);
  if (!header || !header->hasMagic(GSC_OBJ::T7_MAGIC)) {
    return std::nullopt;
  }
  const GSC_OBJ obj = *header;

  std::map<ScrVarCanonicalName_t, ScrVarCanonicalName_t> spaces;
  std::set<std::pair<ScrVarCanonicalName_t, ScrVarCanonicalName_t>> own;
  for (uint16_t i = 0; i < obj.exports_count; ++i) {
    auto *item = at<GSC_EXPORT_ITEM>(
        buf, obj.exports_offset + size_t{i} * sizeof(GSC_EXPORT_ITEM));
    if (!item) {
      return std::nullopt;
    }
    own.emplace(item->name, item->name_space);
    auto [space, added] = spaces.try_emplace(item->name_space);
    if (added) {
      space->second = game::scr::builtin::fnv1a(
          std::format("{:x}_{}", item->name_space, tag).c_str());
    }
    item->name_space = space->second;
  }

  size_t offset = obj.imports_offset;
  for (uint16_t i = 0; i < obj.imports_count; ++i) {
    auto *item = at<GSC_IMPORT_ITEM>(buf, offset);
    if (!item) {
      return std::nullopt;
    }
    if (own.contains({item->name, item->name_space})) {
      item->name_space = spaces[item->name_space];
    }
    offset += sizeof(GSC_IMPORT_ITEM) + size_t{item->num_address} * 4;
  }

  const std::string system = "clientids";
  std::optional<uint32_t> renamed_system;
  offset = obj.stringtablefixup_offset;
  for (uint16_t i = 0; i < obj.stringtablefixup_count; ++i) {
    auto *item = at<GSC_STRINGTABLE_ITEM>(buf, offset);
    if (!item || item->string >= buf.size()) {
      return std::nullopt;
    }
    const size_t next =
        offset + sizeof(GSC_STRINGTABLE_ITEM) + size_t{item->num_address} * 4;
    if (std::string_view(buf.data() + item->string) == system) {
      if (!renamed_system) {
        renamed_system = append_string(buf, system + "_" + tag);
      }
      at<GSC_STRINGTABLE_ITEM>(buf, offset)->string = *renamed_system;
    }
    offset = next;
  }

  const uint32_t name =
      append_string(buf, new_name.substr(0, new_name.size() - 4));
  at<GSC_OBJ>(buf, 0)->name = name;
  return buf;
}

XAssetEntry *link_asset_stub(XAssetEntry *entry, const game::qboolean allow) {
  if (entry->asset.type == XAssetType::BG_CACHE) {
    merge_precache(entry);
  }
  if (is_ui_file(entry)) {
    track_ui_owner();
  }
  if (entry->asset.type == XAssetType::SCRIPTPARSETREE) {
    capture_script_copy(entry);
    track_script_owner(entry);
  }
  XAssetEntry *result = link_asset_hook.invoke<XAssetEntry *>(entry, allow);
  if (game::is_client() && entry->asset.type == XAssetType::SOUND) {
    for (const XAssetEntry *linked : {entry, result}) {
      const std::optional<route> owner =
          find_route(linked == entry ? loading_zone() : zone_name(linked));
      std::scoped_lock lock(mutex);
      if (owner) {
        sound_owners[linked->asset.header.data] = owner->id;
      } else {
        sound_owners.erase(linked->asset.header.data);
      }
    }
  }
  return result;
}

void apply_queued_stub(const game::LocalClientNum_t client) {
  apply_queued_hook.invoke<void>(client);
  const game::ugc::ModLoadState state = game::ugc::active_mod->loadState;
  if (client != game::LOCAL_CLIENT_0 || !reload_pending ||
      state == game::ugc::ModLoadState::LOADING ||
      (state == game::ugc::ModLoadState::COMPLETE &&
       game::ugc::UGC_ModLoad_TryComplete(client)) ||
      game::ugc::UGC_ModLoad_XZoneLoading()) {
    return;
  }
  reload_pending = false;
  const std::string main = main_mod();
  if (!main.empty() && !is_loaded()) {
    game::ugc::UGC_LoadModByPublisherId_Impl(client, main.c_str(), true);
  }
}

const char *mod_label_stub(const char *, const char *loaded, const char *,
                           const char *) {
  const std::string main = main_mod();
  const game::ugc::UGCItem *mod = game::ugc::UGC_UnwrapOptionalUGCItem(
      game::ugc::UGC_GetModByPublisherId(main.c_str()));
  const std::string title =
      mod && mod->title[0] ? mod->title : game::ugc::active_mod->internalName;
  const size_t extras = get().size();
  if (!extras) {
    return utils::string::va("%s%s", loaded, title.c_str());
  }
  return utils::string::va("%s%s (and %zu more)", loaded, title.c_str(),
                           extras);
}

luaReturnCount_e lua_get(lua_State *luaVM) {
  const char *main = lua_tostring(luaVM, 1);
  lua_pushstring(luaVM,
                 main ? utils::string::join(get(main), ",") : serialize());
  return luaReturnCount_e::ONE;
}

luaReturnCount_e lua_set(lua_State *luaVM) {
  const char *list = lua_tostring(luaVM, 1);
  set(list ? list : "");
  return luaReturnCount_e::NONE;
}

luaReturnCount_e lua_reload(lua_State *) {
  reload_pending = true;
  return luaReturnCount_e::NONE;
}
} // namespace

std::vector<std::string> get(const std::string &main) {
  std::vector<std::string> result;
  if (main.empty() || main == "usermaps") {
    return result;
  }
  const std::optional<std::string_view> list =
      game::get_dvar_string("fs_extramods");
  for (std::string id :
       utils::string::split(list ? std::string(*list) : startup_list, ',')) {
    utils::string::trim(id);
    if (!id.empty() && id != main &&
        std::ranges::find(result, id) == result.end()) {
      result.push_back(std::move(id));
    }
  }
  return result;
}

std::vector<std::string> get() { return get(main_mod()); }

std::string serialize() { return utils::string::join(get(), ","); }

std::vector<std::pair<std::string, std::string>> script_copies() {
  std::vector<script_copy> copies;
  {
    std::scoped_lock lock(mutex);
    std::erase_if(script_copies_, [](const script_copy &copy) {
      return !zone_alive(copy.zone);
    });
    copies = script_copies_;
  }

  std::vector<std::pair<std::string, std::string>> result;
  std::vector<std::string_view> seen;
  for (const script_copy &copy : copies) {
    const game::scr::ScriptParseTree *active =
        game::db::xasset::DB_FindXAssetHeader(XAssetType::SCRIPTPARSETREE,
                                              copy.name.c_str(), false, 0)
            .scriptParseTree;
    if ((active && active->buffer &&
         std::string_view(reinterpret_cast<const char *>(active->buffer),
                          active->len) == copy.data) ||
        std::ranges::find(seen, copy.data) != seen.end()) {
      continue;
    }
    seen.push_back(copy.data);

    const std::optional<route> owner = find_route(copy.zone);
    const std::string tag = owner ? owner->id : "main";
    const std::string name =
        copy.name.substr(0, copy.name.size() - 4) + "_" + tag + ".gsc";
    if (std::optional<std::string> data = rename_script(copy.data, name, tag)) {
      result.emplace_back(name, std::move(*data));
    } else {
      printf("[mod_stack] could not load %s from %s\n", copy.name.c_str(),
             copy.zone.c_str());
    }
  }
  return result;
}

void set(const std::string &list) {
  if (game::EngineDependentDvarMut dvar = game::get_dvar("fs_extramods")) {
    dvar.set(list.c_str());
  }
}

bool is_loaded() {
  const std::vector<std::string> extras = get();
  std::scoped_lock lock(mutex);
  return loaded == extras;
}

class component final : public generic_component {
  DEFINE_COMPONENT_NAME("mod_stack");

public:
  void post_unpack() override {
    if (!game::is_server() && !game::is_new_client()) {
      return;
    }

    startup_list = command_line_list();
    scheduler::once(
        [] {
          game::register_dvar_string(
              "fs_extramods", startup_list.c_str(), game::DVAR_NONE,
              "Extra mods loaded together with fs_game, comma separated, "
              "highest priority first. fs_game always has the highest "
              "priority.");
        },
        scheduler::main);

    using game::db::load::DB_LoadXAssets;
    utils::hook::call(
        DB_LoadXAssets.offset(game::select_value(0x347, 0x0, 0x32D)),
        prepare_zones_stub);
    utils::hook::call(
        DB_LoadXAssets.offset(game::select_value(0x353, 0x0, 0x339)),
        schedule_zones_stub);
    zone_path_hook.create(game::fs::FS_BuildOSPathForThread.get(),
                          zone_path_stub);
    link_asset_hook.create(game::db::load::DB_LinkXAssetEntry.get(),
                           link_asset_stub);
    asset_priority_hook.create(game::db::load::DB_OverrideAsset.get(),
                               asset_priority_stub);

    if (game::is_client()) {
      zone_dependency_hook.create(game::com::KeyValuePairs_GetNextValue.get(),
                                  zone_dependency_stub);
      open_sound_bank_hook.create(game::snd::SND_StartHeaderRead.get(),
                                  open_sound_bank_stub);
      utils::hook::call(game::ui::UIElement_RenderLoadedMod.offset(0xD5),
                        mod_label_stub);
      apply_queued_hook.create(game::ugc::UGC_ModChange_ApplyQueued.get(),
                               apply_queued_stub);

      static constexpr const luaL_Reg ModStackLibrary[] = {
          lua_state::luaL_LoggedReg<"ModStack", "Get", lua_get>(),
          lua_state::luaL_LoggedReg<"ModStack", "Set", lua_set>(),
          lua_state::luaL_LoggedReg<"ModStack", "Reload", lua_reload>(),
          {nullptr, nullptr},
      };
      lua_state::register_library("ModStack", ModStackLibrary);
    }
  }
};
} // namespace mod_stack

REGISTER_COMPONENT(mod_stack::component)
