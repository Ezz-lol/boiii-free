#pragma once

#include "structs.hpp"
#include "loader/component_loader.hpp"

#define WEAK __declspec(selectany)

namespace game
{
	// CL
	WEAK symbol<void(int controllerIndex, XSESSION_INFO* hostInfo, const netadr_t* addr, int numPublicSlots,
	                 int numPrivateSlots, const char* mapname, const char* gametype)> CL_ConnectFromLobby
		{0x14134C570_g};

	// Com
	WEAK symbol<void(int channel, unsigned int label, const char* fmt, ...)> Com_Printf{0x1421499C0_g};
	WEAK symbol<void(const char* file, int line, int code, const char* fmt, ...)> Com_Error_{0x1420F8BD0_g};
	WEAK symbol<bool(eModes mode)> Com_SessionMode_IsMode{0x1420F7DD0_g};

	WEAK symbol<void(int localClientNum, const char* text)> Cbuf_AddText{0x1420EC8B0_g};
	WEAK symbol<void(const char* cmdName, xcommand_t function, cmd_function_s* allocedCmd)> Cmd_AddCommandInternal{
		0x1420ED530_g
	};
	WEAK symbol<void(char* text, int maxSize)> Con_GetTextCopy{0x14133A7D0_g};

	// DB
	WEAK symbol<void(XZoneInfo* zoneInfo, uint32_t zoneCount, bool sync, bool suppressSync)> DB_LoadXAssets{
		0x1414236A0_g
	};

	// Live
	WEAK symbol<bool(uint64_t, int*, bool)> Live_GetConnectivityInformation{0x141E0C410_g};

	// MSG
	WEAK symbol<uint8_t(msg_t* msg)> MSG_ReadByte{0x142155EB0_g};

	// NET
	WEAK symbol<bool(netsrc_t sock, int length, const void* data, const netadr_t* to)> NET_SendPacket{0x142332F70_g};
	WEAK symbol<bool(char const*, netadr_t*)> NET_StringToAdr{0x1421731E0_g};
	WEAK symbol<bool(netadr_t*, char const*)> NetAdr_InitFromString{0x142332F70_g};

	// Sys
	WEAK symbol<int()> Sys_Milliseconds{0x142333430_g};
	WEAK symbol<void()> Sys_ShowConsole{0x142333F80_g};
	WEAK symbol<TLSData*()> Sys_GetTLS{0x142184210_g};

	// Dvar
	WEAK symbol<const dvar_t*(const char* dvarName)> Dvar_FindVar{0x1422BD730_g};
	WEAK symbol<unsigned int(const char* str)> Dvar_GenerateHash{0x14133DBF0_g};
	WEAK symbol<dvar_t*(unsigned int hash)> Dvar_FindMalleableVar{0x1422BD6A0_g};
	WEAK symbol<const char*(const dvar_t* dvar)> Dvar_GetDebugName{0x1422BDCB0_g};
	WEAK symbol<const char*(const dvar_t* dvar)> Dvar_GetString{0x1422BFFF0_g};
	WEAK symbol<void(const char* dvarName, const char* string, bool createIfMissing)> Dvar_SetFromStringByName{
		0x1422C7F60_g
	};

	// Rendering
	WEAK symbol<void(const char*, int, void*, float, float, float, float, float, float*, int)> R_AddCmdDrawText{
		0x141CD98D0_g
	};

	// Variables
	WEAK symbol<cmd_function_s> cmd_functions{0x15689FF58_g};
	WEAK symbol<CmdArgs> sv_cmd_args{0x15689CE30_g};

	WEAK symbol<SOCKET> ip_socket{0x157E77818_g};

	WEAK symbol<Join> s_join{0x15574C640_g};

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
