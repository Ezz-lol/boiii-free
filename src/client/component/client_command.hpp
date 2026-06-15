#pragma once

#include <functional>
#include <string>
namespace client_command {
using callback = std::function<void(game::level::gentity_s *ent,
                                    const command::params_sv &params)>;
void add(const std::string &name, const callback &cmd);
} // namespace client_command