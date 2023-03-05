#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include "command.hpp"
#include "console_command.hpp"

namespace console_command
{
	namespace
	{
		utils::hook::detour console_command_hook;

		std::unordered_map<std::string, callback> handlers;

		int console_command_stub()
		{
			const command::params params;

			const auto command = utils::string::to_lower(params.get(0));
			if (const auto got = handlers.find(command); got != handlers.end())
			{
				got->second(params);
				return 1;
			}

			return console_command_hook.invoke<int>();
		}
	}

	void add_console(const std::string& name, const callback& cmd)
	{
		const auto command = utils::string::to_lower(name);
		handlers[command] = cmd;
	}

	class component final : public server_component
	{
	public:
		void post_unpack() override
		{
			console_command_hook.create(0x1402FF8C0_g, &console_command_stub);
		}
	};
}

REGISTER_COMPONENT(console_command::component)
