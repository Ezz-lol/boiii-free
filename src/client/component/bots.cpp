#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "command.hpp"

#include <utils/nt.hpp>
#include <utils/hook.hpp>

#include <game/game.hpp>

namespace bots
{
	namespace
	{
		const std::vector<std::string>& get_bot_names()
		{
			static const auto bot_names = []
			{
				std::vector<std::string> names{
					"momo5502",
					"Maurice",
					"Jasmin",
				};

				std::random_device rd;
				std::mt19937 gen(rd());
				std::ranges::shuffle(names, gen);
				return names;
			}();

			return bot_names;
		}

		const char* get_bot_name()
		{
			static size_t current = 0;
			const auto& names = get_bot_names();

			current = (current + 1) % names.size();
			return names.at(current).data();
		}
	}

	struct component final : client_component_interface
	{
		void post_unpack() override
		{
			utils::hook::jump(0x141653B70_g, get_bot_name);
			utils::hook::jump(0x141654280_g, get_bot_name);

			command::add("spawnBot", [](const command::params& params)
			{
				size_t count = 1;
				if (params.size() > 1)
				{
					if (params[1] == "all"s)
					{
						count = 18;
					}
					else
					{
						count = atoi(params[1]);
					}
				}

				for (size_t i = 0; i < count; ++i)
				{
					if (!game::SV_AddTestClient())
					{
						break;
					}
				}
			});
		}
	};
}

REGISTER_COMPONENT(bots::component)
