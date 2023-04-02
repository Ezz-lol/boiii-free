#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdMarketingComms::bdMarketingComms() : service(104, "bdMarketingComms")
	{
		this->register_task(1, &bdMarketingComms::getMessages);
		this->register_task(4, &bdMarketingComms::reportFullMessagesViewed);
	}

	void bdMarketingComms::getMessages(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdMarketingComms::reportFullMessagesViewed(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
