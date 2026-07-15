#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include <game/game.hpp>
#include <game/utils.hpp>
#include "scheduler.hpp"

#include <utils/hook.hpp>

namespace dvars_patches {
namespace {
void patch_dvars() {
  game::com_pauseSupported = game::register_sessionmode_dvar_bool(
      "com_pauseSupported", !game::is_server(), game::DVAR_SERVERINFO,
      "Whether pause is supported by the game mode");
}

void patch_flags() {
  if (game::is_client()) {
    game::dvar_set_flags("r_dof_enable", game::DVAR_ARCHIVE);
    game::dvar_set_flags("r_lodbiasrigid", game::DVAR_ARCHIVE);
    game::dvar_set_flags("gpad_stick_deadzone_max", game::DVAR_ARCHIVE);
    game::dvar_set_flags("gpad_stick_deadzone_min", game::DVAR_ARCHIVE);
    game::dvar_set_flags("cg_drawLagometer", game::DVAR_ARCHIVE);
  }

  scheduler::execute(scheduler::pipeline::dvars_flags_patched);
}

void strip_cheat_flags() {
  if (!game::is_client())
    return;

  game::dvar_remove_flags("cg_drawGun", game::DVAR_CHEAT);
  game::dvar_remove_flags("g_speed", game::DVAR_CHEAT);
  game::dvar_remove_flags("bg_gravity", game::DVAR_CHEAT);
  game::dvar_remove_flags("player_sustainAmmo", game::DVAR_CHEAT);
  game::dvar_remove_flags("r_fog", game::DVAR_CHEAT);
  game::dvar_remove_flags("timescale", game::DVAR_CHEAT);
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

void dvar_disablebool_cb(game::EngineDependentDvar dvar) {
  if (dvar.get_bool()) {
    dvar.set(false);
  }
}

game::EngineDependentDvarMut
Dvar_RegisterDisable_Bool(game::dvarStrHash_t hash, const char *dvarName,
                          [[maybe_unused]] bool value, game::DvarFlags flags,
                          const char *description) {
  const game::EngineDependentDvarMut dvar =
      game::Dvar_RegisterBool(hash, dvarName, false, flags, description);

  game::Dvar_SetModifiedCallback(dvar, dvar_disablebool_cb);

  return dvar;
}
game::EngineDependentDvarMut Dvar_RegisterDisable_Bool_Inlined(
    game::dvarStrHash_t hash, const char *dvarName, game::dvarType_t type,
    game::DvarFlags flags, game::DvarValue *value, game::DvarLimits *domain,
    const char *description, bool isSessionModeDvar) {
  value->enabled() = false;
  const game::EngineDependentDvarMut dvar =
      game::Dvar_RegisterVariant(hash, dvarName, type, flags, value, domain,
                                 description, isSessionModeDvar);
  game::Dvar_SetModifiedCallback(dvar, dvar_disablebool_cb);
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
} // namespace

class component final : public generic_component {
public:
  void post_unpack() override {
    scheduler::once(patch_dvars, scheduler::pipeline::main);
    scheduler::once(patch_flags, scheduler::pipeline::main);
    scheduler::loop(strip_cheat_flags, scheduler::pipeline::main, 5s);

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