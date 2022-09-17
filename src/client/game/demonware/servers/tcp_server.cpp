#include <std_include.hpp>
#include "tcp_server.hpp"

namespace demonware
{
	void tcp_server::handle_input(const char* buf, size_t size)
	{
		in_queue_.access([&](data_queue& queue)
		{
			queue.emplace(buf, size);
		});
	}

	size_t tcp_server::handle_output(char* buf, size_t size)
	{
		if (out_queue_.get_raw().empty())
		{
			return 0;
		}

		return out_queue_.access<size_t>([&](stream_queue& queue)
		{
			for (size_t i = 0; i < size; ++i)
			{
				if (queue.empty())
				{
					return i;
				}

				buf[i] = queue.front();
				queue.pop();
			}

			return size;
		});
	}

	bool tcp_server::pending_data()
	{
		return !this->out_queue_.get_raw().empty();
	}

	void tcp_server::frame()
	{
		if (this->in_queue_.get_raw().empty())
		{
			return;
		}

		while (true)
		{
			std::string packet{};
			const auto result = this->in_queue_.access<bool>([&](data_queue& queue)
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

			this->handle(packet);
		}
	}

	void tcp_server::send(const std::string& data)
	{
		out_queue_.access([&](stream_queue& queue)
		{
			for (const auto& val : data)
			{
				queue.push(val);
			}
		});
	}
}
