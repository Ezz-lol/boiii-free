#include <std_include.hpp>
#include <loader/component_loader.hpp>
#include <game/game.hpp>

#include <utils/hook.hpp>
#include <utils/flags.hpp>

namespace intro {
namespace {
utils::hook::detour cinematic_start_playback_hook;

static bool disable_all_cinematics = false;
static bool no_intro = false;
constexpr char LOGOSEQUENCE_CINEMATIC_NAME[] = "BO3_Global_Logo_LogoSequence";
void cinematic_start_playback_stub(const char *name, const char *key,
                                   const unsigned int playback_flags,
                                   const float volume, void *callback_info,
                                   const int id) {
  if (!disable_all_cinematics &&
      (!no_intro || strcmp(name, LOGOSEQUENCE_CINEMATIC_NAME))) {

    cinematic_start_playback_hook.invoke(name, key, playback_flags, volume,
                                         callback_info, id);
  }
}
} // namespace

class component final : public client_component {
public:
  void post_unpack() override {
    no_intro = utils::flags::has_flag("nointro");
    disable_all_cinematics = utils::flags::has_flag("nocinematics");
    if (no_intro || disable_all_cinematics) {
      cinematic_start_playback_hook.create(
          game::cinematic::Cinematic_StartPlayback,
          cinematic_start_playback_stub);
    }
  }
};
} // namespace intro

REGISTER_COMPONENT(intro::component)