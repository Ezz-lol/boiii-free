#pragma once

#include "structs.hpp"

#define WEAK __declspec(selectany)

namespace game
{
#define Com_Error(code, fmt, ...) Com_Error_(__FILE__, __LINE__, code, fmt, ##__VA_ARGS__)

	// CL
	WEAK symbol<void(int controllerIndex, XSESSION_INFO* hostInfo, const netadr_t* addr, int numPublicSlots,
	                 int numPrivateSlots, const char* mapname, const char* gametype)> CL_ConnectFromLobby
		{0x14134C570};

	// Game
	WEAK symbol<void(gentity_s* ent, gentity_s* target, int mode, const char* chatText)> G_Say{0x0, 0x140299170};
	WEAK symbol<void(const char* fmt, ...)> G_LogPrintf{0x0, 0x1402A7BB0};

	// Com
	WEAK symbol<void(int channel, unsigned int label, const char* fmt, ...)> Com_Printf{0x1421499C0, 0x140505630};
	WEAK symbol<void(const char* file, int line, int code, const char* fmt, ...)> Com_Error_{0x1420F8BD0};
	WEAK symbol<bool(eModes mode)> Com_SessionMode_IsMode{0x1420F7DD0};
	WEAK symbol<void(uint32_t localClientNum, eModes fromMode, eModes toMode, uint32_t flags)> Com_SwitchMode{
		0x14214AF30
	};

	WEAK symbol<void(uint32_t localClientNum, const char* text)> Cbuf_AddText{0x1420EC8B0, 0x1404F75B0};
	WEAK symbol<void(int localClientNum, ControllerIndex_t controllerIndex, const char* buffer)> Cbuf_ExecuteBuffer{0x0, 0x1404F78D0};
	WEAK symbol<void(const char* cmdName, xcommand_t function, cmd_function_s* allocedCmd)> Cmd_AddCommandInternal{
		0x1420ED530, 0x1404F8210
	};
	WEAK symbol<void()> Cbuf_AddServerText_f{0x0, 0x1407DB4C0};
	WEAK symbol<void(const char* cmdName, xcommand_t function, cmd_function_s* allocedCmd)> Cmd_AddServerCommandInternal{
		0x0, 0x1404F8280
	};
	WEAK symbol<void(uint32_t localClientNum, ControllerIndex_t controllerIndex, const char* text,
	                 bool fromRemoteConsol)> Cmd_ExecuteSingleCommand{
		0x1420EDC20
	};
	WEAK symbol<void(char* text, int maxSize)> Con_GetTextCopy{0x14133A7D0, 0x140182C40};

	// DB
	WEAK symbol<void(XZoneInfo* zoneInfo, uint32_t zoneCount, bool sync, bool suppressSync)> DB_LoadXAssets{
		0x1414236A0
	};

	// Live
	WEAK symbol<bool(uint64_t, int*, bool)> Live_GetConnectivityInformation{0x141E0C410};

	// MSG
	WEAK symbol<uint8_t(msg_t* msg)> MSG_ReadByte{0x142155EB0, 0x14050D1B0};

	// NET
	WEAK symbol<bool(netsrc_t sock, int length, const void* data, const netadr_t* to)> NET_SendPacket{
		0x142332F70, 0x140596E40
	};
	WEAK symbol<bool(char const*, netadr_t*)> NET_StringToAdr{0x1421731E0, 0x140515110};

	// Sys
	WEAK symbol<int()> Sys_Milliseconds{0x142333430};
	WEAK symbol<void()> Sys_ShowConsole{0x142333F80, 0x140597E40};
	WEAK symbol<TLSData*()> Sys_GetTLS{0x142184210, 0x140525EB0};
	WEAK symbol<TLSData*()> Sys_IsDatabaseReady{0x1421844C0};

	// Dvar
	WEAK symbol<const dvar_t*(const char* dvarName)> Dvar_FindVar{0x1422BD730, 0x140575540};
	WEAK symbol<unsigned int(const char* str)> Dvar_GenerateHash{0x14133DBF0};
	WEAK symbol<dvar_t*(unsigned int hash)> Dvar_FindMalleableVar{0x1422BD6A0};
	WEAK symbol<const char*(const dvar_t* dvar)> Dvar_GetDebugName{0x1422BDCB0};
	WEAK symbol<const char*(const dvar_t* dvar)> Dvar_GetString{0x1422BFFF0, 0x140575E30};
	WEAK symbol<void(const char* dvarName, const char* string, bool createIfMissing)> Dvar_SetFromStringByName{
		0x1422C7F60
	};

	// Scr
	WEAK symbol<void(scriptInstance_t inst, const char* value)> Scr_AddString{0x0, 0x14016F320};
	WEAK symbol<const char* (scriptInstance_t inst, unsigned int index)> Scr_GetString{0x0, 0x140171490};
	WEAK symbol<void(gentity_s* ent, ScrVarCanonicalName_t stringValue, unsigned int paramcount)> Scr_Notify_Canon{0x0, 0x1402F5FF0};
	WEAK symbol<unsigned int(scriptInstance_t inst)> Scr_GetNumParam{0x0, 0x140171320};

	WEAK symbol<void(uint64_t id, bool cancelAll)> Cinematic_StopPlayback{0x1412BEA70};

	// Rendering
	WEAK symbol<void(const char*, int, const void*, float, float, float, float, float, const float*, int)>
	R_AddCmdDrawText{
		0x141CD98D0
	};

	// SV
	WEAK symbol<void*()> SV_AddTestClient{0x1422499A0, 0x14052E3E0};
	WEAK symbol<void(client_s* cl_0, svscmd_type type, const char* fmt, ...)> SV_SendServerCommand{0x0, 0x140537F10};

	// Variables

	WEAK symbol<cmd_function_s> cmd_functions{0x15689FF58, 0x14946F860};
	WEAK symbol<CmdArgs> sv_cmd_args{0x15689CE30, 0x14944C740};

	WEAK symbol<gentity_s> g_entities{0x0, 0x1471031B0};

	WEAK symbol<int> level_time{0x0, 0x1474FDC94};

	WEAK symbol<SOCKET> ip_socket{0x157E77818, 0x14A640988};

	WEAK symbol<Join> s_join{0x15574C640};

	namespace s_wcd
	{
		WEAK symbol<HWND> codLogo{0x157E77A50, 0x14A640BC0};
		WEAK symbol<HFONT> hfBufferFont{0x157E77A58, 0x14A640BC8};
		WEAK symbol<HWND> hWnd{0x157E77A40, 0x14A640BB0};
		WEAK symbol<HWND> hwndBuffer{0x157E77A48, 0x14A640BB8};
		WEAK symbol<HWND> hwndInputLine{0x157E77A60, 0x14A640BD0};
		WEAK symbol<int> windowHeight{0x157E7806C, 0x14A6411DC};
		WEAK symbol<int> windowWidth{0x157E78068, 0x14A6411D8};
		WEAK symbol<WNDPROC> SysInputLineWndProc{0x157E78070, 0x14A6411E0};
	}

	// Global game definitions
	constexpr auto CMD_MAX_NESTING = 8;

	// Re-implementations
	eModes Com_SessionMode_GetMode();

	bool I_islower(int c);
	bool I_isupper(int c);

	unsigned int Scr_CanonHash(const char* str);
}
