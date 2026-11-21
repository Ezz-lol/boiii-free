#include <std_include.hpp>

#include "foreign_dlls.hpp"

#include <game/game.hpp>

#include <utils/io.hpp>
#include <utils/string.hpp>

#include <CommCtrl.h>

namespace foreign_dlls {
namespace {
constexpr std::string_view KEPT_LIST = "boiii_players/user/kept_dlls.txt";
constexpr std::string_view DISABLED_FOLDER = "boiii/disabled_dlls";

std::filesystem::path system_folder() {
  wchar_t buffer[MAX_PATH]{};
  const UINT length = GetSystemDirectoryW(buffer, MAX_PATH);
  return length && length < MAX_PATH ? std::filesystem::path(buffer)
                                     : std::filesystem::path{};
}

std::string fingerprint(const std::filesystem::path &file) {
  std::error_code error;
  const uintmax_t size = std::filesystem::file_size(file, error);
  return std::format("{}|{}", utils::string::to_lower(file.filename().string()),
                     error ? 0 : size);
}

std::unordered_set<std::string> read_kept() {
  std::unordered_set<std::string> kept;
  std::string data;
  if (utils::io::read_file(KEPT_LIST, &data)) {
    for (std::string &line : utils::string::split(data, '\n')) {
      std::erase(line, '\r');
      if (!line.empty()) {
        kept.insert(line);
      }
    }
  }
  return kept;
}

void remember_kept(const std::string &entry) {
  utils::io::write_file(KEPT_LIST, entry + "\n", true);
}

bool ask_disable(const std::vector<std::filesystem::path> &files) {
  constexpr int DISABLE_BUTTON = 100;
  constexpr int KEEP_BUTTON = 101;
  const bool single = files.size() == 1;
  const TASKDIALOG_BUTTON buttons[] = {
      {.nButtonID = DISABLE_BUTTON,
       .pszButtonText =
           single ? L"Disable it\nMoves it to boiii\\disabled_dlls, so you can "
                    L"put it back later."
                  : L"Disable them\nMoves them to boiii\\disabled_dlls, so you "
                    L"can put them back later."},
      {.nButtonID = KEEP_BUTTON,
       .pszButtonText =
           single ? L"Keep using it\nYou will not be asked again for this file."
                  : L"Keep using them\nYou will not be asked again for these "
                    L"files."},
  };

  std::wstring names;
  for (const std::filesystem::path &file : files) {
    names += L"\n    " + file.filename().wstring();
  }
  const std::wstring headline =
      single ? L"'" + files.front().filename().wstring() +
                   L"' is not part of the game"
             : std::to_wstring(files.size()) +
                   L" files in the game folder are not part of the game";
  const std::wstring content =
      std::wstring(single ? L"This file in the game folder is"
                          : L"These files in the game folder are") +
      L" not part of BOIII or Black Ops III:" + names +
      L"\n\nWindows loads them into the game instead of its own copies. "
      L"Files like this come from mod loaders, overlays, patches or pirated "
      L"copies of the game, and they can stop the game from starting or make "
      L"it crash.";

  TASKDIALOGCONFIG config{};
  config.cbSize = sizeof(config);
  config.dwFlags = TDF_USE_COMMAND_LINKS | TDF_SIZE_TO_CONTENT;
  config.pszWindowTitle = L"BOIII";
  config.pszMainIcon = TD_WARNING_ICON;
  config.pszMainInstruction = headline.c_str();
  config.pszContent = content.c_str();
  config.pButtons = buttons;
  config.cButtons = static_cast<UINT>(std::size(buttons));
  config.nDefaultButton = DISABLE_BUTTON;

  int pressed = KEEP_BUTTON;
  return SUCCEEDED(TaskDialogIndirect(&config, &pressed, nullptr, nullptr)) &&
         pressed == DISABLE_BUTTON;
}

bool disable(const std::filesystem::path &file) {
  const std::filesystem::path folder = game::get_game_path() / DISABLED_FOLDER;
  std::error_code error;
  std::filesystem::create_directories(folder, error);
  std::filesystem::rename(file, folder / file.filename(), error);
  return !error;
}
} // namespace

void check() {
  if (game::is_headless()) {
    return;
  }
  const std::filesystem::path system = system_folder();
  if (system.empty()) {
    return;
  }

  const std::unordered_set<std::string> kept = read_kept();
  std::vector<std::filesystem::path> found;
  std::error_code error;
  for (const auto &entry :
       std::filesystem::directory_iterator(game::get_game_path(), error)) {
    const std::filesystem::path &file = entry.path();
    if (entry.is_regular_file() &&
        utils::string::to_lower(file.extension().string()) == ".dll" &&
        utils::io::file_exists(system / file.filename()) &&
        !kept.contains(fingerprint(file))) {
      found.emplace_back(file);
    }
  }
  if (found.empty()) {
    return;
  }

  const bool disable_files = ask_disable(found);
  for (const std::filesystem::path &file : found) {
    if (!disable_files || !disable(file)) {
      remember_kept(fingerprint(file));
    }
  }
}
} // namespace foreign_dlls
