#pragma once

#include "func.hpp"
#include <cstdint>
namespace game {
namespace cmd {

typedef fastcall_t<void()> xcommand_t;

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