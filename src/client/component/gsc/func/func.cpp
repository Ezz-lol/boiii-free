#include <std_include.hpp>

#include "func.hpp"

#include <string>

#include <game/impl/scr/scr.hpp>
#include <game/impl/scr/var.hpp>
#include <game/impl/sv/sv.hpp>
#include <game/utils.hpp>

#include <component/name.hpp>
#include <loader/component_loader.hpp>

#include <utils/hook.hpp>

using namespace game;
using namespace game::scr;

namespace gsc::custom_builtins {
static ScrPool<CustomBuiltinMap<BuiltinFunctionDef>> functions;
static ScrPool<CustomBuiltinMap<BuiltinMethodDef>> methods;
} // namespace gsc::custom_builtins

namespace gsc {
namespace func {

using namespace gsc;
namespace {

// =====================================================
// Core builtins
// =====================================================

utils::hook::detour Scr_GetFunction_hook;
BuiltinFunction Scr_GetFunction_SearchCustom(ScrVarCanonicalName_t canonId,
                                             BuiltinType *type,
                                             int32_t *min_args,
                                             int32_t *max_args) {
  if (custom_builtins::functions.server.map.contains(canonId)) {
    const BuiltinFunctionDef *def =
        &custom_builtins::functions.server.map[canonId];

    *type = def->type;
    *min_args = def->min_args;
    *max_args = def->max_args;

    return def->actionFunc;
  }

  return Scr_GetFunction_hook.invoke<BuiltinFunction>(canonId, type, min_args,
                                                      max_args);
}

utils::hook::detour Scr_GetMethod_hook;
BuiltinMethod Scr_GetMethod_SearchCustom(ScrVarCanonicalName_t canonId,
                                         BuiltinType *type, int32_t *min_args,
                                         int32_t *max_args) {
  if (custom_builtins::methods.server.map.contains(canonId)) {
    const BuiltinMethodDef *def = &custom_builtins::methods.server.map[canonId];

    *type = def->type;
    *min_args = def->min_args;
    *max_args = def->max_args;

    return def->actionFunc;
  }

  return Scr_GetMethod_hook.invoke<BuiltinMethod>(canonId, type, min_args,
                                                  max_args);
}

utils::hook::detour Scr_GetFunctionReverseLookup_hook;
ScrVarCanonicalName_t
Scr_GetFunctionReverseLookup_SearchCustom(BuiltinFunction func) {
  if (custom_builtins::functions.server.reverse.contains(func)) {
    return custom_builtins::functions.server.reverse[func];
  }
  return Scr_GetFunctionReverseLookup_hook.invoke<ScrVarCanonicalName_t>(func);
}

utils::hook::detour Scr_GetMethodReverseLookup_hook;
ScrVarCanonicalName_t
Scr_GetMethodReverseLookup_SearchCustom(BuiltinMethod method) {
  if (custom_builtins::methods.server.reverse.contains(method)) {
    return custom_builtins::methods.server.reverse[method];
  }
  return Scr_GetMethodReverseLookup_hook.invoke<ScrVarCanonicalName_t>(method);
}

utils::hook::detour CScr_GetFunction_hook;
BuiltinFunction CScr_GetFunction_SearchCustom(ScrVarCanonicalName_t canonId,
                                              BuiltinType *type,
                                              int32_t *min_args,
                                              int32_t *max_args) {
  if (custom_builtins::functions.client.map.contains(canonId)) {
    const BuiltinFunctionDef *def =
        &custom_builtins::functions.client.map[canonId];

    *type = def->type;
    *min_args = def->min_args;
    *max_args = def->max_args;

    return def->actionFunc;
  }

  return CScr_GetFunction_hook.invoke<BuiltinFunction>(canonId, type, min_args,
                                                       max_args);
}

utils::hook::detour CScr_GetMethod_hook;
BuiltinMethod CScr_GetMethod_SearchCustom(ScrVarCanonicalName_t canonId,
                                          BuiltinType *type, int32_t *min_args,
                                          int32_t *max_args) {
  if (custom_builtins::methods.client.map.contains(canonId)) {
    const BuiltinMethodDef *def = &custom_builtins::methods.client.map[canonId];

    *type = def->type;
    *min_args = def->min_args;
    *max_args = def->max_args;

    return def->actionFunc;
  }

  return CScr_GetMethod_hook.invoke<BuiltinMethod>(canonId, type, min_args,
                                                   max_args);
}

utils::hook::detour CScr_GetFunctionReverseLookup_hook;
ScrVarCanonicalName_t
CScr_GetFunctionReverseLookup_SearchCustom(BuiltinFunction func) {
  if (custom_builtins::functions.client.reverse.contains(func)) {
    return custom_builtins::functions.client.reverse[func];
  }
  return CScr_GetFunctionReverseLookup_hook.invoke<ScrVarCanonicalName_t>(func);
}

utils::hook::detour CScr_GetMethodReverseLookup_hook;
ScrVarCanonicalName_t
CScr_GetMethodReverseLookup_SearchCustom(BuiltinMethod method) {
  if (custom_builtins::methods.client.reverse.contains(method)) {
    return custom_builtins::methods.client.reverse[method];
  }
  return CScr_GetMethodReverseLookup_hook.invoke<ScrVarCanonicalName_t>(method);
}

void PlayerCmd_IsHost_DelegateToFirstClient(scriptInstance_t inst,
                                            scr_entref_t *entref) {
  if (entref->classnum == 0) {
    const level::gentity_t *ent = level::entity(entref->u.entnum);
    if (ent && ent->client) {
      push(inst, ent->client->sess.cs.clientIndex == game::CLIENT_INDEX_0);
    } else {
      Scr_ObjectError(
          SCRIPTINSTANCE_SERVER,
          utils::string::va("entity %i is not a player", entref->u.entnum));
    }
  } else {
    Scr_ObjectError(SCRIPTINSTANCE_SERVER, "not an entity");
  }
}

} // namespace

void Scr_StubFunc([[maybe_unused]] scriptInstance_t inst) {}

void apply_bgb_hooks() {
  // Workaround for "Out of X" gobblegum
  game::scr::builtin::table::gscr::builtin_methods->GetBGBRemaining.actionFunc =
      ScrCmd_Return<std::numeric_limits<uint8_t>::max()>;
  game::scr::builtin::table::gscr::builtin_functions
      ->__protected__SetBGBUnlocked.actionFunc = &Scr_StubFunc;
  game::scr::builtin::table::gscr::builtin_functions
      ->__protected__GetBGBUnlocked.actionFunc = &Scr_Return<true>;
}

BuiltinFunction GScr_SetDvar_orig;
void GScr_SetDvar_SkipStockCheatDvar(const scriptInstance_t inst) {
  if (Scr_GetNumParam(inst) > 0 && *game::dvar_cheats &&
      !game::dvar_cheats->get_bool() &&
      _stricmp(Scr_GetString(inst, 0), "cg_thirdPersonAngle") == 0) {
    return;
  }
  GScr_SetDvar_orig(inst);
}

void apply_set_dvar_hook() {
  GScr_SetDvar_orig =
      game::scr::builtin::table::common_functions->SetDvar.actionFunc;
  game::scr::builtin::table::common_functions->SetDvar.actionFunc =
      GScr_SetDvar_SkipStockCheatDvar;
}

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("gsc/func");

  void post_unpack() override {

    Scr_GetFunctionReverseLookup_hook.create(
        game::scr::builtin::Scr_GetFunctionReverseLookup.get(),
        Scr_GetFunctionReverseLookup_SearchCustom);
    Scr_GetMethodReverseLookup_hook.create(
        game::scr::builtin::Scr_GetMethodReverseLookup.get(),
        Scr_GetMethodReverseLookup_SearchCustom);

    Scr_GetFunction_hook.create(game::scr::builtin::Scr_GetFunction.get(),
                                Scr_GetFunction_SearchCustom);
    Scr_GetMethod_hook.create(game::scr::builtin::Scr_GetMethod.get(),
                              Scr_GetMethod_SearchCustom);

    CScr_GetFunctionReverseLookup_hook.create(
        game::scr::builtin::cscr::CScr_GetFunctionReverseLookup.get(),
        CScr_GetFunctionReverseLookup_SearchCustom);
    CScr_GetMethodReverseLookup_hook.create(
        game::scr::builtin::cscr::CScr_GetMethodReverseLookup.get(),
        CScr_GetMethodReverseLookup_SearchCustom);

    CScr_GetFunction_hook.create(
        game::scr::builtin::cscr::CScr_GetFunction.get(),
        CScr_GetFunction_SearchCustom);
    CScr_GetMethod_hook.create(game::scr::builtin::cscr::CScr_GetMethod.get(),
                               CScr_GetMethod_SearchCustom);

    apply_bgb_hooks();
    apply_set_dvar_hook();

    /*
      In dedicated server, there is no host player.

      This breaks custom maps and mods that require the host player to configure
      the game using an options menu before any clients are permitted to leave
      the menu and begin the game.

      This is generally fixed via map-specific GSC scripts that automatically
      configure the game and close configuration menus.

      To ensure these maps allow game configuration by _one_ player by default,
      the following hook modifies the `ishost` builtin function to return
      \`true\` if the player has `clientIndex` `0`, and false otherwise.
    */
    if (game::is_server()) {
      game::scr::builtin::table::player_methods->IsHost.actionFunc =
          PlayerCmd_IsHost_DelegateToFirstClient;
    }
  }
};
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::component)
