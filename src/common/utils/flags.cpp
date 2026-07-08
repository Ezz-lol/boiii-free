#include "flags.hpp"
#include <unordered_map>

namespace utils::flags {

static std::unordered_map<std::string_view, int32_t> ARG_SKIPS = {
    {"+set", 2},         {"+connect_lobby", 1}, {"+nosnd", 0},
    {"+showconsole", 0}, {"+7fvi9jt5", 0},      {"+bigdump", 0},
    {"+megadump", 0},    {"+autodump", 0},      {"+minidump", 0},
    {"+nodump", 0},

};
argparse::ArgumentParser program("boiii");

/*
  TODO: this should also filter out commands.

  This will be difficult because we add new commands in boiii and it would be
  best to not need to add each additional command to the skipped args here.

  We also cannot use the list of commands stored in the engine to automatically
  skip all command arguments because we need to check arguments before we've
  initialized the engine.

  If/when commands _are_ filtered out here somehow, we can then use `parse_args`
  instead of `parse_known_args` below, and be able to automatically generate a
  help message if an incorrect argument is passed to boiiii, rather than simply
  ignore the argument.
*/
std::vector<std::string> remove_engine_args(int32_t argc, char *argv[]) {
  std::vector<std::string> result = {};
  for (int32_t i = 0; i < argc; ++i) {
    std::string_view arg = argv[i];

    if (ARG_SKIPS.contains(arg)) {
      i += ARG_SKIPS[arg];
    } else {
      std::string arg_plus_prefixed = "+" + std::string(arg);
      if (ARG_SKIPS.contains(arg_plus_prefixed)) {
        i += ARG_SKIPS[arg_plus_prefixed];
      } else {
        result.push_back(std::string(arg));
      }
    }
  }

  return result;
}

int32_t parse_flags(int argc, char *argv[]) {
  const std::vector<std::string> filtered_args = remove_engine_args(argc, argv);
  program.add_argument("-unsafe-lua")
      .help("Allow mods to use unsafe Lua functions (required for some mods "
            "like All-Around Enhancement)")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-dedicated")
      .help("Launch as a dedicated server")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-nosteam")
      .help("Bypass Steam entirely")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-nointro")
      .help("Skip intro videos")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-windowed")
      .help("Launch in windowed mode")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-safe")
      .help("Launch in safe mode (disable mods)")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-console")
      .help("Enable developer console")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-port")
      .help("XXXX Set server port (default: 27017)")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-launch")
      .help("Start the game immediately, skipping some launcher UI and "
            "pre-checks")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-noupdate")
      .help("Disable automatic updates (not recommended)")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-update")
      .help("Force enable updates (including host binary in debug builds)")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-norelaunch")
      .help("Skip automatic relaunch after updates")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-headless")
      .help("Run in headless mode (no GUI)")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-nopatch")
      .help("Disable selected runtime patches")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-plugins")
      .help("Load additional plugins from the `plugins/` directory")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-trimlogs")
      .help("Trim or rotate old log files on startup")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-keep-launcher")
      .help("Keep the launcher process running after starting the game")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-noconsole")
      .help("Suppress the external launcher console window")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-nobranding")
      .help("Disable EZZ watermark and console prefix")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-noratelimit")
      .help("Disable rate limiting in dedicated server")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-quiet-crash")
      .help("On crash, disable message box and minidump directory popups.")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-mitigatepacketspam")
      .help("In dedicated server, attempt to reduce unnecessary reliable "
            "command packets sent by some custom maps' scripts. Fixes Kowloon "
            "and Daybreak client load-in failures.")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("newsteamclient")
      .help("For development: patch Arxan checksum comparison patterns for new "
            "steam client.")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("-dump")
      .help("Dump game or server executable to file in containing directory "
            "after Arxan unpack")
      .default_value(false)
      .implicit_value(true);

  try {
    program.parse_known_args(filtered_args);
  } catch (const std::runtime_error &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  return 0;
}
bool has_flag(const char *flag) { return program.get<bool>(flag); }
} // namespace utils::flags