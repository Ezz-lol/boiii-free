#pragma once

#define WEAK __declspec(selectany)

namespace game
{
	WEAK symbol<void(int localClientNum, const char* text)> Cbuf_AddText{0x1420EC8B0_g};
	WEAK symbol<void(int channel, unsigned int label, const char* fmt, ...)> Com_Printf{0x1421499C0_g};
	WEAK symbol<void(char* text, int maxSize)> Con_GetTextCopy{0x14133A7D0_g};
	WEAK symbol<void()> Sys_ShowConsole{0x142333F80_g};

	namespace s_wcd
	{
		WEAK symbol<HWND> codLogo{0x157E77A50_g};
		WEAK symbol<HFONT> hfBufferFont{0x157E77A58_g};
		WEAK symbol<HWND> hWnd{0x157E77A40_g};
		WEAK symbol<HWND> hwndBuffer{0x157E77A48_g};
		WEAK symbol<HWND> hwndInputLine{0x157E77A60_g};
		WEAK symbol<int> windowHeight{0x157E7806C_g};
		WEAK symbol<int> windowWidth{0x157E78068_g};
		WEAK symbol<WNDPROC> SysInputLineWndProc{0x157E78070_g};
	}
}
