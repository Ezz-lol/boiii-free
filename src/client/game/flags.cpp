#include <std_include.hpp>
#include "flags.hpp"

namespace game {

static std::once_flag extract_flag;
static bool extract;

void set_extract_assets() {
  extract = utils::flags::has_flag("extract-assets");
}

bool extract_assets() {
  std::call_once(extract_flag, set_extract_assets);
  return extract;
}

static std::once_flag pattern_flag;
static std::regex pattern;

void set_extract_pattern() {
  pattern = std::regex(
      utils::flags::get<std::string>("extract-assets").value_or("^.*$"));
}

std::regex extract_pattern() {
  std::call_once(pattern_flag, set_extract_pattern);
  return pattern;
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
std::filesystem::path tracing_logfile() {
  std::call_once(tracing_flag, set_tracing);
  return tracing;
}
#endif

static bool ultrawide_enabled;
static std::once_flag ultrawide_flag;
void set_ultrawide() {
  ultrawide_enabled = utils::flags::has_flag("ultrawide");
}

bool ultrawide() {
  std::call_once(ultrawide_flag, set_ultrawide);
  return ultrawide_enabled;
}

static bool cheats_enabled;
static std::once_flag cheats_flag;
void set_cheats() { cheats_enabled = utils::flags::has_flag("cheats"); }

bool cheats() {
  std::call_once(cheats_flag, set_cheats);
  return cheats_enabled;
}

static bool loadlib_disabled;
static std::once_flag loadlib_disabled_flag;
void set_loadlib_disabled() {
  loadlib_disabled = utils::flags::has_flag("disable-loadlib");
}

bool disable_loadlib() {
  std::call_once(loadlib_disabled_flag, set_loadlib_disabled);
  return loadlib_disabled;
}
} // namespace game