#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdObjectStore::bdObjectStore() : service(15, "bdObjectStore")
	{
		this->register_task(2, &bdObjectStore::getUserObjects);
	}

	void bdObjectStore::getUserObjects(service_server* server, byte_buffer* /*buffer*/) const
	{
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
