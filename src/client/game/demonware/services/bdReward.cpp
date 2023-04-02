#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdReward::bdReward() : service(139, "bdReward")
	{
		this->register_task(1, &bdReward::incrementTime);
		this->register_task(2, &bdReward::claimRewardRoll);
		this->register_task(3, &bdReward::claimClientAchievements);
		this->register_task(4, &bdReward::reportRewardEvents);
	}

	void bdReward::incrementTime(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdReward::claimRewardRoll(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdReward::claimClientAchievements(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdReward::reportRewardEvents(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
