#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdStats2::bdStats2() : service(19, "bdStats")
	{
		this->register_task(1, &bdStats2::startArbitratedSession);
		this->register_task(2, &bdStats2::writeArbitratedStats);
	}

	void bdStats2::startArbitratedSession(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats2::writeArbitratedStats(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}