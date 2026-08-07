#include <std_include.hpp>
#include <utils/nt.hpp>
#include <utils/flags.hpp>
#include "base.hpp"

namespace game {
namespace {
const utils::nt::library &get_host_library() {
  static const utils::nt::library host_library = [] {
    utils::nt::library host{};
    if (!host || host == utils::nt::library::get_by_address(get_engine_base)) {
      throw std::runtime_error("Invalid host application - Make sure you place "
                               "Boiii.exe next to BlackOps3.exe!");
    }

    return host;
  }();

  return host_library;
}
} // namespace

uintptr_t get_engine_base() {
  static const uintptr_t base =
      reinterpret_cast<uintptr_t>(get_host_library().get_ptr());
  return base;
}

bool is_server() {
  static const bool is_server =
      get_host_library().get_optional_header()->CheckSum == 0x14C28B4;
  return is_server;
}

bool is_client() {
  static const bool is_client = []() -> bool {
    if (utils::flags::has_flag("newsteamclient")) {
      return get_host_library().get_optional_header()->CheckSum == 0x6517980;
    }

    return get_host_library().get_optional_header()->CheckSum == 0x888C368;
  }();

  return is_client;
}

bool is_legacy_client() {
  static const bool is_legacy_client =
      get_host_library().get_optional_header()->CheckSum == 0x8880704;
  return is_legacy_client;
}

std::filesystem::path game_directory() {
  return get_host_library().get_path().parent_path();
}

size_t current_module_size() {
  static const size_t module_size = []() -> size_t {
    MODULEINFO modInfo;
    // Pass NULL to get information for the current executable/module
    if (GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL),
                             &modInfo, sizeof(modInfo))) {
      return modInfo.SizeOfImage; // Size in bytes
    }
    return 0;
  }();
  return module_size;
}

} // namespace game