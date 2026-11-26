#pragma once

#include "game/game.hpp"
#include "common_helper_source_pair.hpp"
#include <string>
#include <string_view>

namespace zombies::coop::schema_assets {
using script_asset = game::scr::ScriptParseTree;

using common_lifecycle = common_helper::lifecycle<script_asset>;
using common_request = common_lifecycle::request;
using common_binding = common_helper::binding<script_asset>;
using common_seal = common_helper::sealed_binding<script_asset>;
bool common_required() noexcept;
std::optional<common_request> common_begin_load() noexcept;
std::optional<common_binding>
bind_common_loader(script_asset *selected,
                   std::span<const std::uint8_t> compiled,
                   std::span<const std::uint8_t> readable = {},
                   std::span<const std::uint8_t> cleaned = {},
                   bool readable_success = false) noexcept;
std::optional<common_seal> seal_common_loader(const common_binding &binding,
                                              script_asset *selected) noexcept;
bool preregister_common(const common_request &request,
                        const common_seal &seal) noexcept;
void common_load_returned(const common_request &request) noexcept;
void common_finish_load(const common_request &request) noexcept;
void common_load_failed(const common_request &request) noexcept;
bool common_source_pair_ready() noexcept;
std::string common_source_pair_diagnostic();
script_asset *common_backing(const char *name) noexcept;
script_asset *powerup_backing(const char *name) noexcept;
void observe_common_detour(std::uintptr_t target) noexcept;
void observe_common_replacement(const char *name,
                                std::span<const std::uint8_t> compiled,
                                std::span<const std::uint8_t> readable = {},
                                std::span<const std::uint8_t> cleaned = {},
                                bool readable_success = false) noexcept;

// Retained map module/helper code dependencies in the current server VM.
bool handles_map_stock(const char *name) noexcept;
bool handles_map_helper(const char *name) noexcept;
script_asset *transform_map_stock(const char *name, script_asset *original) noexcept;
script_asset *lookup_map_helper(const char *name) noexcept;
bool pin_all_map_helpers(std::string &error) noexcept;
script_asset *current_map_backing(const char *name) noexcept;
bool map_mechanics_ready() noexcept;
std::string map_source_diagnostic();
// Optional Ezz replacefunc rejection for targets inside current owned helpers.
bool reject_map_helper_detour(std::uintptr_t target) noexcept;

// Pin the exact bytes whose SHA-256 was checked by the session manifest owner.
// Once pinned, an in-process payload replacement is refused.
bool pin_payloads(std::string_view server, std::string_view client,
                  std::string_view island, std::string &error);
void begin_vm_generation(bool client) noexcept;
bool prepare_client_generation(std::string_view map, bool fresh_request,
                               bool &changed) noexcept;
void reset_client_session_generation() noexcept;
// Current native server VM must own the exact stock score import adapter.
bool score_producer_ready() noexcept;
std::string score_source_diagnostic();
bool retention_exhausted() noexcept;
bool handles_companion(const char *name) noexcept;
script_asset *lookup_companion(const char *name) noexcept;
script_asset *transform(const char *name, script_asset *original) noexcept;
} // namespace zombies::coop::schema_assets
