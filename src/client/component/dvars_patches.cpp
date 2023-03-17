#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "game/utils.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>

namespace dvars_patches
{
	namespace
	{
		void patch_dvars()
		{
			game::register_sessionmode_dvar_bool("com_pauseSupported", !game::is_server(), game::DVAR_SERVERINFO, "Whether is pause is ever supported by the game mode");
		}

		void patch_flags()
		{
			if (game::is_client())
			{
				game::dvar_set_flags("r_dof_enable", game::DVAR_ARCHIVE);
				game::dvar_set_flags("r_lodbiasrigid", game::DVAR_ARCHIVE);
				game::dvar_set_flags("gpad_stick_deadzone_max", game::DVAR_ARCHIVE);
				game::dvar_set_flags("gpad_stick_deadzone_min", game::DVAR_ARCHIVE);
				game::dvar_set_flags("cg_drawLagometer", game::DVAR_ARCHIVE);
			}

			scheduler::execute(scheduler::pipeline::dvars_flags_patched);
		}

		void dof_enabled_stub(utils::hook::assembler& a)
		{
			const auto update_ads_dof = a.newLabel();

			a.pushad64();
			a.push(rax);

			a.mov(rax, qword_ptr(0x14AE95478_g));  // r_dof_enable
			a.cmp(byte_ptr(rax, 0x28), 1);

			a.pop(rax);
			a.je(update_ads_dof);

			a.popad64();
			a.jmp(0x141116ECB_g);

			a.bind(update_ads_dof);
			a.lea(rdx, ptr(rbx, 0x131EB4));
			a.mov(ecx, esi);
			a.call_aligned(0x141107EC0_g); // CG_UpdateAdsDof

			a.popad64();
			a.jmp(0x141116F49_g);
		}
	}

	class component final : public generic_component
	{
	public:
		void post_unpack() override
		{
			scheduler::once(patch_dvars, scheduler::pipeline::main);
			scheduler::once(patch_flags, scheduler::pipeline::main);

			if (game::is_server())
			{
				return;
			}

			// toggle ADS dof based on r_dof_enable
			//utils::hook::jump(0x141116EBB_g, utils::hook::assemble(dof_enabled_stub));
		}
	};
}

REGISTER_COMPONENT(dvars_patches::component)
