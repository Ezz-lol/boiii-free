#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include <game/game.hpp>

#include <utils/hook.hpp>

namespace fov {
namespace {
void cg_calc_fov_stub(const game::LocalClientNum_t local_client_num,
                      float *fov_x, float *dx_dz_at_default_aspect_ratio,
                      float *dx_dz, float *dy_dz) {
  game::cg::CG_CalcFOVfromLens.call_safe(
      local_client_num, fov_x, dx_dz_at_default_aspect_ratio, dx_dz, dy_dz);

  if (game::cg_fovScale && !game::com::Com_IsRunningUILevel()) {
    const float scale = game::cg_fovScale->get_float();

    *fov_x *= scale;
    *dx_dz *= scale;
    *dy_dz *= scale;
  }
}
} // namespace

struct component final : client_component {
#ifndef NDEBUG
  std::string name() override { return "fov"; }
#endif

  void post_unpack() override {
    // Hook CG_CalcFOVfromLens within CG_CalcFov
    utils::hook::call(game::select(0x1404DADA7, 0x1404DADA7, 0x0),
                      cg_calc_fov_stub);

    // Patch cg_fovScale flags
    utils::hook::set<uint32_t>(game::select(0x14090E735, 0x14090E735, 0x0),
                               game::DVAR_ARCHIVE);

    // Don't reset cg_fovScale
    utils::hook::set<uint8_t>(game::select(0x140926D2A, 0x140926D2A, 0x0),
                              0xC3);
  }
};
} // namespace fov

REGISTER_COMPONENT(fov::component)
