#pragma once

#include "structs/structs.hpp"
#include <utils/flags.hpp>

namespace game {
struct flag_setting {
  const char *dvar_name;
  const char *flag;
  const char *description;
  mutable EngineDependentDvarMut dvar{};

  inline bool enabled() const noexcept {
    return dvar ? dvar.get_bool() : utils::flags::has_flag(flag);
  }
};

static constinit flag_setting skip_intro_flag{
    "boiii_skipIntro", "nointro", "Skip the intro video on startup"};
static constinit flag_setting skip_cinematics_flag{
    "boiii_skipCinematics", "nocinematics", "Skip all cinematics and videos"};
static constinit flag_setting skip_forest_cinematic_flag{
    "boiii_skipForestCinematic", "noforestcinematic",
    "Skip the forest video that plays before joining a Zombies game"};
static constinit flag_setting full_logs_flag{
    "boiii_fullLogs", "fulllogs", "Show every log message in the console"};
static constinit flag_setting log_script_errors_flag{
    "boiii_logScriptErrors", "log-script-errors",
    "Log every script error, including non-fatal ones"};
static constinit flag_setting ultrawide_flag{
    "boiii_ultrawide", "ultrawide",
    "Use the full width of ultrawide screens (requires a restart)"};
static constinit flag_setting allow_cheats_flag{
    "boiii_allowCheats", "cheats",
    "Allow cheat commands like noclip and god mode"};

constexpr flag_setting *all_settings[] = {
    &skip_intro_flag,   &skip_cinematics_flag,   &skip_forest_cinematic_flag,
    &full_logs_flag,    &log_script_errors_flag, &ultrawide_flag,
    &allow_cheats_flag,
};

inline bool skip_intro() { return skip_intro_flag.enabled(); }
inline bool skip_cinematics() { return skip_cinematics_flag.enabled(); }
inline bool skip_forest_cinematic() {
  return skip_forest_cinematic_flag.enabled();
}
inline bool full_logs() { return full_logs_flag.enabled(); }
inline bool log_script_errors() { return log_script_errors_flag.enabled(); }
inline bool ultrawide() { return ultrawide_flag.enabled(); }
inline bool cheats() { return allow_cheats_flag.enabled(); }
} // namespace game
