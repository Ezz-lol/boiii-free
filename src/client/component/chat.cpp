#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "chat.hpp"
#include "game/game.hpp"
#include "game/utils.hpp"

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

		uint64_t* divert_xuid_to_client_num_stub(int, int client_num, int)
		{
			static thread_local uint64_t value;
			// zero xuid is invalid, so increase the clientnum to prevent 0 values
			value = static_cast<uint64_t>(client_num) + 1;
			return &value;
		}
	}

	const char* get_client_name(const uint64_t xuid)
	{
		if (xuid < 19 && !game::is_server())
		{
			char buffer[256];
			game::CL_GetClientName(0, static_cast<int>(xuid - 1), buffer, sizeof(buffer), true);

			return utils::string::va("%s\n", buffer);
		}

		return "Unknown Soldier";
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

				// Overwrite say command
				utils::hook::jump(0x14052A6C0_g, +[]
				{
					if (!game::get_dvar_bool("sv_running"))
					{
						printf("Server is not running\n");
						return;
					}

					const command::params params{};
					const auto text = params.join(1);

					const auto* format = reinterpret_cast<const char*>(0x140E25180_g);
					const auto* message = utils::string::va(format, 'O', text.data());

					game::SV_GameSendServerCommand(-1, game::SV_CMD_CAN_IGNORE_0, message);
				});

				// Overwrite tell command
				utils::hook::jump(0x14052A7E0_g, +[]
				{
					if (!game::get_dvar_bool("sv_running"))
					{
						printf("Server is not running\n");
						return;
					}

					const command::params params{};
					if (params.size() < 2)
					{
						return;
					}

					const auto client = atoi(params[1]);
					const auto text = params.join(2);

					const auto* format = reinterpret_cast<const char*>(0x140E25180_g);
					const auto* message = utils::string::va(format, 'O', text.data());

					game::SV_GameSendServerCommand(client, game::SV_CMD_CAN_IGNORE_0, message);
				});

				// Kill say fallback
				utils::hook::set<uint8_t>(0x1402FF987_g, 0xEB);
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
