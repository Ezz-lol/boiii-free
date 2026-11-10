#pragma once

#include "structs/structs.hpp"
#include <utils/flags.hpp>

namespace game {
struct flag_setting {
  const char *dvar_name;
  const char *flag;
  const char *description;
  mutable EngineDependentDvarMut dvar{};

  alwaysinline bool enabled() const noexcept {
    return dvar ? dvar.get_bool() : utils::flags::has_flag(flag);
  }
};

inline constinit flag_setting skip_intro_flag{
    .dvar_name = "boiii_skipIntro",
    .flag = "nointro",
    .description = "Skip the intro video on startup"};
inline constinit flag_setting skip_cinematics_flag{
    .dvar_name = "boiii_skipCinematics",
    .flag = "nocinematics",
    .description = "Skip all cinematics and videos"};
inline constinit flag_setting skip_forest_cinematic_flag{
    .dvar_name = "boiii_skipForestCinematic",
    .flag = "noforestcinematic",
    .description =
        "Skip the forest video that plays before joining a Zombies game"};
inline constinit flag_setting full_logs_flag{
    .dvar_name = "boiii_fullLogs",
    .flag = "fulllogs",
    .description = "Show every log message in the console"};
inline constinit flag_setting log_script_errors_flag{
    .dvar_name = "boiii_logScriptErrors",
    .flag = "log-script-errors",
    .description = "Log every script error, including non-fatal ones"};
inline constinit flag_setting script_debug_flag{
    .dvar_name = "boiii_scriptDebug",
    .flag = "script-debug",
    .description = "Add runtime diagnostics for common and programmatically "
                   "detectable erroneous or poorly optimised GSC logic."};
inline constinit flag_setting ultrawide_flag{
    .dvar_name = "boiii_ultrawide",
    .flag = "ultrawide",
    .description =
        "Use the full width of ultrawide screens (requires a restart)"};
inline constinit flag_setting allow_cheats_flag{
    .dvar_name = "boiii_allowCheats",
    .flag = "cheats",
    .description = "Allow cheat commands like noclip and god mode"};
inline constinit flag_setting xp_bar_flag{
    .dvar_name = "boiii_xpBar",
    .flag = "xpbar",
    .description = "Show an XP progress bar in Multiplayer and Zombies"};

inline constexpr flag_setting *ALL_SETTINGS[] = {
    &skip_intro_flag, &skip_cinematics_flag,   &skip_forest_cinematic_flag,
    &full_logs_flag,  &log_script_errors_flag, &script_debug_flag,
    &ultrawide_flag,  &allow_cheats_flag,      &xp_bar_flag,
};

inline bool skip_intro() { return skip_intro_flag.enabled(); }
inline bool skip_cinematics() { return skip_cinematics_flag.enabled(); }
inline bool skip_forest_cinematic() {
  return skip_forest_cinematic_flag.enabled();
}
inline bool full_logs() { return full_logs_flag.enabled(); }
inline bool log_script_errors() { return log_script_errors_flag.enabled(); }
inline bool script_debug() { return script_debug_flag.enabled(); }
inline bool ultrawide() { return ultrawide_flag.enabled(); }
inline bool cheats() { return allow_cheats_flag.enabled(); }
} // namespace game
