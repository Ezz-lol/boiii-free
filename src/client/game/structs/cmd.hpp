#pragma once

#include <cstdint>
#include <structs/func.hpp>
namespace game {
namespace cmd {

typedef fastcall_t<void()> xcommandFunc_t;
typedef xcommandFunc_t *xcommand_t;

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
