#include <cstdint>
#include <std_include.hpp>
#include <loader/component_loader.hpp>
#include <game/game.hpp>

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

#include <utils/hook.hpp>
#include <utils/flags.hpp>

namespace branding {
namespace {
void draw_branding() {
  if (game::com::Com_IsInGame()) {
    return;
  }

  constexpr float x = 4.0;
  constexpr float y = 0.0;
  constexpr float scale = 0.45f;
  game::vec4_t color = {.r = 0.666f, .g = 0.666f, .b = 0.666f, .a = 0.666f};

  const uint32_t *font = reinterpret_cast<uint32_t *(*)()>(0x141CAC8E0_g)();
  if (!font)
    return;

  game::render::R_AddCmdDrawText(
      "EZZ: " VERSION, std::numeric_limits<int>::max(), font, x,
      y + static_cast<float>(font[2]) * scale, scale, scale, 0.0f, &color,
      game::itemTextStyle::NORMAL);
}

const char *get_ingame_console_prefix_stub() { return "EZZ> "; }
} // namespace

struct component final : client_component {
  void post_unpack() override {
    if (!utils::flags::has_flag("nobranding")) {

      scheduler::loop(draw_branding, scheduler::renderer);

      // Change window title prefix
      utils::hook::copy_string(0x14303F3D8_g, "EZZ");

      // Change ingame console prefix
      utils::hook::call(0x141339970_g, get_ingame_console_prefix_stub);
    }
  }
};
} // namespace branding

REGISTER_COMPONENT(branding::component)