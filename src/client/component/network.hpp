#pragma once
#include "game/game.hpp"

namespace network
{
	using data_view = std::basic_string_view<uint8_t>;
	using callback = std::function<void(const game::netadr_t&, const data_view&)>;

	void on(const std::string& command, const callback& callback);
	void send(const game::netadr_t& address, const std::string& command, const std::string& data = {},
	          char separator = ' ');

	void send_data(const game::netadr_t& address, const void* data, size_t length);
	void send_data(const game::netadr_t& address, const std::string& data);
}
