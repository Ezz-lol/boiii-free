#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "command.hpp"

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

#include <game/game.hpp>
#include "getinfo.hpp"

namespace bots
{
	namespace
	{
		constexpr auto* bot_format_string = "connect \"\\invited\\1\\cg_predictItems\\1\\cl_anonymous\\0\\color\\4\\head\\default\\model\\multi\\snaps\\20\\rate\\"
			"5000\\name\\%s\\clanAbbrev\\%s\\xuid\\%s\\xnaddr\\%s\\natType\\2\\protocol\\%d\\netfieldchk\\%d\\sessionmode\\%s\\qport\\%d\"";

		using bot_name = std::pair<std::string, std::string>;

		std::vector<bot_name> load_bots_names()
		{
			std::vector<bot_name> bot_names =
			{
				{"momo5502", "IW5x"},
				{"Jasmin", "<3"},
				{"Dss0", "IW3x"},
				{"FutureRave", "FR"},
				{"Diamante", "IW2x"},
				{"St0rm", "NN"},
				{"Joel", "NN"},
				{"Louve", "IW5x"},
			};

			std::string buffer;
			if (!utils::io::read_file("boiii_cfg/bots.txt", &buffer) || buffer.empty())
			{
				return bot_names;
			}

			auto data = utils::string::split(buffer, '\n');
			for (auto& entry : data)
			{
				utils::string::replace(entry, "\r", "");
				utils::string::trim(entry);

				if (entry.empty())
				{
					continue;
				}

				std::string clan_abbrev;
				// Check if there is a clan tag
				if (const auto pos = entry.find(','); pos != std::string::npos)
				{
					// Only start copying over from non-null characters (otherwise it can be "<=")
					if ((pos + 1) < entry.size())
					{
						clan_abbrev = entry.substr(pos + 1);
					}

					entry = entry.substr(0, pos);
				}

				bot_names.emplace_back(std::make_pair(entry, clan_abbrev));
			}

			return bot_names;
		}

		const std::vector<bot_name>& get_bot_names()
		{
			static const auto bot_names = []
			{
				auto names = load_bots_names();

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
			return names.at(current).first.data();
		}

		int format_bot_string(char* buffer, [[maybe_unused]] const char* format, const char* name, const char* xuid,
			const char* xnaddr, int protocol, int net_field_chk, const char* session_mode, int qport)
		{
			const auto find_name = [](const std::string& needle) -> const char*
			{
				for (const auto& entry : get_bot_names())
				{
					if (entry.first == needle)
					{
						return entry.second.data();
					}
				}

				return "3arc";
			};

			return sprintf_s(buffer, 1024, bot_format_string, name, find_name(name), xuid, xnaddr, protocol, net_field_chk, session_mode, qport);
		}
	}

	struct component final : generic_component
	{
		static_assert(offsetof(game::client_s, bIsTestClient) == 0xBB360);

		void post_unpack() override
		{
			utils::hook::jump(game::select(0x141653B70, 0x1402732E0), get_bot_name);
			utils::hook::call(game::select(0x142249097, 0x14052E53A), format_bot_string);

			if (!game::is_server())
			{
				utils::hook::jump(0x141654280_g, get_bot_name); // SV_ZombieNameRandom
			}

			command::add("spawnBot", [](const command::params& params)
			{
				if (!getinfo::is_host())
				{
					return;
				}

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
