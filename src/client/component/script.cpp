#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include "game_event.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>

namespace script
{
	namespace
	{
		constexpr size_t GSC_MAGIC = 0x1C000A0D43534780;

		utils::hook::detour db_find_x_asset_header_hook;
		utils::hook::detour gscr_get_bgb_remaining_hook;

		utils::memory::allocator allocator;
		std::unordered_map<std::string, game::RawFile*> loaded_scripts;

		game::RawFile* get_loaded_script(const std::string& name)
		{
			const auto itr = loaded_scripts.find(name);
			return (itr == loaded_scripts.end()) ? nullptr : itr->second;
		}

		void print_loading_script(const std::string& name)
		{
			printf("Loading GSC script '%s'\n", name.data());
		}

		void load_script(std::string& name, const std::string& data, const bool is_custom)
		{
			const auto appdata_path = (game::get_appdata_path() / "data/").generic_string();
			const auto host_path = (utils::nt::library{}.get_folder() / "boiii/").generic_string();

			auto i = name.find(appdata_path);
			if (i != std::string::npos)
			{
				name.erase(i, appdata_path.length());
			}

			i = name.find(host_path);
			if (i != std::string::npos)
			{
				name.erase(i, host_path.length());
			}

			auto base_name = name;
			if (!utils::string::ends_with(name, ".gsc"))
			{
				printf("GSC script '%s' failed to load due to invalid suffix.\n", name.data());
				return;
			}

			if (is_custom)
			{
				// .gsc suffix will be added back by Scr_LoadScript
				base_name = name.substr(0, name.size() - 4);
				if (base_name.empty())
				{
					printf("GSC script '%s' failed to load due to invalid name.\n", name.data());
					return;
				}
			}

			auto* raw_file = allocator.allocate<game::RawFile>();
			// use script name with .gsc suffix for DB_FindXAssetHeader hook
			raw_file->name = allocator.duplicate_string(name);
			raw_file->buffer = allocator.duplicate_string(data);
			raw_file->len = static_cast<int>(data.length());

			loaded_scripts[name] = raw_file;

			if (is_custom)
			{
				game::Scr_LoadScript(game::SCRIPTINSTANCE_SERVER, base_name.data());
			}
		}

		void load_scripts_folder(const std::string& script_dir, const bool is_custom)
		{
			if (!utils::io::directory_exists(script_dir))
			{
				return;
			}

			const auto scripts = utils::io::list_files(script_dir);

			std::error_code e;
			for (const auto& script : scripts)
			{
				std::string data;
				auto script_file = script.generic_string();
				if (!std::filesystem::is_directory(script, e) && utils::io::read_file(script_file, &data))
				{
					if (data.size() >= sizeof(GSC_MAGIC) && !std::memcmp(data.data(), &GSC_MAGIC, sizeof(GSC_MAGIC)))
					{
						print_loading_script(script_file);
						load_script(script_file, data, is_custom);
					}

					continue;
				}

				// Do not traverse directories for custom scripts.
				if (std::filesystem::is_directory(script, e) && !is_custom)
				{
					load_scripts_folder(script_file, is_custom);
				}
			}
		}

		std::optional<std::filesystem::path> get_game_type_specific_folder()
		{
			switch (game::Com_SessionMode_GetMode())
			{
			case game::MODE_MULTIPLAYER:
				return "mp";
			case game::MODE_ZOMBIES:
				return "zm";
			case game::MODE_CAMPAIGN:
				return "cp";
			default:
				return {};
			}
		}

		void load_scripts()
		{
			const utils::nt::library host{};

			const auto data_folder = game::get_appdata_path() / "data";
			const auto boiii_folder = host.get_folder() / "boiii";

			const auto load = [&data_folder, &boiii_folder](const std::filesystem::path& folder, const bool is_custom)
			{
				load_scripts_folder((data_folder / folder).string(), is_custom);
				load_scripts_folder((boiii_folder / folder).string(), is_custom);
			};

			// scripts folder is for overriding stock scripts the game uses
			load("scripts", false);

			// custom_scripts is for loading completely custom scripts the game doesn't use
			load("custom_scripts", true);

			if (const auto game_type = get_game_type_specific_folder(); game_type.has_value())
			{
				load("custom_scripts" / game_type.value(), true);
			}
		}

		game::RawFile* db_find_x_asset_header_stub(const game::XAssetType type, const char* name,
		                                        const bool error_if_missing,
		                                        const int wait_time)
		{
			auto* asset_header = db_find_x_asset_header_hook.invoke<game::RawFile*>(
				type, name, error_if_missing, wait_time);

			if (type != game::ASSET_TYPE_SCRIPTPARSETREE)
			{
				return asset_header;
			}

			auto* script = get_loaded_script(name);
			if (script)
			{
				return script;
			}

			return asset_header;
		}

		void clear_script_memory()
		{
			loaded_scripts.clear();
			allocator.clear();
		}

		void begin_load_scripts_stub(game::scriptInstance_t inst, int user)
		{
			game::Scr_BeginLoadScripts(inst, user);

			if (game::Com_IsInGame() && !game::Com_IsRunningUILevel())
			{
				load_scripts();
			}
		}

		int server_script_checksum_stub()
		{
			return 1;
		}

		void scr_loot_get_item_quantity_stub([[maybe_unused]] game::scriptInstance_t inst,
		                                     [[maybe_unused]] game::scr_entref_t entref)
		{
			game::Scr_AddInt(game::SCRIPTINSTANCE_SERVER, 255);
		}
	}

	struct component final : generic_component
	{
		void post_unpack() override
		{
			// Return custom or overrided scripts if found
			db_find_x_asset_header_hook.create(game::select(0x141420ED0, 0x1401D5FB0), db_find_x_asset_header_stub);

			// Free our scripts when the game ends
			game_event::on_g_shutdown_game(clear_script_memory);

			// Load our custom/overriding scripts
			utils::hook::call(game::select(0x141AAE92F, 0x1402D81FF), begin_load_scripts_stub);

			// Force GSC checksums to be valid
			utils::hook::call(game::select(0x1408F2E5D, 0x1400E2D22), server_script_checksum_stub);

			// Workaround for "Out of X" gobblegum
			gscr_get_bgb_remaining_hook.create(game::select(0x141A8CAB0, 0x1402D2310),
			                                   scr_loot_get_item_quantity_stub);
		}
	};
};

REGISTER_COMPONENT(script::component)
