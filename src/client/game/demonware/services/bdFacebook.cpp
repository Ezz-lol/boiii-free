#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdFacebook::bdFacebook() : service(36, "bdFacebook")
	{
		this->register_task(1, &bdFacebook::registerAccount);
		this->register_task(2, &bdFacebook::post);
		this->register_task(3, &bdFacebook::unregisterAccount);
		this->register_task(5, &bdFacebook::isRegistered);
		this->register_task(6, &bdFacebook::getInfo);
		this->register_task(7, &bdFacebook::getRegisteredAccounts);
		this->register_task(8, &bdFacebook::getFriends);
		this->register_task(9, &bdFacebook::getProfilePictures);
		this->register_task(10, &bdFacebook::uploadPhoto);
		this->register_task(11, &bdFacebook::registerToken);
		this->register_task(12, &bdFacebook::uploadVideo);
		this->register_task(13, &bdFacebook::getFriendsByID);
		this->register_task(14, &bdFacebook::setLikeStatus);
	}

	void bdFacebook::registerAccount(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdFacebook::post(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdFacebook::unregisterAccount(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdFacebook::isRegistered(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdFacebook::getInfo(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdFacebook::getRegisteredAccounts(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdFacebook::getFriends(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdFacebook::getProfilePictures(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdFacebook::uploadPhoto(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdFacebook::registerToken(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdFacebook::uploadVideo(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdFacebook::getFriendsByID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdFacebook::setLikeStatus(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
