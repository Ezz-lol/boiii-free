#pragma once

#include <utils/nt.hpp>

namespace steam_proxy
{
	const utils::nt::library& get_overlay_module();
	const char* get_player_name();

	struct subscribed_item
	{
		bool available;
		std::string path;
		uint32_t state;
		uint32_t time_stamp;
		uint64_t size_on_disk;
	};

	using subscribed_item_map = std::unordered_map<uint64_t, subscribed_item>;

	void initialize();
	void create_ugc();

	void update_subscribed_items();
	void access_subscribed_items(const std::function<void(const subscribed_item_map&)>& callback);
}
