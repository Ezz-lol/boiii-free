#pragma once

namespace error_help {
std::string explain_module(const std::filesystem::path &module_path);
std::string strip_colors(const std::string &text);
} // namespace error_help
