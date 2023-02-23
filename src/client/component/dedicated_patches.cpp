#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>

namespace dedicated_patches
{
	namespace
	{
		void scr_are_textures_loaded_stub([[maybe_unused]] game::scriptInstance_t inst)
		{
			game::Scr_AddInt(game::SCRIPTINSTANCE_SERVER, 1);
		}

		game::eNetworkModes get_online_mode()
		{
			return game::MODE_NETWORK_ONLINE;
		}
	}

	struct component final : server_component
	{
		void post_unpack() override
		{
			// Fix infinite loop
			utils::hook::jump(0x1402E86B0_g, scr_are_textures_loaded_stub);

			utils::hook::jump(0x1405003E0_g, get_online_mode);
			utils::hook::jump(0x1405003B0_g, get_online_mode);

			scheduler::once([]()
			{
				game::Com_SessionMode_SetNetworkMode(game::MODE_NETWORK_ONLINE);
				game::Com_SessionMode_SetGameMode(game::MODE_GAME_MATCHMAKING_PLAYLIST);
			}, scheduler::pipeline::main, 1s);
		}
	};
}

REGISTER_COMPONENT(dedicated_patches::component)
