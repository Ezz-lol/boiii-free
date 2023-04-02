#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdPresence::bdPresence() : service(103, "bdPresence")
	{
		this->register_task(1, &bdPresence::unk1);
		this->register_task(3, &bdPresence::unk3);
	}

	void bdPresence::unk1(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdPresence::unk3(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
