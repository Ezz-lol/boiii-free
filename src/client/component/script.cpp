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
		utils::hook::detour db_findxassetheader_hook;

		struct globals_t
		{
			std::unordered_map<std::string, game::RawFile*> loaded_scripts;
		};

		globals_t globals;

		game::RawFile* get_loaded_script(const std::string& name)
		{
			const auto itr = globals.loaded_scripts.find(name);
			return (itr == globals.loaded_scripts.end()) ? NULL : itr->second;
		}

		void print_loading_script(const std::string& name)
		{
			printf("Loading GSC script '%s'\n", name.data());
		}

		void load_script(std::string& name, const std::string& data)
		{
			auto& allocator = *utils::memory::get_allocator();
			auto file_string = static_cast<char*>(allocator.allocate(data.length()));
			std::memcpy(file_string, data.data(), data.length());

			const utils::nt::library host{};
			auto appdata_path = (game::get_appdata_path() / "data/").generic_string();
			auto host_path = (host.get_folder() / "boiii/").generic_string();
			std::string::size_type i = name.find(appdata_path);
			if (i != std::string::npos)
				name.erase(i, appdata_path.length());
			i = name.find(host_path);
			if (i != std::string::npos)
				name.erase(i, host_path.length());

			auto rawfile = static_cast<game::RawFile*>(allocator.allocate(24));
			rawfile->name = name.c_str();
			rawfile->buffer = file_string;
			rawfile->len = static_cast<int>(data.length());

			globals.loaded_scripts[name] = rawfile;
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
					print_loading_script(script_file);
					load_script(script_file, data);
				}
				else if (std::filesystem::is_directory(script))
				{
					load_scripts_folder(script_file);
				}
			}
		}

		void load_scripts()
		{
			globals = {};
			const utils::nt::library host{};
			load_scripts_folder((game::get_appdata_path() / "data/scripts").string());
			load_scripts_folder((host.get_folder() / "boiii/scripts").string());
		}

		game::RawFile* db_findxassetheader_stub(game::XAssetType type, const char* name, bool errorIfMissing, int waitTime)
		{
			if (type != game::ASSET_TYPE_SCRIPTPARSETREE)
			{
				return db_findxassetheader_hook.invoke<game::RawFile*>(type, name, errorIfMissing, waitTime);
			}

			auto asset_header = db_findxassetheader_hook.invoke<game::RawFile*>(type, name, errorIfMissing, waitTime);
			if (globals.loaded_scripts.contains(name))
			{
				auto script = get_loaded_script(name);

				utils::hook::copy((void*)(script->buffer + 0x8), asset_header->buffer + 0x8, 4); // Copy over the checksum of the original script

				return script;
			}

			return asset_header;
		}
	}
		
	struct component final : generic_component
	{
		void post_unpack() override
		{
			if (game::is_server())
			{
				load_scripts();
			}
			else
			{
				scheduler::once(load_scripts, scheduler::pipeline::renderer);
			}

			db_findxassetheader_hook.create(game::select(0x141420ED0, 0x1401D5FB0), db_findxassetheader_stub);
		}

	};
};

REGISTER_COMPONENT(script::component)
