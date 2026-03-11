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

	// friends
	void access_steam_friends(const std::function<void(const std::vector<std::pair<uint64_t, std::string>>&)>& callback);
	void invite_friend(uint64_t xuid, const std::string& connect_string);
	std::string get_pending_game_invite();
	std::string get_pending_game_invite(uint64_t* out_friend_id);
	std::string get_steam_friend_name(uint64_t steam_id);
	bool set_rich_presence(const std::string& key, const std::string& value);
	void clear_rich_presence();
	uint64_t get_own_steam_id();
	std::string get_friend_rich_presence(uint64_t friend_id, const std::string& key);
	void request_friend_rich_presence(uint64_t friend_id);
	void clear_invite_presence();
}
