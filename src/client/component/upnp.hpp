#pragma once

#include <cstdint>
#include <string>

namespace upnp {
void open_port(uint16_t port);
void close_port();
std::string external_endpoint();
} // namespace upnp
