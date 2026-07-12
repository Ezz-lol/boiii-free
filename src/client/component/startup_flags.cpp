#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "game/utils.hpp"
#include "scheduler.hpp"

#include <utils/flags.hpp>

namespace startup_flags {
namespace {
void apply_startup_flags() {
  if (utils::flags::has_flag("windowed")) {
    game::set_dvar_bool(*game::r_fullscreen, false);
  }

  if (utils::flags::has_flag("safe")) {
    game::set_dvar_bool(*game::r_fullscreen, false);
    game::set_dvar_bool(*game::r_vsync, false);
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