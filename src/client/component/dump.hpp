#pragma once

#include <game/game.hpp>
#include <utils/io.hpp>

namespace dump {
using namespace game;
using namespace game::db::xasset;

struct AssetBuffer {
  const uint8_t *buffer;
  size_t size;
};

struct AssetBytes {
  std::vector<AssetBuffer> buffers;
  std::string name;
  size_t size; // total
};

AssetBytes asset_bytes(const XAssetType type, const char *name,
                       XAssetHeader header);

void dump_requested_assets(const XAssetType type, const char *name,
                           XAssetHeader header);

} // namespace dump