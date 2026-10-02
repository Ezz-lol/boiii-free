#include <std_include.hpp>

#include <game/game.hpp>
#include <loader/component_loader.hpp>

#include "settings.hpp"
#include <utils/hook.hpp>

namespace intro {
namespace {
utils::hook::detour cinematic_start_playback_hook;

settings::flag_setting skip_intro{"ezz_skipIntro", "nointro", true, false,
                                  "Skip the intro video on startup"};
settings::flag_setting skip_cinematics{"ezz_skipCinematics", "nocinematics",
                                       true, false,
                                       "Skip all cinematics and videos"};
settings::flag_setting skip_forest{
    "ezz_skipForestCinematic", "noforestcinematic", true, false,
    "Skip the forest video that plays before joining a Zombies game"};

constexpr std::string_view LOGOSEQUENCE_CINEMATIC_NAME =
    "BO3_Global_Logo_LogoSequence";
constexpr std::string_view FOREST_CINEMATIC_NAME = "zm_frontend_load";

bool should_skip(const std::string_view name) {
  return skip_cinematics.enabled() ||
         (skip_intro.enabled() && name == LOGOSEQUENCE_CINEMATIC_NAME) ||
         (skip_forest.enabled() && name.starts_with(FOREST_CINEMATIC_NAME));
}

void cinematic_start_playback_stub(const char *name, const char *key,
                                   const unsigned int playback_flags,
                                   const float volume, void *callback_info,
                                   const int id) {
  if (name && should_skip(name)) {
    return;
  }
  cinematic_start_playback_hook.invoke(name, key, playback_flags, volume,
                                       callback_info, id);
}
} // namespace

class component final : public client_component {
#ifndef NDEBUG
  std::string name() override { return "intro"; }
#endif

public:
  void post_unpack() override {
    cinematic_start_playback_hook.create(
        game::cinematic::Cinematic_StartPlayback,
        cinematic_start_playback_stub);
  }
};
} // namespace intro

REGISTER_COMPONENT(intro::component)
