#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include "command.hpp"
#include "client_command.hpp"

namespace client_command
{
	namespace
	{
		std::unordered_map<std::string, callback> handlers;

		void client_command_stub(const int client_num)
		{
			const auto ent = &game::g_entities[client_num];

			if (ent->client == nullptr)
			{
				// Client is not fully in game
				return;
			}

			const command::params_sv params;

			const auto command = utils::string::to_lower(params.get(0));
			if (const auto got = handlers.find(command); got != handlers.end())
			{
				got->second(ent, params);
				return;
			}

			utils::hook::invoke<void>(0x140295C40_g, client_num);
		}
	}

	void add(const std::string& name, const callback& cmd)
	{
		const auto command = utils::string::to_lower(name);
		handlers[command] = cmd;
	}

	class component final : public server_component
	{
	public:
		void post_unpack() override
		{
			utils::hook::call(0x14052F81B_g, client_command_stub);
		}
	};
}

REGISTER_COMPONENT(client_command::component)
