#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdDDL::bdDDL() : service(96, "bdDDL")
	{
		this->register_task(1, &bdDDL::idk);
	}

	void bdDDL::idk(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO: Read data as soon as needed
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}
}
