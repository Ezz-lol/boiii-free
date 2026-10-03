#pragma once

#include <game/game.hpp>

namespace script_error {
struct report {
  std::string text;
  std::string summary;
};

report build_report(game::errorParm code, const char *message,
                    const char *source_file);
void print_report(const report &report, const std::string &outcome);

void report_runaway_loop(game::scr::scriptInstance_t inst, const uint8_t *pos,
                         uint32_t elapsed_ms);

} // namespace script_error
