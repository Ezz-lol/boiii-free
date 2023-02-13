#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "scheduler.hpp"

namespace client_patches
{
	namespace
	{
		void reduce_process_affinity()
		{
			const DWORD_PTR affinity = (2ULL << (std::min(std::thread::hardware_concurrency(), 4U))) - 1;
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
	}

	class component final : public client_component
	{
	public:
		void post_unpack() override
		{
			fix_amd_cpu_stuttering();
		}
	};
}

REGISTER_COMPONENT(client_patches::component)
