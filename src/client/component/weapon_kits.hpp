#pragma once

namespace weapon_kits {
void on_asset_linked(const game::db::xasset::XAssetEntry *entry);
void set_server_support(bool supported);
} // namespace weapon_kits
