#include <std_include.hpp>

#include "scheduler.hpp"

#include <game/utils.hpp>
#include <loader/component_loader.hpp>

#include <utils/flags.hpp>
#include <utils/hook.hpp>
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

// The engine's XP scale dvars aren't saved, so these keep the player's choice
EngineDependentDvarMut boiii_xpScale;
EngineDependentDvarMut boiii_gunXpScale;

void apply_xp_scales() {
  const auto xp_scale = static_cast<float>(boiii_xpScale.get_int());
  scr_xpscalemp->set(xp_scale);
  scr_xpscalezm->set(xp_scale);

  const auto gun_xp_scale = static_cast<float>(boiii_gunXpScale.get_int());
  scr_gunxpscalemp->set(gun_xp_scale);
  scr_gunxpscalezm->set(gun_xp_scale);
}

void handle_xp_scale_change([[maybe_unused]] EngineDependentDvarMut dvar) {
  apply_xp_scales();
}

utils::hook::detour playlist_run_rules_hook;

// Starting a match resets the XP scale dvars from the playlist's configs
bool playlist_run_rules_stub(const ControllerIndex_t controller_index,
                             const int32_t playlist_id) {
  const bool result =
      playlist_run_rules_hook.invoke<bool>(controller_index, playlist_id);
  apply_xp_scales();
  return result;
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
        "boiii_xpBarColor", 0, 0, 7, game::dvarFlags_e::ARCHIVE,
        "XP bar color: 0 = orange, 1 = gold, 2 = blue, 3 = green, 4 = red, "
        "5 = purple, 6 = white, 7 = automatic by progress");
    game::register_dvar_int(
        "boiii_loadingScreen", 0, 0, 2, game::dvarFlags_e::ARCHIVE,
        "Loading screen: 0 = default, 1 = map image, 2 = cinematic");
    game::register_dvar_bool(
        "boiii_advancedBots", false, game::dvarFlags_e::ARCHIVE,
        "Multiplayer bots use the advanced AI from custom_bots.gsc");

    boiii_xpScale = game::register_dvar_int(
        "boiii_xpScale", 1, 1, 3, game::dvarFlags_e::ARCHIVE,
        "XP scale for matches you host in Multiplayer and Zombies");
    boiii_gunXpScale = game::register_dvar_int(
        "boiii_gunXpScale", 1, 1, 3, game::dvarFlags_e::ARCHIVE,
        "Weapon XP scale for matches you host in Multiplayer and Zombies");
    playlist_run_rules_hook.create(game::playlist::Playlist_RunRules.get(),
                                   playlist_run_rules_stub);

    apply_flags();
    scheduler::once(
        [] {
          apply_flags();
          apply_xp_scales();
          game::Dvar_SetModifiedCallback(boiii_xpScale, handle_xp_scale_change);
          game::Dvar_SetModifiedCallback(boiii_gunXpScale,
                                         handle_xp_scale_change);
        },
        scheduler::pipeline::dvars_loaded);
  }
};
} // namespace settings

REGISTER_COMPONENT(settings::component)
