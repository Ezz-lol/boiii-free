#include <std_include.hpp>

#include "scheduler.hpp"

#include <game/utils.hpp>
#include <loader/component_loader.hpp>

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace perf_overlay {
namespace {
// The engine's FPS counter layout on its 640x480 virtual screen
constexpr float VIRTUAL_HEIGHT = 480.0f;
constexpr float BASELINE = 35.0f;
constexpr float SIZES[] = {0.7f, 1.0f, 1.3f};
constexpr const char COLOR_HELP[] =
    "0 = automatic, 1 = white, 2 = yellow, 3 = green, 4 = orange, "
    "5 = red, 6 = cyan";

enum class DrawColor : uint8_t {
  WHITE = 0x0,
  YELLOW = 0x1,
  GREEN = 0x2,
  ORANGE = 0x3,
  RED = 0x4,
  CYAN = 0x5,
  COUNT = 0x6
};
IMPL_ENUM_OPERATORS(DrawColor);

enum class DvarDrawColor : uint8_t {
  AUTOMATIC = 0x0,
  WHITE = 0x1,
  YELLOW = 0x2,
  GREEN = 0x3,
  ORANGE = 0x4,
  RED = 0x5,
  CYAN = 0x6,
  COUNT = 0x7
};
IMPL_ENUM_OPERATORS(DvarDrawColor);

constexpr game::vec4_t WHITE = {.r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f};
constexpr game::vec4_t YELLOW = {.r = 1.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f};
constexpr game::vec4_t GREEN = {.r = 0.3f, .g = 0.9f, .b = 0.3f, .a = 1.0f};
constexpr game::vec4_t ORANGE = {.r = 1.0f, .g = 0.6f, .b = 0.1f, .a = 1.0f};
constexpr game::vec4_t RED = {.r = 1.0f, .g = 0.25f, .b = 0.2f, .a = 1.0f};
constexpr game::vec4_t CYAN = {.r = 0.3f, .g = 0.85f, .b = 1.0f, .a = 1.0f};
constexpr game::vec4_t COLORS[+DrawColor::COUNT] = {WHITE,  YELLOW, GREEN,
                                                    ORANGE, RED,    CYAN};

game::EngineDependentDvarMut fps_size;
game::EngineDependentDvarMut fps_color;
game::EngineDependentDvarMut draw_ping;
game::EngineDependentDvarMut ping_size;
game::EngineDependentDvarMut ping_color;

std::atomic<int32_t> current_ping{-1};

const uint32_t *console_font() {
  return reinterpret_cast<const uint32_t *>(0x1432D2A30_g);
}

const game::vec4_t &pick_color(const DvarDrawColor option, const bool good,
                               const bool okay) {
  if (option > DvarDrawColor::AUTOMATIC && (+option - 1) < +DrawColor::COUNT) {
    return COLORS[+option - 1];
  }
  if (good) {
    return GREEN;
  }

  return okay ? ORANGE : RED;
}

float size_of(game::EngineDependentDvarMut dvar) {
  return SIZES[std::clamp(dvar.get_int(), 0,
                          static_cast<int32_t>(std::size(SIZES)) - 1)];
}

int32_t measure_fps() {
  using clock = std::chrono::steady_clock;
  static clock::time_point window_start = clock::now();
  static int32_t frames = 0;
  static int32_t fps = 0;

  ++frames;
  const clock::time_point now = clock::now();
  const std::chrono::duration<float> elapsed = now - window_start;
  if (elapsed.count() >= 0.5f) {
    fps = static_cast<int32_t>(std::lround(frames / elapsed.count()));
    frames = 0;
    window_start = now;
  }
  return fps;
}

void update_ping() {
  if (!draw_ping.get_bool() || !game::com::Com_IsInGame() ||
      game::server_running() ||
      game::cg::clientUIActives->actives[0].connectionState !=
          game::connstate_t::ACTIVE) {
    current_ping = -1;
    return;
  }
  const game::level::cl::cg_t &cg = game::cg::cgArray[0];
  const game::level::cl::snapshot_t *snap = cg.snap;
  if (snap == &cg.activeSnapshots[0] || snap == &cg.activeSnapshots[1]) {
    current_ping = snap->ping;
  }
}

float text_width(const char *text, const float scale) {
  return static_cast<float>(game::render::R_TextWidth(
             game::LOCAL_CLIENT_0, text, std::numeric_limits<int32_t>::max(),
             console_font())) *
         scale;
}

void draw_text(const char *text, const float x, const float scale,
               const game::vec4_t &color) {
  game::render::R_AddCmdDrawText(text, std::numeric_limits<int32_t>::max(),
                                 console_font(), x, BASELINE * scale, scale,
                                 scale, 0.0f, &color,
                                 game::itemTextStyle::NORMAL);
}

void draw_overlay() {
  const int32_t fps = measure_fps();
  const float screen_scale =
      static_cast<float>(game::r::vidConfig->displayHeight) / VIRTUAL_HEIGHT;

  if (game::com_drawfps_pc && game::com_drawfps_pc->get_bool()) {
    const float scale = screen_scale * size_of(fps_size);
    const char *text = utils::string::va("%d ", fps);
    draw_text(text,
              static_cast<float>(game::r::vidConfig->displayWidth) -
                  text_width(text, scale),
              scale,
              pick_color(static_cast<DvarDrawColor>(fps_color.get_int()),
                         fps >= 60, fps >= 30));
  }

  const int32_t ping = current_ping;
  if (ping >= 0) {
    const float scale = screen_scale * size_of(ping_size);
    draw_text(utils::string::va("%d ms", ping), text_width(" ", scale), scale,
              pick_color(static_cast<DvarDrawColor>(ping_color.get_int()),
                         ping < 60, ping <= 120));
  }
}

void skip_engine_fps(game::LocalClientNum_t) {}
} // namespace

struct component final : client_component {
  DEFINE_COMPONENT_NAME("perf_overlay");

  void post_unpack() override {
    fps_size = game::register_dvar_int("boiii_fpsSize", 1, 0, 2,
                                       game::dvarFlags_e::ARCHIVE,
                                       "FPS counter size: 0-2");
    fps_color = game::register_dvar_int("boiii_fpsColor", 0, 0,
                                        +DvarDrawColor::AUTOMATIC,
                                        game::dvarFlags_e::ARCHIVE, COLOR_HELP);
    draw_ping = game::register_dvar_bool("boiii_drawPing", false,
                                         game::dvarFlags_e::ARCHIVE,
                                         "Show your ping next to the FPS");
    ping_size =
        game::register_dvar_int("boiii_pingSize", 0, 0, 2,
                                game::dvarFlags_e::ARCHIVE, "Ping size: 0-2");
    ping_color = game::register_dvar_int(
        "boiii_pingColor", 0, 0, +DvarDrawColor::AUTOMATIC,
        game::dvarFlags_e::ARCHIVE, COLOR_HELP);

    utils::hook::call(game::cl::CL_UpdateScreen_DrawFpsCall.get(),
                      skip_engine_fps);
    scheduler::loop(update_ping, scheduler::pipeline::main);
    scheduler::loop(draw_overlay, scheduler::pipeline::renderer);
  }
};
} // namespace perf_overlay

REGISTER_COMPONENT(perf_overlay::component)
