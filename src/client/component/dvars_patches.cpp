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
			(void)game::register_sessionmode_dvar_bool("com_pauseSupported", !game::is_server(), game::DVAR_SERVERINFO, "Whether pause is supported by the game mode");
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

			a.mov(rax, qword_ptr(0x14AE95478_g));  // r_dof_enable

			a.test(rax, rax);
			a.jz(update_ads_dof);

			a.cmp(byte_ptr(rax, 0x28), 1);

			a.je(update_ads_dof);

			a.jmp(0x141116ECB_g);

			a.bind(update_ads_dof);
			a.lea(rdx, ptr(rbx, 0x131EB4));
			a.jmp(0x141116EC2_g); // CG_UpdateAdsDof
		}
	}

	class component final : public generic_component
	{
	public:
		void post_unpack() override
		{
			scheduler::once(patch_dvars, scheduler::pipeline::main);
			scheduler::once(patch_flags, scheduler::pipeline::main);

			if (game::is_client()) this->patch_client();
			else this->patch_server();
		}

		static void patch_client()
		{
			// toggle ADS dof based on r_dof_enable
			utils::hook::jump(0x141116EBB_g, utils::hook::assemble(dof_enabled_stub));
		}

		static void patch_server()
		{
			// Set the max value of 'sv_network_fps'
			utils::hook::set<uint32_t>(0x140534FE7_g, 1000);

			// Set the flag of 'sv_network_fps'
			utils::hook::set<uint32_t>(0x140534FD8_g, game::DVAR_NONE);
		}
	};
}

REGISTER_COMPONENT(dvars_patches::component)
