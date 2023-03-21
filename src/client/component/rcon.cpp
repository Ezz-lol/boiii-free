#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "network.hpp"
#include "console.hpp"
#include "command.hpp"
#include "scheduler.hpp"

#include <utils/finally.hpp>
#include <utils/string.hpp>

#include <game/utils.hpp>

namespace rcon
{
	namespace
	{
		std::optional<std::string> get_and_validate_rcon_command(const std::string& data)
		{
			const command::params params{data.data()};

			if (params.size() <= 1)
			{
				return {};
			}

			if (params[0] != game::get_dvar_string("rcon_password"))
			{
				return {};
			}

			return params.join(1);
		}

		void rcon_executer(const game::netadr_t& target, const std::string& data)
		{
			const auto command = get_and_validate_rcon_command(data);
			if (!command)
			{
				return;
			}

			std::string console_buffer{};

			console::scoped_interceptor _([&console_buffer](const std::string& text)
			{
				console_buffer += text;
			});

			game::Cmd_ExecuteSingleCommand(0, game::CONTROLLER_INDEX_FIRST, command->data(), true);

			network::send(target, "print", console_buffer);
		}

		void rcon_handler(const game::netadr_t& target, const network::data_view& data)
		{
			auto str_data = std::string(reinterpret_cast<const char*>(data.data()), data.size());
			scheduler::once([target, s = std::move(str_data)]
			{
				rcon_executer(target, s);
			}, scheduler::main);
		}
	}

	struct component final : server_component
	{
		void post_unpack() override
		{
			network::on("rcon", rcon_handler);
		}
	};
}

REGISTER_COMPONENT(rcon::component)
