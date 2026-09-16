#pragma once

#include <game/base.hpp>
#include <std_include.hpp>
#include <utils/flags.hpp>

namespace game {
bool extract_assets();
std::regex extract_pattern();
std::filesystem::path asset_output();
#ifndef NDEBUG
std::filesystem::path tracing_logfile_path();
std::ofstream &tracing_logfile();
#endif

bool ultrawide();
bool cheats();
bool disable_loadlib();

bool alias();
bool quiet_crash();
bool is_headless();
} // namespace game