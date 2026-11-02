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
  for (flag_setting *setting : ALL_SETTINGS) {
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
    for (flag_setting *setting : ALL_SETTINGS) {
      setting->dvar = game::register_dvar_bool(setting->dvar_name, false,
                                               game::dvarFlags_e::ARCHIVE,
                                               setting->description);
    }

    game::register_dvar_int("boiii_xpBarPosition", 0, 0, 1,
                            game::dvarFlags_e::ARCHIVE,
                            "Where the XP bar sits: 0 = bottom, 1 = top");
    game::register_dvar_bool(
        "boiii_xpBarPopups", true, game::dvarFlags_e::ARCHIVE,
        "Show a popup with the XP earned next to the XP bar");
    game::register_dvar_int(
        "boiii_xpBarColor", 0, 0, 6, game::dvarFlags_e::ARCHIVE,
        "XP bar color: 0 = orange, 1 = gold, 2 = blue, 3 = green, 4 = red, "
        "5 = purple, 6 = white");
    game::register_dvar_int(
        "boiii_loadingScreen", 0, 0, 2, game::dvarFlags_e::ARCHIVE,
        "Loading screen: 0 = default, 1 = map image, 2 = cinematic");
    game::register_dvar_bool(
        "boiii_advancedBots", false, game::dvarFlags_e::ARCHIVE,
        "Multiplayer bots use the advanced AI from custom_bots.gsc");

    apply_flags();
    scheduler::once(apply_flags, scheduler::pipeline::dvars_loaded);
  }
};
} // namespace settings

REGISTER_COMPONENT(settings::component)
