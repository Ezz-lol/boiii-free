#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace scr {
namespace builtin {
namespace bg {
WEAK symbol<void(scriptInstance_t inst)> BGScr_IsProfileBuild{0x1401660E0,
                                                              0x140099230};
}
} // namespace builtin
} // namespace scr
} // namespace game