#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdDML::bdDML() : service(27, "bdDML")
	{
		this->register_task(1, &bdDML::recordIP);
		this->register_task(2, &bdDML::getUserData);
		this->register_task(3, &bdDML::getUserHierarchicalData);
		this->register_task(4, &bdDML::getUsersLastLogonData);
	}

	void bdDML::recordIP(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdDML::getUserData(service_server* server, byte_buffer* /*buffer*/) const
	{
		auto result = std::make_unique<bdDMLRawData>();
		result->country_code = "US";
		result->country_code = "'Murica";
		result->region = "New York";
		result->city = "New York";
		result->latitude = 0;
		result->longitude = 0;

		result->asn = 0x2119;
		result->timezone = "+01:00";

		auto reply = server->create_reply(this->task_id());
		reply.add(result);
		reply.send();
	}

	void bdDML::getUserHierarchicalData(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdDML::getUsersLastLogonData(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
