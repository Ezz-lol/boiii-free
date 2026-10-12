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
#include <component/toast.hpp>
#include <loader/component_loader.hpp>

#include <utils/hook.hpp>
#include <utils/io.hpp>

#include <rapidjson/writer.h>

using namespace game;
using namespace game::scr;

namespace gsc {
namespace func {
namespace name {
std::unordered_map<game::ClientNum_t, std::unordered_set<std::string>>
    client_dvar_changes;

std::string trim_copy(std::string value) {
  const size_t start = value.find_first_not_of(" \t");
  if (start == std::string::npos) {
    return {};
  }

  const size_t end = value.find_last_not_of(" \t");
  return value.substr(start, end - start + 1);
}

std::optional<std::string> extract_dvar_name(const char *dvar_cmd) {
  if (!dvar_cmd || !dvar_cmd[0]) {
    return {};
  }

  const std::string trimmed = trim_copy(dvar_cmd);
  if (trimmed.empty()) {
    return {};
  }

  std::function<std::string(size_t &)> next_token =
      [&](size_t &cursor) -> std::string {
    while (cursor < trimmed.size() &&
           (trimmed[cursor] == ' ' || trimmed[cursor] == '\t')) {
      cursor++;
    }

    const size_t start = cursor;
    while (cursor < trimmed.size() && trimmed[cursor] != ' ' &&
           trimmed[cursor] != '\t') {
      cursor++;
    }

    return trimmed.substr(start, cursor - start);
  };

  size_t cursor = 0;
  std::string first = next_token(cursor);
  if (first.empty()) {
    return {};
  }

  const std::string lowered = gsc::func::command::normalize_command_name(first);
  if (lowered == "set" || lowered == "seta" || lowered == "sets" ||
      lowered == "reset") {
    std::string second = next_token(cursor);
    if (!second.empty()) {
      return second;
    }
  }

  return first;
}

void reset_tracked_client_dvars() {
  for (const std::pair<const game::ClientNum_t, std::unordered_set<std::string>>
           &pair : client_dvar_changes) {
    const game::ClientNum_t &client_num = pair.first;
    const std::unordered_set<std::string> &dvars = pair.second;

    if (game::valid_client_num(client_num)) {

      for (const std::string &dvar_name : dvars) {
        sv::SV_GameSendServerCommand(
            client_num, game::net::SV_CMD_CAN_IGNORE,
            utils::string::va("c \"reset %s\"", dvar_name.c_str()));
      }
    }
  }
}

// =====================================================
// Player name/tag overrides (server-only)
// =====================================================

namespace gscr_setname {
void set(scriptInstance_t inst, game::ClientNum_t client_num,
         uint32_t name_index) {
  const char *player_name = game::scr::Scr_GetString(inst, name_index);
  if (game::valid_client_num(client_num) && player_name) {
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
  int32_t &flags = *reinterpret_cast<int32_t *>(
      reinterpret_cast<uint8_t *>(ent->client) +
      game::select<uint32_t>(0x16FE4, 0x0, 0x16FF4));
  flags ^= 1;
  Scr_AddInt(inst, flags & 1);
}

namespace gscr_settag {
void set(scriptInstance_t inst, game::ClientNum_t client_num,
         uint32_t tag_index) {
  const char *tag = game::scr::Scr_GetString(inst, tag_index);
  if (!game::valid_client_num(client_num) || !tag) {
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
  if (!game::valid_client_num(client_num)) {
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
  if (!game::valid_client_num(client_num)) {
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
  if (game::valid_client_num(client_num) && dvar_cmd) {
    const std::optional<std::string> dvar_name =
        dvar_cmd ? extract_dvar_name(dvar_cmd) : std::nullopt;
    if (dvar_name.has_value()) {
      client_dvar_changes[client_num].insert(*dvar_name);
    }

    sv::SV_GameSendServerCommand(client_num, game::net::SV_CMD_CAN_IGNORE,
                                 utils::string::va("c \"%s\"", dvar_cmd));
  } else {
    Scr_ParamError(inst, command_index,
                   "^1[setclientdvar] Invalid arguments\n");
  }
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

    game_event::on_g_shutdown_game([]() {
      reset_tracked_client_dvars();
      client_dvar_changes.clear();
    });
    game_event::on_g_init_game([]() { client_dvar_changes.clear(); });
  }
};
} // namespace name
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::name::component)
