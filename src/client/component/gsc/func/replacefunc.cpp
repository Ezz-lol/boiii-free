#include <std_include.hpp>

#include "func.hpp"

#include <game/impl/scr/scr.hpp>
#include <game/impl/scr/var.hpp>
#include <game/impl/sv/sv.hpp>
#include <game/utils.hpp>

#include <component/command.hpp>
#include <component/game_event.hpp>
#include <component/name.hpp>
#include <component/script.hpp>
#include <component/toast.hpp>
#include <loader/component_loader.hpp>

#include <utils/hook.hpp>
#include <utils/io.hpp>

#include <rapidjson/writer.h>

using namespace game;
using namespace game::scr;

namespace gsc {
namespace func {
namespace replacefunc {

std::unordered_map<uintptr_t, uintptr_t> function_replacements;

std::atomic_bool detours_enabled = false;

// replacefunc: redirect all calls to target_func to replacement_func
void gscr_replacefunc(scriptInstance_t inst) {
  const uintptr_t target_addr = game::scr::Scr_GetFunc(inst, 0);
  const uintptr_t replacement_addr = game::scr::Scr_GetFunc(inst, 1);

  if (target_addr && replacement_addr) {
    function_replacements[target_addr] = replacement_addr;
    detours_enabled.store(true, std::memory_order_release);
  }
}

// clearreplacefuncs: remove all active function replacements
void gscr_clearreplacefuncs([[maybe_unused]] scriptInstance_t inst) {
  function_replacements.clear();
  detours_enabled.store(false, std::memory_order_release);
}

void gscr_getfunction(scriptInstance_t inst) {
  const char *script_name = Scr_GetString(inst, 0);
  const char *func_name = Scr_GetString(inst, 1);
  if (script_name && func_name) {
    uint8_t *addr = script::find_export_address(script_name, func_name);
    push(inst, reinterpret_cast<int64_t>(addr));
  } else {
    push(inst, 0);
  }
}

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("gsc/func/replacefunc");

  void post_unpack() override {
    register_builtin(SCRIPTINSTANCE_SERVER, "replacefunc", gscr_replacefunc, 2);
    register_builtin(SCRIPTINSTANCE_SERVER, "getfunction", gscr_getfunction, 2);
    register_builtin(SCRIPTINSTANCE_SERVER, "clearreplacefuncs",
                     gscr_clearreplacefuncs, 0);

    game_event::on_g_shutdown_game([] {
      function_replacements.clear();
      detours_enabled.store(false, std::memory_order_release);
    });

    game_event::on_g_init_game([] {
      function_replacements.clear();
      detours_enabled.store(false, std::memory_order_release);
    });
  }
};
} // namespace replacefunc
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::replacefunc::component)
