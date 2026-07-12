#pragma once

#include "game.hpp"
#include <functional>
#include <type_traits>

namespace game {

template <typename T> inline EngineDependentDvarMut dvar_variant(T *dvar) {
  if (game::is_server()) {
    return reinterpret_cast<dvar_t *>(dvar);
  }

  return reinterpret_cast<dvar_t_cl *>(dvar);
}
template <typename T> inline EngineDependentDvar dvar_variant(const T *dvar) {
  if (game::is_server()) {
    return reinterpret_cast<const dvar_t *>(dvar);
  }

  return reinterpret_cast<const dvar_t_cl *>(dvar);
}

template <typename T> inline EngineDependentDvarMut dvar_variant_mut(T *dvar) {
  if (game::is_server()) {
    return reinterpret_cast<dvar_t *>(dvar);
  }

  return reinterpret_cast<dvar_t_cl *>(dvar);
}

template <typename Client, typename Server>
inline bool engine_dependent_nonnull(EngineDependent<Client *, Server *> v) {
  return std::visit([](auto *resolved) -> bool { return resolved != nullptr; },
                    v);
}

template <typename Client, typename Server>
inline bool
engine_dependent_nonnull(EngineDependent<const Client *, const Server *> v) {
  return std::visit([](auto *resolved) -> bool { return resolved != nullptr; },
                    v);
}

template <typename Client, typename Server>
inline EngineDependent<const Client *, const Server *>
engine_dependent_toggle_const(EngineDependent<Client *, Server *> v) {
  return std::visit(
      [](auto *resolved) -> EngineDependent<const Client *, const Server *> {
        // Isolate the base type (Client or Server) and add const to it
        using Pointee = std::remove_pointer_t<decltype(resolved)>;
        return const_cast<const Pointee *>(resolved);
      },
      v);
}

template <typename Client, typename Server>
inline EngineDependent<Client *, Server *> engine_dependent_toggle_const(
    EngineDependent<const Client *, const Server *> v) {
  return std::visit(
      [](auto *resolved) -> EngineDependent<Client *, Server *> {
        // Isolate the base type (const Client or const Server) and remove const
        using Pointee = std::remove_pointer_t<decltype(resolved)>;
        return const_cast<std::remove_const_t<Pointee> *>(resolved);
      },
      v);
}

// In case of casting resolved type, which cannot be used or defined easily at
// point of cast
template <typename T> inline const T *engine_dependent_toggle_const(T *v) {
  return const_cast<const T *>(v);
}

template <typename T> inline T *engine_dependent_toggle_const(const T *v) {
  return const_cast<T *>(v);
}

[[nodiscard]] EngineDependentDvarMut get_dvar(const char *name);
[[nodiscard]] std::optional<std::string_view>
get_dvar_string(EngineDependentDvar dvar);
[[nodiscard]] std::optional<std::string_view>
get_dvar_string(const char *dvar_name);
[[nodiscard]] int32_t get_dvar_int(EngineDependentDvar dvar);
[[nodiscard]] std::optional<int32_t> get_dvar_int(const char *dvar_name);
[[nodiscard]] uint32_t get_dvar_uint(EngineDependentDvar dvar);
[[nodiscard]] std::optional<uint32_t> get_dvar_uint(const char *dvar_name);
[[nodiscard]] int64_t get_dvar_int64(EngineDependentDvar dvar);
[[nodiscard]] std::optional<int64_t> get_dvar_int64(const char *dvar_name);
[[nodiscard]] uint64_t get_dvar_uint64(EngineDependentDvar dvar);
[[nodiscard]] std::optional<uint64_t> get_dvar_uint64(const char *dvar_name);
[[nodiscard]] float get_dvar_float(EngineDependentDvar dvar);
[[nodiscard]] std::optional<float> get_dvar_float(const char *dvar_name);
[[nodiscard]] bool get_dvar_bool(EngineDependentDvar dvar);
[[nodiscard]] std::optional<bool> get_dvar_bool(const char *dvar_name);

int set_dvar_int(EngineDependentDvar dvar, int32_t val,
                 DvarSetSource source = DvarSetSource::INTERNAL);
std::optional<int> set_dvar_int(const char *dvar_name, int32_t val,
                                DvarSetSource source = DvarSetSource::INTERNAL);
int64_t set_dvar_int64(EngineDependentDvar dvar, int64_t val,
                       DvarSetSource source = DvarSetSource::INTERNAL);
std::optional<int64_t>
set_dvar_int64(const char *dvar_name, int64_t val,
               DvarSetSource source = DvarSetSource::INTERNAL);
uint64_t set_dvar_uint64(EngineDependentDvar dvar, uint64_t val,
                         DvarSetSource source = DvarSetSource::INTERNAL);
std::optional<uint64_t>
set_dvar_uint64(const char *dvar_name, uint64_t val,
                DvarSetSource source = DvarSetSource::INTERNAL);
bool set_dvar_bool(EngineDependentDvar dvar, bool val,
                   DvarSetSource source = DvarSetSource::INTERNAL);
std::optional<bool>
set_dvar_bool(const char *dvar_name, bool val,
              DvarSetSource source = DvarSetSource::INTERNAL);
float set_dvar_float(EngineDependentDvar dvar, float val,
                     DvarSetSource source = DvarSetSource::INTERNAL);
std::optional<float>
set_dvar_float(const char *dvar_name, float val,
               DvarSetSource source = DvarSetSource::INTERNAL);
std::optional<std::string>
set_dvar_string(EngineDependentDvar dvar, const char *val,
                DvarSetSource source = DvarSetSource::INTERNAL);
std::optional<std::string>
set_dvar_string(const char *dvar_name, const char *val,
                DvarSetSource source = DvarSetSource::INTERNAL);

[[nodiscard]] EngineDependentDvar register_dvar_bool(const char *dvar_name,
                                                     bool value, uint32_t flags,
                                                     const char *description);
[[nodiscard]] EngineDependentDvar register_dvar_int(const char *dvar_name,
                                                    int32_t value, int32_t min,
                                                    int32_t max, uint32_t flags,
                                                    const char *description);
[[nodiscard]] EngineDependentDvar register_dvar_float(const char *dvar_name,
                                                      float value, float min,
                                                      float max, uint32_t flags,
                                                      const char *description);
[[nodiscard]] EngineDependentDvar
register_sessionmode_dvar_bool(const char *dvar_name, bool value,
                               uint32_t flags, const char *description,
                               eModes mode = eModes::COUNT);
[[nodiscard]] EngineDependentDvar register_dvar_string(const char *dvar_name,
                                                       const char *value,
                                                       uint32_t flags,
                                                       const char *description);
void dvar_add_flags(EngineDependentDvarMut dvar, uint32_t flags);
void dvar_set_flags(EngineDependentDvarMut dvar, uint32_t flags);
void dvar_remove_flags(EngineDependentDvarMut dvar, uint32_t flags);

void dvar_add_flags(const char *dvar_name, uint32_t flags);
void dvar_set_flags(const char *dvar_name, uint32_t flags);
void dvar_remove_flags(const char *dvar_name, uint32_t flags);

[[nodiscard]] inline std::optional<std::string_view> get_mapname() {
  return get_dvar_string(mapname());
}
[[nodiscard]] inline std::optional<std::string_view> get_g_gametype() {
  return get_dvar_string(g_gametype());
}
[[nodiscard]] inline std::optional<std::string_view> gametype() {
  return get_dvar_string(g_gametype());
}
[[nodiscard]] inline std::optional<std::string_view> get_g_password() {
  return get_dvar_string(g_password());
}
[[nodiscard]] inline std::optional<std::string_view> password() {
  return get_dvar_string(g_password());
}

[[nodiscard]] inline std::optional<std::string_view> get_sv_maprotation() {
  return get_dvar_string(sv_maprotation());
}
[[nodiscard]] inline std::optional<std::string_view> maprotation() {
  return get_dvar_string(sv_maprotation());
}
[[nodiscard]] inline std::optional<std::string_view>
get_sv_maprotationcurrent() {
  return get_dvar_string(sv_maprotationcurrent());
}
[[nodiscard]] inline std::optional<std::string_view> maprotationcurrent() {
  return get_dvar_string(sv_maprotationcurrent());
}
[[nodiscard]] inline std::optional<std::string_view>
get_live_steam_server_name() {
  return get_dvar_string(live_steam_server_name());
}
[[nodiscard]] inline std::optional<std::string_view> server_name() {
  return get_live_steam_server_name();
}
[[nodiscard]] inline std::optional<std::string_view> get_sv_hostname() {
  return get_dvar_string(sv_hostname());
}
[[nodiscard]] inline std::optional<std::string_view> hostname() {
  if (is_server()) {
    return get_live_steam_server_name();
  }
  return get_sv_hostname();
}
[[nodiscard]] inline std::optional<std::string_view>
get_live_steam_server_description() {
  return get_dvar_string(live_steam_server_description());
}
[[nodiscard]] inline std::optional<std::string_view> server_description() {
  return get_live_steam_server_description();
}
[[nodiscard]] inline std::optional<std::string_view>
get_live_steam_server_password() {
  return get_dvar_string(live_steam_server_password());
}
[[nodiscard]] inline std::optional<std::string_view> server_password() {
  return get_live_steam_server_password();
}
[[nodiscard]] inline std::optional<std::string_view> get_rcon_password() {
  return get_dvar_string(rcon_password());
}
[[nodiscard]] inline uint32_t get_net_port() {
  return get_dvar_uint(net_port());
}
[[nodiscard]] inline uint16_t port() {
  return static_cast<uint16_t>(get_net_port());
}
[[nodiscard]] inline std::optional<std::string_view> get_sv_wwwBaseURL() {
  if (engine_dependent_nonnull(sv_wwwBaseURL)) {
    return get_dvar_string(sv_wwwBaseURL);
  }
  return std::nullopt;
}
[[nodiscard]] inline std::optional<std::string_view> wwwBaseURL() {
  return get_sv_wwwBaseURL();
}
[[nodiscard]] inline std::optional<std::string_view> fastdl_uri() {
  return get_sv_wwwBaseURL();
}
[[nodiscard]] inline std::optional<bool> get_sv_wwwDownload() {
  if (engine_dependent_nonnull(sv_wwwDownload)) {
    return get_dvar_bool(sv_wwwDownload);
  }
  return std::nullopt;
}
[[nodiscard]] inline std::optional<bool> wwwDownload() {
  return get_sv_wwwDownload();
}
[[nodiscard]] inline std::optional<bool> fastdl_enabled() {
  return get_sv_wwwDownload();
}
[[nodiscard]] inline std::optional<bool> get_sv_wwwDlDisconnected() {
  if (engine_dependent_nonnull(sv_wwwDlDisconnected)) {
    return get_dvar_bool(sv_wwwDlDisconnected);
  }
  return std::nullopt;
}
[[nodiscard]] inline std::optional<bool> wwwDlDisconnected() {
  return get_sv_wwwDlDisconnected();
}
[[nodiscard]] inline std::optional<bool> fastdl_disconnected() {
  return get_sv_wwwDlDisconnected();
}
[[nodiscard]] inline bool get_sv_running() {
  return get_dvar_bool(com_sv_running());
}
[[nodiscard]] inline bool server_running() { return get_sv_running(); }
[[nodiscard]] inline int32_t get_com_maxclients() {
  return get_dvar_int(com_maxclients());
}
[[nodiscard]] inline size_t get_max_client_count() {
  return static_cast<size_t>(get_com_maxclients());
}
[[nodiscard]] inline std::optional<std::string_view> get_workshop_id() {
  if (engine_dependent_nonnull(workshop_id)) {
    return get_dvar_string(workshop_id);
  }
  return std::nullopt;
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

level::gentity_pool *gentity_pool();

template <typename T, typename = typename std::enable_if<
                          std::is_convertible<T, uint32_t>::value>::type>
inline level::gentity_t *entity(T input_index) {
  uint32_t index = static_cast<uint32_t>(input_index);
  if (index < level::GENTITY_POOL_LEN) {
    level::gentity_pool *pool = gentity_pool();
    if (pool) {
      return &pool->pool[index];
    }
  }
  return nullptr;
}

template <typename T, typename = typename std::enable_if<std::is_convertible<
                          T, game::ClientNum_t>::value>::type>
inline level::gentity_t *client_ent(T index) {
  level::gentity_pool *pool = gentity_pool();
  ClientNum_t clientNum = static_cast<game::ClientNum_t>(index);
  if (valid_client_num(clientNum)) {
    return &pool->pool[clientNum];
  }

  return nullptr;
}
} // namespace game