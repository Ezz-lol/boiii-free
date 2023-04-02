#include <std_include.hpp>
#include "../services.hpp"
#include "game/game.hpp"

namespace demonware
{
	bdContentStreaming::bdContentStreaming() : service(50, "bdContentStreaming")
	{
		this->register_task(2, &bdContentStreaming::unk2);
		this->register_task(3, &bdContentStreaming::list_all_publisher_files);
	}

	void bdContentStreaming::unk2(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdContentStreaming::list_all_publisher_files(service_server* server, byte_buffer* buffer) const
	{
		std::uint32_t start_date;
		buffer->read_uint32(&start_date);

		std::uint16_t max_num_results;
		buffer->read_uint16(&max_num_results);

		std::uint16_t offset;
		buffer->read_uint16(&offset);

		std::uint16_t category;
		buffer->read_uint16(&category);

		std::string filename;
		buffer->read_string(&filename);

		/*if(filename.empty())
		{
			server->create_reply(this->task_id(), game::BD_NO_FILE).send();
			return;
		}*/

		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
