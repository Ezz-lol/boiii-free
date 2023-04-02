#include <std_include.hpp>
#include "../services.hpp"

#include <utils/io.hpp>

namespace demonware
{
	bdMarketplace::bdMarketplace() : service(80, "bdMarketplace")
	{
		this->register_task(42, &bdMarketplace::startExchangeTransaction); // COD POINTS purchase ?
		//this->register_task(43, &bdMarketplace::purchaseOnSteamInitialize); // COD POINTS purchase ?
		this->register_task(49, &bdMarketplace::getExpiredInventoryItems);
		this->register_task(60, &bdMarketplace::steamProcessDurable);
		this->register_task(122, &bdMarketplace::purchaseSkus);
		this->register_task(130, &bdMarketplace::getBalance);
		this->register_task(165, &bdMarketplace::getInventoryPaginated);
		this->register_task(193, &bdMarketplace::putPlayersInventoryItems);
		this->register_task(232, &bdMarketplace::getEntitlements);
	}

	void bdMarketplace::startExchangeTransaction(service_server* server, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdMarketplace::purchaseOnSteamInitialize(service_server* server, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdMarketplace::getExpiredInventoryItems(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdMarketplace::steamProcessDurable(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdMarketplace::purchaseSkus(service_server* server, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdMarketplace::getBalance(service_server* server, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());

		reply.send();
	}

	void bdMarketplace::getInventoryPaginated(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdMarketplace::putPlayersInventoryItems(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdMarketplace::getEntitlements(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
