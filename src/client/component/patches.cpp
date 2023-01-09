#include <std_include.hpp>

#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>

namespace patches
{
	namespace
	{
		void scr_are_textures_loaded_stub([[maybe_unused]] game::scriptInstance_t inst)
		{
			game::Scr_AddInt(game::SCRIPTINSTANCE_SERVER, 1);
		}
	}

	struct component final : generic_component
	{
		void post_unpack() override
		{
			if (game::is_server())
			{
				// Fix infinite loop
				utils::hook::jump(0x1402E86B0_g, scr_are_textures_loaded_stub);
			}
		}
	};
}

REGISTER_COMPONENT(patches::component)
