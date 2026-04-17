#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace cmd {

// Global game definitions
constexpr auto CMD_MAX_NESTING = 8;

WEAK symbol<cmd_function_s> cmd_functions{0x15689DF58, 0x14946F860};

WEAK symbol<void(const char *cmdName, xcommand_t function,
                 cmd_function_s *allocedCmd)>
    Cmd_AddCommandInternal{0x1420ECC90, 0x1404F8210};
// Reused _a lot_ in the engine for feature-gated function calls.
// There are a few dozen of these.
WEAK symbol<void()> Stub{0x0, 0x1407DB4C0};
WEAK symbol<void(const char *cmdName, xcommand_t function,
                 cmd_function_s *allocedCmd)>
    Cmd_AddServerCommandInternal{0x0, 0x1404F8280};
WEAK symbol<void(int localClientNum, ControllerIndex_t controllerIndex,
                 const char *text, bool fromRemoteConsole)>
    Cmd_ExecuteSingleCommand{0x1420ED380, 0x1404F8890};
WEAK symbol<void(int localClientNum, ControllerIndex_t localControllerIndex,
                 const char *text_in, int max_tokens, bool evalExpressions,
                 CmdArgs *args)>
    Cmd_TokenizeStringKernel{0x1420EED60, 0x1404FA300};
WEAK symbol<void()> Cmd_EndTokenizedString{0x1420ECED0, 0x1404F8420};
} // namespace cmd
} // namespace game

#endif
