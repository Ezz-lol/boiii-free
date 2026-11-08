#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include <utils/hook.hpp>

namespace intro {
namespace {
utils::hook::detour cinematic_start_playback_hook;

constexpr std::string_view LOGOSEQUENCE_CINEMATIC_NAME =
    "BO3_Global_Logo_LogoSequence";
constexpr std::string_view FOREST_CINEMATIC_NAME = "zm_frontend_load";

inline bool should_skip(const std::string_view &name) {
  return game::skip_cinematics() ||
         (game::skip_intro() && name == LOGOSEQUENCE_CINEMATIC_NAME) ||
         (game::skip_forest_cinematic() &&
          name.starts_with(FOREST_CINEMATIC_NAME));
}

void cinematic_start_playback_stub(const char *name, const char *key,
                                   const uint32_t playback_flags,
                                   const float volume, void *callback_info,
                                   const int32_t id) {
  if (!name || !should_skip(name)) {
    cinematic_start_playback_hook.invoke(name, key, playback_flags, volume,
                                         callback_info, id);
  }
}

bool ugc_video_loaded() {
  return game::ugc::active_usermap->publisherId[0] ||
         game::ugc::active_mod->publisherId[0];
}

const char *ugc_video_root() {
  return game::ugc::active_usermap->publisherId[0]
             ? game::ugc::active_usermap->absolutePathZoneFiles
             : game::ugc::active_mod->absolutePathZoneFiles;
}
} // namespace

class component final : public client_component {
  DEFINE_COMPONENT_NAME("intro");

public:
  void post_unpack() override {
    cinematic_start_playback_hook.create(
        game::cinematic::Cinematic_StartPlayback,
        cinematic_start_playback_stub);

    if (!game::is_legacy_client()) {
      utils::hook::call(
          game::cinematic::Cinematic_ResolvePaths_UsermapLoadedCall.get(),
          ugc_video_loaded);
      utils::hook::call(
          game::cinematic::Cinematic_ResolvePaths_UsermapPathCall.get(),
          ugc_video_root);
      utils::hook::call(
          game::cinematic::Cinematic_ResolvePath_UsermapLoadedCall.get(),
          ugc_video_loaded);
      utils::hook::call(
          game::cinematic::Cinematic_ResolvePath_UsermapPathCall.get(),
          ugc_video_root);
    }
  }
};
} // namespace intro

REGISTER_COMPONENT(intro::component)
