#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdPublisherVariables::bdPublisherVariables() : service(95, "bdPublisherVariables")
	{
		this->register_task(1, &bdPublisherVariables::retrievePublisherVariables);
	}

	void bdPublisherVariables::retrievePublisherVariables(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
