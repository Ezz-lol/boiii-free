#pragma once
#include <utils/info_string.hpp>

namespace party
{
	using query_callback_func = void(bool success, const game::netadr_t& host, const ::utils::info_string& info, uint32_t ping);
	using query_callback = std::function<query_callback_func>;

	void query_server(const game::netadr_t& host, query_callback callback);

	game::netadr_t get_connected_server();

	bool is_host(const game::netadr_t& addr);
}
