#include <std_include.hpp>
#include <utils/nt.hpp>
#include <utils/flags.hpp>
#include "base.hpp"

namespace game {
namespace {
const utils::nt::library &get_host_library() {
  static const utils::nt::library host_library = [] {
    utils::nt::library host{};
    if (!host || host == utils::nt::library::get_by_address(get_base)) {
      throw std::runtime_error("Invalid host application - Make sure you place "
                               "Boiii.exe next to BlackOps3.exe!");
    }

    return host;
  }();

  return host_library;
}
} // namespace

size_t get_base() {
  static const size_t base =
      reinterpret_cast<size_t>(get_host_library().get_ptr());
  return base;
}

bool is_server() {
  static const bool server =
      get_host_library().get_optional_header()->CheckSum == 0x14C28B4;
  return server;
}

bool is_client() {
  if (utils::flags::has_flag("newsteamclient")) {
    static const bool server =
        get_host_library().get_optional_header()->CheckSum == 0x6517980;
    return server;
  }
  static const bool server =
      get_host_library().get_optional_header()->CheckSum == 0x888C368;
  return server;
}

bool is_legacy_client() {
  static const bool server =
      get_host_library().get_optional_header()->CheckSum == 0x8880704;
  return server;
}

} // namespace game