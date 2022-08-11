#pragma once

namespace game
{
	typedef void (*xcommand_t)();

	enum errorCode
	{
		ERROR_NONE = 0x0,
		ERROR_FATAL = 0x1,
		ERROR_DROP = 0x2,
		ERROR_FROM_STARTUP = 0x4,
		ERROR_SERVERDISCONNECT = 0x8,
		ERROR_DISCONNECT = 0x10,
		ERROR_SCRIPT = 0x20,
		ERROR_SCRIPT_DROP = 0x40,
		ERROR_LOCALIZATION = 0x80,
		ERROR_UI = 0x100,
		ERROR_LUA = 0x200,
		ERROR_SOFTRESTART = 0x400,
		ERROR_SOFTRESTART_KEEPDW = 0x800,
	};

	struct cmd_function_s
	{
		cmd_function_s* next;
		const char* name;
		const char* autoCompleteDir;
		const char* autoCompleteExt;
		xcommand_t function;
	};

	struct CmdArgs
	{
		int nesting;
		int localClientNum[8];
		int controllerIndex[8];
		int argshift[8];
		int argc[8];
		const char** argv[8];
		char textPool[8192];
		const char* argvPool[512];
		int usedTextPool[8];
		int totalUsedArgvPool;
		int totalUsedTextPool;
	};

	struct va_info_t
	{
		char va_string[4][1024];
		int index;
	};

	struct TLSData
	{
		va_info_t* vaInfo;
		jmp_buf* errorJmpBuf;
		void* traceInfo;
		CmdArgs* cmdArgs;
		void* errorData;
	};

	struct dvar_t
	{
		unsigned int name;
	}; // Incomplete
}
