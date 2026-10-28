#include <std_include.hpp>

#include "game.hpp"
#include "log.hpp"

#include <utils/finally.hpp>
#include <utils/flags.hpp>
#include <utils/io.hpp>
#include <utils/properties.hpp>

#include <combaseapi.h>

namespace game {
void show_error(const std::string &text, const std::string &title) {
  if (quiet_crash()) {
    fflush(stdout);
    fflush(stderr);

    fprintf(stderr, "%s\n%s\n", title.c_str(), text.c_str());

    fflush(stderr);

    trace("[Error]{}\n{}\n", title, text);
  } else if (is_headless()) {
    puts(text.data());
  } else {
    MessageBoxA(nullptr, text.data(), title.data(),
                MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
  }
}

const std::filesystem::path &get_appdata_path() {
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

bool is_beta() {
  static const bool beta = [] {
    const bool dedicated =
        utils::flags::has_flag("dedicated") ||
        (!utils::io::file_exists("BlackOps3.exe") &&
         utils::io::file_exists("BlackOps3_UnrankedDedicatedServer.exe"));
    if (utils::flags::has_flag("beta") && dedicated) {
      return true;
    }
    const std::optional<std::string> selected =
        utils::properties::load("selectedVersion");
    return selected && *selected == "beta";
  }();
  return beta;
}

const std::filesystem::path &get_data_path() {
  static const std::filesystem::path data_path =
      get_appdata_path() / (is_beta() ? "beta_data" : "data");
  return data_path;
}

const std::filesystem::path &get_game_path() {
  static const std::filesystem::path cwd = std::filesystem::current_path();
  return cwd;
}
} // namespace game
