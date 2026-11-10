#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {

namespace demo {
WEAK symbol<bool()> Demo_IsPlaying{0x1425870A0, 0x0};

constexpr int32_t DEMO_MAX_XANIMS = 2048;
WEAK symbol<void(scr::ScrString_t name, int32_t type)> Demo_AddXAnim{0x141433320, 0x1401E25E0};
WEAK symbol<int32_t> demo_xanimCount{0x1593C0D58, 0x14AB5F7E8};

} // namespace demo
} // namespace game
