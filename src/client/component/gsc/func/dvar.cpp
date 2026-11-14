#include <std_include.hpp>

#include "command.hpp"
#include "func.hpp"

#include <game/impl/scr/scr.hpp>
#include <game/impl/sv/sv.hpp>
#include <game/utils.hpp>

#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>

#include <utils/hook.hpp>
#include <utils/string.hpp>

using namespace game;
using namespace game::scr;

namespace gsc {
namespace func {
namespace dvar {
constexpr uint32_t PROTECTED_DVAR_FLAGS = static_cast<uint32_t>(
    dvarFlags_e::USERINFO | dvarFlags_e::LATCH | dvarFlags_e::ROM |
    dvarFlags_e::INIT | dvarFlags_e::CHEAT);

constexpr uint32_t ARCHIVE_FLAG = static_cast<uint32_t>(dvarFlags_e::ARCHIVE);

struct server_dvar_original {
  std::string value;
  bool archived;
};

std::unordered_map<std::string, server_dvar_original> server_dvar_originals;

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
  for (const auto &[name, original] : server_dvar_originals) {
    Dvar_SetFromStringByName(name.c_str(), original.value.c_str(), false);
    if (EngineDependentDvarMut dvar = Dvar_FindVar(name.c_str());
        dvar && original.archived) {
      dvar.flags()._raw |= ARCHIVE_FLAG;
    }
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
  const bool archived = dvar.flags()._raw & ARCHIVE_FLAG;
  if (server_dvar_originals
          .try_emplace(
              utils::string::to_lower(name),
              server_dvar_original{Dvar_DisplayableValue(dvar), archived})
          .second) {
    dvar.flags()._raw &= ~ARCHIVE_FLAG;
  }
  Dvar_SetFromStringByName(name, value, false);
}

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
  DEFINE_COMPONENT_NAME("gsc/func/dvar");

  void post_unpack() override {
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
} // namespace dvar
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::dvar::component)
