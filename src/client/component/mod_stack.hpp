#pragma once

#include <string>
#include <utility>
#include <vector>

namespace mod_stack {
std::vector<std::string> get(const std::string &main_mod);
std::vector<std::string> get();
std::string serialize();
void set(const std::string &list);
void begin_load();
bool is_loaded();

std::vector<std::pair<std::string, std::string>> script_copies();

} // namespace mod_stack
