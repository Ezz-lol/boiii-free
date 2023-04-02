#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdRichPresence::bdRichPresence() : service(68, "bdRichPresence")
	{
		this->register_task(1, &bdRichPresence::setInfo);
		this->register_task(2, &bdRichPresence::getInfo);
	}

	void bdRichPresence::setInfo(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdRichPresence::getInfo(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
