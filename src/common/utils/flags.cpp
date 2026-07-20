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
  program.add_argument("-unsafe-lua", "--unsafe-lua")
      .help("Allow mods to use unsafe Lua functions (required for some mods "
            "like All-Around Enhancement)")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-dedicated", "--dedicated")
      .help("Launch as a dedicated server")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-nosteam", "--nosteam")
      .help("Bypass Steam entirely")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-nointro", "--nointro")
      .help("Skip intro videos")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-windowed", "--windowed")
      .help("Launch in windowed mode")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-safe", "--safe")
      .help("Launch in safe mode (disable mods)")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-console", "--console")
      .help("Enable developer console")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-port", "--port")
      .help("XXXX Set server port (default: 27017)")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-launch", "--launch")
      .help("Start the game immediately, skipping some launcher UI and "
            "pre-checks")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-noupdate", "--noupdate")
      .help("Disable automatic updates (not recommended)")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-update", "--update")
      .help("Force enable updates (including host binary in debug builds)")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-norelaunch", "--norelaunch")
      .help("Skip automatic relaunch after updates")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-headless", "--headless")
      .help("Run in headless mode (no GUI)")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-nopatch", "--nopatch")
      .help("Disable selected runtime patches")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-plugins", "--plugins")
      .help("Load additional plugins from the `plugins/` directory")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-trimlogs", "--trimlogs")
      .help("Trim or rotate old log files on startup")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-keep-launcher", "--keep-launcher")
      .help("Keep the launcher process running after starting the game")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-noconsole", "--noconsole")
      .help("Suppress the external launcher console window")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-nobranding", "--nobranding")
      .help("Disable EZZ watermark and console prefix")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-noratelimit", "--noratelimit")
      .help("Disable rate limiting in dedicated server")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-quiet-crash", "--quiet-crash")
      .help("On crash, disable message box and minidump directory popups.")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-mitigatepacketspam", "--mitigatepacketspam")
      .help("In dedicated server, attempt to reduce unnecessary reliable "
            "command packets sent by some custom maps' scripts. Fixes Kowloon "
            "and Daybreak client load-in failures.")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-newsteamclient", "--newsteamclient")
      .help("For development: patch Arxan checksum comparison patterns for new "
            "steam client.")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-dump", "--dump")
      .help("Dump game or server executable to file in containing directory "
            "after Arxan unpack")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-noplugins", "--noplugins")
      .help("Disable plugin load.")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-nologs", "--nologs")
      .help("Disable all patches to developer console.")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-fulllogs", "--fulllogs")
      .help("Disable console buffer size limit.")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-nosnd", "--nosnd")
      .help("Disable attempt to load and use sound assets in dedicated server.")
      .default_value(false)
      .implicit_value(true);
#ifndef NDEBUG
  program.add_argument("-alias", "--alias")
      .help("For development: use a different key for XUID generation, "
            "allowing two local clients (the first launched without this flag) "
            "to connect to the same server.")
      .default_value(false)
      .implicit_value(true);
#endif
  program.add_argument("-e", "-extract-assets", "--extract-assets")
      .help("Dump assets with name matching the provided pattern when loaded "
            "to output directory specified by the -o/--output flag")
      .default_value(std::string(""))
      .implicit_value(std::string("^.*$"));
  program.add_argument("-o", "-output", "--output")
      .help("Output directory for dumped assets")
      .default_value(std::string("assets"));

  try {
    program.parse_known_args(filtered_args);
  } catch (const std::runtime_error &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  return 0;
}

std::optional<std::string> find_variant(const std::string_view &flag) {
  try {
    if (program.is_used(flag)) {
      return std::string(flag);
    }
  } catch (...) {
  }

  std::string hyphen_prefixed = std::string("-") + flag.data();
  try {
    if (program.is_used(hyphen_prefixed)) {
      return hyphen_prefixed;
    }
  } catch (...) {
  }

  try {
    hyphen_prefixed = "-" + hyphen_prefixed;
    if (program.is_used(hyphen_prefixed)) {
      return hyphen_prefixed;
    }
  } catch (...) {
  }

  return std::nullopt;
}

bool has_flag(const std::string_view &flag) {
  return find_variant(flag).has_value();
}
} // namespace utils::flags