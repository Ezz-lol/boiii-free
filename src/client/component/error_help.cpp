#include <std_include.hpp>

#include "error_help.hpp"
#include "getinfo.hpp"
#include "scheduler.hpp"
#include "script.hpp"

#include <game/game.hpp>
#include <loader/component_loader.hpp>

#include <utils/hook.hpp>
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

std::string unprefixed(const char *fmt) {
  return fmt[0] == '\x15' ? fmt + 1 : fmt;
}

template <typename... Args>
void Com_Error_Explained(const char *file, const int32_t line,
                         const game::errorParm code, const std::string &help,
                         const char *fmt, const Args... args) {
  game::com::Com_Error_(file, line, code, "\x15%s\n\n^7Error: %s", help.c_str(),
                        utils::string::va(unprefixed(fmt).c_str(), args...));
}

template <typename... Args>
void Sys_Error_Explained(const std::string &help, const char *fmt,
                         const Args... args) {
  game::sys::Sys_Error("%s\n\nError: %s", help.c_str(),
                       utils::string::va(fmt, args...));
}

std::string explain_default_asset(const char *asset, const char *type) {
  return std::format(
      "^1The game is missing a file it needs: '{}' ({}).\n^3Fix: ^7If you are "
      "playing a Workshop map or mod, re-download or update it. Otherwise: {}",
      asset, std::string_view(asset).ends_with(".lua") ? "a UI script" : type,
      VERIFY_FILES);
}

constexpr const char *LUI_OUT_OF_MEMORY_HELP =
    "^1The menu/HUD system (LUI) ran out of memory.\n^3Fix: ^7Restart the "
    "game. Mods with custom UI scripts are the most common cause, so try "
    "without them. If it happens without mods, please report it.";

void Com_Error_ZoneNotFound(const char *file, const int32_t line,
                            const game::errorParm code, const char *fmt,
                            const char *zone) {
  Com_Error_Explained(file, line, code, explain_zone(zone), fmt, zone);
}

void Com_Error_MissingConfig(const char *file, const int32_t line,
                             const game::errorParm code, const char *fmt,
                             const char *config) {
  Com_Error_Explained(
      file, line, code,
      std::format("^1A required game file is missing or damaged (config file "
                  "'{}' was not found).\n^3Fix: ^7{}",
                  config, VERIFY_FILES),
      fmt, config);
}

void Com_Error_DirectX(const char *file, const int32_t line,
                       const game::errorParm code, const char *fmt,
                       const char *error, const uint32_t result) {
  if (!std::string_view(error).starts_with("DXGI_ERROR_DEVICE_")) {
    game::com::Com_Error_(file, line, code, fmt, error, result);
    return;
  }
  Com_Error_Explained(
      file, line, code,
      std::format("^1Your graphics card stopped responding (DirectX reported "
                  "{}). This is a graphics driver crash or timeout, not a "
                  "problem with the game files.\n^3Fix: ^7{}",
                  error, GPU_FIX),
      fmt, error, result);
}

void Com_Error_LuiOutOfMemory(const char *file, const int32_t line,
                              const game::errorParm code, const char *fmt) {
  Com_Error_Explained(file, line, code, LUI_OUT_OF_MEMORY_HELP, fmt);
}

void Sys_Error_LuiOutOfMemory(const char *fmt) {
  Sys_Error_Explained(strip_colors(LUI_OUT_OF_MEMORY_HELP), fmt);
}

void Com_Error_AssetLimit(const char *file, const int32_t line,
                          const game::errorParm code, const char *fmt,
                          const int32_t limit, const char *type) {
  Com_Error_Explained(
      file, line, code,
      std::format(
          "^1The map or mods loaded more '{}' assets than the game allows "
          "(limit: {}){}.\n^3Fix: ^7Disable some mods, or ask the map/mod "
          "author to reduce the number of {} assets.",
          type, limit,
          std::string_view(type) == "scriptparsetree"
              ? " - these are GSC/CSC script files"
              : "",
          type),
      fmt, limit, type);
}

void Com_Error_DefaultAsset(const char *file, const int32_t line,
                            const game::errorParm code, const char *fmt,
                            const char *default_asset, const char *type,
                            const char *asset) {
  Com_Error_Explained(file, line, code, explain_default_asset(asset, type), fmt,
                      default_asset, type, asset);
}

void Sys_Error_DefaultAsset(const char *fmt, const char *type,
                            const char *asset) {
  Sys_Error_Explained(strip_colors(explain_default_asset(asset, type)), fmt,
                      type, asset);
}

void Com_Error_GameSettings(const char *file, const int32_t line,
                            const game::errorParm code, const char *fmt) {
  Com_Error_Explained(
      file, line, code,
      "^1The game settings could not be received from the server while "
      "connecting. This is a connection or server problem, not a problem "
      "with your game files.\n^3Fix: ^7Try connecting again or join a "
      "different server.",
      fmt);
}

constexpr char HELP_CONFIGSTRING_OVERFLOW[] =
    "^1The map or mods use more unique models, effects, sounds or text than "
    "the game can track.\n^3Fix: ^7This is a limit in the map or mod itself. "
    "Use fewer mods together, or report it to the map/mod author.";
constexpr char HELP_NO_FREE_ENTITIES[] =
    "^1The game ran out of entities (too many objects exist at once, such as "
    "zombies, pickups or effects).\n^3Fix: ^7This is a limit in the map or "
    "mod. Restart the match, and report it to the map/mod author if it keeps "
    "happening.";
constexpr char HELP_ITEM_LIMIT[] =
    "^1The map or mods loaded more items of one type than the game "
    "allows.\n^3Fix: ^7Disable some mods, or report it to the map/mod author.";
constexpr char HELP_SCRIPT_STACK_OVERFLOW[] =
    "^1A script called itself too many times (script stack overflow), usually "
    "endless recursion in a map or mod script.\n^3Fix: ^7Report it to the "
    "map/mod author, or play without that mod.";
constexpr char HELP_SCRIPT_STRINGS[] =
    "^1Scripts created more text values than the game can store.\n^3Fix: "
    "^7This is a problem in a map or mod script. Report it to the map/mod "
    "author, or play without that mod.";
constexpr char HELP_MISSING_SCRIPT_FUNCTION[] =
    "^1A script calls a function that does not exist. The map or mod scripts "
    "are incomplete, or a mod does not match the map.\n^3Fix: ^7Update or "
    "re-download the map and mods, and only combine mods made to work "
    "together.";
constexpr char HELP_GAME_MISMATCH[] =
    "^1Your game and the server are not running the same mod or "
    "version.\n^3Fix: ^7Load the same mod as the server (or none), and make "
    "sure both are up to date.";
constexpr char HELP_CONFIGSTRING_MISMATCH[] =
    "^1Your game and the server disagree about the match data (models, "
    "effects or settings).\n^3Fix: ^7Your game and the server are probably "
    "not on the same BOIII version or mod files. Update BOIII, load the same "
    "mod as the server, or join another server.";
constexpr char HELP_RELIABLE_CYCLED_OUT[] =
    "^1The server sent updates faster than your game could confirm them, so "
    "the connection fell out of sync.\n^3Fix: ^7This is usually a lag spike or "
    "a server script/mod sending too many updates at once. Join again. If it "
    "keeps happening on this server, the server or its mods are the cause.";
constexpr char HELP_MISSING_MAP[] =
    "^1You do not have the map this server is running.\n^3Fix: ^7Download it "
    "from the Steam Workshop or the launcher's Workshop tab, then join again.";
constexpr char HELP_MISSING_MOD[] =
    "^1This server uses a mod you do not have installed.\n^3Fix: ^7Download it "
    "from the Steam Workshop or the launcher's Workshop tab, then join again.";
constexpr char HELP_ZONE_LIMIT[] =
    "^1Too many map and mod files are loaded at once.\n^3Fix: ^7Unload the "
    "current mod or use fewer mods together.";
constexpr char HELP_INVALID_FILE[] =
    "^1A game file is missing or damaged.\n^3Fix: ^7If it belongs to a "
    "Workshop map or mod, re-download it. Otherwise: Verify the game files in "
    "Steam (Black Ops III > Properties > Installed Files > Verify integrity of "
    "game files).";
constexpr char HELP_DAMAGED_FASTFILE[] =
    "^1A game data file (fast file) is damaged or incomplete.\n^3Fix: ^7If it "
    "belongs to a Workshop map or mod, re-download it. Otherwise: Verify the "
    "game files in Steam (Black Ops III > Properties > Installed Files > "
    "Verify integrity of game files).";
constexpr char HELP_OUT_OF_MEMORY[] =
    "^1The game ran out of memory while loading the map.\n^3Fix: ^7Close other "
    "programs and restart the game. Very large custom maps can need more "
    "memory than the game is able to use.";
constexpr char HELP_LOCAL_ENTITY_LIMIT[] =
    "^1The map uses more client-side entities than the game allows.\n^3Fix: "
    "^7This is a limit in the map itself. Report it to the map author.";

template <const char *Help, typename... Args>
void Com_Error_WithHelp(const char *file, const int32_t line,
                        const game::errorParm code, const char *fmt,
                        const Args... args) {
  Com_Error_Explained(file, line, code, Help, fmt, args...);
}

template <const char *Help, typename... Args>
void Sys_Error_WithHelp(const char *fmt, const Args... args) {
  Sys_Error_Explained(strip_colors(Help), fmt, args...);
}

template <typename Stub>
void hook_calls(const std::initializer_list<uintptr_t> client_sites,
                const std::initializer_list<uintptr_t> server_sites,
                Stub *stub) {
  for (const uintptr_t site : game::is_server() ? server_sites : client_sites) {
    utils::hook::call(game::relocate(site), stub);
  }
}

std::string explain_server_timeout() {
  if (!getinfo::is_host()) {
    return "^1The server stopped sending updates to your game.\n^3Fix: ^7The "
           "server froze or crashed, or your connection dropped. Check your "
           "internet connection and join again, or try another server.";
  }
  using namespace game::scr;
  if (vm::gScrVmPub->instance[SCRIPTINSTANCE_SERVER].function_count > 0) {
    for (const script::script_frame &frame :
         script::get_script_frames(SCRIPTINSTANCE_SERVER)) {
      if (!frame.file.empty()) {
        return std::format(
            "^1Your game's server froze while running a script, so it "
            "stopped sending updates.\n^3Script: ^7{}:{}\n^3Source: ^7{}\n"
            "^3Fix: ^7This script runs without waiting, usually a loop "
            "without a wait. Report it to the map/mod author with this "
            "location, or play without that mod.",
            frame.file, frame.line, frame.source);
      }
    }
  }
  return "^1Your game's server stopped responding, so it stopped sending "
         "updates.\n^3Fix: ^7This is usually a map or mod script doing too "
         "much at once. Try without mods, or start the game with -scr-debug "
         "to find the script that does not wait.";
}

void Com_Error_ServerTimeout(const char *file, const int32_t line,
                             const game::errorParm code, const char *fmt) {
  Com_Error_Explained(file, line, code, explain_server_timeout(), fmt);
}

void Com_Error_MissingBsp(const char *file, const int32_t line,
                          const game::errorParm code, const char *fmt,
                          const char *map) {
  if (std::string_view(map).find("core_frontend") == std::string_view::npos) {
    const std::string name = std::filesystem::path(map).stem().string();
    Com_Error_Explained(
        file, line, code,
        std::filesystem::exists(std::format("zone/{}.ff", name))
            ? std::format("^1The server changed to '{}' while you were "
                          "joining, so your game loaded the wrong map.\n^3Fix: "
                          "^7Join the server again.",
                          name)
            : std::format("^1You don't have the map '{}' installed.\n^3Fix: "
                          "^7Install the DLC or Workshop map it comes from, or "
                          "join another server.",
                          name),
        fmt, map);
    return;
  }

  printf("[Com][Error] %s\n", utils::string::va(unprefixed(fmt).c_str(), map));
  scheduler::once(
      [] {
        game::ui::UI_OpenErrorPopupWithMessage(
            game::LOCAL_CLIENT_0, game::errorCode::NONE,
            "Missing map BSP detected.\nYou are probably in the main menu or "
            "not currently playing a map.");
      },
      scheduler::pipeline::main, 500ms);
}
} // namespace

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

struct component final : generic_component {
DEFINE_COMPONENT_NAME(error_help);

  void post_unpack() override {
    if (game::is_legacy_client()) {
      return;
    }

    if (game::is_client()) {
      utils::hook::set<uint8_t>(0x1420EBA93_g, 0xEB);
      utils::hook::jump(0x14135CAB9_g, Com_Error_ServerTimeout);
      utils::hook::jump(0x142245108_g,
                        static_cast<void (*)(const char *, int32_t,
                                             game::errorParm, const char *)>(
                            Com_Error_WithHelp<HELP_CONFIGSTRING_MISMATCH>));
      hook_calls({0x14131FCBD}, {},
                 Com_Error_WithHelp<HELP_RELIABLE_CYCLED_OUT>);
    }

    hook_calls({0x1414251A9}, {0x1401DA2BE}, Com_Error_ZoneNotFound);
    hook_calls({0x141C46B19}, {0x140338B8E}, Com_Error_DirectX);
    hook_calls({0x14141F84B}, {0x1401D48E0}, Com_Error_AssetLimit);
    hook_calls({0x14141FF0A}, {0x1401D4FC2}, Com_Error_DefaultAsset);
    hook_calls({0x141425CA3}, {0x1401DAE9C}, Sys_Error_DefaultAsset);
    hook_calls({0x1413653DD}, {0x140195DD7}, Com_Error_GameSettings);
    hook_calls({0x1420E08C5}, {}, Com_Error_MissingConfig);
    hook_calls({0x14200C7B3}, {}, Com_Error_LuiOutOfMemory);
    hook_calls({0x14200C7D2}, {}, Sys_Error_LuiOutOfMemory);
    hook_calls({0x14141FEC2}, {}, Com_Error_MissingBsp);
    hook_calls(
        {0x141B68E31, 0x141B755A3, 0x141B75A53, 0x141B75C53},
        {0x140305CDE, 0x140308AB6, 0x140308F76, 0x140309176},
        Com_Error_WithHelp<HELP_CONFIGSTRING_OVERFLOW, int32_t, const char *>);
    hook_calls({0x141B750D2}, {0x1403085F7},
               Com_Error_WithHelp<HELP_NO_FREE_ENTITIES>);
    hook_calls({0x141B752B5}, {0x1403087D8},
               Com_Error_WithHelp<HELP_NO_FREE_ENTITIES>);
    hook_calls({0x141062C74}, {0x14012422A},
               Com_Error_WithHelp<HELP_NO_FREE_ENTITIES>);
    hook_calls({0x1420F561B, 0x1420F5C26}, {0x1405087A1, 0x140508D7B},
               Com_Error_WithHelp<HELP_NO_FREE_ENTITIES>);
    hook_calls({0x1400A482C, 0x1400A493C, 0x1400A4A5A, 0x1400A4B5C, 0x1400A4C6C,
                0x1400A4D7C, 0x1400A4E8C, 0x1400A4F9C, 0x1400A50AC, 0x1400A6419,
                0x1400A6569, 0x1400A66B9, 0x1400A6809, 0x1400A6959, 0x1400A6AA9,
                0x1400A6BF9, 0x1400A6D49, 0x1400A6F43},
               {0x1400431EF, 0x1400432F5, 0x14004377E, 0x140043988},
               Com_Error_WithHelp<HELP_ITEM_LIMIT, const char *, int32_t,
                                  const char *>);
    hook_calls({0x1412E953F, 0x1412E9691, 0x1412E9711, 0x1412E97C2, 0x1412E9842,
                0x1412E98D1, 0x1412E9A12, 0x1412E9A92, 0x1412E9D82, 0x1412E9DFA,
                0x1412E9E6C, 0x1412E9FE4, 0x1412EA07C, 0x1412EC8A4},
               {0x14016ED9F, 0x14016EEF1, 0x14016F092, 0x14016F112, 0x14016F1A1,
                0x14016F2E2, 0x14016F362, 0x14016F652, 0x14016F6CA, 0x14016F73C,
                0x14016F8B4, 0x14016F94C, 0x140172254},
               Sys_Error_WithHelp<HELP_SCRIPT_STACK_OVERFLOW>);
    hook_calls({0x1412D8A82, 0x1412D8B7A}, {0x140164EDE, 0x140164FDC},
               Com_Error_WithHelp<HELP_SCRIPT_STRINGS>);
    hook_calls(
        {0x140093DF6, 0x141AA144B, 0x141AA1563, 0x141AA15E4, 0x141AA1665,
         0x141AA16E6, 0x141AA1767, 0x141AA17E8, 0x141AA1869, 0x141AA18EA,
         0x141AA196B, 0x141AA19EC, 0x141AA1A6D, 0x141AA1AEE, 0x141AA1B6F,
         0x141AA1BF0, 0x141AA1C71, 0x141AA1CF2, 0x141AA1D73, 0x141AA1DF4,
         0x141AA1EBD, 0x141AA1F3E, 0x141AA2513, 0x141AA25EF, 0x141AA2683,
         0x141AA2717, 0x141AA27AB, 0x141AA285B, 0x141AA28F6, 0x141AA2993,
         0x141AA2A27, 0x141AA2ABB, 0x141AA2B4F, 0x141AA2BE3, 0x141AA2C77,
         0x141AA2D0B, 0x141AA2D9F, 0x141AA2E33, 0x141AA2EC7, 0x141AA2F5B,
         0x141AA359E},
        {0x140039219, 0x1402D710E, 0x1402D7226, 0x1402D72AA, 0x1402D732E,
         0x1402D73B2, 0x1402D7436, 0x1402D74BA, 0x1402D753E, 0x1402D75C2,
         0x1402D7646, 0x1402D76CA, 0x1402D774E, 0x1402D77D2, 0x1402D7856,
         0x1402D78DA, 0x1402D795E, 0x1402D79E2, 0x1402D7A66, 0x1402D7AEA,
         0x1402D7BB6, 0x1402D81A6, 0x1402D8292, 0x1402D8329, 0x1402D83C0,
         0x1402D8457, 0x1402D850A, 0x1402D85A8, 0x1402D8648, 0x1402D86DF,
         0x1402D8776, 0x1402D880D, 0x1402D88A4, 0x1402D893B, 0x1402D89D2,
         0x1402D8A69, 0x1402D8B00, 0x1402D8B97, 0x1402D8C2E, 0x1402D9261},
        Com_Error_WithHelp<HELP_MISSING_SCRIPT_FUNCTION, const char *,
                           const char *>);
    hook_calls({0x1416466F2}, {0x14026CCA7},
               Com_Error_WithHelp<HELP_MISSING_SCRIPT_FUNCTION, const char *,
                                  const char *>);
    hook_calls(
        {0x1408F2BF4}, {0x1400E2AD9},
        Com_Error_WithHelp<HELP_GAME_MISMATCH, const char *, const char *>);
    hook_calls({0x14135A241, 0x1421E9AA1},
               {0x14017D7B4, 0x14018F3BA, 0x14052BCF6},
               Com_Error_WithHelp<HELP_MISSING_MAP>);
    hook_calls({0x14135CDC1}, {0x140191907},
               Com_Error_WithHelp<HELP_MISSING_MOD>);
    hook_calls({0x141423AA5}, {0x1401D8B2E},
               Com_Error_WithHelp<HELP_ZONE_LIMIT>);
    hook_calls({0x1425F2F25, 0x1425F3049, 0x1425F3455, 0x1425F357E, 0x1425F3BFA,
                0x1426705E7},
               {0x1406EAFFC},
               Com_Error_WithHelp<HELP_INVALID_FILE, const char *>);
    hook_calls(
        {0x1420F2DF7}, {0x140507AE7},
        Sys_Error_WithHelp<HELP_DAMAGED_FASTFILE, const char *, const char *>);
    hook_calls({0x1413F0009, 0x1413F05C5}, {0x1401A5340, 0x1401A58D7},
               Com_Error_WithHelp<HELP_DAMAGED_FASTFILE, const char *>);
    hook_calls({0x14224F266}, {},
               Com_Error_WithHelp<HELP_OUT_OF_MEMORY, int64_t, int64_t, int64_t,
                                  int64_t>);
    hook_calls({0x14224F400}, {},
               Com_Error_WithHelp<HELP_OUT_OF_MEMORY, int64_t, int64_t, int64_t,
                                  int64_t>);
    hook_calls(
        {0x1408F321B}, {0x1400E3185},
        Com_Error_WithHelp<HELP_LOCAL_ENTITY_LIMIT, int32_t, int32_t, int32_t>);
  }
};
} // namespace error_help

REGISTER_COMPONENT(error_help::component)
