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
		utils::hook::detour sv_addtestclient_hook;

		void stop_zombies_intro_if_needed()
		{
			if (game::Com_SessionMode_GetMode() != game::MODE_ZOMBIES)
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
			game::Com_GametypeSettings_SetGametype(gametype, false, false);
			stop_zombies_intro_if_needed();
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
	}

	class component final : public client_component
	{
	public:
		void post_unpack() override
		{
			fix_amd_cpu_stuttering();

			// Kill microphones for now
			utils::hook::set(0x15AAEB254_g, mixer_open_stub);

			preload_map_hook.create(0x14135A1E0_g, preload_map_stub);
		}
	};
}

REGISTER_COMPONENT(client_patches::component)
