#pragma once

#include "servers/base_server.hpp"
#include <utils/cryptography.hpp>

namespace demonware
{
	template <typename T>
	class server_registry
	{
		static_assert(std::is_base_of<base_server, T>::value, "Invalid server registry type");

	public:
		template <typename S, typename ...Args>
		void create(Args&&... args)
		{
			static_assert(std::is_base_of<T, S>::value, "Invalid server type");

			auto server = std::make_unique<S>(std::forward<Args>(args)...);
			const auto address = server->get_address();
			servers_[address] = std::move(server);
		}

		void for_each(const std::function<void(T&)>& callback) const
		{
			for (auto& server : servers_)
			{
				callback(*server.second);
			}
		}

		T* find(const std::string& name)
		{
			const auto address = utils::cryptography::jenkins_one_at_a_time::compute(name);
			return find(address);
		}

		T* find(const uint32_t address)
		{
			const auto it = servers_.find(address);
			if (it == servers_.end())
			{
				return nullptr;
			}

			return it->second.get();
		}

		void frame()
		{
			for (auto& server : servers_)
			{
				server.second->frame();
			}
		}

	private:
		std::unordered_map<uint32_t, std::unique_ptr<T>> servers_;
	};
}
