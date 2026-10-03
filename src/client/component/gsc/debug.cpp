#include <std_include.hpp>

#include <game/game.hpp>

#include <component/game_event.hpp>
#include <loader/component_loader.hpp>

namespace gsc {
namespace debug {
namespace {
using namespace game::scr;
using namespace game::scr::vm;

inline void detect_infinite_loop() {
  // TODO
}

inline void detect_optimizations() { detect_infinite_loop(); }

inline void detect_instability() {
  // TODO: automatic detection for logic that causes VM instability
}
} // namespace

struct component final : generic_component {
#ifndef NDEBUG
  std::string name() override { return "gsc/debug"; }
#endif

  void post_unpack() override {
    if (game::scr_debug()) {
      detect_optimizations();
      detect_instability();
    }
  }
};
} // namespace debug
} // namespace gsc

REGISTER_COMPONENT(gsc::debug::component)
