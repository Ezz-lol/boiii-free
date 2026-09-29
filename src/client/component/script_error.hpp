#pragma once

#include <game/game.hpp>

namespace script_error {
struct report {
  std::string text;
  std::string summary;
};

bool is_script_vm_failure(const char *source_file, game::errorParm code);

report build_report(game::errorParm code, const char *message,
                    const char *source_file);
void print_report(const report &report, const std::string &outcome);

void mark_reported(const std::string &message);
bool is_reported(const char *message);
} // namespace script_error
