#pragma once

#include "game.hpp"
#include <functional>
#include <string>
#include <type_traits>

namespace game {
[[nodiscard]] dvar_t *get_dvar(const char *name);
[[nodiscard]] std::string_view get_dvar_string(const dvar_t *dvar);
[[nodiscard]] std::string_view get_dvar_string(const char *dvar_name);
[[nodiscard]] int32_t get_dvar_int(const dvar_t *dvar);
[[nodiscard]] int32_t get_dvar_int(const char *dvar_name);
[[nodiscard]] uint32_t get_dvar_uint(const dvar_t *dvar);
[[nodiscard]] uint32_t get_dvar_uint(const char *dvar_name);
[[nodiscard]] int64_t get_dvar_int64(const dvar_t *dvar);
[[nodiscard]] int64_t get_dvar_int64(const char *dvar_name);
[[nodiscard]] uint64_t get_dvar_uint64(const dvar_t *dvar);
[[nodiscard]] uint64_t get_dvar_uint64(const char *dvar_name);
[[nodiscard]] float get_dvar_float(const dvar_t *dvar);
[[nodiscard]] float get_dvar_float(const char *dvar_name);
[[nodiscard]] bool get_dvar_bool(const dvar_t *dvar);
[[nodiscard]] bool get_dvar_bool(const char *dvar_name);

int set_dvar_int(const dvar_t *dvar, int32_t val,
                 DvarSetSource source = DvarSetSource::INTERNAL);
int set_dvar_int(const char *dvar_name, int32_t val,
                 DvarSetSource source = DvarSetSource::INTERNAL);
int64_t set_dvar_int64(const dvar_t *dvar, int64_t val,
                       DvarSetSource source = DvarSetSource::INTERNAL);
int64_t set_dvar_int64(const char *dvar_name, int64_t val,
                       DvarSetSource source = DvarSetSource::INTERNAL);
uint64_t set_dvar_uint64(const dvar_t *dvar, uint64_t val,
                         DvarSetSource source = DvarSetSource::INTERNAL);
uint64_t set_dvar_uint64(const char *dvar_name, uint64_t val,
                         DvarSetSource source = DvarSetSource::INTERNAL);
bool set_dvar_bool(const dvar_t *dvar, bool val,
                   DvarSetSource source = DvarSetSource::INTERNAL);
bool set_dvar_bool(const char *dvar_name, bool val,
                   DvarSetSource source = DvarSetSource::INTERNAL);
float set_dvar_float(const dvar_t *dvar, float val,
                     DvarSetSource source = DvarSetSource::INTERNAL);
float set_dvar_float(const char *dvar_name, float val,
                     DvarSetSource source = DvarSetSource::INTERNAL);

[[nodiscard]] const dvar_t *register_dvar_bool(const char *dvar_name,
                                               bool value, uint32_t flags,
                                               const char *description);
[[nodiscard]] const dvar_t *register_dvar_int(const char *dvar_name,
                                              int32_t value, int32_t min,
                                              int32_t max, uint32_t flags,
                                              const char *description);
[[nodiscard]] const dvar_t *register_dvar_float(const char *dvar_name,
                                                float value, float min,
                                                float max, uint32_t flags,
                                                const char *description);
[[nodiscard]] const dvar_t *
register_sessionmode_dvar_bool(const char *dvar_name, bool value,
                               uint32_t flags, const char *description,
                               eModes mode = eModes::COUNT);
[[nodiscard]] const dvar_t *register_dvar_string(const char *dvar_name,
                                                 const char *value,
                                                 uint32_t flags,
                                                 const char *description);

void dvar_add_flags(const char *dvar_name, uint32_t flags);
void dvar_set_flags(const char *dvar_name, uint32_t flags);
void dvar_remove_flags(const char *dvar_name, uint32_t flags);

[[nodiscard]] bool is_server_running();
[[nodiscard]] size_t get_max_client_count();

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