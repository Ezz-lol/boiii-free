#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "network.hpp"
#include "console.hpp"
#include "command.hpp"
#include "scheduler.hpp"

#include <utils/finally.hpp>

#include <game/utils.hpp>

namespace rcon
{
	namespace
	{
		const game::dvar_t* rcon_timeout;

		std::unordered_map<game::netadr_t, int> rate_limit_map;

		std::optional<std::string> get_and_validate_rcon_command(const std::string& data)
		{
			const command::params params{data};

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

		bool rate_limit_check(const game::netadr_t& address, const int time)
		{
			const auto last_time = rate_limit_map[address];

			if (last_time && (time - last_time) < rcon_timeout->current.value.integer)
			{
				return false; // Flooding
			}

			rate_limit_map[address] = time;
			return true;
		}

		void rate_limit_cleanup(const int time)
		{
			for (auto i = rate_limit_map.begin(); i != rate_limit_map.end();)
			{
				// No longer at risk of flooding, remove
				if ((time - i->second) > rcon_timeout->current.value.integer)
				{
					i = rate_limit_map.erase(i);
				}
				else
				{
					++i;
				}
			}
		}

		void rcon_handler(const game::netadr_t& target, const network::data_view& data)
		{
			const auto time = game::Sys_Milliseconds();
			if (!rate_limit_check(target, time))
			{
				return;
			}

			rate_limit_cleanup(time);

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

			rcon_timeout = game::register_dvar_int("rcon_timeout", 500, 100, 10000, game::DVAR_NONE, "");
		}
	};
}

REGISTER_COMPONENT(rcon::component)
