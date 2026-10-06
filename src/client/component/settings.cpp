#include <std_include.hpp>

#include "scheduler.hpp"

#include <game/game.hpp>
#include <game/utils.hpp>
#include <loader/component_loader.hpp>

#include <utils/flags.hpp>
#include <utils/io.hpp>

namespace settings {
using namespace game;
namespace {

void apply_flags() {
  for (flag_setting *setting : all_settings) {
    if (setting->dvar && utils::flags::has_flag(setting->flag)) {
      setting->dvar.set(true);
    }
  }
}
} // namespace

class component final : public client_component {
  DEFINE_COMPONENT_NAME("settings");

public:
  void post_unpack() override {
    for (flag_setting *setting : all_settings) {
      setting->dvar = game::register_dvar_bool(
          setting->dvar_name, false, game::DVAR_ARCHIVE, setting->description);
    }

    apply_flags();
    scheduler::once(apply_flags, scheduler::pipeline::dvars_loaded);
  }
};
} // namespace settings

REGISTER_COMPONENT(settings::component)
