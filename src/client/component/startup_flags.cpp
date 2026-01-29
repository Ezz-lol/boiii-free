#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "scheduler.hpp"

#include <utils/flags.hpp>

namespace startup_flags
{
	namespace
	{
		void apply_startup_flags()
		{
			if (utils::flags::has_flag("windowed"))
			{
				game::Dvar_SetFromStringByName("r_fullscreen", "0", true);
			}

			if (utils::flags::has_flag("safe"))
			{
				game::Dvar_SetFromStringByName("r_fullscreen", "0", true);
				game::Dvar_SetFromStringByName("r_vsync", "0", true);
			}
		}
	}

	class component final : public client_component
	{
	public:
		void post_unpack() override
		{
			// Schedule after dvars are loaded from config so we can override them
			scheduler::once(apply_startup_flags, scheduler::pipeline::dvars_loaded);
		}
	};
}

REGISTER_COMPONENT(startup_flags::component)
