#pragma once

#include <filesystem>
#include <windows.h>
#include <psapi.h>

#pragma comment(lib, "psapi.lib")

namespace game {
uintptr_t get_engine_base();
bool is_server();
bool is_client();
bool is_legacy_client();
std::filesystem::path game_directory();

inline uintptr_t get_base() {
  return reinterpret_cast<uintptr_t>(GetModuleHandleW(NULL));
}

size_t current_module_size();
} // namespace game