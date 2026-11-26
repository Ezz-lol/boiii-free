#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace zombies::coop {
inline constexpr int companion_revision = 3;
inline constexpr std::string_view companion_contract =
    "e1a7df2021d9fea869c1871e836af092c6e9e8fb6828f77a3f7dbfc5cde1eb3d";

// Native session/request state, independent of saved preferences or script dvars.
int frozen_schema_capacity() noexcept;
std::string frozen_schema_map();
void schema_asset_failure(std::string_view reason);
bool schema_assets_failed() noexcept;
bool prepare_schema_session(int capacity, std::string_view map, std::string& error);
// Only a caller which verified the remote ZM schema descriptor may use this.
bool prepare_peer_schema_session(int capacity, std::string_view map, std::string& error, std::uint64_t request_id);
void cancel_peer_schema_session() noexcept;
void refresh_schema_session() noexcept;
void schema_begin_server_vm();
void reset_schema_session() noexcept;
bool companion_files_ready(std::string& error);
bool schema_server_ready() noexcept;
std::string score_schema_wrapper();
} // namespace zombies::coop
