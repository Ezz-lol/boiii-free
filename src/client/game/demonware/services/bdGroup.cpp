#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdGroup::bdGroup() : service(28, "bdGroup")
	{
		this->register_task(1, &bdGroup::setGroups);
		this->register_task(2, &bdGroup::setGroupsForEntity);
		this->register_task(3, &bdGroup::getEntityGroups);
		this->register_task(4, &bdGroup::getGroupCounts);
	}

	void bdGroup::setGroups(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdGroup::setGroupsForEntity(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdGroup::getEntityGroups(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdGroup::getGroupCounts(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
