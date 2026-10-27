#include <std_include.hpp>

#include "scheduler.hpp"

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

    game::register_dvar_int("boiii_xpBarPosition", 0, 0, 1, game::DVAR_ARCHIVE,
                            "Where the XP bar sits: 0 = bottom, 1 = top");
    game::register_dvar_bool(
        "boiii_xpBarPopups", true, game::DVAR_ARCHIVE,
        "Show a popup with the XP earned next to the XP bar");
    game::register_dvar_int(
        "boiii_xpBarColor", 0, 0, 6, game::DVAR_ARCHIVE,
        "XP bar color: 0 = orange, 1 = gold, 2 = blue, 3 = green, 4 = red, "
        "5 = purple, 6 = white");
    game::register_dvar_bool(
        "boiii_advancedBots", false, game::DVAR_ARCHIVE,
        "Multiplayer bots use the advanced AI from custom_bots.gsc");

    apply_flags();
    scheduler::once(apply_flags, scheduler::pipeline::dvars_loaded);
  }
};
} // namespace settings

REGISTER_COMPONENT(settings::component)
