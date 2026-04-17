#pragma once

#include <cstdint>
namespace game {
namespace cmd {

using xcommand_t = void (*)();

struct cmd_function_s {
  cmd_function_s *next;
  const char *name;
  const char *autoCompleteDir;
  const char *autoCompleteExt;
  xcommand_t function;
  int32_t autoComplete;
};
} // namespace cmd
} // namespace game