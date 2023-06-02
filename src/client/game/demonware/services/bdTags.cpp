#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdTags::bdTags() : service(52, "bdTags")
	{
		this->register_task(1, &bdTags::getTagsForEntityIDs);
		this->register_task(2, &bdTags::setTagsForEntityID);
		this->register_task(3, &bdTags::removeTagsForEntityID);
		this->register_task(4, &bdTags::removeAllTagsForEntityID);
		this->register_task(5, &bdTags::searchByTagsBase);
	}
	bdService55::bdService55() : service(55, "bdService55")
	{
		this->register_task(3, &bdService55::k9);
	}

	void bdService55::k9(service_server* server, byte_buffer* buffer) const
	{
		// TODO:
		uint16_t file;
		uint32_t fileID;
		uint32_t fileID2;

		buffer->read_uint16(&file);
		buffer->read_uint32(&fileID);
		buffer->read_uint32(&fileID2);


		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTags::getTagsForEntityIDs(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTags::setTagsForEntityID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTags::removeTagsForEntityID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTags::removeAllTagsForEntityID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTags::searchByTagsBase(service_server* server, byte_buffer* /*buffer*/) const
	{
		__int64 count = std::count_if(std::filesystem::directory_iterator("data\\demos"), std::filesystem::directory_iterator(),
			[](const auto& file) {
				return file.path().extension() == ".summary";
			});
		auto reply3 = server->create_reply(this->task_id());
		for (__int64 i = count; i > 0; i--)
		{
			auto result3 = std::make_unique<bdFileID>();
			result3->m_fileID = i;
			reply3.add(result3);
		}
		reply3.send();
	}
}
