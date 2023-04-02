#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdLeague::bdLeague() : service(81, "bdLeague")
	{
		this->register_task(1, &bdLeague::getTeamID);
		this->register_task(2, &bdLeague::getTeamIDsForUser);
		this->register_task(3, &bdLeague::getTeamSubdivisions);
		this->register_task(4, &bdLeague::setTeamName);
		this->register_task(5, &bdLeague::setTeamIcon);
		this->register_task(6, &bdLeague::getTeamInfos);
		this->register_task(7, &bdLeague::getTeamLeaguesAndSubdivisions);
		this->register_task(8, &bdLeague::getTeamMemberInfos);
		this->register_task(10, &bdLeague::incrementGamesPlayedCount);
		this->register_task(20, &bdLeague::getSubdivisionInfos);
		this->register_task(21, &bdLeague::getTeamSubdivisionHistory);
	}

	void bdLeague::getTeamID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdLeague::getTeamIDsForUser(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdLeague::getTeamSubdivisions(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdLeague::setTeamName(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdLeague::setTeamIcon(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdLeague::getTeamInfos(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdLeague::getTeamLeaguesAndSubdivisions(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdLeague::getTeamMemberInfos(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdLeague::incrementGamesPlayedCount(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdLeague::getSubdivisionInfos(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdLeague::getTeamSubdivisionHistory(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}