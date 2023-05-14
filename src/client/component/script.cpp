#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include "scheduler.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>

namespace script
{
	namespace
	{
		constexpr size_t GSC_MAGIC = 0x1C000A0D43534780;

		utils::hook::detour db_findxassetheader_hook;
		utils::hook::detour gscr_get_bgb_remaining_hook;

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

		void load_script(std::string& name, const std::string& data)
		{
			auto& allocator = *utils::memory::get_allocator();
			const auto* file_string = allocator.duplicate_string(data);

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
			if (utils::string::ends_with(name, ".gsc"))
			{
				base_name = name.substr(0, name.size() - 4); // .gsc suffix will be readded by Scr_LoadScript
			}

			if (base_name.empty())
			{
				printf("Not loading GSC script '%s' due to invalid name.\n", name.data());
				return;
			}

			auto* rawfile = allocator.allocate<game::RawFile>();
			rawfile->name = name.data(); // use script name with .gsc suffix for FindXAssetHeader hook
			rawfile->buffer = file_string;
			rawfile->len = static_cast<int>(data.length());

			loaded_scripts[name] = rawfile;

			game::Scr_LoadScript(game::SCRIPTINSTANCE_SERVER, base_name.data());
		}

		void load_scripts_folder(const std::string& script_dir)
		{
			if (!utils::io::directory_exists(script_dir))
			{
				return;
			}

			const auto scripts = utils::io::list_files(script_dir);

			for (const auto& script : scripts)
			{
				std::string data;
				auto script_file = script.generic_string();
				if (!std::filesystem::is_directory(script) && utils::io::read_file(script_file, &data))
				{
					if (data.size() >= sizeof(GSC_MAGIC) && !std::memcmp(data.data(), &GSC_MAGIC, sizeof(GSC_MAGIC)))
					{
						print_loading_script(script_file);
						load_script(script_file, data);
					}
				}
				else if (std::filesystem::is_directory(script))
				{
					load_scripts_folder(script_file);
				}
			}
		}

		void load_scripts()
		{
			loaded_scripts = {};
			const utils::nt::library host{};
			load_scripts_folder((game::get_appdata_path() / "data/scripts").string());
			load_scripts_folder((host.get_folder() / "boiii/scripts").string());
		}

		game::RawFile* db_findxassetheader_stub(const game::XAssetType type, const char* name,
		                                        const bool error_if_missing,
		                                        const int wait_time)
		{
			auto* asset_header = db_findxassetheader_hook.invoke<game::RawFile*>(
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

		void load_gametype_script_stub()
		{
			if (!game::Com_IsInGame() || game::Com_IsRunningUILevel())
			{
				game::GScr_LoadGametypeScript();
				return;
			}

			game::GScr_LoadGametypeScript();
			load_scripts();
		}

		int server_script_checksum_stub()
		{
			return 1;
		}

		void gscr_get_bgb_remaining_stub(game::scriptInstance_t inst, void* entref)
		{
			game::Scr_AddInt(game::SCRIPTINSTANCE_SERVER, 255);
		}
	}

	struct component final : generic_component
	{
		void post_unpack() override
		{
			// Return custom or overrided scripts if found
			db_findxassetheader_hook.create(game::select(0x141420ED0, 0x1401D5FB0), db_findxassetheader_stub);

			// Load our scripts when the gametype script is loaded
			utils::hook::call(game::select(0x141AAF37C, 0x1402D8C7F), load_gametype_script_stub);

			// Force GSC checksums to be valid
			utils::hook::call(game::select(0x1408F2E5D, 0x1400E2D22), server_script_checksum_stub);

			// Workaround for "Out of X" gobblegum
			gscr_get_bgb_remaining_hook.create(game::select(0x141A8CAB0, 0x1402D2310), gscr_get_bgb_remaining_stub);
		}
	};
};

REGISTER_COMPONENT(script::component)
