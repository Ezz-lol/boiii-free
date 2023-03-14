#pragma once

#include <game/game.hpp>

namespace network
{
	using data_view = std::basic_string_view<uint8_t>;
	using callback = std::function<void(const game::netadr_t&, const data_view&)>;

	void on(const std::string& command, const callback& callback);
	void send(const game::netadr_t& address, const std::string& command, const std::string& data = {},
	          char separator = ' ');

	void send_data(const game::netadr_t& address, const void* data, size_t length);
	void send_data(const game::netadr_t& address, const std::string& data);

	game::netadr_t address_from_string(const std::string& address);
	game::netadr_t address_from_ip(uint32_t ip, uint16_t port);

	bool are_addresses_equal(const game::netadr_t& a, const game::netadr_t& b);
}

inline bool operator==(const game::netadr_t& a, const game::netadr_t& b)
{
	return network::are_addresses_equal(a, b); //
}

inline bool operator!=(const game::netadr_t& a, const game::netadr_t& b)
{
	return !(a == b); //
}

namespace std
{
	template <>
	struct equal_to<game::netadr_t>
	{
		using result_type = bool;

		bool operator()(const game::netadr_t& lhs, const game::netadr_t& rhs) const
		{
			return network::are_addresses_equal(lhs, rhs);
		}
	};

	template <>
	struct hash<game::netadr_t>
	{
		size_t operator()(const game::netadr_t& x) const noexcept
		{
			const auto type_hash = hash<uint32_t>()(x.type);

			if (x.type != game::NA_IP && x.type != game::NA_RAWIP)
			{
				return type_hash;
			}

			return type_hash ^ hash<uint32_t>()(x.addr) ^ hash<
				uint16_t>()(x.port);
		}
	};
}
