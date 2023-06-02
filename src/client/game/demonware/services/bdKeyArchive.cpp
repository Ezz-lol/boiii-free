#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdKeyArchive::bdKeyArchive() : service(15, "bdKeyArchive")
	{
		this->register_task(1, &bdKeyArchive::write);
		this->register_task(2, &bdKeyArchive::read);
		this->register_task(3, &bdKeyArchive::readAll);
		this->register_task(5, &bdKeyArchive::readMultipleEntityIDs);
		this->register_task(6, &bdKeyArchive::writeMultipleEntityIDs);
	}

	void bdKeyArchive::write(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdKeyArchive::read(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdKeyArchive::readAll(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdKeyArchive::readMultipleEntityIDs(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
	void bdKeyArchive::writeMultipleEntityIDs(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
