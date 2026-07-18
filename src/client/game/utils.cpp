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

__inline_def EngineDependentDvarMut get_dvar(const char *name) {
  return Dvar_FindVar(name);
}

std::optional<std::string_view> get_dvar_string(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);

  if (dvar) {
    return dvar.get_string();
  }

  return std::nullopt;
}

std::optional<int32_t> get_dvar_int(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);

  if (dvar) {
    return dvar.get_int();
  }

  return std::nullopt;
}

std::optional<int32_t> set_dvar_int(const char *dvar_name, int32_t val,
                                    DvarSetSource source) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.set(val, source);
  }

  return std::nullopt;
}

std::optional<uint32_t> get_dvar_uint(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.get_uint();
  }

  return std::nullopt;
}

std::optional<uint64_t> get_dvar_uint64(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.get_uint64();
  }

  return std::nullopt;
}

std::optional<uint64_t> set_dvar_uint64(const char *dvar_name, uint64_t val,
                                        DvarSetSource source) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.set(val, source);
  }
  return std::nullopt;
}

std::optional<int64_t> get_dvar_int64(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.get_int64();
  }

  return std::nullopt;
}

std::optional<int64_t> set_dvar_int64(const char *dvar_name, int64_t val,
                                      DvarSetSource source) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.set(val, source);
  }
  return std::nullopt;
}

std::optional<bool> get_dvar_bool(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.get_bool();
  }

  return std::nullopt;
}

std::optional<bool> set_dvar_bool(const char *dvar_name, bool val,
                                  DvarSetSource source) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.set(val, source);
  }
  return std::nullopt;
}

std::optional<float> get_dvar_float(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.get_float();
  }

  return std::nullopt;
}

std::optional<float> set_dvar_float(const char *dvar_name, float val,
                                    DvarSetSource source) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (dvar) {
    return dvar.set(val, source);
  }
  return std::nullopt;
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