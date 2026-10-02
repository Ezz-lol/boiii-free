#include <std_include.hpp>

#include "error_help.hpp"

#include <game/game.hpp>

#include <utils/string.hpp>

namespace error_help {
namespace {
constexpr const char *VERIFY_FILES =
    "Verify the game files in Steam (Black Ops III > Properties > Installed "
    "Files > Verify integrity of game files).";

constexpr const char *GPU_FIX =
    "Update or clean-reinstall your graphics driver, remove GPU "
    "overclocks/undervolts and close overlays that hook DirectX (MSI "
    "Afterburner/RTSS, Discord, GeForce Experience).";

const std::unordered_set<std::string> LANGUAGE_PREFIXES{
    "en", "fr", "de", "it", "es", "ea", "ru", "pl", "po",
    "br", "pt", "ja", "ko", "tc", "sc", "cz", "ar"};

const std::unordered_map<std::string, std::string> OVERLAYS{
    {"gameoverlayrenderer64.dll", "the Steam overlay"},
    {"discordhook64.dll", "the Discord overlay"},
    {"rtsshooks64.dll", "RivaTuner Statistics Server (MSI Afterburner)"},
    {"graphics-hook64.dll", "OBS game capture"},
    {"nvspcap64.dll", "the GeForce Experience overlay"},
    {"ow-graphics-hook64.dll", "the Overwolf overlay"}};

const std::unordered_set<std::string> PROXY_DLLS{
    "dxgi.dll",    "d3d11.dll", "d3d9.dll",      "dinput8.dll",
    "version.dll", "winmm.dll", "xinput1_3.dll", "dsound.dll"};

std::string capture(const std::string &message, const std::string &pattern) {
  std::smatch match;
  const std::regex expression(pattern);
  return std::regex_search(message, match, expression) ? match[1].str()
                                                       : std::string{};
}

std::string explain_zone(const std::string &zone) {
  const size_t separator = zone.find('_');
  if (separator != std::string::npos &&
      LANGUAGE_PREFIXES.contains(zone.substr(0, separator))) {
    return std::format(
        "^1The game's '{}' language files are missing or damaged "
        "(zone '{}' was not found).\n^3Fix: ^7{} If you changed the "
        "game language, make sure that language is installed.",
        zone.substr(0, separator), zone, VERIFY_FILES);
  }
  return std::format(
      "^1A required game file is missing or damaged (zone '{}' "
      "was not found).\n^3Fix: ^7If it belongs to a Workshop map or "
      "mod, re-download it. Otherwise: {}",
      zone, VERIFY_FILES);
}

std::string gpu_vendor(const std::string &name) {
  if (name.starts_with("nv")) {
    return "NVIDIA ";
  }
  if (name.starts_with("ati") || name.starts_with("amd")) {
    return "AMD ";
  }
  if (name.starts_with("ig")) {
    return "Intel ";
  }
  return {};
}
} // namespace

std::string explain_error(const std::string &message) {
  if (const std::string zone =
          capture(message, R"re(Could not find zone '([^']+)')re");
      !zone.empty()) {
    return explain_zone(zone);
  }

  if (const std::string file =
          capture(message, R"re(Missing config file "([^"]+)")re");
      !file.empty()) {
    return std::format("^1A required game file is missing or damaged (config "
                       "file '{}' was not found).\n^3Fix: ^7{}",
                       file, VERIFY_FILES);
  }

  if (const std::string code =
          capture(message, R"re((DXGI_ERROR_DEVICE_(?:HUNG|REMOVED|RESET)))re");
      !code.empty()) {
    return std::format(
        "^1Your graphics card stopped responding (DirectX "
        "reported {}). This is a graphics driver crash or "
        "timeout, not a problem with the game files.\n^3Fix: ^7{}",
        code, GPU_FIX);
  }

  if (message.find("EXE_LUI_OUT_OF_MEMORY") != std::string::npos) {
    return "^1The menu/HUD system (LUI) ran out of memory.\n^3Fix: ^7Restart "
           "the "
           "game. Mods with custom UI scripts are the most common cause, so "
           "try without them. If it happens without mods, please report it.";
  }

  if (std::smatch limit; std::regex_search(
          message, limit,
          std::regex(R"re(Exceeded limit of (\d+) '([^']+)')re"))) {
    const std::string type = limit[2].str();
    return std::format(
        "^1The map or mods loaded more '{}' assets than the game allows "
        "(limit: "
        "{}){}.\n^3Fix: ^7Disable some mods, or ask the map/mod author to "
        "reduce "
        "the number of {} assets.",
        type, limit[1].str(),
        type == "scriptparsetree" ? " - these are GSC/CSC script files" : "",
        type);
  }

  if (const std::string type = capture(
          message,
          R"re(Could not load default asset '[^']*' for asset type '([^']+)')re");
      !type.empty()) {
    const std::string asset =
        capture(message, R"re(Tried to load asset '([^']+)')re");
    return std::format(
        "^1A required '{}' asset is missing{}.\n^3Fix: ^7If it belongs to a "
        "Workshop "
        "map or mod, re-download or update it. Otherwise: {}",
        asset.ends_with(".lua") ? "rawfile (Lua UI script)" : type,
        asset.empty() ? "" : std::format(": '{}'", asset), VERIFY_FILES);
  }

  if (message.find("EXE_GAMESETTINGS") != std::string::npos ||
      message.find("Error downloading game settings") != std::string::npos) {
    return "^1The game settings could not be received from the server while "
           "connecting. This is a connection or server problem, not a problem "
           "with your game files.\n^3Fix: ^7Try connecting again or join a "
           "different server.";
  }

  return {};
}

std::string strip_colors(const std::string &text) {
  std::string plain;
  plain.reserve(text.size());
  for (size_t i = 0; i < text.size(); ++i) {
    if (text[i] == '^' && i + 1 < text.size() && text[i + 1] >= '0' &&
        text[i + 1] <= '9') {
      ++i;
      continue;
    }
    plain += text[i];
  }
  return plain;
}

std::string explain_module(const std::filesystem::path &module_path) {
  const std::string name =
      utils::string::to_lower(module_path.filename().string());
  const std::string path = utils::string::to_lower(module_path.string());

  if (const auto overlay = OVERLAYS.find(name); overlay != OVERLAYS.end()) {
    return std::format(
        "^1The crash happened inside {} ({}).\n^3Fix: ^7Disable {} "
        "for this game and try again.",
        overlay->second, name, overlay->second);
  }

  if (path.find("\\driverstore\\") != std::string::npos) {
    return std::format("^1The crash happened inside your {}graphics driver "
                       "({}).\n^3Fix: ^7{}",
                       gpu_vendor(name), name, GPU_FIX);
  }

  std::error_code error;
  if (std::filesystem::equivalent(module_path.parent_path(),
                                  game::get_game_path(), error)) {
    if (PROXY_DLLS.contains(name)) {
      return std::format(
          "^1The crash happened inside '{}' in the game folder, which is not "
          "part of the game (for example ReShade or another mod "
          "loader).\n^3Fix: ^7"
          "Remove or update '{}' in the game folder.",
          name, name);
    }
    return {};
  }

  char windows[MAX_PATH]{};
  GetWindowsDirectoryA(windows, MAX_PATH);
  if (path.starts_with(utils::string::to_lower(windows))) {
    if (name == "d3d11.dll" || name == "dxgi.dll" || name == "d3d9.dll") {
      return std::format("^1The crash happened inside DirectX ({}), which is "
                         "usually caused by the graphics driver or an "
                         "overlay.\n^3Fix: ^7{}",
                         name, GPU_FIX);
    }
    return {};
  }

  return std::format(
      "^1The crash happened inside '{}' ({}), which is not part of the game. "
      "It was loaded into the game by other software, such as an overlay, "
      "recorder, monitoring/RGB tool or antivirus.\n^3Fix: ^7Close or "
      "uninstall "
      "that software, or add the game folder to its exclusions.",
      name, module_path.string());
}
} // namespace error_help
