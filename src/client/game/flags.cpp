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

} // namespace game