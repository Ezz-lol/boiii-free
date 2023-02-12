#pragma once
#include <game/game.hpp>

namespace server_list
{
	bool get_master_server(game::netadr_t& address);

	using callback = std::function<void(bool, const std::unordered_set<game::netadr_t>&)>;
	void request_servers(callback callback);
}
