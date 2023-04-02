#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdTitleUtilities::bdTitleUtilities() : service(12, "bdTitleUtilities")
	{
		this->register_task(6, &bdTitleUtilities::get_server_time);
	}

	void bdTitleUtilities::get_server_time(service_server* server, byte_buffer* /*buffer*/) const
	{
		auto time_result = std::make_unique<bdTimeStamp>();
		time_result->unix_time = uint32_t(time(nullptr));

		auto reply = server->create_reply(this->task_id());
		reply.add(time_result);
		reply.send();
	}
}
