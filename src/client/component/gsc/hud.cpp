#include <std_include.hpp>

#include <game/impl/scr/scr.hpp>
#include <game/impl/scr/var.hpp>
#include <game/impl/sv/sv.hpp>
#include <game/utils.hpp>

#include <component/command.hpp>
#include <component/game_event.hpp>
#include <component/name.hpp>
#include <component/toast.hpp>
#include <loader/component_loader.hpp>

#include <utils/hook.hpp>
#include <utils/io.hpp>

#include <rapidjson/writer.h>

using namespace game;
using namespace game::scr;

namespace gsc {
namespace hud {
using namespace game::scr;
using namespace game::scr::builtin;

/*
  Ensure HECmd_SetText reuses config strings where possible, rather than
  registering a new config string each time `SetText` is called,
  quickly overflowing registered config string limit.
*/

// Generating BG_Cache name entry table at compile time
namespace {
using namespace game::bg::cache;
using namespace game::ui::he;
using namespace game::scr::he;
constexpr char hudelem_cfgstr_pool_entry_name_prefix[] =
    "__HUDELEM_CFGSTR_POOL_ENTRY_";
constexpr uint8_t hudelem_cfgstr_pool_entry_name_number_max_suffix_len =
    sizeof(uint16_t) * 2 /* characters per byte */;
constexpr uint8_t hudelem_cfgstr_pool_entry_name_len =
    std::size(hudelem_cfgstr_pool_entry_name_prefix) +
    hudelem_cfgstr_pool_entry_name_number_max_suffix_len;
typedef str<hudelem_cfgstr_pool_entry_name_len> HudElemCfgStrPoolEntryName;
consteval ui::he::HudElementPool<HudElemCfgStrPoolEntryName>
build_hudelem_cfgstr_name_pool(
    ui::he::HudElementPool<HudElemCfgStrPoolEntryName> pool = {}) {
  for (uint16_t i = 0; i < pool.size(); ++i) {
    append_hex<std::size(hudelem_cfgstr_pool_entry_name_prefix), uint16_t>(
        hudelem_cfgstr_pool_entry_name_prefix, i, pool.pool[i]);
  }
  return pool;
}
/*
   Note: 0xE2D is the config string count limit in the engine, so <=1 config
   string per hud element should be fine, given loaded mod and map do not
   collectively register a truly degenerate number of config strings.
*/
static constexpr ui::he::HudElementPool<HudElemCfgStrPoolEntryName>
    HUDELEM_CFGSTR_POOL_ENTRY_NAMES = build_hudelem_cfgstr_name_pool({});

typedef str<ui::he::MAX_HUDELEM_MESSAGE_LEN> HudElemMessage;
struct RegisteredCfgString {
  atomic_optional<int32_t> idx;

  static constexpr BGCacheTypes CACHE_TYPE = BGCacheTypes::LOCSTRING;

  inline void clear() volatile noexcept {
    idx.store(std::nullopt, std::memory_order_release);
  }

  inline void set(int32_t loc_cfgstr_idx) volatile {
    idx.store(loc_cfgstr_idx, std::memory_order_release);
  }

  inline bool has_value() volatile {
    return idx.load(std::memory_order_acquire).has_value();
  }

  // Caller needs to have checked if idx has value prior to call
  inline int32_t get_idx() volatile {
    return idx.load(std::memory_order_acquire).value();
  }

  /*
     Gets config string index relative to complete config string pool - not
     just the subsection allocated for localized strings.

     Safety: Caller needs to have checked if `idx` has value prior to call
  */
  inline int32_t abs_idx() volatile {
    return s_bgCacheTypeInfo->get(CACHE_TYPE).configStringStart + get_idx();
  }
};
struct HudElemCfgStringPool : ui::he::HudElementPool<RegisteredCfgString> {
  inline bool contains(int32_t loc_cfgstr_idx) volatile {
    for (size_t i = 0; i < SIZE; ++i) {
      if (this->pool[i].has_value() &&
          this->pool[i].get_idx() == loc_cfgstr_idx) {
        return true;
      }
    }
    return false;
  }
};

static HudElemCfgStringPool hudelem_cfgstr_pool = {};

void unregister_clear_hudelem_cfgstr(uint16_t hudElemIdx) {
  volatile RegisteredCfgString *entry = &hudelem_cfgstr_pool[hudElemIdx];
  if (entry->has_value()) {
    ui::he::g_hudelems->get(hudElemIdx).elem.text = 0;
    // TAC-protected on client, so we use a re-implementation to circumvent.
    sv::SV_SetConfigString_Impl(entry->abs_idx(), "");

    s_bgCache->server.dataSet.localizedStrings[entry->get_idx()].reset();

    hudelem_cfgstr_pool[hudElemIdx].clear();
  }
}

namespace hecmd_settext {
static thread_local HudElemMessage message_buf = {0};
static thread_local HudElemMessage cleaned_message_buf = {0};
inline void clear_message_bufs() {
  memset(message_buf, 0, std::size(message_buf));
  memset(cleaned_message_buf, 0, std::size(cleaned_message_buf));
}

inline void ensure_message_bufs_clear() {
  if (message_buf[0]) {
    memset(message_buf, 0, std::size(message_buf));
  }

  if (cleaned_message_buf[0]) {
    memset(cleaned_message_buf, 0, std::size(cleaned_message_buf));
  }
}

void HECmd_SetText_ReuseCfgString(scriptInstance_t inst, scr_entref_t *entref) {
  if (entref->is_hudelem()) [[likely]] {
    ensure_message_bufs_clear();
    const uint16_t hudElemIdx = entref->u.hudElemIndex;
    volatile game_hudelem_t *elem = &g_hudelems->get(hudElemIdx);

    elem->reset_value();
    const uint32_t argc = Scr_GetNumParam(inst);
    Scr_ConstructMessageString(0, argc - 1, "Hud Elem String", message_buf,
                               std::size(message_buf));
    com::Com_CleanStringForNetwork(message_buf, cleaned_message_buf,
                                   std::size(cleaned_message_buf));

    // Assign to underlying directly to allow copy-assignment to volatile.
    // Volatile copy-assign is not provided by default and definition breaks
    // C++-03 PoD constraints :(.
    elem->elem.type.underlying = he_type_field_t::TEXT;

    volatile RegisteredCfgString *pool_entry = &hudelem_cfgstr_pool[hudElemIdx];

    const bgCacheInstance cache_inst = static_cast<bgCacheInstance>(inst);
    const int32_t localized_cfgstring_index =
        BG_Cache_GetLocStringIndex(cache_inst, cleaned_message_buf);
    if (localized_cfgstring_index > 0 &&
        !hudelem_cfgstr_pool.contains(localized_cfgstring_index)) {
      elem->elem.text = localized_cfgstring_index;
    }
    // Not a localized string. Need to register and/or modify the config string
    // value.
    else if (get_sv_running()) {
      if (!pool_entry->has_value()) {
        // Register new config string
        pool_entry->set(
            s_bgCacheTypeInfo->get(BGCacheTypes::LOCSTRING)
                .registerFunc(cache_inst,
                              HUDELEM_CFGSTR_POOL_ENTRY_NAMES[hudElemIdx]));
#ifndef NDEBUG
        trace("[Scr][HECmd_SetText] Registered localized string "
              "configstring for "
              "hudelement 0x{:03X} with "
              "index 0x{:X}",
              hudElemIdx, pool_entry->get_idx());
#endif
      }

      volatile bgCachedGenericData *data =
          &s_bgCache->server.dataSet.localizedStrings[pool_entry->get_idx()];

#ifndef NDEBUG
      trace("[Scr][HECmd_SetText] Localized config string entry with "
            "index 0x{:X}, "
            "absolute config string index 0x{:X}: got localized string data "
            "pointer: {:p}",
            pool_entry->get_idx(), pool_entry->abs_idx(),
            const_cast<void *>(
                static_cast<volatile void *>(game::derelocate(data))));
#endif
      data->setName(cleaned_message_buf);
      if (!data->refCount) {
        data->add_ref();
      }
      s_bgCache->get(cache_inst).checksum.isDirty = true;

#ifndef NDEBUG
      trace("[Scr][HECmd_SetText] Localized config string entry with "
            "index 0x{:X}, "
            "absolute config string index 0x{:X}: setting value to \"{}\"",
            pool_entry->get_idx(), pool_entry->abs_idx(), cleaned_message_buf);
#endif
      // TAC-protected on client, so we use a re-implementation to circumvent.
      sv::SV_SetConfigString_Impl(pool_entry->abs_idx(), cleaned_message_buf);
      elem->elem.text = pool_entry->get_idx();
    }
    clear_message_bufs();
  } else [[unlikely]] {
    Scr_ObjectError(inst, "not a hud element");
  }
}
} // namespace hecmd_settext

void forget_hudelem_cfgstr_pool() {
  for (uint16_t slot = 0; slot < ui::he::HUD_ELEMENT_POOL_SIZE; ++slot) {
    hudelem_cfgstr_pool[slot].clear();
  }
}

utils::hook::detour HudElem_DestroyAll_hook;

void HudElem_DestroyAll_ClearCfgStrEntry_Invoke() {
  forget_hudelem_cfgstr_pool();
  return HudElem_DestroyAll_hook.invoke();
}

utils::hook::detour BG_Cache_HandleConfigStringChange_hook;
void BG_Cache_HandleConfigStringChange_ReuseExisting(
    [[maybe_unused]] LocalClientNum_t localClientNum, int32_t index) {
  const char *name = cl::CL_GetConfigString(index);
#ifndef NDEBUG
  trace("[BG][Cache][{}][{}] Received config string change with index: 0x{:X}, "
        "name: \"{}\"",
        +bgCacheInstance::CLIENT, +localClientNum, index,
        readable_ptr(name) ? name : "");
#endif
  const bool is_localized_string =
      index >= s_bgCacheTypeInfo->locstring.configStringStart &&
      index < s_bgCacheTypeInfo->locstring.configStringStart +
                  static_cast<int32_t>(
                      std::size(s_bgCache->client.dataSet.localizedStrings));

  if (is_localized_string) {
    volatile bgCachedGenericData *data =
        &s_bgCache->client.dataSet
             .localizedStrings[index -
                               s_bgCacheTypeInfo->locstring.configStringStart];
    data->setName(name);
    if (!data->refCount) {
      data->add_ref();
    }
    s_bgCache->client.checksum.isDirty = true;
  } else {
    BG_Cache_HandleConfigStringChange_hook.invoke(localClientNum, index);
  }
}

template <const BuiltinMethod &hooked>
void HECmd_ClearInvoke(scriptInstance_t inst, scr_entref_t *entref) {
  if (entref->is_hudelem() && sv::sv->running()) {
    unregister_clear_hudelem_cfgstr(entref->u.hudElemIndex);
  }
  hooked(inst, entref);
}

// HECmd script VM method hooks
inline void apply_hecmd_hooks() {
  game::scr::builtin::table::hudElem_methods->SetText.actionFunc =
      hecmd_settext::HECmd_SetText_ReuseCfgString;

#ifndef HOOK_CLEAR_HECMD
#define HOOK_CLEAR_HECMD(name)                                                 \
  static const BuiltinMethod HECmd_##name##_Orig =                             \
      game::scr::builtin::table::hudElem_methods->name.actionFunc;             \
  game::scr::builtin::table::hudElem_methods->name.actionFunc =                \
      HECmd_ClearInvoke<HECmd_##name##_Orig>;
#endif

  HOOK_CLEAR_HECMD(SetValue);
  HOOK_CLEAR_HECMD(SetTimerUp);
  HOOK_CLEAR_HECMD(SetTimer);
  HOOK_CLEAR_HECMD(SetTenthsTimerUp);
  HOOK_CLEAR_HECMD(SetTenthsTimer);
  HOOK_CLEAR_HECMD(SetShader);
  HOOK_CLEAR_HECMD(SetPlayerNameString);
  HOOK_CLEAR_HECMD(SetMapNameString);
  HOOK_CLEAR_HECMD(SetGameTypeString);
  HOOK_CLEAR_HECMD(SetClockUp);
  HOOK_CLEAR_HECMD(SetClock);
  HOOK_CLEAR_HECMD(Reset);
  HOOK_CLEAR_HECMD(Destroy);
}

inline void apply_hudelem_hooks() {

  HudElem_DestroyAll_hook.create(game::ui::he::HudElem_DestroyAll,
                                 HudElem_DestroyAll_ClearCfgStrEntry_Invoke);

  // Client-side: don't unnecessarily re-register an entry on value change if
  // the given index was previously registered
  BG_Cache_HandleConfigStringChange_hook.create(
      game::bg::cache::BG_Cache_HandleConfigStringChange,
      BG_Cache_HandleConfigStringChange_ReuseExisting);

  apply_hecmd_hooks();
}
} // namespace

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("gsc/hud");

  void post_unpack() override {

    apply_hudelem_hooks();

    game_event::on_g_shutdown_game(forget_hudelem_cfgstr_pool);

    game_event::on_g_init_game(forget_hudelem_cfgstr_pool);
  }
};
} // namespace hud
} // namespace gsc

REGISTER_COMPONENT(gsc::hud::component)
