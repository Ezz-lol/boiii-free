#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>

namespace dedicated_patches
{
	namespace
	{
		utils::hook::detour spawn_server_hook;

		void scr_are_textures_loaded_stub([[maybe_unused]] game::scriptInstance_t inst)
		{
			game::Scr_AddInt(game::SCRIPTINSTANCE_SERVER, 1);
		}

		game::eNetworkModes get_online_mode_stub()
		{
			return game::MODE_NETWORK_ONLINE;
		}

		bool is_online_stub()
		{
			return true;
		}

		bool is_mod_loaded_stub()
		{
			return false;
		}

		void patch_is_mod_loaded_checks()
		{
			const std::vector<uintptr_t> is_mod_loaded_addresses =
			{
				{0x14019CFC4_g},
				{0x14024D4A0_g},
				{0x14024D669_g},
				{0x14024D939_g},
				{0x14024DC64_g},
				{0x14024E13A_g},
				{0x14024E5A3_g},
				{0x14024FFB9_g},
				{0x140251E9E_g},
				{0x140253680_g},
				{0x140257BF6_g},
				{0x1402D296D_g},
				{0x1402D58E9_g},
				{0x140468374_g},
				{0x14046B796_g},
				{0x14048003D_g},
			};

			for (const auto& address : is_mod_loaded_addresses)
			{
				utils::hook::call(address, is_mod_loaded_stub);
			}
		}

		void spawn_server_stub(int controllerIndex, const char* server, game::MapPreload preload, bool savegame)
		{
			game::Com_SessionMode_SetNetworkMode(game::MODE_NETWORK_ONLINE);
			game::Com_SessionMode_SetGameMode(game::MODE_GAME_MATCHMAKING_PLAYLIST);

			spawn_server_hook.invoke(controllerIndex, server, preload, savegame);
		}

		uint64_t sv_get_player_xuid_stub(const int client_num)
		{
			const auto* clients = *game::svs_clients;
			if (!clients)
			{
				return 0;
			}

			return clients[client_num].xuid;
		}
	}

	struct component final : server_component
	{

		void post_unpack() override
		{
			// Fix infinite loop
			utils::hook::jump(0x1402E86B0_g, scr_are_textures_loaded_stub);

			// Online classes
			utils::hook::jump(0x1405003E0_g, get_online_mode_stub);
			utils::hook::jump(0x1405003B0_g, get_online_mode_stub);

			// Progression / Ranked
			utils::hook::jump(0x140500A50_g, is_online_stub);
			utils::hook::jump(0x140500980_g, is_online_stub);
			utils::hook::jump(0x1402565D0_g, is_online_stub);
			patch_is_mod_loaded_checks();

			spawn_server_hook.create(game::SV_SpawnServer, spawn_server_stub);

			// Don't count server as client
			utils::hook::jump(0x14052F0F5_g, 0x14052F139_g);

			utils::hook::call(0x1402853D7_g, sv_get_player_xuid_stub); // PlayerCmd_GetXuid

			// Stop executing default_dedicated.cfg & language_settings.cfg
			utils::hook::set<uint8_t>(0x1405063C0_g, 0xC3);
		}
	};
}

REGISTER_COMPONENT(dedicated_patches::component)
