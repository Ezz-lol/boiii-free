#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdStats3::bdStats3() : service(91, "bdStats")
	{
		this->register_task(1, &bdStats3::deleteCSFileStats);
		this->register_task(3, &bdStats3::readStatsByEntityID);
		this->register_task(4, &bdStats3::readStatsByRank);
		this->register_task(5, &bdStats3::readStatsByPivot);
		this->register_task(6, &bdStats3::readStatsByRating);
		this->register_task(7, &bdStats3::readStatsByMultipleRanks);
		this->register_task(11, &bdStats3::readStatsByLeaderboardIDsAndEntityIDs);
	}

	void bdStats3::deleteCSFileStats(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats3::readStatsByEntityID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats3::readStatsByRank(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats3::readStatsByPivot(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats3::readStatsByRating(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats3::readStatsByMultipleRanks(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats3::readStatsByLeaderboardIDsAndEntityIDs(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}