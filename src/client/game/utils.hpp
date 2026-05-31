#pragma once

#include "structs/structs.hpp"
#include <functional>
#include <string>
#include <type_traits>

namespace game {
[[nodiscard]] std::string get_dvar_string(const char *dvar_name);
[[nodiscard]] int get_dvar_int(const char *dvar_name);
[[nodiscard]] uint32_t get_dvar_uint(const char *dvar_name);
[[nodiscard]] int64_t get_dvar_int64(const char *dvar_name);
[[nodiscard]] uint64_t get_dvar_uint64(const char *dvar_name);
[[nodiscard]] float get_dvar_float(const char *dvar_name);
[[nodiscard]] bool get_dvar_bool(const char *dvar_name);
int set_dvar_int(const char *dvar_name, int val,
                 DvarSetSource source = DvarSetSource::INTERNAL);
int64_t set_dvar_int64(const char *dvar_name, int64_t val,
                       DvarSetSource source = DvarSetSource::INTERNAL);
uint64_t set_dvar_uint64(const char *dvar_name, uint64_t val,
                         DvarSetSource source = DvarSetSource::INTERNAL);
bool set_dvar_bool(const char *dvar_name, bool val,
                   DvarSetSource source = DvarSetSource::INTERNAL);
float set_dvar_float(const char *dvar_name, float val,
                     DvarSetSource source = DvarSetSource::INTERNAL);

[[nodiscard]] const dvar_t *register_dvar_bool(const char *dvar_name,
                                               bool value, unsigned int flags,
                                               const char *description);
[[nodiscard]] const dvar_t *register_dvar_int(const char *dvar_name, int value,
                                              int min, int max,
                                              unsigned int flags,
                                              const char *description);
[[nodiscard]] const dvar_t *register_dvar_float(const char *dvar_name,
                                                float value, float min,
                                                float max, unsigned int flags,
                                                const char *description);
[[nodiscard]] const dvar_t *
register_sessionmode_dvar_bool(const char *dvar_name, bool value,
                               unsigned int flags, const char *description,
                               eModes mode = eModes::COUNT);
[[nodiscard]] const dvar_t *register_dvar_string(const char *dvar_name,
                                                 const char *value,
                                                 unsigned int flags,
                                                 const char *description);

void dvar_add_flags(const char *dvar, unsigned int flags);
void dvar_set_flags(const char *dvar_name, unsigned int flags);
void dvar_remove_flags(const char *dvar_name, unsigned int flags);

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

level::gentity_pool *gentity_pool();

template <typename T, typename = typename std::enable_if<std::is_convertible<
                          T, game::ClientNum_t>::value>::type>
inline level::gentity_t *client_ent(T index) {
  level::gentity_pool *pool = gentity_pool();
  return &pool->pool[static_cast<game::ClientNum_t>(index)];
}

template <typename T, typename = typename std::enable_if<
                          std::is_convertible<T, uint32_t>::value>::type>
inline level::gentity_t *entity(T input_index) {
  uint32_t index = static_cast<uint32_t>(input_index);
  if (index < level::GENTITY_POOL_COUNT) {
    level::gentity_pool *pool = gentity_pool();
    if (pool) {
      return &pool->pool[index];
    }
  }
  return nullptr;
}

template <typename E>
concept EnumType = std::is_enum_v<E>;

template <EnumType T, const T MinValue, const T MaxValue>
inline constexpr bool valid(T enum_val) {
  uint32_t casted = static_cast<uint32_t>(enum_val);
  return casted >= static_cast<uint32_t>(MinValue) &&
         casted < static_cast<uint32_t>(MaxValue);
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

} // namespace game