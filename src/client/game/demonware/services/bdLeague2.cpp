#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdLeague2::bdLeague2() : service(82, "bdLeague")
	{
		this->register_task(1, &bdLeague2::writeStats);
		this->register_task(2, &bdLeague2::readStatsByTeamID);
		this->register_task(3, &bdLeague2::readStatsByRank);
		this->register_task(4, &bdLeague2::readStatsByPivot);
	}

	void bdLeague2::writeStats(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdLeague2::readStatsByTeamID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdLeague2::readStatsByRank(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdLeague2::readStatsByPivot(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}