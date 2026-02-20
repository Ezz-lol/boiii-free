#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "command.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

#include <mutex>

namespace dvars
{
	namespace
	{
		std::atomic_bool dvar_write_scheduled{false};
		bool initial_config_read = false;
		utils::hook::detour dvar_set_variant_hook;
		utils::hook::detour cmd_execute_single_command_hook;

		std::mutex binds_mutex;
		std::unordered_map<std::string, std::string> active_binds;
		std::atomic_bool binds_dirty{false};

		std::string get_binds_file_path()
		{
			return "boiii_players/user/binds.cfg";
		}

		void write_binds_file()
		{
			std::string buffer;
			{
				std::lock_guard lock(binds_mutex);
				for (const auto& [key, cmd] : active_binds)
				{
					buffer.append(cmd);
					buffer.append("\n");
				}
			}
			utils::io::write_file(get_binds_file_path(), buffer);
		}

		void schedule_binds_write()
		{
			if (binds_dirty.exchange(true))
			{
				return; // already scheduled
			}

			scheduler::once([]
			{
				binds_dirty = false;
				write_binds_file();
			}, scheduler::main, 3s);
		}

		void load_binds_from_file()
		{
			const auto path = get_binds_file_path();
			std::string data;
			if (!utils::io::read_file(path, &data) || data.empty())
			{
				return;
			}

			{
				std::lock_guard lock(binds_mutex);
				std::istringstream stream(data);
				std::string line;
				while (std::getline(stream, line))
				{
					if (!line.empty() && line.back() == '\r') line.pop_back();
					if (line.empty()) continue;

					if (utils::string::starts_with(line, "bind "))
					{
						auto rest = line.substr(5);
						auto key_start = rest.find_first_not_of(" \t");
						if (key_start == std::string::npos) continue;
						auto key_end = rest.find_first_of(" \t", key_start);
						if (key_end == std::string::npos) continue;
						auto key = rest.substr(key_start, key_end - key_start);
						active_binds[utils::string::to_lower(key)] = line;
					}
				}
			}

			game::Cbuf_ExecuteBuffer(0, game::ControllerIndex_t::CONTROLLER_INDEX_0, data.c_str());
		}

		void migrate_binds_from_config()
		{
			const auto binds_path = get_binds_file_path();
			if (utils::io::file_exists(binds_path))
			{
				return;
			}

			const std::string config_path = "boiii_players/user/config.cfg";
			std::string config_data;
			if (!utils::io::read_file(config_path, &config_data) || config_data.empty())
			{
				return;
			}

			std::string bind_lines;
			std::istringstream stream(config_data);
			std::string line;
			while (std::getline(stream, line))
			{
				if (!line.empty() && line.back() == '\r') line.pop_back();
				if (utils::string::starts_with(line, "bind "))
				{
					bind_lines.append(line);
					bind_lines.append("\n");
				}
			}

			if (!bind_lines.empty())
			{
				utils::io::write_file(binds_path, bind_lines);
			}
		}

		void cmd_execute_single_command_stub(int localClientNum,
			game::ControllerIndex_t controllerIndex, const char* text, bool fromRemoteConsole)
		{
			cmd_execute_single_command_hook.invoke(localClientNum, controllerIndex, text, fromRemoteConsole);

			if (!initial_config_read || !text || !*text)
			{
				return;
			}

			std::string cmd_text(text);

			auto start = cmd_text.find_first_not_of(" \t");
			if (start == std::string::npos) return;
			cmd_text = cmd_text.substr(start);

			const auto lower = utils::string::to_lower(cmd_text);

			if (utils::string::starts_with(lower, "bind "))
			{
				auto rest = cmd_text.substr(5);
				auto key_start = rest.find_first_not_of(" \t");
				if (key_start == std::string::npos) return;
				auto key_end = rest.find_first_of(" \t", key_start);
				if (key_end == std::string::npos) return;

				auto key = rest.substr(key_start, key_end - key_start);
				{
					std::lock_guard lock(binds_mutex);
					active_binds[utils::string::to_lower(key)] = cmd_text;
				}
				schedule_binds_write();
			}
			else if (utils::string::starts_with(lower, "unbind "))
			{
				auto rest = cmd_text.substr(7);
				auto key_start = rest.find_first_not_of(" \t");
				if (key_start == std::string::npos) return;
				auto key_end = rest.find_first_of(" \t\n\r", key_start);
				auto key = (key_end == std::string::npos)
					? rest.substr(key_start)
					: rest.substr(key_start, key_end - key_start);
				{
					std::lock_guard lock(binds_mutex);
					active_binds.erase(utils::string::to_lower(key));
				}
				schedule_binds_write();
			}
			else if (utils::string::starts_with(lower, "unbindall"))
			{
				{
					std::lock_guard lock(binds_mutex);
					active_binds.clear();
				}
				schedule_binds_write();
			}
		}

		void dvar_for_each_name_stub(void (*callback)(const char*))
		{
			for (int i = 0; i < *game::g_dvarCount; ++i)
			{
				const auto offset = game::is_server() ? 136 : 160;
				const auto* dvar = reinterpret_cast<game::dvar_t*>(&game::s_dvarPool[offset * i]);

				if (dvar->debugName //
					&& (dvar->flags & 0x8000) == 0 //
					&& (!game::Com_SessionMode_IsMode(game::MODE_COUNT)
						|| !game::Dvar_IsSessionModeBaseDvar(dvar)))
				{
					callback(dvar->debugName);
				}
			}
		}

		void dvar_for_each_name_client_num_stub(int localClientNum, void (*callback)(int, const char*))
		{
			for (int i = 0; i < *game::g_dvarCount; ++i)
			{
				const auto offset = game::is_server() ? 136 : 160;
				const auto* dvar = reinterpret_cast<game::dvar_t*>(&game::s_dvarPool[offset * i]);

				if (dvar->debugName //
					&& (dvar->flags & 0x8000) == 0 //
					&& (!game::Com_SessionMode_IsMode(game::MODE_COUNT)
						|| !game::Dvar_IsSessionModeBaseDvar(dvar)))
				{
					callback(localClientNum, dvar->debugName);
				}
			}
		}

		void read_dvar_name_hashes_data(std::unordered_map<std::uint32_t, std::string>& map)
		{
			const auto path = game::get_appdata_path() / "data/lookup_tables/dvar_list.txt";
			std::string data;

			if (!utils::io::read_file(path, &data))
			{
				printf("Failed to read Dvar lookup table\n");
				return;
			}

			const auto [beg, end] = std::ranges::remove(data, '\r');
			data.erase(beg, end);

			std::istringstream stream(data);
			std::string debug_name;

			while (std::getline(stream, debug_name, '\n'))
			{
				if (utils::string::starts_with(debug_name, "//"))
				{
					continue;
				}

				if (!debug_name.empty())
				{
					map.emplace(game::Dvar_GenerateHash(debug_name.data()), debug_name);
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

		std::string get_config_file_path()
		{
			return "boiii_players/user/config.cfg";
		}

		bool is_archive_dvar(const game::dvar_t* dvar)
		{
			if (!dvar->debugName)
			{
				return false;
			}

			return (dvar->flags & game::DVAR_ARCHIVE);
		}

		void write_archive_dvars()
		{
			const auto path = get_config_file_path();
			std::string config_buffer;

			for (int i = 0; i < *game::g_dvarCount; ++i)
			{
				const auto* dvar = reinterpret_cast<const game::dvar_t*>(&game::s_dvarPool[160 * i]);

				if (!is_archive_dvar(dvar))
				{
					continue;
				}

				const auto name = dvar->debugName;
				const auto value = game::Dvar_DisplayableValue(dvar);

				config_buffer.append(utils::string::va("set %s \"%s\"\n", name, value));
			}

			if (config_buffer.empty())
			{
				return;
			}

			utils::io::write_file(path, config_buffer);
		}

		void schedule_dvar_write()
		{
			if (dvar_write_scheduled.exchange(true))
			{
				return;
			}

			scheduler::once([]
			{
				dvar_write_scheduled = false;
				write_archive_dvars();
			}, scheduler::main, 10s);
		}

		void dvar_set_variant_stub(game::dvar_t* dvar, game::DvarValue* value, unsigned int source)
		{
			dvar_set_variant_hook.invoke(dvar, value, source);

			if (initial_config_read && is_archive_dvar(dvar))
			{
				schedule_dvar_write();
			}
		}

		void read_archive_dvars()
		{
			const std::string path = get_config_file_path();

			// Migrate bind lines from config.cfg to binds.cfg (one-time)
			migrate_binds_from_config();

			if (utils::io::file_exists(path))
			{
				std::string filedata;
				utils::io::read_file(path, &filedata);

				if (!filedata.empty())
				{
					game::Cbuf_ExecuteBuffer(0, game::ControllerIndex_t::CONTROLLER_INDEX_0, filedata.c_str());
				}
			}

			load_binds_from_file();

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
				scheduler::once(read_archive_dvars, scheduler::pipeline::dvars_flags_patched);
				dvar_set_variant_hook.create(0x1422C9030_g, dvar_set_variant_stub);

				// Hook command execution to intercept bind/unbind/unbindall
				cmd_execute_single_command_hook.create(0x1420ED380_g, cmd_execute_single_command_stub);

				// Show all known dvars in console
				utils::hook::jump(0x1422BCE30_g, dvar_for_each_name_stub);
				utils::hook::jump(0x1422BCD80_g, dvar_for_each_name_client_num_stub);
			}

			scheduler::once(copy_dvar_names_to_pool, scheduler::pipeline::main);

			// All dvars are recognized as command
			utils::hook::nop(game::select(0x142151F1A, 0x14050949A), 2);
			// Show all dvars in dvarlist command
			utils::hook::nop(game::select(0x142152227, 0x140509797), 6);
			// Show all dvars in dvardump command
			utils::hook::nop(game::select(0x142151BF9, 0x140509179), 6);
			// Stops game from deleting debug names from archive dvars
			utils::hook::set<uint8_t>(game::select(0x1422C5DE0, 0x1405786D0), 0xC3);
		}
	};
}

REGISTER_COMPONENT(dvars::component)
