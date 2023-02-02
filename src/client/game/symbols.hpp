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

	// Com
	WEAK symbol<void(int channel, unsigned int label, const char* fmt, ...)> Com_Printf{0x1421499C0};
	WEAK symbol<void(const char* file, int line, int code, const char* fmt, ...)> Com_Error_{0x1420F8BD0};
	WEAK symbol<bool(eModes mode)> Com_SessionMode_IsMode{0x1420F7DD0};
	WEAK symbol<bool()> Com_IsRunningUILevel{0x142148DB0};
	WEAK symbol<void(uint32_t localClientNum, eModes fromMode, eModes toMode, uint32_t flags)> Com_SwitchMode{
		0x14214AF30
	};

	WEAK symbol<void(uint32_t localClientNum, const char* text)> Cbuf_AddText{0x1420EC8B0};
	WEAK symbol<void(const char* cmdName, xcommand_t function, cmd_function_s* allocedCmd)> Cmd_AddCommandInternal{
		0x1420ED530
	};
	WEAK symbol<void(uint32_t localClientNum, ControllerIndex_t controllerIndex, const char* text,
	                 bool fromRemoteConsol)> Cmd_ExecuteSingleCommand{
		0x1420EDC20
	};
	WEAK symbol<void(char* text, int maxSize)> Con_GetTextCopy{0x14133A7D0};
	WEAK symbol<void(uint32_t localClientNum, ControllerIndex_t controllerIndex, const char* buffer)> Cbuf_ExecuteBuffer{0x14133BE10};

	// DB
	WEAK symbol<void(XZoneInfo* zoneInfo, uint32_t zoneCount, bool sync, bool suppressSync)> DB_LoadXAssets{
		0x1414236A0
	};
	WEAK symbol<XAssetHeader(XAssetType type, const char* name, bool errorIfMissing, int waitTime)> DB_FindXAssetHeader{ 0x141420ED0 };

	// Live
	WEAK symbol<bool(uint64_t, int*, bool)> Live_GetConnectivityInformation{0x141E0C410};

	// MSG
	WEAK symbol<uint8_t(msg_t* msg)> MSG_ReadByte{0x142155EB0};

	// NET
	WEAK symbol<bool(netsrc_t sock, int length, const void* data, const netadr_t* to)> NET_SendPacket{0x142332F70};
	WEAK symbol<bool(char const*, netadr_t*)> NET_StringToAdr{0x1421731E0};
	WEAK symbol<bool(netadr_t*, char const*)> NetAdr_InitFromString{0x142332F70};

	// Sys
	WEAK symbol<int()> Sys_Milliseconds{0x142333430};
	WEAK symbol<void()> Sys_ShowConsole{0x142333F80};
	WEAK symbol<TLSData*()> Sys_GetTLS{0x142184210};

	// Dvar
	WEAK symbol<dvar_t*(const char* dvarName)> Dvar_FindVar{0x1422BD730};
	WEAK symbol<unsigned int(const char* str)> Dvar_GenerateHash{0x14133DBF0};
	WEAK symbol<dvar_t*(unsigned int hash)> Dvar_FindMalleableVar{0x1422BD6A0};
	WEAK symbol<const char*(const dvar_t* dvar)> Dvar_GetDebugName{0x1422BDCB0};
	WEAK symbol<const char*(const dvar_t* dvar)> Dvar_DisplayableValue{0x1422BCAE0};
	WEAK symbol<const char*(const dvar_t* dvar)> Dvar_GetString{0x1422BFFF0};
	WEAK symbol<bool(const dvar_t* dvar)> Dvar_GetBool{ 0x1422BD930 };
	WEAK symbol<dvar_t*(dvarStrHash_t hash, const char* dvarName, bool value, dvarFlags_e flags, const char* description)> Dvar_RegisterBool{
		0x1422D1360
	};
	WEAK symbol<void (void (*callback)(const dvar_t*, void*), void* userData)> Dvar_ForEach{ 0x1422BD760 };
	WEAK symbol<void(const char* dvarName, const char* string, bool createIfMissing)> Dvar_SetFromStringByName{
		0x1422C7F60
	};
	WEAK symbol<char> s_dvarPool{ 0x157AC8220 };
	WEAK symbol<int> g_dvarCount{ 0x157AC81CC };

	// Rendering
	WEAK symbol<void(const char*, int, const void*, float, float, float, float, float, const float*, int)>
	R_AddCmdDrawText{
		0x141CD98D0
	};

	// Rendering
	WEAK symbol<void*()> SV_AddTestClient{0x1422499A0};

	// Variables

	WEAK symbol<cmd_function_s> cmd_functions{0x15689FF58};
	WEAK symbol<CmdArgs> sv_cmd_args{0x15689CE30};

	WEAK symbol<SOCKET> ip_socket{0x157E77818};

	WEAK symbol<Join> s_join{0x15574C640};

	namespace s_wcd
	{
		WEAK symbol<HWND> codLogo{0x157E77A50};
		WEAK symbol<HFONT> hfBufferFont{0x157E77A58};
		WEAK symbol<HWND> hWnd{0x157E77A40};
		WEAK symbol<HWND> hwndBuffer{0x157E77A48};
		WEAK symbol<HWND> hwndInputLine{0x157E77A60};
		WEAK symbol<int> windowHeight{0x157E7806C};
		WEAK symbol<int> windowWidth{0x157E78068};
		WEAK symbol<WNDPROC> SysInputLineWndProc{0x157E78070};
	}

	// Global game definitions
	constexpr auto CMD_MAX_NESTING = 8;

	// Re-implementations
	eModes Com_SessionMode_GetMode();

	namespace hks
	{
		WEAK symbol<lua_State*> lua_state { 0x159C78D88 };
		WEAK symbol<void(lua_State* s, const char* str, unsigned int l)> hksi_lua_pushlstring{ 0x140A18430 };

		WEAK symbol<void(lua_State* s, const HksObject* tbl, const HksObject* key, const HksObject* val)> hks_obj_settable{ 0x141D4B660 };
		WEAK symbol<HksObject* (HksObject* result, lua_State* s, const HksObject* table, const HksObject* key)> hks_obj_gettable{ 0x141D4ABF0 };
		WEAK symbol<void(lua_State* s, int nargs, int nresults, const unsigned int* pc)> vm_call_internal{ 0x141D71070 };
		WEAK symbol<HashTable* (lua_State* s, unsigned int arraySize, unsigned int hashSize)> Hashtable_Create{ 0x141D3B5F0 };
		WEAK symbol<cclosure* (lua_State* s, lua_function function, int num_upvalues, int internal_, int profilerTreatClosureAsFunc)> cclosure_Create{ 0x141D3B7E0 };
		WEAK symbol<int(lua_State* s, int t)> hksi_luaL_ref{ 0x141D4D1A0 };
		WEAK symbol<void(lua_State* s, int t, int ref)> hksi_luaL_unref{ 0x141D4D320 };

		WEAK symbol<int(lua_State* s, const HksCompilerSettings* options, const char* buff, unsigned __int64 sz, const char* name)> hksi_hksL_loadbuffer{ 0x141D4BD80 };
		WEAK symbol<int(lua_State* s, const char* what, lua_Debug* ar)> hksi_lua_getinfo{ 0x141D4D960 };
		WEAK symbol<int(lua_State* s, int level, lua_Debug* ar)> hksi_lua_getstack{ 0x141D4DC20 };
		WEAK symbol<void(lua_State* s, const char* fmt, ...)> hksi_luaL_error{ 0x141D4D050 };
		WEAK symbol<const char*> s_compilerTypeName{ 0x140A18430 };
	}
}
