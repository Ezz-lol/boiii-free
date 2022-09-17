#pragma once

#include "udp_server.hpp"

namespace demonware
{
	class stun_server : public udp_server
	{
	public:
		using udp_server::udp_server;

	private:
		void handle(const endpoint_data& endpoint, const std::string& packet) override;

		void ip_discovery(const endpoint_data& endpoint);
		void nat_discovery(const endpoint_data& endpoint);
	};
}
