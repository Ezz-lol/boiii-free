#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdEventLog::bdEventLog() : service(67, "bdEventLog")
	{
		this->register_task(1, &bdEventLog::recordEvent);
		this->register_task(2, &bdEventLog::recordEventBin);
		this->register_task(3, &bdEventLog::recordEvents);
		this->register_task(4, &bdEventLog::recordEventsBin);
		this->register_task(6, &bdEventLog::initializeFiltering);
	}

	void bdEventLog::recordEvent(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdEventLog::recordEventBin(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdEventLog::recordEvents(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdEventLog::recordEventsBin(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdEventLog::initializeFiltering(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
