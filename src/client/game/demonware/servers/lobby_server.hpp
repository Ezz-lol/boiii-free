#pragma once

#include "tcp_server.hpp"
#include "service_server.hpp"
#include "../service.hpp"

namespace demonware
{
	class lobby_server : public tcp_server, service_server
	{
	public:
		lobby_server(std::string name);

		template <typename T>
		void register_service()
		{
			static_assert(std::is_base_of<service, T>::value, "service must inherit from service");

			auto service = std::make_unique<T>();
			const uint8_t id = service->id();

			this->services_[id] = std::move(service);
		}

		void send_reply(reply* data) override;

	private:
		std::unordered_map<uint8_t, std::unique_ptr<service>> services_;

		void handle(const std::string& packet) override;
		void call_service(uint8_t id, const std::string& data);
	};
}
