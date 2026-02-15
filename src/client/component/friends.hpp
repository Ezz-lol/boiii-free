#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace friends
{
	enum class status : int
	{
		offline = 0,
		online = 1,
		in_game = 2,
	};

	struct friend_entry
	{
		uint64_t steam_id{};
		std::string name;
		status state{status::offline};
		std::string server_address;
	};

	struct friend_server_info
	{
		uint64_t steam_id{};
		std::string address;
		std::string player_name;
	};

	void add_friend(uint64_t steam_id, const std::string& name);
	void remove_friend(uint64_t steam_id);
	bool is_friend(uint64_t steam_id);

	int get_friend_count();
	friend_entry get_friend_by_index(int index);
	std::vector<friend_entry> get_friends();

	bool invite_to_game(uint64_t steam_id);
	bool has_pending_invite();
	std::string consume_pending_invite();

	std::string get_presence_server(uint64_t steam_id);
	std::vector<friend_server_info> get_friend_server_addresses();
}
