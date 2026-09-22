#include <std_include.hpp>

#include <game/game.hpp>
#include <loader/component_loader.hpp>

// In case of clangd compilation
#if __has_include("version.hpp")
#include "version.hpp"
#else
#ifndef VERSION
#define VERSION "0"
#endif
#ifndef SHORTVERSION
#define SHORTVERSION "0"
#endif
#endif

#include "scheduler.hpp"

#include <utils/flags.hpp>
#include <utils/hook.hpp>

namespace branding {
namespace {
constexpr const char FONT[] = "fonts/default.ttf";
void draw_branding() {
  if (!game::com::Com_IsInGame()) {
    constexpr float x = 4.0;
    constexpr float y = 0.0;
    constexpr float scale = 0.45f;
    game::vec4_t color = {.r = 0.666f, .g = 0.666f, .b = 0.666f, .a = 0.666f};

    game::render::R_AddCmdDrawText(
        "EZZ: " VERSION, std::numeric_limits<int>::max(), FONT, x, y, scale,
        scale, 0.0f, &color, game::itemTextStyle::NORMAL);
  }
}

const char *get_ingame_console_prefix_stub() { return "EZZ> "; }
} // namespace

struct component final : client_component {
  void post_unpack() override {
    if (!utils::flags::has_flag("nobranding")) {

      scheduler::loop(draw_branding, scheduler::renderer);

      // Change window title prefix
      utils::hook::copy_string(game::select(0x142FC02D8, 0x14303F3D8, 0x0),
                               "EZZ");

      // Change ingame console prefix
      utils::hook::call(game::con::Con_DrawInput.offset(0x90),
                        get_ingame_console_prefix_stub);
    }
  }
};
} // namespace branding

REGISTER_COMPONENT(branding::component)