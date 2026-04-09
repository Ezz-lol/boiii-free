#pragma once

namespace network_password {
uint64_t hash_password(const std::string &password);
std::string get_password_hash_string();
bool is_password_set();
} // namespace network_password
