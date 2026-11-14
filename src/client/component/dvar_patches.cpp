#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include "command.hpp"
#include "scheduler.hpp"

#include <game/utils.hpp>

#include <utils/flags.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace dvars_patches {
namespace {

using namespace game;
template <const bool Value>
void dvar_bool_modification_force(EngineDependentDvarMut dvar) {
  if (dvar.get_bool() != Value) {
    dvar.set(Value);
  }
}

template <const int32_t Value>
inline void dvar_int_modification_force(EngineDependentDvarMut dvar) {
  if (dvar.get_int() != Value) {
    dvar.set(Value);
  }
}

#ifndef NDEBUG
template <const int32_t Value>
inline void dvar_int_force(EngineDependentDvarMut dvar) {
  dvar.set(Value);
  Dvar_SetModifiedCallback(dvar, dvar_int_modification_force<Value>);
}
#endif

#ifndef NDEBUG
template <const ConstString Value>
void dvar_boolstring_modification_force(EngineDependentDvarMut dvar) {
  if (dvar.get_string().value_or("0") != Value) {
    dvar.set(Value);
  }
}

template <const ConstString Value>
inline void dvar_boolstring_force(EngineDependentDvarMut dvar) {
  dvar.set(Value);
  Dvar_SetModifiedCallback(dvar, dvar_boolstring_modification_force<Value>);
}
#endif

template <const bool Value>
inline void dvar_bool_force(EngineDependentDvarMut dvar) {
  Dvar_SetModifiedCallback(dvar, dvar_bool_modification_force<Value>);
  dvar.set(Value);
}

void apply_cheats_setting(EngineDependentDvarMut) {
  for (EngineDependentDvarMut dvar : {*sv_cheats, *dvar_cheats}) {
    if (game::cheats()) {
      dvar_bool_force<true>(dvar);
    } else {
      dvar_bool_force<false>(dvar);
    }
  }
}

#ifndef NDEBUG
inline bool enable_debug_dvars() {

  if (utils::flags::has_flag("vehicle-debug")) {
    static constexpr game::symbol<game::EngineDependentDvarMut> dependencies[] =
        {game::g_vehicleDebug, game::g_vehicleDrawSplines,
         game::g_vehicleDrawPath, game::com_clientfieldsdebug};
    for (const game::symbol<game::EngineDependentDvarMut> &dependency :
         dependencies) {
      if (!*dependency.get()) {
        return scheduler::cond_continue;
      }
    }

    dvar_boolstring_force<"1">(*game::g_vehicleDrawPath);
    dvar_bool_force<true>(*game::g_vehicleDrawSplines);
    dvar_int_force<1>(*game::g_vehicleDebug);
  }
  if (utils::flags::has_flag("lobby-debug")) {

    static constexpr game::symbol<game::EngineDependentDvarMut> dependencies[] =
        {game::lobby_showSVDebug,        game::lobbyDebugLogJoins,
         game::lobbyDebugLogJoinSuccess, game::lobbymsg_prints,
         game::ui_lobbyDebugVis,         game::ui_lobbyDebugOverlay,
         game::ui_lobbyDebugSessionSQJ,  game::ui_lobbyDebugVoice};
    for (const game::symbol<game::EngineDependentDvarMut> &dependency :
         dependencies) {
      if (!*dependency.get()) {
        return scheduler::cond_continue;
      }
    }
    dvar_bool_force<true>(*game::lobby_showSVDebug);
    dvar_bool_force<true>(*game::lobbyDebugLogJoins);
    dvar_bool_force<true>(*game::lobbyDebugLogJoinSuccess);
    dvar_bool_force<true>(*game::lobbymsg_prints);
    // 0 - 3
    dvar_int_force<3>(*game::ui_lobbyDebugVis);
    dvar_bool_force<true>(*game::ui_lobbyDebugOverlay);
    dvar_bool_force<true>(*game::ui_lobbyDebugSessionSQJ);
    dvar_bool_force<true>(*game::ui_lobbyDebugVoice);
  }

  static constexpr game::symbol<game::EngineDependentDvarMut> dependencies[] = {
      game::com_clientfieldsdebug};
  for (const game::symbol<game::EngineDependentDvarMut> &dependency :
       dependencies) {
    if (!*dependency.get()) {
      return scheduler::cond_continue;
    }
  }

  dvar_bool_force<true>(*game::com_clientfieldsdebug);
  return scheduler::cond_end;
}
#endif

void patch_dvars() {
  com_pauseSupported = register_sessionmode_dvar_bool(
      "com_pauseSupported", !is_server(), game::dvarFlags_e::SERVERINFO,
      "Whether pause is supported by the game mode");
}

void patch_flags() {
  if (is_client()) {
    dvar_set_flags("r_dof_enable", game::dvarFlags_e::ARCHIVE);
    dvar_set_flags("r_lodbiasrigid", game::dvarFlags_e::ARCHIVE);
    dvar_set_flags("r_modellodbias", game::dvarFlags_e::ARCHIVE);
    dvar_set_flags("gpad_stick_deadzone_max", game::dvarFlags_e::ARCHIVE);
    dvar_set_flags("gpad_stick_deadzone_min", game::dvarFlags_e::ARCHIVE);
    dvar_set_flags("cg_drawLagometer", game::dvarFlags_e::ARCHIVE);
  }

  scheduler::execute(scheduler::pipeline::dvars_flags_patched);
}

void strip_cheat_flags() {
  if (is_client()) {
    dvar_remove_flags("cg_drawGun", game::dvarFlags_e::CHEAT);
    dvar_remove_flags("g_speed", game::dvarFlags_e::CHEAT);
    dvar_remove_flags("bg_gravity", game::dvarFlags_e::CHEAT);
    dvar_remove_flags("player_sustainAmmo", game::dvarFlags_e::CHEAT);
    dvar_remove_flags("r_fog", game::dvarFlags_e::CHEAT);
    dvar_remove_flags("timescale", game::dvarFlags_e::CHEAT);
    dvar_remove_flags("r_lodbiasrigid", game::dvarFlags_e::CHEAT);
    dvar_remove_flags("r_modellodbias", game::dvarFlags_e::CHEAT);
  }
}

void dof_enabled_stub(utils::hook::assembler &a) {
  const asmjit::Label update_ads_dof = a.get().new_label();

  a.get().mov(rax, qword_ptr(reinterpret_cast<uintptr_t>(r_dof_enable.get())));

  a.get().test(rax, rax);
  a.get().jz(update_ads_dof);

  a.get().cmp(byte_ptr(rax, 0x28), 1);

  a.get().je(update_ads_dof);

  a.jmp(game::select(0x141116eeb, 0x0));

  a.get().bind(update_ads_dof);
  a.get().lea(rdx, ptr(rbx, 0x131EB4));
  a.jmp(game::select(0x141116ee2, 0x0)); // CG_UpdateAdsDof
}

template <const bool Value>
EngineDependentDvarMut
Dvar_RegisterBool_Force(dvarStrHash_t hash, const char *dvarName,
                        [[maybe_unused]] bool value, DvarFlags flags,
                        const char *description) {
  flags.archive = false;
  const EngineDependentDvarMut dvar =
      Dvar_RegisterBool(hash, dvarName, Value, flags, description);

  Dvar_SetModifiedCallback(dvar, dvar_bool_modification_force<Value>);

  return dvar;
}
template <const bool Value>
EngineDependentDvarMut Dvar_RegisterBool_Inlined_Force(
    dvarStrHash_t hash, const char *dvarName, dvarType_t type, DvarFlags flags,
    DvarValue *value, DvarLimits *domain, const char *description,
    bool isSessionModeDvar) {
  value->enabled() = Value;
  flags.archive = false;
  const EngineDependentDvarMut dvar =
      Dvar_RegisterVariant(hash, dvarName, type, flags, value, domain,
                           description, isSessionModeDvar);
  Dvar_SetModifiedCallback(dvar, dvar_bool_modification_force<Value>);
  return dvar;
}

template <const IntegralLike<int32_t> auto Value>
EngineDependentDvarMut
Dvar_RegisterInt_Force(dvarStrHash_t hash, const char *dvarName,
                       [[maybe_unused]] int32_t value, int32_t min, int32_t max,
                       DvarFlags flags, const char *description) {
  constexpr int32_t ValueInt = static_cast<int32_t>(Value);
  flags.archive = false;
  const EngineDependentDvarMut dvar =
      Dvar_RegisterInt(hash, dvarName, ValueInt, min, max, flags, description);

  Dvar_SetModifiedCallback(dvar, dvar_int_modification_force<ValueInt>);

  return dvar;
}

template <const bool Value> inline void sv_cheats_force() {
  /*
     1. sv_cheats used to enable/disable cheat commands - both in console
     and in SV commands.
  */
  {
    // R_RegisterDvars
    utils::hook::call(game::select(0x141C9A844, 0x140379E51),
                      Dvar_RegisterBool_Force<Value>);
    // SV_Init
    utils::hook::call(0x140534DD2_g, Dvar_RegisterBool_Force<Value>);
  }
  /*
     2. sv_cheats used to enable/disable cheat dvars - controls whether cheat
     protection on a dvar to be modified is checked and respected in internal
     setters.
     Global is named `dvar_cheats` in engine.
     This is the one that GSC scripts can modify, if not for this hook.
  */
  {
    // Dvar_Init
    utils::hook::call(0x1405767F5_g, Dvar_RegisterBool_Inlined_Force<Value>);
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
    // Internally, this falls back to the base dvar if the
    // sessionmode-specific dvar for the current mode is a nullptr - just as
    // the engine does.
    if (!resolved) {
      resolved = base.resolve();
    }

    return resolved;
  }
  default: {
    const char *debug_name = base.debugName();
    const char *name =
        debug_name == nullptr
            ? utils::string::va("UNKNOWN(hash: 0x%X)", base.name())
            : debug_name;
    com::Com_Printf(consoleChannel_e::CHANNEL_DONT_FILTER,
                    consoleLabel_e::DEFAULT,
                    "Warning: Sessionmode not set while attempting to get "
                    "sessionmode specific dvar for mode: %s from base dvar : "
                    "\"%s\". Falling back to "
                    "first available sessionmode-specific dvar.\n",
                    game::serialize(mode), name);
    const SessionModePool<EngineDependentDvar> &session_mode_specific_dvars =
        base.indirect();
    for (eModes session_mode = eModes::FIRST; session_mode < eModes::COUNT;
         ++session_mode) {
      if (session_mode_specific_dvars[session_mode]) {
        return session_mode_specific_dvars[session_mode];
      }
    }
    com::Com_Printf(
        consoleChannel_e::CHANNEL_DONT_FILTER, consoleLabel_e::DEFAULT,
        "Warning: Sessionmode not set while attempting to get "
        "sessionmode specific dvar for mode: %s from base dvar : \"%s\", and "
        "none of the "
        "sessionmode-specific dvars were available. Returning base dvar.\n",
        serialize(mode), name);
    return base.resolve();
  }
  }
}

float GetXPScale_Server([[maybe_unused]] ClientNum_t clientNum) {
  return xp_scale();
}

ddl::statValue_t
GScr_GetGunXPScaled_Server([[maybe_unused]] ClientNum_t clientNum,
                           ddl::statValue_t value) {
  value.intValue = lround(static_cast<float>(value.intValue) * gun_xp_scale());
  return value;
}

utils::hook::detour cmd_exec_f_hook;

// default_xboxlive.cfg resets the XP scales, and every gametype config execs
// it, so loading a gametype would undo the ones server.cfg sets
void Cmd_Exec_f_Server() {
  const command::params params{};
  if (!utils::string::ends_with(params[1], "default_xboxlive.cfg")) {
    cmd_exec_f_hook.invoke<void>();
    return;
  }

  std::array dvars{*scr_xpscalemp, *scr_xpscalezm, *scr_gunxpscalemp,
                   *scr_gunxpscalezm};
  std::array<float, dvars.size()> scales{};
  for (size_t i = 0; i < dvars.size(); ++i) {
    scales[i] = dvars[i].get_float();
  }

  cmd_exec_f_hook.invoke<void>();

  for (size_t i = 0; i < dvars.size(); ++i) {
    dvars[i].set(scales[i]);
  }
}

bool return_false() { return false; }
bool return_true() { return true; }
} // namespace

class component final : public generic_component {
  DEFINE_COMPONENT_NAME("dvar_patches");

public:
  void post_unpack() override {
    /*
      Disable error:
      "Attempt to set ClientField pre finalization of ClientField system.
      Fields cannot be set pre the script systems first wait command."
      when `com_clientfieldsdebug` is enabled. This error is intended only for
      internal Treyarch script testing to optimize clientfield modification
      timing, and is not necessary outside of hyper-optimized script
      development.

      Each of these patches is either in `BG_CheckForFieldSetPreFinalize` or in
      an inlined call to `BG_CheckForFieldSetPreFinalize`.
    */
    // BG_IncrementClientFieldCounterVal (CL) /
    // BG_CheckForFieldSetPreFinalize (SV)
    utils::hook::call(game::select(0x140134368, 0x1400577A0), return_false);
    // BG_SetClientFieldFloatVal
    utils::hook::call(game::select(0x140136DB4, 0x14005AC64), return_false);
    // BG_SetClientFieldIntVal
    utils::hook::call(game::select(0x140136E85, 0x14005AD45), return_false);

    scheduler::once(patch_dvars, scheduler::pipeline::main);
#ifndef NDEBUG
    scheduler::schedule(enable_debug_dvars, scheduler::pipeline::main);
#endif

    scheduler::once(patch_flags, scheduler::pipeline::main);
    scheduler::loop(strip_cheat_flags, scheduler::pipeline::main, 5s);
    Dvar_GetSessionModeSpecificDvarInternal_hook.create(
        game::Dvar_GetSessionModeSpecificDvarInternal.get(),
        Dvar_GetSessionModeSpecificDvarInternal_FallbackDefault);
    utils::hook::jump(game::Dvar_CanSetConfigDvar.get(), return_true);

    if (game::is_client()) {
      patch_client();
    } else {
      patch_server();
    }
  }

  static void patch_client() {

    // Disable `live_uselpc`
    utils::hook::call(game::select(0x141E00411, 0x0),
                      Dvar_RegisterBool_Force<false>);

    // toggle ADS dof based on r_dof_enable
    utils::hook::jump(game::select(0x141116edb, 0x0),
                      utils::hook::assemble(dof_enabled_stub));

    // Disable `lui_oom`
    utils::hook::call(0x142231BF0_g, Dvar_RegisterInt_Force<0>);

    scheduler::schedule(
        []() {
          if (*sv_cheats && *dvar_cheats) {
            apply_cheats_setting({});
            Dvar_SetModifiedCallback(Dvar_FindVar("boiii_allowCheats"),
                                     apply_cheats_setting);
            return scheduler::cond_end;
          }
          return scheduler::cond_continue;
        },
        scheduler::pipeline::main);
  }

  static void patch_server() {
    // Set the max value of 'sv_network_fps'
    utils::hook::set<uint32_t>(0x140534FE7_g, 1000);

    // Set the flag of 'sv_network_fps'
    utils::hook::set<dvarFlags_e>(0x140534FD8_g, game::dvarFlags_e::NONE);

    // Disable `sv_pure`
    utils::hook::call(0x140534DF2_g, Dvar_RegisterBool_Force<false>);

    utils::hook::jump(game::scr::gscr::GetXPScale.get(), GetXPScale_Server);
    utils::hook::jump(game::scr::gscr::GScr_GetGunXPScaled.get(),
                      GScr_GetGunXPScaled_Server);
    cmd_exec_f_hook.create(game::cmd::Cmd_Exec_f.get(), Cmd_Exec_f_Server);

    // Enable or disable both (??) sv_cheats dvars immediately after
    // registration
    if (game::cheats()) {
      sv_cheats_force<true>();
    } else {
      sv_cheats_force<false>();
    }
  }
};
} // namespace dvars_patches

REGISTER_COMPONENT(dvars_patches::component)
