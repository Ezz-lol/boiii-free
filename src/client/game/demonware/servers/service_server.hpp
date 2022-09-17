#pragma once

#include "../reply.hpp"

namespace demonware
{
	class service_server
	{
	public:
		virtual ~service_server() = default;

		virtual std::shared_ptr<remote_reply> create_message(uint8_t type)
		{
			auto reply = std::make_shared<remote_reply>(this, type);
			return reply;
		}


		virtual std::shared_ptr<service_reply> create_reply(uint8_t type, uint32_t error = 0)
		{
			auto reply = std::make_shared<service_reply>(this, type, error);
			return reply;
		}

		virtual void send_reply(reply* data) = 0;
	};
}
