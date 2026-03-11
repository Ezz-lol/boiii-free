#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdCounter::bdCounter() : service(23, "bdCounter")
	{
		this->register_task(1, &bdCounter::incrementCounters);
		this->register_task(2, &bdCounter::getCounterTotals);
		this->register_task(3, &bdCounter::getCountersByGroup);
	}

	void bdCounter::incrementCounters(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdCounter::getCounterTotals(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdCounter::getCountersByGroup(service_server* server, byte_buffer* /*buffer*/) const
	{
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
