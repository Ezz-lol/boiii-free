#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include <game/game.hpp>
#include <game/utils.hpp>
#include "scheduler.hpp"

#include <utils/string.hpp>
#include <utils/hook.hpp>

namespace dvars_patches {
namespace {
using namespace game;
void patch_dvars() {
  com_pauseSupported = register_sessionmode_dvar_bool(
      "com_pauseSupported", !is_server(), DVAR_SERVERINFO,
      "Whether pause is supported by the game mode");
}

void patch_flags() {
  if (is_client()) {
    dvar_set_flags("r_dof_enable", DVAR_ARCHIVE);
    dvar_set_flags("r_lodbiasrigid", DVAR_ARCHIVE);
    dvar_set_flags("gpad_stick_deadzone_max", DVAR_ARCHIVE);
    dvar_set_flags("gpad_stick_deadzone_min", DVAR_ARCHIVE);
    dvar_set_flags("cg_drawLagometer", DVAR_ARCHIVE);
  }

  scheduler::execute(scheduler::pipeline::dvars_flags_patched);
}

void strip_cheat_flags() {
  if (!is_client())
    return;

  dvar_remove_flags("cg_drawGun", DVAR_CHEAT);
  dvar_remove_flags("g_speed", DVAR_CHEAT);
  dvar_remove_flags("bg_gravity", DVAR_CHEAT);
  dvar_remove_flags("player_sustainAmmo", DVAR_CHEAT);
  dvar_remove_flags("r_fog", DVAR_CHEAT);
  dvar_remove_flags("timescale", DVAR_CHEAT);
}

void dof_enabled_stub(utils::hook::assembler &a) {
  const auto update_ads_dof = a.newLabel();

  a.mov(rax, qword_ptr(0x14AE95478_g)); // r_dof_enable

  a.test(rax, rax);
  a.jz(update_ads_dof);

  a.cmp(byte_ptr(rax, 0x28), 1);

  a.je(update_ads_dof);

  a.jmp(0x141116ECB_g);

  a.bind(update_ads_dof);
  a.lea(rdx, ptr(rbx, 0x131EB4));
  a.jmp(0x141116EC2_g); // CG_UpdateAdsDof
}

void dvar_disablebool_cb(EngineDependentDvar dvar) {
  if (dvar.get_bool()) {
    dvar.set(false);
  }
}

EngineDependentDvarMut Dvar_RegisterDisable_Bool(dvarStrHash_t hash,
                                                 const char *dvarName,
                                                 [[maybe_unused]] bool value,
                                                 DvarFlags flags,
                                                 const char *description) {
  const EngineDependentDvarMut dvar =
      Dvar_RegisterBool(hash, dvarName, false, flags, description);

  Dvar_SetModifiedCallback(dvar, dvar_disablebool_cb);

  return dvar;
}
EngineDependentDvarMut Dvar_RegisterDisable_Bool_Inlined(
    dvarStrHash_t hash, const char *dvarName, dvarType_t type, DvarFlags flags,
    DvarValue *value, DvarLimits *domain, const char *description,
    bool isSessionModeDvar) {
  value->enabled() = false;
  const EngineDependentDvarMut dvar =
      Dvar_RegisterVariant(hash, dvarName, type, flags, value, domain,
                           description, isSessionModeDvar);
  Dvar_SetModifiedCallback(dvar, dvar_disablebool_cb);
  return dvar;
}

inline void disable_sv_cheats() {
  /*
     1. sv_cheats used to enable/disable cheat commands - both in console
     and in SV commands.
  */
  {
    // R_RegisterDvars
    utils::hook::call(0x140379E80_g, Dvar_RegisterDisable_Bool);
    // SV_Init
    utils::hook::call(0x140534DF2_g, Dvar_RegisterDisable_Bool);
  }
  /*
     2. sv_cheats used to enable/disable cheat dvars - controls whether cheat
     protection on a dvar to be modified is checked and respected in internal
     setters.
     Global is named `dvar_cheats` in engine.
     This is the one that GSC scripts can modify. If not for this hook, anyway.
  */
  {
    // Dvar_Init
    utils::hook::call(0x1405767F5_g, Dvar_RegisterDisable_Bool_Inlined);
  }
}

inline constexpr const char *serialize(eModes mode) {
  switch (mode) {
  case eModes::ZOMBIES:
    return "MODE_ZOMBIES";
  case eModes::MULTIPLAYER:
    return "MODE_MULTIPLAYER";
  case eModes::CAMPAIGN:
    return "MODE_CAMPAIGN";
  default:
    return "MODE_INVALID";
  }
}

utils::hook::detour Dvar_GetSessionModeSpecificDvarInternal_hook;
EngineDependentDvar Dvar_GetSessionModeSpecificDvarInternal_FallbackDefault(
    EngineDependentDvar base, eModes modeArg) {

  eModes mode = modeArg;
  if (mode == eModes::INVALID && com::Com_IsRunningUILevel()) {
    mode = eModes::MULTIPLAYER;
  }
  switch (mode) {
  case eModes::ZOMBIES:
  case eModes::MULTIPLAYER:
  case eModes::CAMPAIGN: {
    EngineDependentDvar resolved = base.sessionModeSpecific(mode);

    // Try to get sessionmode-specific dvar for _current_ mode.
    // Internally, this falls back to the base dvar if the sessionmode-specific
    // dvar for the current mode is a nullptr - just as the engine does.
    if (!resolved) {
      resolved = base.resolve();
    }

    return resolved;
  }
  default: {
    const char *debugName = base.debugName();
    const char *name =
        debugName ? debugName
                  : utils::string::va("UNKNOWN(hash: 0x%X)", base.name());
    com::Com_Printf(0, consoleLabel_e::DEFAULT,
                    "Warning: Sessionmode not set while attempting to get "
                    "sessionmode specific dvar for mode: %s from base dvar : "
                    "\"%s\". Falling back to "
                    "first available sessionmode-specific dvar.",
                    serialize(mode), name);
    const SessionModePool<EngineDependentDvar> &sessionModeSpecificDvars =
        base.indirect();
    for (eModes sessionMode = eModes::FIRST; sessionMode < eModes::COUNT;
         ++sessionMode) {
      if (sessionModeSpecificDvars[sessionMode]) {
        return sessionModeSpecificDvars[sessionMode];
      }
    }
    com::Com_Printf(
        0, consoleLabel_e::DEFAULT,
        "Warning: Sessionmode not set while attempting to get "
        "sessionmode specific dvar for mode: %s from base dvar : \"%s\", and "
        "none of the "
        "sessionmode-specific dvars were available. Returning base dvar.",
        serialize(mode), name);
    return base.resolve();
  }
  }
}
} // namespace

class component final : public generic_component {
public:
  void post_unpack() override {
    scheduler::once(patch_dvars, scheduler::pipeline::main);
    scheduler::once(patch_flags, scheduler::pipeline::main);
    scheduler::loop(strip_cheat_flags, scheduler::pipeline::main, 5s);
    Dvar_GetSessionModeSpecificDvarInternal_hook.create(
        game::Dvar_GetSessionModeSpecificDvarInternal.get(),
        Dvar_GetSessionModeSpecificDvarInternal_FallbackDefault);

    if (game::is_client())
      this->patch_client();
    else
      this->patch_server();
  }

  static void patch_client() {
    // toggle ADS dof based on r_dof_enable
    utils::hook::jump(0x141116EBB_g, utils::hook::assemble(dof_enabled_stub));
  }

  static void patch_server() {
    // Set the max value of 'sv_network_fps'
    utils::hook::set<uint32_t>(0x140534FE7_g, 1000);

    // Set the flag of 'sv_network_fps'
    utils::hook::set<uint32_t>(0x140534FD8_g, game::DVAR_NONE);

    // Disable both (??) sv_cheats dvars immediately after registration
    disable_sv_cheats();
  }
};
} // namespace dvars_patches

REGISTER_COMPONENT(dvars_patches::component)