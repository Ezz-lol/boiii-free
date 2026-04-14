#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <utils/byte_buffer.hpp>
#include <utils/concurrency.hpp>

#include "../component/network.hpp"
#include "structs/net.hpp"

namespace game::fragment_handler {
bool handle(const net::netadr_t &target, utils::byte_buffer &buffer,
            std::string &final_packet);

void clean();

void fragment_data(
    const void *data, size_t size,
    const std::function<void(const utils::byte_buffer &buffer)> &callback);
} // namespace game::fragment_handler