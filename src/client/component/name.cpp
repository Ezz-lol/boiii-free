#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "name.hpp"
#include "steam_proxy.hpp"
#include "command.hpp"

#include <utils/nt.hpp>
#include <utils/string.hpp>
#include <utils/properties.hpp>
#include <utils/concurrency.hpp>

namespace name
{
	namespace
	{
		utils::concurrency::container<std::string> player_name{};

		void store_player_name(const std::string& name)
		{
			utils::properties::store("playerName", name);
		}

		void activate_player_name(std::string new_name)
		{
			player_name.access([&](std::string& name)
			{
				name = std::move(new_name);
			});
		}

		void update_player_name(const std::string& new_name)
		{
			activate_player_name(new_name);
			store_player_name(new_name);
		}

		void setup_player_name()
		{
			std::string initial_name = steam_proxy::get_player_name();

			if (initial_name.empty())
			{
				initial_name = utils::nt::get_user_name();
			}

			if (initial_name.empty())
			{
				initial_name = "Unknown Soldier";
			}

			update_player_name(initial_name);
		}

		void load_player_name()
		{
			const auto stored_name = utils::properties::load("playerName");

			if (stored_name)
			{
				activate_player_name(*stored_name);
			}
			else
			{
				setup_player_name();
			}
		}
	}

	struct component final : client_component
	{
		void post_load() override
		{
			load_player_name();
		}

		void post_unpack() override
		{
			command::add("name", [](const command::params& params)
			{
				if (params.size() != 2)
				{
					return;
				}

				update_player_name(params[1]);
			});
		}

		component_priority priority() const override
		{
			return component_priority::name;
		}
	};

	const char* get_player_name()
	{
		const auto name = player_name.copy();
		return utils::string::va("%.*s", static_cast<int>(name.size()), name.data());
	}
}

REGISTER_COMPONENT(name::component)
