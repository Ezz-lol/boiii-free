#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdMatchMaking::bdMatchMaking() : service(138, "bdMatchMaking")
	{
		this->register_task(1, &bdMatchMaking::createSession);
		this->register_task(2, &bdMatchMaking::updateSession);
		this->register_task(3, &bdMatchMaking::deleteSession);
		this->register_task(4, &bdMatchMaking::findSessionFromID);
		this->register_task(5, &bdMatchMaking::findSessions);
		this->register_task(6, &bdMatchMaking::notifyJoin);
		this->register_task(8, &bdMatchMaking::inviteToSession);
		this->register_task(9, &bdMatchMaking::submitPerformance);
		this->register_task(10, &bdMatchMaking::getPerformanceValues);
		this->register_task(11, &bdMatchMaking::getSessionInvites);
		this->register_task(12, &bdMatchMaking::updateSessionPlayers);
		this->register_task(13, &bdMatchMaking::findSessionsPaged);
		this->register_task(14, &bdMatchMaking::findSessionsByEntityIDs);
		this->register_task(15, &bdMatchMaking::findSessionsFromIDs);
		this->register_task(16, &bdMatchMaking::findSessionsTwoPass);
	}

	void bdMatchMaking::createSession(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdMatchMaking::updateSession(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdMatchMaking::deleteSession(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdMatchMaking::findSessionFromID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdMatchMaking::findSessions(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdMatchMaking::getPerformanceValues(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdMatchMaking::getSessionInvites(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdMatchMaking::updateSessionPlayers(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdMatchMaking::submitPerformance(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdMatchMaking::notifyJoin(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdMatchMaking::inviteToSession(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdMatchMaking::findSessionsPaged(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdMatchMaking::findSessionsByEntityIDs(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdMatchMaking::findSessionsFromIDs(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdMatchMaking::findSessionsTwoPass(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}
}
