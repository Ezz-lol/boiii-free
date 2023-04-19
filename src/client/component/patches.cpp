#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <game/game.hpp>
#include <game/utils.hpp>

#include "network.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>

namespace patches
{
	namespace
	{
		utils::hook::detour sv_execute_client_messages_hook;

		void sv_execute_client_messages_stub(game::client_s* client, game::msg_t* msg)
		{
			if ((client->reliableSequence - client->reliableAcknowledge) < 0)
			{
				client->reliableAcknowledge = client->reliableSequence;
				network::send(client->address, "error", "EXE_LOSTRELIABLECOMMANDS");
				return;
			}

			sv_execute_client_messages_hook.invoke<void>(client, msg);
		}

		void script_errors_stub(const char* file, int line, unsigned int code, const char* fmt, ...)
		{
			char buffer[0x1000];

			{
				va_list ap;
				va_start(ap, fmt);
				vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, ap);
				va_end(ap);
			}

			game::Com_Error(game::ERROR_SCRIPT_DROP, "%s", buffer);
		}
	}

	struct component final : generic_component
	{
		void post_unpack() override
		{
			// don't make script errors fatal error
			utils::hook::call(game::select(0x1412CAC4D, 0x140158EB2), script_errors_stub);

			// Change 4 character name limit to 3 characters
			utils::hook::set<uint8_t>(game::select(0x14224DA53, 0x140531143), 3);
			utils::hook::set<uint8_t>(game::select(0x14224DBB4, 0x1405312A8), 3);
			utils::hook::set<uint8_t>(game::select(0x14224DF8C, 0x1405316DC), 3);

			// make sure client's reliableAck are not negative
			sv_execute_client_messages_hook.create(game::select(0x14224A460, 0x14052F840), sv_execute_client_messages_stub);

			scheduler::once([]
			{
				game::register_dvar_string("password", "", game::DVAR_USERINFO, "password");
			}, scheduler::pipeline::main);
		}
	};
}

REGISTER_COMPONENT(patches::component)
