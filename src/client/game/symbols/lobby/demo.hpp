#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace demo {
WEAK symbol<void(LocalClientNum_t localClientNum,
                 lobby::msg::Msg_DemoState *msg)>
    Demo_ReadHeartbeat{0x142593950, 0x0, 0x140679BD0};

} // namespace demo
} // namespace lobby
} // namespace game
