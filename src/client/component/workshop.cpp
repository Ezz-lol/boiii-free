#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "workshop.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>

namespace workshop
{
	namespace
	{
		utils::hook::detour setup_server_map_hook;

		bool has_mod(const std::string& pub_id)
		{
			const auto total_mods = *reinterpret_cast<unsigned int*>(0x15678D170_g);

			for (unsigned int i = 0; i < total_mods; ++i)
			{
				const auto mod_data = reinterpret_cast<game::workshop_data*>(0x15678D178_g + (sizeof(game::workshop_data) * i));
				if (mod_data->publisherId == pub_id)
				{
					return true;
				}
			}

			return false;
		}

		void load_usermap_mod_if_needed(const std::string& publisher_id)
		{
			if (!game::isModLoaded() && !publisher_id.empty())
			{
				game::loadMod(0, "usermaps", true);
			}
		}

		void setup_server_map_stub(int localClientNum, const char* mapname, const char* gametype)
		{
			const auto publisher_id = get_usermap_publisher_id(mapname);
			load_usermap_mod_if_needed(publisher_id);

			setup_server_map_hook.invoke(localClientNum, mapname, gametype);
		}

		bool has_workshop_item_stub(int type, const char* mapname, int a3)
		{
			const auto publisher_id = get_usermap_publisher_id(mapname);
			const auto name = publisher_id.empty() ? mapname : publisher_id.data();

			return utils::hook::invoke<bool>(0x1420D6380_g, type, name, a3);
		}

		game::workshop_data* load_usermap_stub(const char* mapname)
		{
			const auto publisher_id = get_usermap_publisher_id(mapname);
			const auto name = publisher_id.empty() ? mapname : publisher_id.data();

			return utils::hook::invoke<game::workshop_data*>(0x1420D5700_g, name);
		}
	}

	std::string get_mod_name(const std::string& mod_id)
	{
		if (mod_id == "usermaps" || !game::is_server())
		{
			return mod_id;
		}

		const utils::nt::library host{};
		const auto base_path = host.get_folder().generic_string();
		const auto path = utils::string::va("%s/mods/%s/zone/workshop.json", base_path.data(), mod_id.data());
		const auto json_str = utils::io::read_file(path);

		if (json_str.empty())
		{
			printf("[ Workshop ] workshop.json has not been found in mod folder: %s\n", mod_id.data());
			return mod_id;
		}

		rapidjson::Document doc;
		const rapidjson::ParseResult parse_result = doc.Parse(json_str);

		if (parse_result.IsError() || !doc.IsObject())
		{
			printf("[ Workshop ] Unable to parse workshop.json\n");
			return mod_id;
		}

		if (doc.HasMember("Title"))
		{
			std::string title = doc["Title"].GetString();

			if (title.size() > 31)
			{
				title.resize(31);
			}

			return title;
		}

		printf("[ Workshop ] workshop.json has no \"Title\" member.\n");
		return mod_id;
	}

	std::string get_usermap_publisher_id(const std::string& mapname)
	{
		const auto total_usermaps = *reinterpret_cast<unsigned int*>(0x1567B3580_g);

		for (unsigned int i = 0; i < total_usermaps; ++i)
		{
			const auto usermap_data = reinterpret_cast<game::workshop_data*>(0x1567B3588_g + (sizeof(game::workshop_data) * i));
			if (usermap_data->folderName == mapname)
			{
				return usermap_data->publisherId;
			}
		}

		return {};
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

	void load_mod_if_needed(const std::string& usermap, const std::string& mod)
	{
		if (!usermap.empty() || mod != "usermaps")
		{
			game::loadMod(0, mod.data(), true);
		}
	}

	class component final : public client_component
	{
	public:
		void post_unpack() override
		{
			setup_server_map_hook.create(0x14135CD20_g, setup_server_map_stub);

			// Allow client to switch maps if server sends zone name instead of publisher id
			utils::hook::call(0x14135CD84_g, has_workshop_item_stub);
			utils::hook::call(0x14135CE48_g, load_usermap_stub);
		}
	};
}

REGISTER_COMPONENT(workshop::component)
