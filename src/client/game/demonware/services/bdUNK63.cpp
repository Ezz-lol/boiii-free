#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdUNK63::bdUNK63() : service(63, "bdUNK63")
	{
		//this->register_task(6, "unk6", &bdUNK63::unk6);
	}

	void bdUNK63::unk(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
