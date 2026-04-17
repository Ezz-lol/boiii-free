#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {

namespace s_wcd {
WEAK symbol<HWND> codLogo{0x157E75A50, 0x14A640BC0};
WEAK symbol<HFONT> hfBufferFont{0x157E75A58, 0x14A640BC8};
WEAK symbol<HWND> hWnd{0x157E75A40, 0x14A640BB0};
WEAK symbol<HWND> hwndBuffer{0x157E75A48, 0x14A640BB8};
WEAK symbol<HWND> hwndInputLine{0x157E75A60, 0x14A640BD0};
WEAK symbol<int> windowHeight{0x157E7606C, 0x14A6411DC};
WEAK symbol<int> windowWidth{0x157E76068, 0x14A6411D8};
WEAK symbol<WNDPROC> SysInputLineWndProc{0x157E76070, 0x14A6411E0};
} // namespace s_wcd
} // namespace game

#endif
