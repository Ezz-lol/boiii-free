#include <std_include.hpp>

#include "game.hpp"

#include <utils/flags.hpp>
#include <utils/finally.hpp>

#include <combaseapi.h>

namespace game {
bool quiet_crash() {
  static const bool quiet_crash = utils::flags::has_flag("quiet-crash");
  return quiet_crash;
}

#ifndef NDEBUG
bool alias() {
  static const bool alias = utils::flags::has_flag("alias");
  return alias;
}
#endif

bool is_headless() {
  static const bool headless = utils::flags::has_flag("headless");
  return headless;
}

void show_error(const std::string &text, const std::string &title) {
  if (quiet_crash()) {
    fflush(stdout);
    fflush(stderr);

    fprintf(stderr, "%s\n%s\n", title.c_str(), text.c_str());

    fflush(stderr);
  } else if (is_headless()) {
    puts(text.data());
  } else {
    MessageBoxA(nullptr, text.data(), title.data(),
                MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
  }
}

std::filesystem::path get_appdata_path() {
  static const std::filesystem::path appdata_path =
      []() -> std::filesystem::path {
    PWSTR path = nullptr;
    if (FAILED(
            SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path))) {
      throw std::runtime_error("Failed to read APPDATA path!");
    }

    auto _ = utils::finally([&path] { CoTaskMemFree(path); });

    // Ensure proper handling of wide character paths
    return std::filesystem::path(path) / L"boiii";
  }();

  return appdata_path;
}

std::filesystem::path get_game_path() {
  return std::filesystem::current_path();
}
} // namespace game