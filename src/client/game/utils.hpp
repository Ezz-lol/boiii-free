#pragma once

#include "game.hpp"
#include <functional>
#include <type_traits>

namespace game {

[[nodiscard]] EngineDependentDvarMut get_dvar(const char *name);
[[nodiscard]] std::optional<std::string_view>
get_dvar_string(const char *dvar_name);
[[nodiscard]] std::optional<int32_t> get_dvar_int(const char *dvar_name);
[[nodiscard]] std::optional<uint32_t> get_dvar_uint(const char *dvar_name);
[[nodiscard]] std::optional<int64_t> get_dvar_int64(const char *dvar_name);
[[nodiscard]] std::optional<uint64_t> get_dvar_uint64(const char *dvar_name);
[[nodiscard]] std::optional<float> get_dvar_float(const char *dvar_name);
[[nodiscard]] std::optional<bool> get_dvar_bool(const char *dvar_name);

std::optional<int> set_dvar_int(const char *dvar_name, int32_t val,
                                DvarSetSource source = DvarSetSource::INTERNAL);
std::optional<int64_t>
set_dvar_int64(const char *dvar_name, int64_t val,
               DvarSetSource source = DvarSetSource::INTERNAL);
std::optional<uint64_t>
set_dvar_uint64(const char *dvar_name, uint64_t val,
                DvarSetSource source = DvarSetSource::INTERNAL);
bool set_dvar_bool(EngineDependentDvar dvar, bool val,
                   DvarSetSource source = DvarSetSource::INTERNAL);
std::optional<bool>
set_dvar_bool(const char *dvar_name, bool val,
              DvarSetSource source = DvarSetSource::INTERNAL);
std::optional<float>
set_dvar_float(const char *dvar_name, float val,
               DvarSetSource source = DvarSetSource::INTERNAL);
std::optional<std::string>
set_dvar_string(const char *dvar_name, const char *val,
                DvarSetSource source = DvarSetSource::INTERNAL);

void record_registered_dvar_name(const char *dvar_name);

template <DvarFlagLike T>
EngineDependentDvar
register_sessionmode_dvar_bool(const char *dvar_name, const bool value, T flags,
                               const char *description,
                               const eModes mode = eModes::COUNT) {
  const game::CanonHash_t hash = Dvar_GenerateHash(dvar_name);
  EngineDependentDvarMut registered_dvar = Dvar_SessionModeRegisterBool(
      hash, dvar_name, value, DvarFlags::from(flags), description);

  if (registered_dvar) {
    registered_dvar.debugName() = dvar_name;
    record_registered_dvar_name(dvar_name);

    if (mode == eModes::COUNT) {
      for (game::eModes i = eModes::FIRST; i < eModes::COUNT; i++) {
        Dvar_SessionModeSetDefaultBool.call_safe(hash, value, i);
      }
    } else {
      Dvar_SessionModeSetDefaultBool.call_safe(hash, value, mode);
    }
  }

  return registered_dvar;
}

template <DvarFlagLike T>
EngineDependentDvar register_dvar_bool(const char *dvar_name, const bool value,
                                       T flags, const char *description) {
  const game::CanonHash_t hash = Dvar_GenerateHash(dvar_name);
  EngineDependentDvarMut registered_dvar = Dvar_RegisterBool(
      hash, dvar_name, value, DvarFlags::from(flags), description);

  if (registered_dvar) {
    registered_dvar.debugName() = dvar_name;
    record_registered_dvar_name(dvar_name);
  }

  return registered_dvar;
}

template <DvarFlagLike T>
EngineDependentDvar register_dvar_int(const char *dvar_name, int32_t value,
                                      int32_t min, int32_t max, T flags,
                                      const char *description) {
  const game::CanonHash_t hash = Dvar_GenerateHash(dvar_name);
  EngineDependentDvarMut registered_dvar = Dvar_RegisterInt(
      hash, dvar_name, value, min, max, DvarFlags::from(flags), description);

  if (registered_dvar) {
    registered_dvar.debugName() = dvar_name;
    record_registered_dvar_name(dvar_name);
  }

  return registered_dvar;
}

template <DvarFlagLike T>
EngineDependentDvar register_dvar_float(const char *dvar_name, float value,
                                        float min, float max, T flags,
                                        const char *description) {
  const game::CanonHash_t hash = Dvar_GenerateHash(dvar_name);
  EngineDependentDvarMut registered_dvar = Dvar_RegisterFloat(
      hash, dvar_name, value, min, max, DvarFlags::from(flags), description);

  if (registered_dvar) {
    registered_dvar.debugName() = dvar_name;
    record_registered_dvar_name(dvar_name);
  }

  return registered_dvar;
}

template <DvarFlagLike T>
EngineDependentDvar register_dvar_string(const char *dvar_name,
                                         const char *value, T flags,
                                         const char *description) {
  const game::CanonHash_t hash = Dvar_GenerateHash(dvar_name);
  EngineDependentDvarMut registered_dvar = Dvar_RegisterString(
      hash, dvar_name, value, DvarFlags::from(flags), description);

  if (registered_dvar) {
    registered_dvar.debugName() = dvar_name;
    record_registered_dvar_name(dvar_name);
  }

  return registered_dvar;
}
template <DvarFlagLike T>
void dvar_add_flags(EngineDependentDvarMut dvar, const T flags) {
  if (!dvar) {
    return;
  }

  dvar.flags() |= flags;
}
template <DvarFlagLike T>
void dvar_add_flags(const char *dvar_name, const T flags) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);

  if (dvar) {
    dvar_add_flags(dvar, flags);
  }
}

template <DvarFlagLike T>
void dvar_set_flags(EngineDependentDvarMut dvar, const T flags) {
  if (!dvar) {
    return;
  }

  dvar.flags().set(flags);
}

template <DvarFlagLike T>
void dvar_set_flags(const char *dvar_name, const T flags) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);

  if (dvar) {
    dvar_set_flags(dvar, flags);
  }
}

template <DvarFlagLike T>
void dvar_remove_flags(EngineDependentDvarMut dvar, const T flags) {

  if (!dvar) {
    return;
  }

  dvar.flags() &= ~flags;
}

template <DvarFlagLike T>
void dvar_remove_flags(const char *dvar_name, const T flags) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);

  if (dvar) {
    dvar_remove_flags(dvar, flags);
  }
}

[[nodiscard]] inline std::optional<std::string_view> get_mapname() {
  return *mapname ? mapname->get_string() : std::nullopt;
}
[[nodiscard]] inline std::optional<std::string_view> get_g_gametype() {
  return *g_gametype ? g_gametype->get_string() : std::nullopt;
}
[[nodiscard]] inline std::optional<std::string_view> gametype() {
  return get_g_gametype();
}
[[nodiscard]] inline std::optional<std::string_view> get_g_password() {
  return *g_password ? g_password->get_string() : std::nullopt;
}
[[nodiscard]] inline std::optional<std::string_view> password() {
  return get_g_password();
}

[[nodiscard]] inline std::optional<std::string_view> get_sv_maprotation() {
  return *sv_maprotation ? sv_maprotation->get_string() : std::nullopt;
}
[[nodiscard]] inline std::optional<std::string_view> maprotation() {
  return get_sv_maprotation();
}
[[nodiscard]] inline std::optional<std::string_view>
get_sv_maprotationcurrent() {
  return *sv_maprotationcurrent ? sv_maprotationcurrent->get_string()
                                : std::nullopt;
}
[[nodiscard]] inline std::optional<std::string_view> maprotationcurrent() {
  return get_sv_maprotationcurrent();
}
[[nodiscard]] inline std::optional<std::string_view>
get_live_steam_server_name() {
  return *live_steam_server_name ? live_steam_server_name->get_string()
                                 : std::nullopt;
}
[[nodiscard]] inline std::optional<std::string_view> server_name() {
  return get_live_steam_server_name();
}
[[nodiscard]] inline std::optional<std::string_view> get_sv_hostname() {
  return *sv_hostname ? sv_hostname->get_string() : std::nullopt;
}
[[nodiscard]] inline std::optional<std::string_view> hostname() {
  if (is_server()) {
    return get_live_steam_server_name();
  }
  return get_sv_hostname();
}
[[nodiscard]] inline std::optional<std::string_view>
get_live_steam_server_description() {
  return *get_live_steam_server_description
             ? live_steam_server_description->get_string()
             : std::nullopt;
}
[[nodiscard]] inline std::optional<std::string_view> server_description() {
  return get_live_steam_server_description();
}
[[nodiscard]] inline std::optional<std::string_view>
get_live_steam_server_password() {
  return *get_live_steam_server_password
             ? live_steam_server_password->get_string()
             : std::nullopt;
}
[[nodiscard]] inline std::optional<std::string_view> server_password() {
  return get_live_steam_server_password();
}
[[nodiscard]] inline std::optional<std::string_view> get_rcon_password() {
  return *rcon_password ? rcon_password->get_string() : std::nullopt;
}
constexpr uint16_t DEFAULT_PORT = 3074;
[[nodiscard]] inline uint32_t get_net_port() {
  return *net_port ? net_port->get_uint() : DEFAULT_PORT;
}
[[nodiscard]] inline uint16_t port() {
  return static_cast<uint16_t>(get_net_port());
}
[[nodiscard]] inline std::optional<std::string_view> get_sv_wwwBaseURL() {
  return sv_wwwBaseURL ? sv_wwwBaseURL.get_string() : std::nullopt;
}
[[nodiscard]] inline std::optional<std::string_view> wwwBaseURL() {
  return get_sv_wwwBaseURL();
}
[[nodiscard]] inline std::optional<std::string_view> fastdl_uri() {
  return get_sv_wwwBaseURL();
}
[[nodiscard]] inline std::optional<bool> get_sv_wwwDownload() {
  return sv_wwwDownload ? std::optional(sv_wwwDownload.get_bool())
                        : std::nullopt;
}
[[nodiscard]] inline std::optional<bool> wwwDownload() {
  return get_sv_wwwDownload();
}
[[nodiscard]] inline std::optional<bool> fastdl_enabled() {
  return get_sv_wwwDownload();
}
[[nodiscard]] inline std::optional<bool> get_sv_wwwDlDisconnected() {
  return sv_wwwDlDisconnected ? std::optional(sv_wwwDlDisconnected.get_bool())
                              : std::nullopt;
}
[[nodiscard]] inline std::optional<bool> wwwDlDisconnected() {
  return get_sv_wwwDlDisconnected();
}
[[nodiscard]] inline std::optional<bool> fastdl_disconnected() {
  return get_sv_wwwDlDisconnected();
}
[[nodiscard]] inline bool get_sv_running() {
  return *com_sv_running ? com_sv_running->get_bool() : false;
}
[[nodiscard]] inline bool server_running() { return get_sv_running(); }
[[nodiscard]] inline ClientNum_t get_com_maxclients() {
  return *com_maxclients ? static_cast<ClientNum_t>(com_maxclients->get_int())
                         : lobby::MAX_PLAYERS;
}
[[nodiscard]] inline size_t get_max_client_count() {
  return static_cast<size_t>(get_com_maxclients());
}
[[nodiscard]] inline std::optional<std::string_view> get_workshop_id() {
  return workshop_id ? workshop_id.get_string() : std::nullopt;
}

void foreach_client(
    const std::function<void(sv::client_s &, size_t index)> &callback);
void foreach_client(const std::function<void(sv::client_s &)> &callback);
void first_client(
    const std::function<bool(sv::client_s &, size_t index)> &callback);
void first_client(const std::function<bool(sv::client_s &)> &callback);

void foreach_connected_client(
    const std::function<void(sv::client_s &, size_t index)> &callback);
void foreach_connected_client(
    const std::function<void(sv::client_s &)> &callback);
void first_connected_client(
    const std::function<bool(sv::client_s &, size_t index)> &callback);
void first_connected_client(
    const std::function<bool(sv::client_s &)> &callback);

bool access_connected_client(
    size_t index, const std::function<void(sv::client_s &)> &callback);

template <typename E>
concept EnumType = std::is_enum_v<E>;

template <EnumType T, const T MinValue, const T MaxValue>
inline constexpr bool valid(T enum_val) {
  using Underlying = std::underlying_type_t<T>;

  Underlying casted = static_cast<Underlying>(enum_val);
  return casted >= static_cast<Underlying>(MinValue) &&
         casted < static_cast<Underlying>(MaxValue);
}

inline constexpr bool valid_client_num(ClientNum_t clientNum) {
  return valid<ClientNum_t, CLIENT_INDEX_0, CLIENT_INDEX_COUNT>(clientNum);
}

inline constexpr bool
valid_controller_index(ControllerIndex_t controllerIndex) {
  return valid<ControllerIndex_t, CONTROLLER_INDEX_0, CONTROLLER_INDEX_COUNT>(
      controllerIndex);
}

inline constexpr bool valid_local_client_num(LocalClientNum_t localClientNum) {
  return valid<LocalClientNum_t, LOCAL_CLIENT_0, LOCAL_CLIENT_COUNT>(
      localClientNum);
}

inline bool valid_dvar(const EngineDependentDvar dvar) {
  const EngineDependentDvarPool pool = dvar_pool();
  return pool.contains(dvar);
}

namespace level {
template <typename T, typename = typename std::enable_if<std::is_convertible<
                          T, game::ClientNum_t>::value>::type>
inline level::gentity_t *client_ent(T index) {
  level::gentity_pool *pool = get_g_entities();
  ClientNum_t clientNum = static_cast<game::ClientNum_t>(index);
  if (valid_client_num(clientNum)) {
    return &pool->pool[clientNum];
  }

  return nullptr;
}
} // namespace level

namespace scr {
inline constexpr bool valid_scrvar_index(scr::scriptInstance_t inst,
                                         scr::ScrVarIndex_t index) {
  return index < scr::SCRIPTVARIABLE_POOL_SIZE.instance[inst];
}

inline scr::ScrVarIndex_t scrvar_index(scr::scriptInstance_t inst,
                                       scr::ScrVar_t *var) {
  uintptr_t scriptVariablesPtr = reinterpret_cast<uintptr_t>(
      scr::gScrVarGlob->instance[inst].scriptVariables);
  uintptr_t varPtr = reinterpret_cast<uintptr_t>(var);
  return static_cast<scr::ScrVarIndex_t>((varPtr - scriptVariablesPtr) /
                                         sizeof(scr::ScrVar_t));
}

inline bool valid_scrvar_ptr(scr::scriptInstance_t inst, scr::ScrVar_t *var) {
  return valid_engine_ptr(var) // Static or stack allocation
         ||
         valid_scrvar_index(inst, scrvar_index(inst, var)); // Pool allocation
}

inline scr::ScrVarIndex_t scrvarvalue_index(scr::scriptInstance_t inst,
                                            scr::ScrVarValue_t *val) {
  uintptr_t valPtr = reinterpret_cast<uintptr_t>(val);
  scr::ScrVar_t *varPtr = reinterpret_cast<scr::ScrVar_t *>(
      valPtr - offsetof(scr::ScrVar_t, value) /* 0 */);
  return scrvar_index(inst, varPtr);
}

inline bool valid_scrvarvalue_ptr(scr::scriptInstance_t inst,
                                  scr::ScrVarValue_t *val) {
  return valid_engine_ptr(val) // Static or stack allocation
         || valid_scrvar_index(inst,
                               scrvarvalue_index(inst, val)); // Pool allocation
}
} // namespace scr
} // namespace game