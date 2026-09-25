#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include <utils/nt.hpp> // IWYU pragma: export

#include <game/base.hpp>  // IWYU pragma: export
#include <game/flags.hpp> // IWYU pragma: export
#include <game/log.hpp>   // IWYU pragma: export
#include <game/ptr.hpp>   // IWYU pragma: export

namespace arxan::detail {
void set_address_to_call(const void *address);
extern void *callstack_proxy_addr;
} // namespace arxan::detail

namespace game {

constexpr uint32_t APP_ID = 311210;
constexpr std::string_view APP_ID_STR = "311210";

void show_error(const std::string &text, const std::string &title = "Error");

const std::filesystem::path &get_appdata_path();
const std::filesystem::path &get_game_path();
const std::vector<std::string> &get_registered_dvar_names();
size_t get_registered_dvar_name_count();

} // namespace game

#include "symbol.hpp" // IWYU pragma: export

#include "structs/structs.hpp" // IWYU pragma: export

#include "symbols/symbols.hpp" // IWYU pragma: export

#include "impl/game/dvar.hpp" // IWYU pragma: export
