#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdDDL::bdDDL() : service(96, "bdDDL")
	{
		this->register_task(1, &bdDDL::verifyDDLFiles);
	}

	void bdDDL::verifyDDLFiles(service_server* server, byte_buffer* buffer) const
	{
		uint32_t count;
		buffer->read_uint32(&count);

		auto reply = server->create_reply(this->task_id());

		for (uint32_t i = 0; i < count; i++)
		{
			auto checksum = std::make_unique<bdDDLChecksumResult>();
			checksum->deserialize(buffer);
			checksum->checksum_matched = true;

			reply.add(checksum);
		}

		reply.send();
	}
}
