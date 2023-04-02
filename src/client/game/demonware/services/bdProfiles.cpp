#include <std_include.hpp>
#include "../services.hpp"

#include "../../../component/profile_infos.hpp"

namespace demonware
{
	bdProfiles::bdProfiles() : service(8, "bdProfiles")
	{
		this->register_task(1, &bdProfiles::getPublicInfos);
		this->register_task(2, &bdProfiles::getPrivateInfo);
		this->register_task(3, &bdProfiles::setPublicInfo);
		this->register_task(4, &bdProfiles::setPrivateInfo);
		this->register_task(5, &bdProfiles::deleteProfile);
		this->register_task(6, &bdProfiles::setPrivateInfoByUserID);
		this->register_task(7, &bdProfiles::getPrivateInfoByUserID);
		this->register_task(8, &bdProfiles::setPublicInfoByUserID);
	}

	void bdProfiles::getPublicInfos(service_server* server, byte_buffer* buffer) const
	{
		std::vector<std::pair<uint64_t, profile_infos::profile_info>> profile_infos{};

		uint64_t entity_id;
		while (buffer->read_uint64(&entity_id))
		{
			auto profile = profile_infos::get_profile_info(entity_id);
			if(profile)
			{
				profile_infos.emplace_back(entity_id, std::move(*profile));
			}
		}

		auto reply = server->create_reply(this->task_id(), profile_infos.empty() ? game::BD_NO_PROFILE_INFO_EXISTS : game::BD_NO_ERROR);

		for(auto& info : profile_infos)
		{
			auto result = std::make_unique<bdPublicProfileInfo>();
			result->m_entityID = info.first;
			result->m_VERSION = info.second.version;
			result->m_ddl = std::move(info.second.ddl);

			reply.add(result);
		}

		reply.send();
	}

	void bdProfiles::setPublicInfo(service_server* server, byte_buffer* buffer) const
	{
		profile_infos::profile_info info{};

		buffer->read_int32(&info.version);
		buffer->read_blob(&info.ddl);

		profile_infos::update_profile_info(info);

		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdProfiles::getPrivateInfo(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdProfiles::setPrivateInfo(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdProfiles::deleteProfile(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdProfiles::setPrivateInfoByUserID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdProfiles::getPrivateInfoByUserID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdProfiles::setPublicInfoByUserID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
