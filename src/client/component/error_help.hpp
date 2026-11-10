#pragma once

namespace error_help {
std::string explain_module(const std::filesystem::path &module_path);
std::string strip_colors(const std::string &text);

void Com_Error_LuiOutOfMemory(const char *file, int32_t line,
                              game::errorParm code, const char *fmt);
void Sys_Error_LuiOutOfMemory(const char *fmt);
} // namespace error_help
