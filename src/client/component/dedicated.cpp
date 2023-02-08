#include <std_include.hpp>
#include "dedicated.hpp"
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "command.hpp"
#include "network.hpp"
#include "scheduler.hpp"
#include "server_list.hpp"

#include <utils/hook.hpp>

namespace dedicated
{
	namespace
	{
		void sv_con_tell_f_stub(game::client_s* cl_0, game::svscmd_type type, [[maybe_unused]] const char* fmt,
		                        [[maybe_unused]] int c, char* text)
		{
			game::SV_SendServerCommand(cl_0, type, "%c \"GAME_SERVER\x15: %s\"", 79, text);
		}
	}

	void send_heartbeat()
	{
		if (!game::is_server())
		{
			return;
		}

		game::netadr_t target{};
		if (server_list::get_master_server(target))
		{
			network::send(target, "heartbeat", "T7");
		}
	}

	struct component final : server_component
	{
		void post_unpack() override
		{
			// Ignore "bad stats"
			//utils::hook::set<uint8_t>(0x14052D523_g, 0xEB);
			//utils::hook::nop(0x14052D4E4_g, 2);

			// Fix tell command for IW4M
			utils::hook::call(0x14052A8CF_g, sv_con_tell_f_stub);

			scheduler::loop(send_heartbeat, scheduler::pipeline::server, 10min);
			command::add("heartbeat", send_heartbeat);
		}
	};
}

REGISTER_COMPONENT(dedicated::component)
