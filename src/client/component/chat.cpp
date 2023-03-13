#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "chat.hpp"
#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include "command.hpp"
#include "client_command.hpp"

namespace chat
{
	namespace
	{
		void cmd_say_f(game::gentity_s* ent, const command::params_sv& params)
		{
			if (params.size() < 2)
			{
				return;
			}

			auto mode = 0;
			if (std::strcmp(params[0], "say_team") == 0)
			{
				mode = 1;
			}

			auto p = params.join(1);
			game::Scr_AddString(game::SCRIPTINSTANCE_SERVER, p.data() + 1); // Skip special char
			game::Scr_Notify_Canon(ent, game::Scr_CanonHash(params[0]), 1);

			game::G_Say(ent, nullptr, mode, p.data());
		}

		void cmd_chat_f(game::gentity_s* ent, const command::params_sv& params)
		{
			auto p = params.join(1);

			// Not a mistake! + 2 is necessary for the GSC script to receive only the actual chat text
			game::Scr_AddString(game::SCRIPTINSTANCE_SERVER, p.data() + 2);
			game::Scr_Notify_Canon(ent, game::Scr_CanonHash(params[0]), 1);

			utils::hook::invoke<void>(0x140298E70_g, ent, p.data());
		}

		void get_chat_client_name_stub(uint64_t xuid, char* buffer, const size_t max_length)
		{
			utils::string::copy(buffer, max_length, utils::string::va("%llX-something", xuid));
			OutputDebugStringA(buffer);
		}
	}

	const char* GetClientName(const uint64_t xuid)
	{
		if (xuid < 19 && !game::is_server())
		{
			char buffer[256];
			game::CL_GetClientName(0, static_cast<int>(xuid - 1), buffer, sizeof(buffer), true);

			return utils::string::va("%s\n", buffer);
		}

		return "Unknown Soldier";
	}

	uint64_t* divert_xuid_to_client_num_stub(int, int client_num, int)
	{
		static thread_local uint64_t value;
		// zero xuid is invalid, so increase the clientnum to prevent 0 values
		value = static_cast<uint64_t>(client_num) + 1;
		return &value;
	}

	class component final : public generic_component
	{
	public:
		void post_unpack() override
		{
			utils::hook::call(game::select(0x141974B04, 0x14029908A), divert_xuid_to_client_num_stub);

			if (game::is_server())
			{
				client_command::add("say", cmd_say_f);
				client_command::add("say_team", cmd_say_f);

				client_command::add("chat", cmd_chat_f);
			}
			else
			{
				// Ignore some check that suppresses the chat
				utils::hook::nop(0x141DEA9BD_g, 2);
			}
		}
	};
}

REGISTER_COMPONENT(chat::component)
