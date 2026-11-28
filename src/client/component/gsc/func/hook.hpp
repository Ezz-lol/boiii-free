#pragma once

#include <game/game.hpp>
#include <macros/macros.hpp>

namespace gscr {
namespace hook {

using namespace game::scr;
using namespace game::scr::builtin;

#ifndef HOOK_SCR_BUILTIN_INNER
#define HOOK_SCR_BUILTIN_INNER(OrigFunc, Hook, TempHookStorage)                \
  static const decltype(OrigFunc) TempHookStorage = OrigFunc;                  \
  OrigFunc = Hook<TempHookStorage>;
#endif

#ifndef HOOK_SCR_BUILTIN
#define HOOK_SCR_BUILTIN(OrigFunc, Hook)                                       \
  HOOK_SCR_BUILTIN_INNER(OrigFunc, Hook,                                       \
                         UNIQUE_VAR(Scr_Hooked_Builtin_Storage_))
#endif

} // namespace hook
} // namespace gscr
