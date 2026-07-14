#include <std_include.hpp>

#include "utils.hpp"
#include <utils/concurrency.hpp>

using namespace game::com;
using namespace game::sv;

namespace game {

namespace {
utils::concurrency::container<std::vector<std::string>>
    registered_dvar_names_{};
std::atomic_size_t registered_dvar_name_count_{0};
} // namespace

void record_registered_dvar_name(const char *dvar_name) {
  if (!dvar_name || !dvar_name[0]) {
    return;
  }
  registered_dvar_names_.access([dvar_name](std::vector<std::string> &names) {
    names.emplace_back(dvar_name);
  });
  registered_dvar_name_count_.fetch_add(1, std::memory_order_seq_cst);
} // namespace

EngineDependentDvarMut
try_get_sessionmode_specific_dvar(EngineDependentDvarMut dvar) {

  if (dvar || dvar.type() != dvarType_t::SESSIONMODE_BASE_DVAR) {
    return dvar;
  }

  if (Com_SessionMode_IsMode(eModes::COUNT)) {
    return nullptr;
  }

  const game::eModes mode = Com_SessionMode_GetMode();
  return Dvar_GetSessionModeSpecificDvar(dvar.sv, static_cast<eModes>(mode));
}

__inline_def EngineDependentDvarMut get_dvar(const char *name) {
  return Dvar_FindVar(name);
}

__inline_def std::optional<std::string_view>
get_dvar_string(EngineDependentDvar dvar) {
  return dvar.get_string();
}

std::optional<std::string_view> get_dvar_string(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);

  if (dvar) {
    return dvar.get_string();
  }

  return std::nullopt;
}

__inline_def int32_t get_dvar_int(EngineDependentDvar dvar) {
  return dvar.get_int();
}

std::optional<int32_t> get_dvar_int(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);

  if (dvar) {
    return dvar.get_int();
  }

  return std::nullopt;
}

int32_t set_dvar_int(EngineDependentDvar dvar, int32_t val,
                     DvarSetSource source) {
  return dvar.set(val, source);
}

std::optional<int32_t> set_dvar_int(const char *dvar_name, int32_t val,
                                    DvarSetSource source) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {

    return dvar.get_int();
  }

  return std::nullopt;
}

__inline_def uint32_t get_dvar_uint(EngineDependentDvar dvar) {
  return dvar.get_uint();
}

std::optional<uint32_t> get_dvar_uint(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.get_uint();
  }

  return std::nullopt;
}

__inline_def uint64_t get_dvar_uint64(EngineDependentDvar dvar) {
  return dvar.get_uint64();
}

std::optional<uint64_t> get_dvar_uint64(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.get_uint64();
  }

  return std::nullopt;
}

uint64_t set_dvar_uint64(EngineDependentDvar dvar, uint64_t val,
                         DvarSetSource source) {
  return dvar.set(val, source);
}

std::optional<uint64_t> set_dvar_uint64(const char *dvar_name, uint64_t val,
                                        DvarSetSource source) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.set(val, source);
  }
  return std::nullopt;
}

__inline_def int64_t get_dvar_int64(EngineDependentDvar dvar) {
  return dvar.get_int64();
}

std::optional<int64_t> get_dvar_int64(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.get_int64();
  }

  return std::nullopt;
}

int64_t set_dvar_int64(EngineDependentDvar dvar, int64_t val,
                       DvarSetSource source) {
  return dvar.set(val, source);
}

std::optional<int64_t> set_dvar_int64(const char *dvar_name, int64_t val,
                                      DvarSetSource source) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.set(val, source);
  }
  return std::nullopt;
}

__inline_def bool get_dvar_bool(EngineDependentDvar dvar) {
  return dvar.get_bool();
}

std::optional<bool> get_dvar_bool(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.get_bool();
  }

  return std::nullopt;
}

bool set_dvar_bool(EngineDependentDvar dvar, bool val, DvarSetSource source) {
  return dvar.set(val, source);
}

std::optional<bool> set_dvar_bool(const char *dvar_name, bool val,
                                  DvarSetSource source) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.set(val, source);
  }
  return std::nullopt;
}

__inline_def float get_dvar_float(EngineDependentDvar dvar) {
  return dvar.get_float();
}

std::optional<float> get_dvar_float(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.get_float();
  }

  return std::nullopt;
}

float set_dvar_float(EngineDependentDvar dvar, float val,
                     DvarSetSource source) {
  return dvar.set(val, source);
}

std::optional<float> set_dvar_float(const char *dvar_name, float val,
                                    DvarSetSource source) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.set(val, source);
  }
  return std::nullopt;
}

std::optional<std::string> set_dvar_string(EngineDependentDvar dvar,
                                           const char *val,
                                           DvarSetSource source) {
  return dvar.set(val, source);
}

std::optional<std::string>
set_dvar_string(const char *dvar_name, const char *val, DvarSetSource source) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.set(val, source);
  }
  return std::nullopt;
}

template <typename T>
static void foreach_client(
    T *client_states,
    const std::function<void(sv::client_s &, size_t index)> &callback) {
  if (!client_states || !callback) {
    return;
  }

  for (size_t i = 0; i < get_max_client_count(); ++i) {
    callback(client_states[i], i);
  }
}

template <typename T>
static void first_client(
    T *client_states,
    const std::function<bool(sv::client_s &, size_t index)> &callback) {
  if (!client_states || !callback) {
    return;
  }

  for (size_t i = 0; i < get_max_client_count(); ++i) {
    if (callback(client_states[i], i)) {
      break;
    }
  }
}

template <typename T>
static bool access_client(T *client_states, const size_t index,
                          const std::function<void(sv::client_s &)> &callback) {
  if (!client_states || !callback) {
    return false;
  }

  if (index >= get_max_client_count()) {
    return false;
  }

  T &client = client_states[index];
  if (client.state == net::CS_FREE) {
    return false;
  }

  callback(client);
  return true;
}

void foreach_client(
    const std::function<void(sv::client_s &, size_t index)> &callback) {
  if (is_server()) {
    foreach_client(*svs_clients, callback);
  } else {
    foreach_client(*svs_clients_cl, callback);
  }
}

void first_client(
    const std::function<bool(sv::client_s &, size_t index)> &callback) {
  if (is_server()) {
    first_client(*svs_clients, callback);
  } else {
    first_client(*svs_clients_cl, callback);
  }
}

void foreach_client(const std::function<void(sv::client_s &)> &callback) {
  foreach_client([&](sv::client_s &client, size_t) { callback(client); });
}
void first_client(const std::function<bool(sv::client_s &)> &callback) {
  first_client([&](sv::client_s &client, size_t) { return callback(client); });
}

void foreach_connected_client(
    const std::function<void(sv::client_s &, size_t index)> &callback) {
  foreach_client([&](sv::client_s &client, const size_t index) {
    if (client.state != net::CS_FREE) {
      callback(client, index);
    }
  });
}

void foreach_connected_client(
    const std::function<void(sv::client_s &)> &callback) {
  foreach_connected_client(
      [&](sv::client_s &client, size_t) { callback(client); });
}

void first_connected_client(
    const std::function<bool(sv::client_s &, size_t index)> &callback) {
  first_client([&](sv::client_s &client, const size_t index) {
    if (client.state != net::CS_FREE) {
      return callback(client, index);
    }
    return false;
  });
}

void first_connected_client(
    const std::function<bool(sv::client_s &)> &callback) {
  first_connected_client(
      [&](sv::client_s &client, size_t) { return callback(client); });
}

bool access_connected_client(
    const size_t index, const std::function<void(sv::client_s &)> &callback) {
  if (is_server()) {
    return access_client(*svs_clients, index, callback);
  }

  return access_client(*svs_clients_cl, index, callback);
}

level::gentity_pool *gentity_pool() {
  if (game::is_client()) {
    /*
      In the client, for each function where g_entities is accessed,
      in each of its calling functions, prior to its callsite,
      Treyarch's anticheat (TAC) will copy the pointer stored in g_entities
      into another memory address or stack pointer and set g_entities to
      0xFFFFFFFFFFFFFFFF. In the called function, g_entities will then be
      accessed through the pointer stored at the destination of the copy,
      using a series of highly obfuscated operations. After the call is
      completed, TAC will copy the g_entities pointer back into the global
      g_entities memory address through, again, a series of highly obfuscated
      operations.

      This obviously makes naive usage of the g_entities global impractical,
      as it will often be set to 0xFFFFFFFFFFFFFFFF during or before access,
      causing unexpected memory access exceptions.

      To circumvent this, and to avoid having to find some way of
      deterministically computing where the correct g_entities pointer was
      copied, we can instead store the g_entities pointer at time of
      allocation into our own global, and use this identically and reliably
      within boiii's code.
    */
    game::level::gentity_pool *stored =
        game::level::g_entities_cl_allocation.load();
    if (stored) {
      return stored;
    }
    return *(level::g_entities_cl.get());
  }

  return level::g_entities.get();
}

std::vector<std::string> get_registered_dvar_names() {
  std::vector<std::string> result;
  registered_dvar_names_.access(
      [&result](const std::vector<std::string> &names) { result = names; });
  return result;
}

size_t get_registered_dvar_name_count() {
  return registered_dvar_name_count_.load();
}

} // namespace game