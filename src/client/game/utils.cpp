#include "../std_include.hpp"

#include "game.hpp"
#include "utils.hpp"

#include "../../common/utils/concurrency.hpp"

using namespace game::com;
using namespace game::sv;

namespace game {

namespace {
utils::concurrency::container<std::vector<std::string>>
    registered_dvar_names_{};
std::atomic_size_t registered_dvar_name_count_{0};

void record_registered_dvar_name(const char *dvar_name) {
  if (!dvar_name || !dvar_name[0]) {
    return;
  }
  registered_dvar_names_.access([dvar_name](std::vector<std::string> &names) {
    names.emplace_back(dvar_name);
  });
  ++registered_dvar_name_count_;
}

EngineDependentDvarMut
try_get_sessionmode_specific_dvar(EngineDependentDvarMut dvar) {
  return std::visit(
      [](auto *resolved) -> EngineDependentDvarMut {
        if (!resolved || resolved->type != dvarType_t::SESSIONMODE_BASE_DVAR) {
          return resolved;
        }

        if (Com_SessionMode_IsMode(eModes::COUNT)) {
          return static_cast<decltype(resolved)>(nullptr);
        }

        const game::eModes mode = Com_SessionMode_GetMode();
        return reinterpret_cast<decltype(resolved)>(
            Dvar_GetSessionModeSpecificDvar(
                reinterpret_cast<dvar_t *>(resolved),
                static_cast<eModes>(mode)));
      },
      dvar);
}
} // namespace

__inline_def EngineDependentDvarMut get_dvar(const char *name) {
  return dvar_variant(Dvar_FindVar(name));
}

__inline_def std::optional<std::string_view>
get_dvar_string(EngineDependentDvar dvar) {
  return std::visit(
      [](const auto *resolved) -> std::optional<std::string_view> {
        const char *str = resolved->current.string();
        return str ? std::optional(std::string_view(str)) : std::nullopt;
      },
      dvar);
}

std::optional<std::string_view> get_dvar_string(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);

  return std::visit(
      [](const auto *resolved) -> std::optional<std::string_view> {
        if (resolved) {
          return get_dvar_string(engine_dependent_toggle_const(resolved));
        }

        return std::nullopt;
      },
      dvar);
}

__inline_def int32_t get_dvar_int(EngineDependentDvar dvar) {
  return std::visit(
      [](auto *resolved) -> int32_t { return resolved->current.integer(); },
      dvar);
}

std::optional<int32_t> get_dvar_int(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);

  if (engine_dependent_nonnull(dvar)) {
    return get_dvar_int(engine_dependent_toggle_const(dvar));
  }

  return std::nullopt;
}

int32_t set_dvar_int(EngineDependentDvar dvar, int32_t val,
                     DvarSetSource source) {
  int32_t prev_val = get_dvar_int(dvar);
  std::visit(
      [val, source](const auto *resolved) -> void {
        Dvar_SetIntFromSource(reinterpret_cast<const dvar_t *>(resolved), val,
                              source);
      },
      dvar);
  return prev_val;
}

std::optional<int32_t> set_dvar_int(const char *dvar_name, int32_t val,
                                    DvarSetSource source) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (engine_dependent_nonnull(dvar)) {

    return set_dvar_int(engine_dependent_toggle_const(dvar), val, source);
  }

  return std::nullopt;
}

__inline_def uint32_t get_dvar_uint(EngineDependentDvar dvar) {
  return std::visit(
      [](const auto *resolved) -> uint32_t {
        return resolved->current.unsignedInt();
      },
      dvar);
}

std::optional<uint32_t> get_dvar_uint(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (engine_dependent_nonnull(dvar)) {
    return get_dvar_uint(engine_dependent_toggle_const(dvar));
  }

  return std::nullopt;
}

__inline_def uint64_t get_dvar_uint64(EngineDependentDvar dvar) {
  return std::visit(
      [](const auto *resolved) -> uint64_t {
        return resolved->current.unsignedInt64();
      },
      dvar);
}

std::optional<uint64_t> get_dvar_uint64(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (engine_dependent_nonnull(dvar)) {
    return get_dvar_uint64(engine_dependent_toggle_const(dvar));
  }

  return std::nullopt;
}

uint64_t set_dvar_uint64(EngineDependentDvar dvar, uint64_t val,
                         DvarSetSource source) {
  const uint64_t prev_val = get_dvar_uint64(dvar);
  std::visit(
      [val, source](const auto *resolved) -> void {
        Dvar_SetUInt64FromSource(reinterpret_cast<const dvar_t *>(resolved),
                                 val, source);
      },
      dvar);
  return prev_val;
}

std::optional<uint64_t> set_dvar_uint64(const char *dvar_name, uint64_t val,
                                        DvarSetSource source) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (engine_dependent_nonnull(dvar)) {
    return set_dvar_uint64(engine_dependent_toggle_const(dvar), val, source);
  }
  return std::nullopt;
}

__inline_def int64_t get_dvar_int64(EngineDependentDvar dvar) {
  return std::visit(
      [](const auto *resolved) -> int64_t {
        return resolved->current.integer64();
      },
      dvar);
}

std::optional<int64_t> get_dvar_int64(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (engine_dependent_nonnull(dvar)) {
    return get_dvar_int64(engine_dependent_toggle_const(dvar));
  }

  return std::nullopt;
}

int64_t set_dvar_int64(EngineDependentDvar dvar, int64_t val,
                       DvarSetSource source) {
  const int64_t prev_val = get_dvar_int64(dvar);
  std::visit(
      [val, source](const auto *resolved) -> void {
        Dvar_SetInt64FromSource(reinterpret_cast<const dvar_t *>(resolved), val,
                                source);
      },
      dvar);
  return prev_val;
}

std::optional<int64_t> set_dvar_int64(const char *dvar_name, int64_t val,
                                      DvarSetSource source) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (engine_dependent_nonnull(dvar)) {
    return set_dvar_int64(engine_dependent_toggle_const(dvar), val, source);
  }
  return std::nullopt;
}

__inline_def bool get_dvar_bool(EngineDependentDvar dvar) {
  return std::visit(
      [](const auto *resolved) -> bool { return resolved->current.enabled(); },
      dvar);
}

std::optional<bool> get_dvar_bool(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (engine_dependent_nonnull(dvar)) {
    return get_dvar_bool(engine_dependent_toggle_const(dvar));
  }

  return std::nullopt;
}

bool set_dvar_bool(EngineDependentDvar dvar, bool val, DvarSetSource source) {
  const bool prev_val = get_dvar_bool(dvar);
  std::visit(
      [val, source](const auto *resolved) -> void {
        Dvar_SetBoolFromSource(reinterpret_cast<const dvar_t *>(resolved), val,
                               source);
      },
      dvar);
  return prev_val;
}

std::optional<bool> set_dvar_bool(const char *dvar_name, bool val,
                                  DvarSetSource source) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (engine_dependent_nonnull(dvar)) {
    set_dvar_bool(engine_dependent_toggle_const(dvar), val, source);
  }
  return std::nullopt;
}

__inline_def float get_dvar_float(EngineDependentDvar dvar) {
  return std::visit(
      [](const auto *resolved) -> float { return resolved->current.value(); },
      dvar);
}

std::optional<float> get_dvar_float(const char *dvar_name) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (engine_dependent_nonnull(dvar)) {
    return get_dvar_float(engine_dependent_toggle_const(dvar));
  }

  return std::nullopt;
}

float set_dvar_float(EngineDependentDvar dvar, float val,
                     DvarSetSource source) {
  const float prev_val = get_dvar_float(dvar);
  std::visit(
      [val, source](const auto *resolved) -> void {
        Dvar_SetFloatFromSource(reinterpret_cast<const dvar_t *>(resolved), val,
                                source);
      },
      dvar);
  return prev_val;
}

std::optional<float> set_dvar_float(const char *dvar_name, float val,
                                    DvarSetSource source) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (engine_dependent_nonnull(dvar)) {
    return set_dvar_float(engine_dependent_toggle_const(dvar), val, source);
  }
  return std::nullopt;
}

std::optional<std::string> set_dvar_string(EngineDependentDvar dvar,
                                           const char *val,
                                           DvarSetSource source) {
  const std::optional<std::string_view> prev_val = get_dvar_string(dvar);
  std::optional<std::string> prev_val_copy;
  if (prev_val.has_value()) {
    prev_val_copy = std::optional(std::string(prev_val.value()));
  } else {
    prev_val_copy = std::nullopt;
  }
  std::visit(
      [val, source](const auto *resolved) -> void {
        Dvar_SetStringFromSource(reinterpret_cast<const dvar_t *>(resolved),
                                 val, source);
      },
      dvar);
  return prev_val_copy;
}

std::optional<std::string>
set_dvar_string(const char *dvar_name, const char *val, DvarSetSource source) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  if (engine_dependent_nonnull(dvar)) {
    return set_dvar_string(engine_dependent_toggle_const(dvar), val, source);
  }
  return std::nullopt;
}

EngineDependentDvar register_sessionmode_dvar_bool(const char *dvar_name,
                                                   const bool value,
                                                   const uint32_t flags,
                                                   const char *description,
                                                   const eModes mode) {
  const game::CanonHash_t hash = Dvar_GenerateHash(dvar_name);
  EngineDependentDvarMut registered_dvar = dvar_variant(
      Dvar_SessionModeRegisterBool(hash, dvar_name, value, flags, description));

  return std::visit(
      [dvar_name, value, mode, hash](auto *resolved) -> EngineDependentDvar {
        if (resolved) {
          resolved->debugName = dvar_name;
          record_registered_dvar_name(dvar_name);

          if (mode == eModes::COUNT) {
            for (game::eModes i = eModes::FIRST; i < eModes::COUNT; i++) {
              Dvar_SessionModeSetDefaultBool.call_safe(hash, value, i);
            }
          } else {
            Dvar_SessionModeSetDefaultBool.call_safe(hash, value, mode);
          }
        }

        return engine_dependent_toggle_const(resolved);
      },
      registered_dvar);
}

EngineDependentDvar register_dvar_bool(const char *dvar_name, const bool value,
                                       const uint32_t flags,
                                       const char *description) {
  const game::CanonHash_t hash = Dvar_GenerateHash(dvar_name);
  EngineDependentDvarMut registered_dvar = dvar_variant(
      Dvar_RegisterBool(hash, dvar_name, value, flags, description));

  return std::visit(
      [dvar_name](auto *resolved) -> EngineDependentDvar {
        if (resolved) {
          resolved->debugName = dvar_name;
          record_registered_dvar_name(dvar_name);
        }

        return engine_dependent_toggle_const(resolved);
      },
      registered_dvar);
}

EngineDependentDvar register_dvar_int(const char *dvar_name, int32_t value,
                                      int32_t min, int32_t max,
                                      const uint32_t flags,
                                      const char *description) {
  const game::CanonHash_t hash = Dvar_GenerateHash(dvar_name);
  EngineDependentDvarMut registered_dvar = dvar_variant(
      Dvar_RegisterInt(hash, dvar_name, value, min, max, flags, description));

  return std::visit(
      [dvar_name](auto *resolved) -> EngineDependentDvar {
        if (resolved) {
          resolved->debugName = dvar_name;
          record_registered_dvar_name(dvar_name);
        }

        return engine_dependent_toggle_const(resolved);
      },
      registered_dvar);
}

EngineDependentDvar register_dvar_float(const char *dvar_name, float value,
                                        float min, float max,
                                        const uint32_t flags,
                                        const char *description) {
  const game::CanonHash_t hash = Dvar_GenerateHash(dvar_name);
  EngineDependentDvarMut registered_dvar = dvar_variant(
      Dvar_RegisterFloat(hash, dvar_name, value, min, max, flags, description));
  return std::visit(
      [dvar_name](auto *resolved) -> EngineDependentDvar {
        if (resolved) {
          resolved->debugName = dvar_name;
          record_registered_dvar_name(dvar_name);
        }

        return engine_dependent_toggle_const(resolved);
      },
      registered_dvar);
}

EngineDependentDvar register_dvar_string(const char *dvar_name,
                                         const char *value,
                                         const uint32_t flags,
                                         const char *description) {
  const game::CanonHash_t hash = Dvar_GenerateHash(dvar_name);
  EngineDependentDvarMut registered_dvar = dvar_variant(
      Dvar_RegisterString(hash, dvar_name, value, flags, description));

  return std::visit(
      [dvar_name](auto *resolved) -> EngineDependentDvar {
        if (resolved) {
          resolved->debugName = dvar_name;
          record_registered_dvar_name(dvar_name);
        }

        return engine_dependent_toggle_const(resolved);
      },
      registered_dvar);
}

void dvar_add_flags(const char *dvar_name, const uint32_t flags) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);

  std::visit(
      [flags](auto *resolved) -> void {
        if (!resolved) {
          return;
        }

        using Resolved = decltype(resolved);
        Resolved dvar_to_change = resolved;
        if (dvar_to_change->type == dvarType_t::SESSIONMODE_BASE_DVAR) {
          dvar_to_change = std::get<Resolved>(
              try_get_sessionmode_specific_dvar(dvar_to_change));
          if (!dvar_to_change) {
            return;
          }
        }

        dvar_to_change->flags |= flags;
      },
      dvar);
}

void dvar_set_flags(const char *dvar_name, const uint32_t flags) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  std::visit(
      [flags](auto *resolved) -> void {
        if (!resolved) {
          return;
        }

        using Resolved = decltype(resolved);
        Resolved dvar_to_change = resolved;
        if (dvar_to_change->type == dvarType_t::SESSIONMODE_BASE_DVAR) {
          dvar_to_change = std::get<Resolved>(
              try_get_sessionmode_specific_dvar(dvar_to_change));
          if (!dvar_to_change) {
            return;
          }
        }

        dvar_to_change->flags = flags;
      },
      dvar);
}

void dvar_remove_flags(const char *dvar_name, const uint32_t flags) {
  EngineDependentDvarMut dvar = get_dvar(dvar_name);
  std::visit(
      [flags](auto *resolved) -> void {
        if (!resolved) {
          return;
        }

        using Resolved = decltype(resolved);
        Resolved dvar_to_change = resolved;
        if (dvar_to_change->type == dvarType_t::SESSIONMODE_BASE_DVAR) {
          dvar_to_change = std::get<Resolved>(
              try_get_sessionmode_specific_dvar(dvar_to_change));
          if (!dvar_to_change) {
            return;
          }
        }

        dvar_to_change->flags &= ~flags;
      },
      dvar);
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