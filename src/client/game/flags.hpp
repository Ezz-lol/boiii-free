#pragma once

#include <std_include.hpp>

#include <game/base.hpp>
#include <utils/flags.hpp>

#include <regex>

namespace game {
bool extract_assets();
std::regex extract_pattern();
std::filesystem::path asset_output();
#ifndef NDEBUG
std::filesystem::path tracing_logfile_path();
std::ofstream &tracing_logfile();
#endif

bool disable_loadlib();

bool alias();
bool quiet_crash();
bool is_headless();
bool scr_debug();

#ifdef NDEBUG
inline bool vm_trace() { return false; }
#else
bool vm_trace();
#endif
} // namespace game
