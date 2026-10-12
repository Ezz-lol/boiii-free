#pragma once

#include <string>

namespace gsc {
namespace func {
namespace command {
std::string normalize_command_name(std::string value);

std::string extract_command_name(const std::string &value);
} // namespace command
} // namespace func
} // namespace gsc
