#pragma once

#include "game/game.hpp"
#include <string>
#include <string_view>

namespace zombies::coop::schema_assets {
using script_asset = game::scr::ScriptParseTree;

// Pin the exact bytes whose SHA-256 was checked by the session manifest owner.
// Once pinned, an in-process payload replacement is refused.
bool pin_payloads(std::string_view server, std::string_view client, std::string& error);
void begin_vm_generation(bool client) noexcept;
bool prepare_client_generation(std::string_view map, bool fresh_request, bool& changed) noexcept;
void reset_client_session_generation() noexcept;
bool retention_exhausted() noexcept;
bool handles_companion(const char* name) noexcept;
script_asset* lookup_companion(const char* name) noexcept;
script_asset* transform(const char* name, script_asset* original) noexcept;
} // namespace zombies::coop::schema_assets
