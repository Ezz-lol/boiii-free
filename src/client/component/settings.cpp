#include <std_include.hpp>

#include "dvars.hpp"
#include "scheduler.hpp"
#include "settings.hpp"

#include <game/game.hpp>
#include <game/utils.hpp>
#include <loader/component_loader.hpp>

#include <utils/flags.hpp>
#include <utils/io.hpp>

namespace settings {
namespace {
std::vector<flag_setting *> &registered_settings() {
  static std::vector<flag_setting *> list;
  return list;
}

void apply_saved_values() {
  std::string config;
  if (utils::io::read_file(dvars::CONFIG_FILE_PATH, &config)) {
    for (flag_setting *setting : registered_settings()) {
      const std::string prefix = std::format("set {} \"", setting->dvar_name);
      const size_t start = config.find(prefix);
      if (start != std::string::npos) {
        setting->dvar.set(config[start + prefix.size()] == '1');
      }
    }
  }
}

void apply_flags() {
  for (flag_setting *setting : registered_settings()) {
    if (setting->dvar && utils::flags::has_flag(setting->flag)) {
      setting->dvar.set(setting->flag_value);
    }
  }
}
} // namespace

flag_setting::flag_setting(const char *dvar_name, const char *flag,
                           const bool flag_value, const bool default_value,
                           const char *description)
    : dvar_name(dvar_name), flag(flag), flag_value(flag_value),
      default_value(default_value), description(description) {
  registered_settings().push_back(this);
}

bool flag_setting::enabled() const {
  if (dvar) {
    return dvar.get_bool();
  }
  return utils::flags::has_flag(flag) ? flag_value : default_value;
}

class component final : public client_component {
#ifndef NDEBUG
  std::string name() override { return "settings"; }
#endif

public:
  void post_unpack() override {
    for (flag_setting *setting : registered_settings()) {
      setting->dvar =
          game::register_dvar_bool(setting->dvar_name, setting->default_value,
                                   game::DVAR_ARCHIVE, setting->description);
    }

    apply_saved_values();
    apply_flags();
    scheduler::once(apply_flags, scheduler::pipeline::dvars_loaded);
  }
};
} // namespace settings

REGISTER_COMPONENT(settings::component)
