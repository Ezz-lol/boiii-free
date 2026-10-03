#include <std_include.hpp>

#include "vm.hpp"

#include <component/game_event.hpp>
#include <component/script_error.hpp>
#include <loader/component_loader.hpp>

namespace gsc::loop_guard {
namespace {
using namespace game::scr;
using namespace game::scr::vm;

constexpr std::chrono::milliseconds RUNAWAY_LIMIT = 1000ms;
constexpr uint32_t CHECK_INTERVAL = 1024;
constexpr float INSERTED_WAIT = 0.05f;

struct loop_slice {
  uint32_t time{};
  var::ScrVarIndex_t thread{};
  std::chrono::steady_clock::time_point start{};
  uint32_t back_jumps{};
};

struct instance_state {
  loop_slice slice{};
  std::unordered_set<const uint8_t *> runaway_loops;
  std::unordered_map<var::ScrVarIndex_t, uint32_t> last_pass;
};

op::VM_OP_FUNC_PTR original_jump{};
std::array<instance_state, SCRIPTINSTANCE_MAX> states{};

void yield_thread(const scriptInstance_t inst, volatile function_stack_t *fs,
                  volatile ScrVmContext_t *vmc, volatile bool *terminate,
                  uint8_t *jump_opcode) {
  fs->pos = jump_opcode;
  fs->top[1].type = var::ScrVarType::FLOAT;
  fs->top[1].u.floatValue = INSERTED_WAIT;
  fs->top += 1;
  (*op::handler(op::Opcode::Wait))(inst, fs, vmc, terminate);
}

void jump_stub(const scriptInstance_t inst, volatile function_stack_t *fs,
               volatile ScrVmContext_t *vmc, volatile bool *terminate) {
  uint8_t *const jump_opcode = fs->pos - sizeof(op::OP_TYPE);
  const uintptr_t operand = (reinterpret_cast<uintptr_t>(fs->pos) + 1) & ~1ull;
  if (*reinterpret_cast<const int16_t *>(operand) >= 0 ||
      inst >= SCRIPTINSTANCE_MAX) {
    original_jump(inst, fs, vmc, terminate);
    return;
  }

  instance_state &state = states[inst];
  const uint32_t now = gScrVarPub->instance[inst].time;
  const var::ScrVarIndex_t thread = fs->threadId;

  if (!state.runaway_loops.empty() &&
      state.runaway_loops.contains(jump_opcode)) {
    uint32_t &pass = state.last_pass[thread];
    if (pass == now) {
      yield_thread(inst, fs, vmc, terminate, jump_opcode);
      return;
    }
    pass = now;
    original_jump(inst, fs, vmc, terminate);
    return;
  }

  loop_slice &slice = state.slice;
  if (slice.time != now || slice.thread != thread) {
    slice = {now, thread, std::chrono::steady_clock::now(), 0};
  }

  if (++slice.back_jumps % CHECK_INTERVAL == 0) {
    const auto elapsed = std::chrono::steady_clock::now() - slice.start;
    if (elapsed >= RUNAWAY_LIMIT) {
      state.runaway_loops.insert(jump_opcode);
      state.last_pass[thread] = now;
      slice = {};
      script_error::report_runaway_loop(
          inst, fs->pos,
          static_cast<uint32_t>(
              std::chrono::duration_cast<std::chrono::milliseconds>(elapsed)
                  .count()));
      yield_thread(inst, fs, vmc, terminate, jump_opcode);
      return;
    }
  }

  original_jump(inst, fs, vmc, terminate);
}

void reset() {
  for (instance_state &state : states) {
    state = {};
  }
}
} // namespace

struct component final : generic_component {
#ifndef NDEBUG
  std::string name() override { return "loop_guard"; }
#endif

  void post_unpack() override {
    if (!game::detect_loops()) {
      return;
    }
    gsc::vm::hook_opcode(op::Opcode::Jump, jump_stub, &original_jump);
    game_event::on_g_shutdown_game(reset);
  }
};
} // namespace gsc::loop_guard

REGISTER_COMPONENT(gsc::loop_guard::component)
