#include "minidump.hpp"

#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")

namespace exception
{
	namespace
	{
		constexpr MINIDUMP_TYPE get_minidump_type()
		{
			constexpr auto type = MiniDumpIgnoreInaccessibleMemory //
				| MiniDumpWithHandleData //
				| MiniDumpScanMemory //
				| MiniDumpWithProcessThreadData //
				| MiniDumpWithFullMemoryInfo //
				| MiniDumpWithThreadInfo //
				| MiniDumpWithUnloadedModules;

			return static_cast<MINIDUMP_TYPE>(type);
		}

		std::string get_temp_filename()
		{
			char filename[MAX_PATH] = {0};
			char pathname[MAX_PATH] = {0};

			GetTempPathA(sizeof(pathname), pathname);
			GetTempFileNameA(pathname, "boiii-", 0, filename);
			return filename;
		}

		HANDLE write_dump_to_temp_file(const LPEXCEPTION_POINTERS exceptioninfo)
		{
			MINIDUMP_EXCEPTION_INFORMATION minidump_exception_info = {GetCurrentThreadId(), exceptioninfo, FALSE};

			auto* const file_handle = CreateFileA(get_temp_filename().data(), GENERIC_WRITE | GENERIC_READ,
			                                      FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS,
			                                      FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
			                                      nullptr);

			if (!MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file_handle, get_minidump_type(),
			                       &minidump_exception_info,
			                       nullptr,
			                       nullptr))
			{
				MessageBoxA(nullptr, "There was an error creating the minidump! Hit OK to close the program.",
				            "Minidump Error", MB_OK | MB_ICONERROR);
				TerminateProcess(GetCurrentProcess(), 123);
			}

			return file_handle;
		}

		std::string read_file(const HANDLE file_handle)
		{
			FlushFileBuffers(file_handle);
			SetFilePointer(file_handle, 0, nullptr, FILE_BEGIN);

			std::string buffer{};

			DWORD bytes_read = 0;
			char temp_bytes[0x2000];

			do
			{
				if (!ReadFile(file_handle, temp_bytes, sizeof(temp_bytes), &bytes_read, nullptr))
				{
					return {};
				}

				buffer.append(temp_bytes, bytes_read);
			}
			while (bytes_read == sizeof(temp_bytes));

			return buffer;
		}
	}

	std::string create_minidump(const LPEXCEPTION_POINTERS exceptioninfo)
	{
		const utils::nt::handle file_handle = write_dump_to_temp_file(exceptioninfo);
		return read_file(file_handle);
	}
}
