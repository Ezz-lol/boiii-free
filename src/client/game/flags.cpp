#include <std_include.hpp>

#include "flags.hpp"

namespace game {

bool extract_assets() {
  static const bool result = utils::flags::has_flag("extract-assets");
  return result;
}

std::regex extract_pattern() {
  static const std::regex result = std::regex(
      utils::flags::get<std::string>("extract-assets").value_or("^.*$"));
  return result;
}

static std::filesystem::path output;
static std::once_flag output_flag;

void set_asset_output() {
  const std::optional<std::string> arg =
      utils::flags::get<std::string>("output");
  if (arg.has_value() && !arg.value().empty()) {
    output = std::filesystem::weakly_canonical(arg.value());
  } else {
    output = game_directory() / "assets";
  }
}

std::filesystem::path asset_output() {
  std::call_once(output_flag, set_asset_output);
  return output;
}

#ifndef NDEBUG
static std::filesystem::path tracing;
static std::once_flag tracing_flag;
void set_tracing() {
  const std::optional<std::string> arg =
      utils::flags::get<std::string>("tracing");
  if (arg.has_value() && !arg.value().empty()) {
    tracing = std::filesystem::weakly_canonical(arg.value());
    if (game::is_server()) {
      std::filesystem::path filename = tracing.filename();
      const std::string extension = filename.extension().string();
      filename.replace_extension("");
      tracing =
          tracing.parent_path() / (filename.string() + "-server" + extension);
    }
  } else {
    tracing = game_directory() /
              (game::is_client() ? "debug.log" : "debug-server.log");
  }
}
std::filesystem::path tracing_logfile_path() {
  std::call_once(tracing_flag, set_tracing);
  return tracing;
}

static std::ofstream logfile;
static std::once_flag tracing_logfile_flag;
void set_tracing_logfile() {
  logfile = std::ofstream(tracing_logfile_path(), std::ios::app);
}

std::ofstream &tracing_logfile() {
  std::call_once(tracing_logfile_flag, set_tracing_logfile);
  return logfile;
}
#endif

bool ultrawide() {
  static const bool result = utils::flags::has_flag("ultrawide");
  return result;
}

bool cheats() {
  static const bool result = utils::flags::has_flag("cheats");
  return result;
}

bool disable_loadlib() {
  static const bool result = utils::flags::has_flag("disable-loadlib");
  return result;
}

bool quiet_crash() {
  static const bool quiet_crash = utils::flags::has_flag("quiet-crash");
  return quiet_crash;
}

bool alias() {
  static const bool alias = utils::flags::has_flag("alias");
  return alias;
}

bool is_headless() {
  static const bool headless = utils::flags::has_flag("headless");
  return headless;
}
} // namespace game
