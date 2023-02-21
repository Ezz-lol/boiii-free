#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include <utils/hook.hpp>

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
	}

	class component final : public server_component
	{
	public:
		void post_unpack() override
		{
			client_command::add("say", cmd_say_f);
			client_command::add("say_team", cmd_say_f);

			client_command::add("chat", cmd_chat_f);
		}
	};
}

REGISTER_COMPONENT(chat::component)
