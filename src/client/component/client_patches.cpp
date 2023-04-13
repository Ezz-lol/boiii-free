#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "scheduler.hpp"

#include <game/game.hpp>
#include <utils/hook.hpp>

#include <mmeapi.h>

namespace client_patches
{
	namespace
	{
		utils::hook::detour preload_map_hook;

		void stop_intro_if_needed()
		{
			if (game::Com_SessionMode_GetMode() != game::MODE_ZOMBIES &&
			    game::Com_SessionMode_GetMode() != game::MODE_CAMPAIGN)
			{
				return;
			}

			scheduler::once([]
			{
				scheduler::schedule([]
				{
					if (!game::Sys_IsDatabaseReady())
					{
						return scheduler::cond_continue;
					}

					game::Cinematic_StopPlayback(0, true);
					return scheduler::cond_end;
				}, scheduler::main);
			}, scheduler::main, 15s);
		}

		void preload_map_stub(int localClientNum, const char* mapname, const char* gametype)
		{
			game::Com_GametypeSettings_SetGametype(gametype, true);
			stop_intro_if_needed();
			preload_map_hook.invoke(localClientNum, mapname, gametype);
		}

		void reduce_process_affinity()
		{
			const DWORD_PTR affinity = (1ULL << (std::min(std::thread::hardware_concurrency(), 4U))) - 1;
			SetProcessAffinityMask(GetCurrentProcess(), affinity);
		}

		void reset_process_affinity()
		{
			DWORD_PTR affinity_proc, affinity_sys;
			GetProcessAffinityMask(GetCurrentProcess(), &affinity_proc, &affinity_sys);
			SetProcessAffinityMask(GetCurrentProcess(), affinity_sys);
		}

		void fix_amd_cpu_stuttering()
		{
			scheduler::once([]
			{
				reduce_process_affinity();
				scheduler::once(reset_process_affinity, scheduler::pipeline::main, 1s);
			}, scheduler::pipeline::main);
		}

		MMRESULT mixer_open_stub()
		{
			return MMSYSERR_NODRIVER;
		}

		bool is_mod_loaded_stub()
		{
			return false;
		}

		void patch_is_mod_loaded_checks()
		{
			const std::vector<uintptr_t> is_mod_loaded_addresses =
			{
				{ 0x1420F7484_g },
				{ 0x1420F74A4_g },
				{ 0x1420F73E4_g },
				{ 0x1420F73B4_g },
				{ 0x1420F6E57_g },
				{ 0x1413E6A54_g },
				{ 0x1415E7EBB_g },
				{ 0x1415E87BB_g },
				{ 0x1415EBAC9_g },
				{ 0x1415F1F09_g },
				{ 0x1415F1FB9_g },
				{ 0x1415F2080_g },
				{ 0x1415F7F40_g },
				{ 0x141A8D0ED_g },
				{ 0x141AA70F9_g },
				{ 0x141EA06FB_g },
				{ 0x141EA8C7E_g },
				{ 0x141EB1A39_g },
				{ 0x141ECBA9D_g },
				{ 0x1420F6E1D_g },
			};

			for (const auto& address : is_mod_loaded_addresses)
			{
				utils::hook::call(address, is_mod_loaded_stub);
			}
		}
	}

	class component final : public client_component
	{
	public:
		void post_unpack() override
		{
			fix_amd_cpu_stuttering();

			// Don't modify process priority
			utils::hook::nop(0x142334C98_g, 6);

			// Kill microphones for now
			utils::hook::set(0x15AAE9254_g, mixer_open_stub);

			preload_map_hook.create(0x14135A1E0_g, preload_map_stub);

			// Keep client ranked when mod loaded
			utils::hook::jump(0x1420D5BA0_g, is_mod_loaded_stub);
			patch_is_mod_loaded_checks();

			// Kill Client/Server Index Mismatch error
			utils::hook::set<uint8_t>(0x1400A7588_g, 0xEB);

			// Always get loadscreen gametype from s_gametype
			utils::hook::set<uint8_t>(0x14228F5DC_g, 0xEB);
		}
	};
}

REGISTER_COMPONENT(client_patches::component)
