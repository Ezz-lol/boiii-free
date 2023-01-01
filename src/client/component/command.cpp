#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "command.hpp"
#include "scheduler.hpp"
#include <utils/hook.hpp>
#include <utils/memory.hpp>

#include <game/game.hpp>

namespace command
{
	namespace
	{
		std::unordered_map<std::string, command_param_function>& get_command_map()
		{
			static std::unordered_map<std::string, command_param_function> command_map{};
			return command_map;
		}

		void execute_custom_command()
		{
			const params params{};

			auto& map = get_command_map();
			const auto entry = map.find(params[0]);
			if (entry != map.end())
			{
				entry->second(params);
			}
		}

		game::CmdArgs* get_cmd_args()
		{
			return game::Sys_GetTLS()->cmdArgs;
		}

		void update_whitelist_stub()
		{
			game::cmd_function_s* current_function = game::cmd_functions;
			while (current_function)
			{
				current_function->autoComplete = 1;
				current_function = current_function->next;
			}
		}
	}

	struct component final : client_component
	{
		void post_unpack() override
		{
			// Disable whitelist
			utils::hook::jump(0x14133CF70_g, update_whitelist_stub);
		}
	};

	params::params()
		: nesting_(get_cmd_args()->nesting)
	{
		assert(this->nesting_ < game::CMD_MAX_NESTING);
	}

	int params::size() const
	{
		return get_cmd_args()->argc[this->nesting_];
	}

	const char* params::get(const int index) const
	{
		if (index >= this->size())
		{
			return "";
		}

		return get_cmd_args()->argv[this->nesting_][index];
	}

	std::string params::join(const int index) const
	{
		std::string result = {};

		for (auto i = index; i < this->size(); i++)
		{
			if (i > index) result.append(" ");
			result.append(this->get(i));
		}
		return result;
	}

	void add(std::string command, command_function function)
	{
		add(std::move(command), [f = std::move(function)](const params&)
		{
			f();
		});
	}

	void add(std::string command, command_param_function function)
	{
		auto& map = get_command_map();
		const auto reregister = map.contains(command);

		if (!reregister)
		{
			auto& allocator = *utils::memory::get_allocator();
			auto* cmd_function = allocator.allocate<game::cmd_function_s>();
			const auto* cmd_string = allocator.duplicate_string(command);

			game::Cmd_AddCommandInternal(cmd_string, execute_custom_command, cmd_function);
			cmd_function->autoComplete = 1;
		}

		map[std::move(command)] = std::move(function);
	}
}

REGISTER_COMPONENT(command::component)
