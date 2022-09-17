#include <std_include.hpp>
#include "../services.hpp"

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

	void bdProfiles::getPublicInfos(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdProfiles::setPublicInfo(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
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
