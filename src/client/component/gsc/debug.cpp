#include <std_include.hpp>

#include "vm.hpp"

#include <game/game.hpp>

#include <component/game_event.hpp>
#include <component/script_error.hpp>
#include <loader/component_loader.hpp>

namespace gsc {
namespace debug {
namespace {
using namespace game::scr;
using namespace game::scr::vm;

constexpr std::chrono::milliseconds LOOP_TIME_LIMIT = 1000ms;
constexpr uint32_t LOOP_CHECK_INTERVAL = 1024;

struct loop_watch {
  uint32_t time{};
  var::ScrVarIndex_t thread{};
  std::chrono::steady_clock::time_point start{};
  uint32_t back_jumps{};
};

op::VM_OP_FUNC_PTR Jump_original{};
std::array<loop_watch, SCRIPTINSTANCE_MAX> loop_watches{};
std::array<std::unordered_set<const uint8_t *>, SCRIPTINSTANCE_MAX>
    reported_loops{};

void Jump_DetectInfiniteLoop(const scriptInstance_t inst,
                             volatile function_stack_t *fs,
                             volatile ScrVmContext_t *vmc,
                             volatile bool *terminate) {
  const uintptr_t operand = (reinterpret_cast<uintptr_t>(fs->pos) + 1) & ~1ull;
  if (inst < SCRIPTINSTANCE_MAX &&
      *reinterpret_cast<const int16_t *>(operand) < 0) {
    loop_watch &watch = loop_watches[inst];
    const uint32_t time = gScrVarPub->instance[inst].time;
    if (watch.time != time || watch.thread != fs->threadId) {
      watch = {time, fs->threadId, std::chrono::steady_clock::now(), 0};
    } else if (++watch.back_jumps % LOOP_CHECK_INTERVAL == 0) {
      const auto elapsed = std::chrono::steady_clock::now() - watch.start;
      if (elapsed >= LOOP_TIME_LIMIT &&
          reported_loops[inst].insert(fs->pos - sizeof(op::OP_TYPE)).second) {
        script_error::report_runaway_loop(
            inst, fs->pos,
            static_cast<uint32_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(elapsed)
                    .count()));
      }
    }
  }
  Jump_original(inst, fs, vmc, terminate);
}

inline void detect_infinite_loop() {
  vm::hook_opcode(op::Opcode::Jump, Jump_DetectInfiniteLoop, &Jump_original);
  game_event::on_g_shutdown_game([] {
    loop_watches = {};
    for (std::unordered_set<const uint8_t *> &reported : reported_loops) {
      reported.clear();
    }
  });
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
