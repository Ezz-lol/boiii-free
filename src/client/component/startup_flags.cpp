#include <std_include.hpp>
#include <loader/component_loader.hpp>
#include <game/game.hpp>
#include <game/utils.hpp>
#include "scheduler.hpp"

#include <utils/flags.hpp>

namespace startup_flags {
namespace {
void apply_startup_flags() {
  if (utils::flags::has_flag("windowed")) {
    game::r_fullscreen->set(false);
  }

  if (utils::flags::has_flag("safe")) {
    game::r_fullscreen->set(false);
    game::r_vsync->set(false);
  }
}
} // namespace

class component final : public client_component {
public:
  void post_unpack() override {
    // Schedule after dvars are loaded from config so we can override them
    scheduler::once(apply_startup_flags, scheduler::pipeline::dvars_loaded);
  }
};
} // namespace startup_flags

REGISTER_COMPONENT(startup_flags::component)