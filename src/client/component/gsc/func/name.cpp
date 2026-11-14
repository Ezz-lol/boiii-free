#include <std_include.hpp>

#include "name.hpp"

#include "command.hpp"
#include "func.hpp"

#include <game/impl/scr/scr.hpp>
#include <game/impl/scr/var.hpp>
#include <game/impl/sv/sv.hpp>
#include <game/utils.hpp>

#include <component/command.hpp>
#include <component/game_event.hpp>
#include <component/name.hpp>
#include <component/scheduler.hpp>
#include <component/toast.hpp>
#include <loader/component_loader.hpp>

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

#include <rapidjson/writer.h>

using namespace game;
using namespace game::scr;

namespace gsc {
namespace func {
namespace name {
constexpr uint32_t PROTECTED_DVAR_FLAGS = static_cast<uint32_t>(
    dvarFlags_e::USERINFO | dvarFlags_e::LATCH | dvarFlags_e::ROM |
    dvarFlags_e::INIT | dvarFlags_e::CHEAT);

std::unordered_map<std::string, std::string> server_dvar_originals;

std::string trim_copy(const std::string_view value) {
  const size_t start = value.find_first_not_of(" \t");
  if (start == std::string_view::npos) {
    return {};
  }
  const size_t end = value.find_last_not_of(" \t");
  return std::string(value.substr(start, end - start + 1));
}

std::optional<std::pair<std::string, std::string>>
parse_dvar_command(const std::string_view command) {
  std::string rest = trim_copy(command);
  size_t split = rest.find_first_of(" \t");
  const std::string first = rest.substr(0, split);
  const std::string lowered = gsc::func::command::normalize_command_name(first);
  if (lowered == "set" || lowered == "seta" || lowered == "sets") {
    rest = split == std::string::npos ? "" : trim_copy(rest.substr(split));
    split = rest.find_first_of(" \t");
  }

  std::string name = rest.substr(0, split);
  std::string value =
      split == std::string::npos ? "" : trim_copy(rest.substr(split));
  if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
    value = value.substr(1, value.size() - 2);
  }
  if (name.empty() || value.find('"') != std::string::npos) {
    return std::nullopt;
  }
  return std::make_pair(std::move(name), std::move(value));
}

void restore_server_dvars() {
  for (const auto &[name, value] : server_dvar_originals) {
    Dvar_SetFromStringByName(name.c_str(), value.c_str(), false);
  }
  server_dvar_originals.clear();
}

void handle_unknown_server_command() {
  const CmdArgs *args = sys::Cmd_Argv();
  const int32_t nesting = args->nesting;
  const char *command = args->argv[nesting][0];
  if (args->argc[nesting] != 3 ||
      command[0] !=
          static_cast<char>(sv::ReliableCommand::RESERVED_UNUSED_UNDERSCORE) ||
      command[1] != '\0') {
    return;
  }

  const char *name = args->argv[nesting][1];
  const char *value = args->argv[nesting][2];
  EngineDependentDvarMut dvar = Dvar_FindVar(name);
  if (!dvar || (dvar.flags()._raw & PROTECTED_DVAR_FLAGS)) {
    return;
  }
  server_dvar_originals.try_emplace(utils::string::to_lower(name),
                                    Dvar_DisplayableValue(dvar));
  Dvar_SetFromStringByName(name, value, false);
}

// =====================================================
// Player name/tag overrides (server-only)
// =====================================================

namespace gscr_setname {
void set(scriptInstance_t inst, game::ClientNum_t client_num,
         uint32_t name_index) {
  const char *player_name = game::scr::Scr_GetString(inst, name_index);
  if (game::validate(client_num) && player_name) {
    ::name::set_name_override(client_num, player_name);
  } else {
    Scr_ParamError(inst, name_index, "^1[setname] Invalid arguments\n");
  }
}

void method(game::scr::scriptInstance_t inst, scr_entref_t *entref) {
  // player setname("PlayerOne");
  set(inst, static_cast<game::ClientNum_t>(entref->u.entnum), 0);
}

void func(scriptInstance_t inst) {
  // setname(client_num, "PlayerOne");
  set(inst, static_cast<game::ClientNum_t>(Scr_GetInt(inst, 0)), 1);
}
} // namespace gscr_setname

void gscr_noclip(scriptInstance_t inst, scr_entref_t *entref) {
  level::gentity_t *ent =
      entref->classnum == 0 ? level::entity(entref->u.entnum) : nullptr;
  if (!ent || !ent->client) {
    Scr_ObjectError(inst, "noclip can only be called on a player");
    return;
  }
  int32_t &flags =
      *reinterpret_cast<int32_t *>(reinterpret_cast<uint8_t *>(ent->client) +
                                   game::select<uint32_t>(0x16FE4, 0x16FF4));
  flags ^= 1;
  Scr_AddInt(inst, flags & 1);
}

namespace gscr_settag {
void set(scriptInstance_t inst, game::ClientNum_t client_num,
         uint32_t tag_index) {
  const char *tag = game::scr::Scr_GetString(inst, tag_index);
  if (!game::validate(client_num) || !tag) {
    Scr_ParamError(inst, tag_index, "^1[settag] Invalid arguments\n");
    return;
  }
  ::name::set_clan_abbrev_override(client_num, tag);
}

void method(game::scr::scriptInstance_t inst, scr_entref_t *entref) {
  // player settag("DEV");
  set(inst, static_cast<game::ClientNum_t>(entref->u.entnum), 0);
}

void func(scriptInstance_t inst) {
  // settag(client_num, "DEV");
  set(inst, static_cast<game::ClientNum_t>(Scr_GetInt(inst, 0)), 1);
}
} // namespace gscr_settag

namespace gscr_resetname {
void reset(scriptInstance_t inst, game::ClientNum_t client_num) {
  if (!game::validate(client_num)) {
    Scr_ParamError(inst, 0, "^1[resetname] Invalid arguments\n");
    return;
  }
  ::name::clear_name_override(client_num);
}

void method(game::scr::scriptInstance_t inst, scr_entref_t *entref) {
  // player resetname();
  reset(inst, static_cast<game::ClientNum_t>(entref->u.entnum));
}

void func(scriptInstance_t inst) {
  // resetname(client_num);
  reset(inst, static_cast<game::ClientNum_t>(Scr_GetInt(inst, 0)));
}
} // namespace gscr_resetname

namespace gscr_resettag {
void reset(scriptInstance_t inst, game::ClientNum_t client_num) {
  if (!game::validate(client_num)) {
    Scr_ParamError(inst, 0, "^1[resettag] Invalid arguments\n");
    return;
  }
  ::name::clear_clan_abbrev_override(client_num);
}

void method(game::scr::scriptInstance_t inst, scr_entref_t *entref) {
  // player resettag();
  reset(inst, static_cast<game::ClientNum_t>(entref->u.entnum));
}

void func(scriptInstance_t inst) {
  // resettag(client_num);
  reset(inst, static_cast<game::ClientNum_t>(Scr_GetInt(inst, 0)));
}
} // namespace gscr_resettag

namespace gscr_setclientdvar {
void set(scriptInstance_t inst, game::ClientNum_t client_num,
         uint32_t command_index) {
  const char *dvar_cmd = game::scr::Scr_GetString(inst, command_index);
  const std::optional<std::pair<std::string, std::string>> dvar =
      dvar_cmd ? parse_dvar_command(dvar_cmd) : std::nullopt;
  if (!game::validate(client_num) || !dvar) {
    Scr_ParamError(inst, command_index,
                   "^1[setclientdvar] Invalid arguments\n");
    return;
  }

  sv::SV_GameSendServerCommand(
      client_num, game::net::svscmd_type::CAN_IGNORE,
      utils::string::va("%c %s \"%s\"",
                        sv::ReliableCommand::RESERVED_UNUSED_UNDERSCORE,
                        dvar->first.c_str(), dvar->second.c_str()));
}

void method(game::scr::scriptInstance_t inst, scr_entref_t *entref) {
  // player setclientdvar("cg_fov 120");
  set(inst, static_cast<game::ClientNum_t>(entref->u.entnum), 0);
}

void func(scriptInstance_t inst) {
  // setclientdvar(client_num, "cg_fov 120");
  set(inst, static_cast<game::ClientNum_t>(Scr_GetInt(inst, 0)), 1);
}
} // namespace gscr_setclientdvar

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("gsc/func/name");

  void post_unpack() override {
    // Player name/tag overrides (server-only)
    register_builtin(SCRIPTINSTANCE_SERVER, "setname", gscr_setname::func, 2);
    register_builtin(SCRIPTINSTANCE_SERVER, "setname", gscr_setname::method, 1);
    register_builtin(SCRIPTINSTANCE_SERVER, "noclip", gscr_noclip, 0);
    register_builtin(SCRIPTINSTANCE_SERVER, "settag", gscr_settag::func, 2);
    register_builtin(SCRIPTINSTANCE_SERVER, "settag", gscr_settag::method, 1);
    register_builtin(SCRIPTINSTANCE_SERVER, "resetname", gscr_resetname::func,
                     1);
    register_builtin(SCRIPTINSTANCE_SERVER, "resetname", gscr_resetname::method,
                     0);
    register_builtin(SCRIPTINSTANCE_SERVER, "resettag", gscr_resettag::func, 1);
    register_builtin(SCRIPTINSTANCE_SERVER, "resettag", gscr_resettag::method,
                     0);
    register_builtin(SCRIPTINSTANCE_SERVER, "setclientdvar",
                     gscr_setclientdvar::func, 2);
    register_builtin(SCRIPTINSTANCE_SERVER, "setclientdvar",
                     gscr_setclientdvar::method, 1);

    if (game::is_client()) {
      utils::hook::call(
          game::cg::CG_DeployServerCommand_UnknownCommandCall.get(),
          handle_unknown_server_command);
      scheduler::loop(
          [] {
            if (!server_dvar_originals.empty() && !game::com::Com_IsInGame()) {
              restore_server_dvars();
            }
          },
          scheduler::pipeline::main, 1s);
    }
  }

  void pre_destroy() override { restore_server_dvars(); }
};
} // namespace name
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::name::component)
