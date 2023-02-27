#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

#include "scheduler.hpp"

namespace dvars
{
	namespace
	{
		bool initial_config_read = false;
		utils::hook::detour dvar_set_variant_hook;

		void dvar_for_each_name_stub(void (*callback)(const char*))
		{
			for (int i = 0; i < *game::g_dvarCount; ++i)
			{
				const auto offset = game::is_server() ? 136 : 160;
				const auto* dvar = reinterpret_cast<game::dvar_t*>(&game::s_dvarPool[offset * i]);

				if ((!game::Com_SessionMode_IsMode(game::MODE_COUNT) ||
					!game::Dvar_IsSessionModeBaseDvar(dvar)) &&
					(dvar->flags & 0x8000) == 0)
				{
					if (dvar->debugName)
					{
						callback(dvar->debugName);
					}
				}
			}
		}

		void dvar_for_each_name_client_num_stub(int localClientNum, void (*callback)(int, const char*))
		{
			for (int i = 0; i < *game::g_dvarCount; ++i)
			{
				const auto offset = game::is_server() ? 136 : 160;
				const auto* dvar = reinterpret_cast<game::dvar_t*>(&game::s_dvarPool[offset * i]);

				if ((!game::Com_SessionMode_IsMode(game::MODE_COUNT) ||
					!game::Dvar_IsSessionModeBaseDvar(dvar)) &&
					(dvar->flags & 0x8000) == 0)
				{
					if (dvar->debugName)
					{
						callback(localClientNum, dvar->debugName);
					}
				}
			}
		}

		void read_dvar_name_hashes_data(std::unordered_map<std::uint32_t, std::string>& map)
		{
			const auto path = game::get_appdata_path() / "data" / "lookup_tables" / "dvar_lookup_table.csv";
			std::string data;

			if (!utils::io::read_file(path, &data))
			{
				printf("Failed to read Dvar lookup table\n");
				return;
			}

			data.erase(std::ranges::remove(data, '\r').begin(), data.end());

			std::istringstream stream(data);
			std::string line;

			while (std::getline(stream, line, '\n'))
			{
				if (utils::string::starts_with(line, "//"))
				{
					continue;
				}

				const auto separator = line.find(',');

				if (separator == std::string::npos)
				{
					continue;
				}

				const auto debug_name = line.substr(separator + 1);

				if (!debug_name.empty())
				{
					std::istringstream hash_string(line.substr(0, separator));
					std::uint32_t hash_value;

					hash_string >> hash_value;

					map.emplace(hash_value, debug_name);
				}
			}
		}

		void copy_dvar_names_to_pool()
		{
			std::unordered_map<std::uint32_t, std::string> dvar_hash_name_map;
			read_dvar_name_hashes_data(dvar_hash_name_map);

			for (int i = 0; i < *game::g_dvarCount; ++i)
			{
				const auto offset = game::is_server() ? 136 : 160;
				auto* dvar = reinterpret_cast<game::dvar_t*>(&game::s_dvarPool[offset * i]);

				if (!dvar->debugName)
				{
					const auto it = dvar_hash_name_map.find(dvar->name);
					if (it != dvar_hash_name_map.end())
					{
						dvar->debugName = game::CopyString(it->second.data());
					}
				}
			}
		}

		const std::string get_config_file_path()
		{
			return "players/user/config.cfg";
		}

		void write_archive_dvars()
		{
			std::string config_buffer;

			for (int i = 0; i < *game::g_dvarCount; ++i)
			{
				const auto* dvar = reinterpret_cast<const game::dvar_t*>(&game::s_dvarPool[160 * i]);

				if (!dvar->debugName)
					continue;

				auto name = dvar->debugName;
				auto value = game::Dvar_DisplayableValue(dvar);

				config_buffer.append(utils::string::va("set %s \"%s\"\n", name, value));
			}

			if (config_buffer.length() == 0)
				return;

			utils::io::write_file(get_config_file_path(), config_buffer);
		}

		void dvar_set_variant_stub(game::dvar_t* dvar, game::DvarValue* value, unsigned int source)
		{
			dvar_set_variant_hook.invoke(dvar, value, source);

			if (initial_config_read && dvar->debugName)
			{
				write_archive_dvars();
			}
		}

		void read_archive_dvars()
		{
			const std::string path = get_config_file_path();

			if (!utils::io::file_exists(path))
			{
				initial_config_read = true;
				return;
			}

			std::string filedata;
			utils::io::read_file(path, &filedata);

			game::Cbuf_ExecuteBuffer(0, game::ControllerIndex_t::CONTROLLER_INDEX_0, filedata.c_str());
			initial_config_read = true;
			scheduler::execute(scheduler::pipeline::dvars_loaded);
		}
	}

	class component final : public generic_component
	{
	public:
		void post_unpack() override
		{
			if (!game::is_server())
			{
				scheduler::once(read_archive_dvars, scheduler::pipeline::main);
				dvar_set_variant_hook.create(0x1422C9A90_g, dvar_set_variant_stub);

				// Show all known dvars in console
				utils::hook::jump(0x1422BD890_g, dvar_for_each_name_stub);
				utils::hook::jump(0x1422BD7E0_g, dvar_for_each_name_client_num_stub);
			}

			scheduler::once(copy_dvar_names_to_pool, scheduler::pipeline::main);

			// All dvars are recognized as command
			utils::hook::nop(game::select(0x14215297A, 0x14050949A), 2);
			// Show all dvars in dvarlist command
			utils::hook::nop(game::select(0x142152C87, 0x140509797), 6);
			// Show all dvars in dvardump command
			utils::hook::nop(game::select(0x142152659, 0x140509179), 6);
		}
	};
}

REGISTER_COMPONENT(dvars::component)
