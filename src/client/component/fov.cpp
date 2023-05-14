#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>

namespace fov
{
	namespace
	{
		void cg_calc_fov_stub(const int local_client_num, float* fov_x, float* dx_dz_at_default_aspect_ratio,
		                      float* dx_dz, float* dy_dz)
		{
			game::CG_CalcFOVfromLens.call_safe(local_client_num, fov_x, dx_dz_at_default_aspect_ratio, dx_dz, dy_dz);

			const game::dvar_t* cg_fovScale = *reinterpret_cast<game::dvar_t**>(0x144A31A88_g);
			if (cg_fovScale && !game::Com_IsRunningUILevel())
			{
				const auto scale = cg_fovScale->current.value.value;

				*fov_x *= scale;
				*dx_dz *= scale;
				*dy_dz *= scale;
			}
		}
	}

	struct component final : client_component
	{
		void post_unpack() override
		{
			// Hook CG_CalcFOVfromLens within CG_CalcFov
			utils::hook::call(0x1404DADA7_g, cg_calc_fov_stub);

			// Patch cg_fovScale flags
			utils::hook::set<uint32_t>(0x14090E735_g, game::DVAR_ARCHIVE);

			// Don't reset cg_fovScale
			utils::hook::set<uint8_t>(0x140926D2A_g, 0xC3);
		}
	};
}

REGISTER_COMPONENT(fov::component)
