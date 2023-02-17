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
		}
	};
}

REGISTER_COMPONENT(client_patches::component)
