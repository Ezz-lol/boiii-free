#pragma once

#include <game/game.hpp>

namespace script_error {
struct report {
  std::string text;
};

void print_report(const report &report, const std::string &outcome);
void show_error_popup(const std::string &text);

void report_runaway_loop(game::scr::scriptInstance_t inst, const uint8_t *pos,
                         uint32_t elapsed_ms);

} // namespace script_error
