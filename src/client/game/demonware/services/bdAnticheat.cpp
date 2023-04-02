#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdAnticheat::bdAnticheat() : service(38, "bdAnticheat")
	{
		this->register_task(2, &bdAnticheat::answerChallenges);
		this->register_task(3, &bdAnticheat::reportConsoleID);
		this->register_task(4, &bdAnticheat::reportConsoleDetails);
		this->register_task(5, &bdAnticheat::answerTOTPChallenge);
		this->register_task(6, &bdAnticheat::idk);
	}

	void bdAnticheat::answerChallenges(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO: Read data as soon as needed
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdAnticheat::reportConsoleID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO: Read data as soon as needed
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
	
	void bdAnticheat::reportConsoleDetails(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO: Read data as soon as needed
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdAnticheat::answerTOTPChallenge(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO: Read data as soon as needed
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdAnticheat::idk(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO: Read data as soon as needed
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
