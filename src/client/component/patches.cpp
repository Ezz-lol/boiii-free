#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <game/game.hpp>
#include <game/utils.hpp>

#include <utils/hook.hpp>

namespace patches
{
	namespace
	{
		const game::dvar_t* lobby_min_players;
		utils::hook::detour com_error_hook;

		void com_error_stub(const char* file, int line, int code, const char* fmt, ...)
		{
			char buffer[0x1000];

			{
				va_list ap;
				va_start(ap, fmt);
				vsnprintf_s(buffer, _TRUNCATE, fmt, ap);
				va_end(ap);
			}

			printf("[Com_Error] Code=%d, File=%s, Line=%d: %s\n", code, file ? file : "unknown", line, buffer);

			// Suppress Clientfield Mismatch errors - convert to a recoverable ERR_DROP
			if (strstr(buffer, "Clientfield Mismatch"))
			{
				printf("[Com_Error] Suppressing Clientfield Mismatch error, converting to ERR_DROP\n");
				// Convert to ERR_DROP (code 1) which is recoverable and returns to menu
				com_error_hook.invoke<void>(file, line, game::ERR_DROP,
					"Mod compatibility issue: %s\nThis mod may require additional patches for boiii.", buffer);
				return;
			}

			// Call original Com_Error for other errors
			com_error_hook.invoke<void>(file, line, code, "%s", buffer);
		}

		void scr_get_num_expected_players()
		{
			auto expected_players = game::LobbyHost_GetClientCount(game::LOBBY_TYPE_GAME,
			                                                       game::LOBBY_CLIENT_TYPE_ALL);

			const auto mode = game::Com_SessionMode_GetMode();
			if ((mode == game::MODE_ZOMBIES || mode == game::MODE_CAMPAIGN))
			{
				if (const auto min_players = lobby_min_players->current.value.integer)
				{
					expected_players = min_players;
				}
			}

			const auto num_expected_players = std::max(1, expected_players);
			game::Scr_AddInt(game::SCRIPTINSTANCE_SERVER, num_expected_players);
		}

		void sv_execute_client_messages_stub(game::client_s* client, game::msg_t* msg)
		{
			if ((client->reliableSequence - client->reliableAcknowledge) < 0)
			{
				client->reliableAcknowledge = client->reliableSequence;
				game::SV_DropClient(client, "EXE_LOSTRELIABLECOMMANDS", true, true);
				return;
			}

			game::SV_ExecuteClientMessage(client, msg);
		}
	}

	struct component final : generic_component
	{
		void post_unpack() override
		{
			// Hook Com_Error to handle mod compatibility issues (e.g., Clientfield Mismatch)
			com_error_hook.create(game::Com_Error_, com_error_stub);

			// print hexadecimal xuids in chat game log command
			utils::hook::set<char>(game::select(0x142FD9362, 0x140E16FA2), 'x');

			// change 4 character min name limit to 3 characters
			utils::hook::set<uint8_t>(game::select(0x14224DA53, 0x140531143), 3);
			utils::hook::set<uint8_t>(game::select(0x14224DBB4, 0x1405312A8), 3);
			utils::hook::set<uint8_t>(game::select(0x14224DF8C, 0x1405316DC), 3);

			// make sure reliableAck is not negative or too big
			utils::hook::call(game::select(0x14225489C, 0x140537C4C), sv_execute_client_messages_stub);

			lobby_min_players = game::register_dvar_int("lobby_min_players", 0, 0, 8, game::DVAR_NONE, "");
			utils::hook::jump(game::select(0x141A7BCF0, 0x1402CB900), scr_get_num_expected_players, true);
		}
	};
}

REGISTER_COMPONENT(patches::component)
