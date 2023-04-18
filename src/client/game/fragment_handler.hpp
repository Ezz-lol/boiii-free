#pragma once

#include <utils/byte_buffer.hpp>
#include <utils/concurrency.hpp>

#include "../component/network.hpp"

namespace game::fragment_handler
{
	bool handle(const netadr_t& target, utils::byte_buffer& buffer,
	            std::string& final_packet);

	void clean();

	void fragment_data(const void* data, const size_t size,
	                   const std::function<void(const utils::byte_buffer& buffer)>& callback);
}
