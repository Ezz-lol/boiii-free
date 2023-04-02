#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdUserGroups::bdUserGroups() : service(65, "bdUserGroups")
	{
		this->register_task(1, &bdUserGroups::createGroup);
		this->register_task(2, &bdUserGroups::deleteGroup);
		this->register_task(3, &bdUserGroups::joinGroup);
		this->register_task(4, &bdUserGroups::leaveGroup);
		this->register_task(5, &bdUserGroups::getMembershipInfo);
		this->register_task(6, &bdUserGroups::changeMemberType);
		this->register_task(7, &bdUserGroups::getNumMembers);
		this->register_task(8, &bdUserGroups::getMembers);
		this->register_task(9, &bdUserGroups::getMemberships);
		this->register_task(10, &bdUserGroups::readStatsByRank);
		this->register_task(11, &bdUserGroups::getGroupLists);
	}

	void bdUserGroups::createGroup(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdUserGroups::deleteGroup(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdUserGroups::joinGroup(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdUserGroups::leaveGroup(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdUserGroups::getMembershipInfo(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdUserGroups::changeMemberType(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdUserGroups::getNumMembers(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdUserGroups::getMembers(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdUserGroups::getMemberships(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdUserGroups::readStatsByRank(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdUserGroups::getGroupLists(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}