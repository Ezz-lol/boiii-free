#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "workshop.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>

namespace workshop
{
	const std::string get_usermap_publisher_id(const std::string& mapname)
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

		return "";
	}

	bool check_valid_publisher_id(const std::string& mapname, const std::string& pub_id)
	{
		if (!game::DB_FileExists(mapname.data(), 0) && pub_id.empty())
		{
			game::Com_Error(0, "Can't find usermap: %s!\nMake sure you're subscribed to the workshop item.", mapname.data());
			return false;
		}

		return true;
	}

	void load_usermap_mod_if_needed(const std::string& pub_id)
	{
		if (!game::isModLoaded() && !pub_id.empty())
		{
			game::loadMod(0, "usermaps", 0);
		}
	}

	namespace
	{
		utils::hook::detour setup_server_map_hook;

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
