#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

namespace dvars
{
	namespace
	{
		std::atomic_bool dvar_write_scheduled{false};
		bool initial_config_read = false;
		utils::hook::detour dvar_set_variant_hook;

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
			return "players/user/config.cfg";
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

			if (config_buffer.length() == 0)
			{
				return;
			}

			utils::io::write_file(get_config_file_path(), config_buffer);
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
				scheduler::once(read_archive_dvars, scheduler::pipeline::dvars_flags_patched);
				dvar_set_variant_hook.create(0x1422C9030_g, dvar_set_variant_stub);

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
