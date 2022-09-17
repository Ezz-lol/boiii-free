#pragma once

namespace demonware
{
	class bdMarketplace final : public service
	{
	public:
		bdMarketplace();

	private:
		void startExchangeTransaction(service_server* server, byte_buffer* buffer) const;
		void purchaseOnSteamInitialize(service_server* server, byte_buffer* buffer) const;
		void getExpiredInventoryItems(service_server* server, byte_buffer* buffer) const;
		void steamProcessDurable(service_server* server, byte_buffer* buffer) const;
		void purchaseSkus(service_server* server, byte_buffer* buffer) const;
		void getBalance(service_server* server, byte_buffer* buffer) const;
		void getInventoryPaginated(service_server* server, byte_buffer* buffer) const;
		void putPlayersInventoryItems(service_server* server, byte_buffer* buffer) const;
		void getEntitlements(service_server* server, byte_buffer* buffer) const;
	};
}
