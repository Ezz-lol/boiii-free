#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lua {
namespace lobby {
namespace vm {
WEAK symbol<void()> LobbyVM_Reload{0x141EE4640, 0x141EF0FB0, 0x1404A1F50};
WEAK symbol<void()> LobbyVM_OnErrorShutdown{0x141EE3710, 0x0, 0x1404A1280};
} // namespace vm
} // namespace lobby
} // namespace lua
} // namespace game
