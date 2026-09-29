#pragma once

#include <string>

namespace map_recovery {
std::string on_map_stopped();
void on_map_started();
void notify_clients(const std::string &reason);
} // namespace map_recovery
