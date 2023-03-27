#include <std_include.hpp>
#include "../services.hpp"
#include "steam/steam.hpp"
#include <utils/io.hpp>

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
		uint64_t entity_id;
		buffer->read_uint64(&entity_id);

		auto* result = new bdPublicProfileInfo;
		result->m_entityID = entity_id;
		result->m_VERSION = 3;

		if (utils::io::read_file(std::format("players/user/profileInfo_{}", entity_id), &result->m_ddl))
		{
			auto reply = server->create_reply(this->task_id());
			reply->add(result);
			reply->send();
		}
		else
		{
			auto reply = server->create_reply(this->task_id(), game::BD_NO_PROFILE_INFO_EXISTS);
			reply->send();
		}
	}

	void bdProfiles::setPublicInfo(service_server* server, byte_buffer* buffer) const
	{
		int32_t version; std::string ddl;
		buffer->read_int32(&version);
		buffer->read_blob(&ddl);

		utils::io::write_file(std::format("players/user/profileInfo_{}", steam::SteamUser()->GetSteamID().bits), ddl);

		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdProfiles::getPrivateInfo(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdProfiles::setPrivateInfo(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdProfiles::deleteProfile(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdProfiles::setPrivateInfoByUserID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdProfiles::getPrivateInfoByUserID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdProfiles::setPublicInfoByUserID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}
}
