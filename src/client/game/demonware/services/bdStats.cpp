#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdStats::bdStats() : service(4, "bdStats")
	{
		this->register_task(1, &bdStats::writeStats);
		this->register_task(2, &bdStats::deleteStats);
		this->register_task(3, &bdStats::unk3); // leaderboards
		this->register_task(4, &bdStats::readStatsByRank);
		this->register_task(5, &bdStats::readStatsByPivot);
		this->register_task(6, &bdStats::readStatsByRating);
		this->register_task(7, &bdStats::readStatsByMultipleRanks);
		this->register_task(8, &bdStats::readExternalTitleStats);
		this->register_task(10, &bdStats::readExternalTitleNamedStats);
		this->register_task(11, &bdStats::readStatsByLeaderboardIDsAndEntityIDs);
		this->register_task(12, &bdStats::readStatsByMultipleRatings);
		this->register_task(13, &bdStats::readStatsByEntityID);
		this->register_task(14, &bdStats::writeServerValidatedStats);
	}

	void bdStats::writeStats(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats::deleteStats(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats::unk3(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats::readStatsByRank(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats::readStatsByPivot(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats::readStatsByRating(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats::readStatsByMultipleRanks(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats::readExternalTitleStats(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats::readExternalTitleNamedStats(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats::readStatsByLeaderboardIDsAndEntityIDs(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats::readStatsByMultipleRatings(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats::readStatsByEntityID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdStats::writeServerValidatedStats(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
