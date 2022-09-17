#pragma once
#include "tcp_server.hpp"
#include "../reply.hpp"

namespace demonware
{
	class auth3_server : public tcp_server
	{
	public:
		using tcp_server::tcp_server;

	private:
		void send_reply(reply* data);
		void handle(const std::string& packet) override;
	};
}
