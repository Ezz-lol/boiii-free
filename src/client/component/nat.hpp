#pragma once

#include <string>
#include <functional>
#include <vector>

namespace nat {
struct friend_presence {
  uint64_t steam_id{};
  std::string token;
  std::string endpoint;
};

using lookup_callback = std::function<void(std::vector<friend_presence>)>;

std::string get_host_token();
std::string get_host_endpoint();
void begin_join(const std::string &token, const std::string &fallback_address);
bool set_open_to_friends(bool enabled);
void refresh_friends(const std::vector<uint64_t> &steam_ids,
                     lookup_callback callback);
} // namespace nat
