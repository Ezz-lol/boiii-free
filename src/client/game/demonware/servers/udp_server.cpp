#include <std_include.hpp>
#include "udp_server.hpp"

namespace demonware
{
	void udp_server::handle_input(const char* buf, size_t size, endpoint_data endpoint)
	{
		in_queue_.access([&](in_queue& queue)
		{
			in_packet p;
			p.data = std::string{buf, size};
			p.endpoint = std::move(endpoint);

			queue.emplace(std::move(p));
		});
	}

	size_t udp_server::handle_output(SOCKET socket, char* buf, size_t size, sockaddr* address, int* addrlen)
	{
		return out_queue_.access<size_t>([&](socket_queue_map& map) -> size_t
		{
			const auto entry = map.find(socket);
			if (entry == map.end())
			{
				return 0;
			}

			auto& queue = entry->second;

			if (queue.empty())
			{
				return 0;
			}

			auto data = std::move(queue.front());
			queue.pop();

			const auto copy_size = std::min(size, data.data.size());
			std::memcpy(buf, data.data.data(), copy_size);
			std::memcpy(address, &data.address, sizeof(data.address));
			*addrlen = sizeof(data.address);

			return copy_size;
		});
	}

	bool udp_server::pending_data(SOCKET socket)
	{
		return this->out_queue_.access<bool>([&](const socket_queue_map& map)
		{
			const auto entry = map.find(socket);
			if (entry == map.end())
			{
				return false;
			}

			return !entry->second.empty();
		});
	}

	void udp_server::send(const endpoint_data& endpoint, std::string data)
	{
		out_queue_.access([&](socket_queue_map& map)
		{
			out_packet p;
			p.data = std::move(data);
			p.address = endpoint.address;

			map[endpoint.socket].emplace(std::move(p));
		});
	}

	void udp_server::frame()
	{
		if (this->in_queue_.get_raw().empty())
		{
			return;
		}

		while (true)
		{
			in_packet packet{};
			const auto result = this->in_queue_.access<bool>([&](in_queue& queue)
			{
				if (queue.empty())
				{
					return false;
				}

				packet = std::move(queue.front());
				queue.pop();
				return true;
			});

			if (!result)
			{
				break;
			}

			this->handle(packet.endpoint, std::move(packet.data));
		}
	}
}
