#include <std_include.hpp>

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
  }
};
} // namespace name
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::name::component)
