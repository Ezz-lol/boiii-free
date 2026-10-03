#include <std_include.hpp>

#include "dvars.hpp"
#include "scheduler.hpp"

#include <game/game.hpp>
#include <game/utils.hpp>
#include <loader/component_loader.hpp>

#include <utils/flags.hpp>
#include <utils/io.hpp>

namespace settings {
namespace {
struct flag_setting {
  const char *dvar_name;
  const char *flag;
  const char *description;
  mutable game::EngineDependentDvarMut dvar{};

  bool enabled() const {
    return dvar ? dvar.get_bool() : utils::flags::has_flag(flag);
  }
};

flag_setting skip_intro{"boiii_skipIntro", "nointro",
                        "Skip the intro video on startup"};
flag_setting skip_cinematics{"boiii_skipCinematics", "nocinematics",
                             "Skip all cinematics and videos"};
flag_setting skip_forest_cinematic{
    "boiii_skipForestCinematic", "noforestcinematic",
    "Skip the forest video that plays before joining a Zombies game"};
flag_setting full_logs{"boiii_fullLogs", "fulllogs",
                       "Show every log message in the console"};
flag_setting log_script_errors{
    "boiii_logScriptErrors", "log-script-errors",
    "Log every script error, including non-fatal ones"};
flag_setting ultrawide{
    "boiii_ultrawide", "ultrawide",
    "Use the full width of ultrawide screens (requires a restart)"};
flag_setting allow_unsafe_lua{
    "boiii_allowUnsafeLua", "unsafe-lua",
    "Allow mods to use unsafe Lua functions without asking"};

const std::array<flag_setting *, 7> all_settings{
    &skip_intro,        &skip_cinematics, &skip_forest_cinematic, &full_logs,
    &log_script_errors, &ultrawide,       &allow_unsafe_lua,
};

void apply_saved_values() {
  std::string config;
  if (utils::io::read_file(dvars::CONFIG_FILE_PATH, &config)) {
    for (flag_setting *setting : all_settings) {
      const std::string prefix = std::format("set {} \"", setting->dvar_name);
      const size_t start = config.find(prefix);
      if (start != std::string::npos) {
        setting->dvar.set(config[start + prefix.size()] == '1');
      }
    }
  }
}

void apply_flags() {
  for (flag_setting *setting : all_settings) {
    if (setting->dvar && utils::flags::has_flag(setting->flag)) {
      setting->dvar.set(true);
    }
  }
}
} // namespace

class component final : public client_component {
#ifndef NDEBUG
  std::string name() override { return "settings"; }
#endif

public:
  void post_unpack() override {
    for (flag_setting *setting : all_settings) {
      setting->dvar = game::register_dvar_bool(
          setting->dvar_name, false, game::DVAR_ARCHIVE, setting->description);
    }

    apply_saved_values();
    apply_flags();
    scheduler::once(apply_flags, scheduler::pipeline::dvars_loaded);
  }
};
} // namespace settings

namespace game {
bool skip_intro() { return settings::skip_intro.enabled(); }
bool skip_cinematics() { return settings::skip_cinematics.enabled(); }
bool skip_forest_cinematic() {
  return settings::skip_forest_cinematic.enabled();
}
bool full_logs() { return settings::full_logs.enabled(); }
bool log_script_errors() { return settings::log_script_errors.enabled(); }
bool ultrawide() { return settings::ultrawide.enabled(); }
bool allow_unsafe_lua() { return settings::allow_unsafe_lua.enabled(); }
} // namespace game

REGISTER_COMPONENT(settings::component)
