#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "workshop.hpp"

#include "game/game.hpp"
#include "game/utils.hpp"
#include "command.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>

namespace workshop
{
	namespace
	{
		utils::hook::detour setup_server_map_hook;
		utils::hook::detour load_usermap_hook;

		bool has_mod(const std::string& pub_id)
		{
			for (unsigned int i = 0; i < *game::modsCount; ++i)
			{
				const auto& mod_data = game::modsPool[i];
				if (mod_data.publisherId == pub_id)
				{
					return true;
				}
			}

			return false;
		}

		void load_usermap_mod_if_needed()
		{
			if (!game::isModLoaded())
			{
				game::loadMod(0, "usermaps", false);
			}
		}

		void setup_server_map_stub(int localClientNum, const char* map, const char* gametype)
		{
			if (utils::string::is_numeric(map) ||
				!get_usermap_publisher_id(map).empty())
			{
				load_usermap_mod_if_needed();
			}

			setup_server_map_hook.invoke(localClientNum, map, gametype);
		}

		void load_workshop_data(game::workshop_data& item)
		{
			const auto base_path = item.absolutePathZoneFiles;
			const auto path = utils::string::va("%s/workshop.json", base_path);
			const auto json_str = utils::io::read_file(path);

			if (json_str.empty())
			{
				printf("[ Workshop ] workshop.json has not been found in folder:\n%s\n", path);
				return;
			}

			rapidjson::Document doc;
			const rapidjson::ParseResult parse_result = doc.Parse(json_str);

			if (parse_result.IsError() || !doc.IsObject())
			{
				printf("[ Workshop ] Unable to parse workshop.json from folder:\n%s\n", path);
				return;
			}

			if (!doc.HasMember("Title") ||
				!doc.HasMember("Description") ||
				!doc.HasMember("FolderName") ||
				!doc.HasMember("PublisherID"))
			{
				printf("[ Workshop ] workshop.json is invalid:\n%s\n", path);
				return;
			}

			utils::string::copy(item.title, doc["Title"].GetString());
			utils::string::copy(item.description, doc["Description"].GetString());
			utils::string::copy(item.folderName, doc["FolderName"].GetString());
			utils::string::copy(item.publisherId, doc["PublisherID"].GetString());
			item.publisherIdInteger = std::strtoul(item.publisherId, nullptr, 10);
		}

		void load_usermap_content_stub(void* usermaps_count, int type)
		{
			utils::hook::invoke<void>(game::select(0x1420D6430, 0x1404E2360), usermaps_count, type);

			for (unsigned int i = 0; i < *game::usermapsCount; ++i)
			{
				auto& usermap_data = game::usermapsPool[i];

				// foldername == title -> non-steam workshop usercontent
				if (std::strcmp(usermap_data.folderName, usermap_data.title) != 0)
				{
					continue;
				}

				load_workshop_data(usermap_data);
			}
		}

		void load_mod_content_stub(void* mods_count, int type)
		{
			utils::hook::invoke<void>(game::select(0x1420D6430, 0x1404E2360), mods_count, type);

			for (unsigned int i = 0; i < *game::modsCount; ++i)
			{
				auto& mod_data = game::modsPool[i];

				if (std::strcmp(mod_data.folderName, mod_data.title) != 0)
				{
					continue;
				}

				load_workshop_data(mod_data);
			}
		}

		game::workshop_data* load_usermap_stub(const char* map_arg)
		{
			std::string pub_id = map_arg;
			if (!utils::string::is_numeric(map_arg))
			{
				pub_id = get_usermap_publisher_id(map_arg);
			}

			return load_usermap_hook.invoke<game::workshop_data*>(pub_id.data());
		}

		bool has_workshop_item_stub(int type, const char* map, int a3)
		{
			std::string pub_id = map;
			if (!utils::string::is_numeric(map))
			{
				pub_id = get_usermap_publisher_id(map);
			}

			return utils::hook::invoke<bool>(0x1420D6380_g, type, pub_id.data(), a3);
		}

		const char* va_mods_path_stub(const char* fmt, const char* root_dir, const char* mods_dir, const char* dir_name)
		{
			const auto original_path = utils::string::va(fmt, root_dir, mods_dir, dir_name);

			// check if the zone folder exists
			if (utils::io::directory_exists(original_path))
			{
				return original_path;
			}

			return utils::string::va("%s/%s/%s", root_dir, mods_dir, dir_name);
		}

		const char* va_user_content_path_stub(const char* fmt, const char* root_dir, const char* user_content_dir)
		{
			const auto original_path = utils::string::va(fmt, root_dir, user_content_dir);

			if (utils::io::directory_exists(original_path))
			{
				return original_path;
			}

			return utils::string::va("%s/%s", root_dir, user_content_dir);
		}
	}

	std::string get_mod_resized_name()
	{
		const std::string loaded_mod_id = game::getPublisherIdFromLoadedMod();

		if (loaded_mod_id == "usermaps" || loaded_mod_id.empty())
		{
			return loaded_mod_id;
		}

		std::string mod_name = loaded_mod_id;

		for (unsigned int i = 0; i < *game::modsCount; ++i)
		{
			const auto& mod_data = game::modsPool[i];

			if (mod_data.publisherId == loaded_mod_id)
			{
				mod_name = mod_data.title;
				break;
			}
		}

		if (mod_name.size() > 31)
		{
			mod_name.resize(31);
		}

		return mod_name;
	}

	std::string get_usermap_publisher_id(const std::string& zone_name)
	{
		for (unsigned int i = 0; i < *game::usermapsCount; ++i)
		{
			const auto& usermap_data = game::usermapsPool[i];
			if (usermap_data.folderName == zone_name)
			{
				if (!utils::string::is_numeric(usermap_data.publisherId))
				{
					printf("[ Workshop ] WARNING: The publisherId is not numerical you might have set your usermap folder incorrectly!\n%s\n",
						usermap_data.absolutePathZoneFiles);
				}

				return usermap_data.publisherId;
			}
		}

		return {};
	}

	std::string get_mod_publisher_id()
	{
		const std::string loaded_mod_id = game::getPublisherIdFromLoadedMod();

		if (loaded_mod_id == "usermaps" || loaded_mod_id.empty())
		{
			return loaded_mod_id;
		}

		if (!utils::string::is_numeric(loaded_mod_id))
		{
			printf("[ Workshop ] WARNING: The publisherId: %s, is not numerical you might have set your mod folder incorrectly!\n",
				loaded_mod_id.data());
		}

		return loaded_mod_id;
	}

	bool check_valid_usermap_id(const std::string& mapname, const std::string& pub_id)
	{
		if (!game::DB_FileExists(mapname.data(), 0) && pub_id.empty())
		{
			game::UI_OpenErrorPopupWithMessage(0, 0x100,
				utils::string::va("Can't find usermap: %s!\nMake sure you're subscribed to the workshop item.", mapname.data()));
			return false;
		}

		return true;
	}

	bool check_valid_mod_id(const std::string& mod)
	{
		if (mod.empty() || mod == "usermaps")
		{
			return true;
		}

		if (!has_mod(mod))
		{
			game::UI_OpenErrorPopupWithMessage(0, 0x100,
				utils::string::va("Can't find mod with publisher id: %s!\nMake sure you're subscribed to the workshop item.", mod.data()));
			return false;
		}

		return true;
	}

	void setup_same_mod_as_host(const std::string& usermap, const std::string& mod)
	{
		if (game::getPublisherIdFromLoadedMod() == mod)
		{
			return;
		}

		if (!usermap.empty() || mod != "usermaps")
		{
			game::loadMod(0, mod.data(), true);
			return;
		}

		if (game::isModLoaded())
		{
			game::loadMod(0, "", true);
		}
	}

	class component final : public generic_component
	{
	public:
		void post_unpack() override
		{
			command::add("userContentReload", [](const command::params& params)
			{
				game::reloadUserContent();
			});

			utils::hook::call(game::select(0x1420D6AA6, 0x1404E2936), va_mods_path_stub);
			utils::hook::call(game::select(0x1420D6577, 0x1404E24A7), va_user_content_path_stub);

			load_usermap_hook.create(game::select(0x1420D5700, 0x1404E18B0), load_usermap_stub);
			utils::hook::call(game::select(0x1420D67F5, 0x1404E25F2), load_usermap_content_stub);

			if (game::is_server())
			{
				utils::hook::jump(0x1404E2635_g, load_mod_content_stub);
				return;
			}

			utils::hook::call(0x1420D6745_g, load_mod_content_stub);
			utils::hook::call(0x14135CD84_g, has_workshop_item_stub);
			setup_server_map_hook.create(0x14135CD20_g, setup_server_map_stub);
		}
	};
}

REGISTER_COMPONENT(workshop::component)
