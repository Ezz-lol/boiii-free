#pragma once

#include "builtin_snapshot.hpp"

#include <Windows.h>

namespace zombies::coop::builtin_snapshot {
struct windows_reader {
  bool query(std::uintptr_t address, memory_region& output) const noexcept {
    MEMORY_BASIC_INFORMATION information{};
    if (VirtualQuery(reinterpret_cast<const void*>(address), &information, sizeof(information)) != sizeof(information))
      return false;
    const auto protection = information.Protect & 0xFFu;
    output.base = reinterpret_cast<std::uintptr_t>(information.BaseAddress);
    output.size = information.RegionSize;
    output.allocation_base = reinterpret_cast<std::uintptr_t>(information.AllocationBase);
    output.committed = information.State == MEM_COMMIT;
    output.guarded = (information.Protect & PAGE_GUARD) != 0;
    output.readable = protection == PAGE_READONLY || protection == PAGE_READWRITE
        || protection == PAGE_WRITECOPY || protection == PAGE_EXECUTE_READ
        || protection == PAGE_EXECUTE_READWRITE || protection == PAGE_EXECUTE_WRITECOPY;
    output.executable = protection == PAGE_EXECUTE_READ || protection == PAGE_EXECUTE_READWRITE
        || protection == PAGE_EXECUTE_WRITECOPY;
    return true;
  }
  bool copy(std::uintptr_t address, void* destination, std::size_t length) const noexcept {
    SIZE_T received = 0;
    return ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<const void*>(address),
                             destination, length, &received) && received == length;
  }
};

inline void capture_and_write(const char* family, bool dedicated, std::uintptr_t image_base) noexcept {
  // This command has no parameters and never invokes an engine function. An
  // unsuccessful automatic capture leaves one bounded manual retry available.
  if (!dedicated) {
    printf("COOP_BUILTIN_CAPTURE|{\"version\":1,\"family\":\"%s\",\"role\":\"client\",\"supported\":false,\"captured\":false,\"reason\":\"unsupported_client\"}\n", family);
    fflush(stdout);
    return;
  }
  static attempt_budget budget;
  const auto attempt = budget.claim();
  if (!attempt) {
    printf("COOP_BUILTIN_CAPTURE|{\"version\":1,\"family\":\"%s\",\"role\":\"dedicated\",\"captured\":false,\"reason\":\"capture_complete_busy_or_limit\"}\n", family);
    fflush(stdout);
    return;
  }
  windows_reader reader;
  const auto result = capture(reader, image_base);
  std::array<char, json_limit> document{};
  const auto document_size = format(result, family, document.data(), document.size());
  std::array<char, 128> relative_path{};
  std::array<wchar_t, 128> native_path{};
  const auto path_size = std::snprintf(relative_path.data(), relative_path.size(),
      "logs/coop-builtin-%lu-%llu-%u.json", GetCurrentProcessId(),
      static_cast<unsigned long long>(GetTickCount64()), attempt);
  bool written = false;
  DWORD io_error = ERROR_SUCCESS;
  if (document_size > 0 && path_size > 0 && static_cast<std::size_t>(path_size) < relative_path.size()) {
    for (int i = 0; i < path_size; ++i)
      native_path[static_cast<std::size_t>(i)] = static_cast<wchar_t>(relative_path[static_cast<std::size_t>(i)]);
    if (CreateDirectoryW(L"logs", nullptr) || GetLastError() == ERROR_ALREADY_EXISTS) {
      const auto output = CreateFileW(native_path.data(), GENERIC_WRITE, FILE_SHARE_READ,
          nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
      if (output != INVALID_HANDLE_VALUE) {
        DWORD received = 0;
        written = WriteFile(output, document.data(), static_cast<DWORD>(document_size), &received, nullptr)
                  && received == static_cast<DWORD>(document_size);
        if (!written) io_error = GetLastError();
        if (!CloseHandle(output)) { written = false; io_error = GetLastError(); }
      } else {
        io_error = GetLastError();
      }
    } else {
      io_error = GetLastError();
    }
  }
  const bool successful = result.valid && written;
  budget.finish(successful);
  printf("COOP_BUILTIN_CAPTURE|{\"version\":1,\"family\":\"%s\",\"role\":\"dedicated\",\"supported\":true,\"captured\":%s,\"attempt\":%u,\"reason\":\"%s\",\"file_written\":%s,\"file\":\"%s\",\"code_bytes\":%u,\"io_error\":%lu}\n",
      family, successful ? "true" : "false", attempt,
      document_size < 0 ? "format_overflow" : !written ? "write_failed" : result.reason,
      written ? "true" : "false", relative_path.data(), result.code_size, io_error);
  fflush(stdout);
}
} // namespace zombies::coop::builtin_snapshot
