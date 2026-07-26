#include <std_include.hpp>
#include <loader/component_loader.hpp>
#include <game/game.hpp>
#include <utils/hook.hpp>

namespace ui_aspect_fix {
namespace {

constexpr float default_ui_aspect = 1.7777779f; // 16:9
constexpr float max_sane_aspect = 5.0f;         // aspect limit

void call_original_141CBDB90(__int64 a1, double xmm1) {
  utils::hook::invoke<void>(0x141CBDB90_g, a1, xmm1);
}

void ui_aspect_stub(__int64 a1, double xmm1) {
  call_original_141CBDB90(a1, xmm1);

  auto *base = reinterpret_cast<char *>(0x14F4E2C20_g);

  const float real_aspect = *reinterpret_cast<float *>(base + 0x1018);
  const float old_ui_aspect = *reinterpret_cast<float *>(base + 0x103C);

  if (real_aspect > default_ui_aspect && real_aspect < max_sane_aspect) {
    *reinterpret_cast<float *>(base + 0x103C) = real_aspect;
    printf("[ui_aspect_fix] patched uiAspect: %.4f -> %.4f (realAspect=%.4f)\n",
           old_ui_aspect, real_aspect, real_aspect);
  } else {
    printf("[ui_aspect_fix] uiAspect left unchanged at %.4f (realAspect=%.4f)\n",
           old_ui_aspect, real_aspect);
  }
}

} // namespace

struct component final : client_component {
  void post_unpack() override {
    printf("[ui_aspect_fix] installing hooks...\n");
    utils::hook::call(0x141CB8D7A_g, ui_aspect_stub);
    utils::hook::call(0x141CB8E69_g, ui_aspect_stub);
    utils::hook::call(0x141CBB3A8_g, ui_aspect_stub);
    utils::hook::call(0x141CBBBDF_g, ui_aspect_stub);
    printf("[ui_aspect_fix] hooks installed\n");
  }
};

} // namespace ui_aspect_fix

REGISTER_COMPONENT(ui_aspect_fix::component)
