#pragma once
#include <game/game.hpp>

#include <utils/concurrency.hpp>

namespace server_list
{
	std::vector<game::netadr_t> get_master_servers();

	using callback = std::function<void(bool, const std::unordered_set<game::netadr_t>&)>;
	void request_servers(callback callback);

	void add_favorite_server(game::netadr_t addr);
	void remove_favorite_server(game::netadr_t addr);
	using server_list = std::unordered_set<game::netadr_t>;
	utils::concurrency::container<server_list>& get_favorite_servers();

	void add_recent_server(game::netadr_t addr);
	void remove_recent_server(game::netadr_t addr);
	using recent_list = std::vector<game::netadr_t>;
	utils::concurrency::container<recent_list>& get_recent_servers();
}
