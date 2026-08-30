#pragma once
#include <game/game.hpp>

namespace asset_limits {
using namespace game::db::xasset;

struct default_pool_config {
  XAssetType type;
  const char *setting_key;
  uint32_t default_size;
};

struct pool_config {
  XAssetType type;
  uint32_t size;
};

std::vector<pool_config> parse_list(std::string &data);
void apply_list(const std::vector<pool_config> &limits);

} // namespace asset_limits