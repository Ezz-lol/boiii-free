#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace live {
namespace anticheat {
WEAK symbol<bool(const ControllerIndex_t controllerIndex)>
    LiveAntiCheat_ConsoleDetailsReported{0x141DE8E20};
}
} // namespace live
} // namespace game