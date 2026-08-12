#pragma once

#include <std_include.hpp>
#include <utils/flags.hpp>
#include <game/base.hpp>

namespace game {
bool extract_assets();
std::regex extract_pattern();
std::filesystem::path asset_output();
#ifndef NDEBUG
std::filesystem::path tracing_logfile();
#endif
} // namespace game