#pragma once

#include <std_include.hpp>

namespace toast {
void patch_hud();

// Thread-safe: schedules onto main thread automatically.
void show(const std::string &title, const std::string &description,
          const std::string &icon = "blacktransparent");
void reward(const std::string &title, const std::string &description,
            const std::string &icon);

// Convenience wrappers with appropriate icons
void success(const std::string &title, const std::string &description);
void error(const std::string &title, const std::string &description);
void info(const std::string &title, const std::string &description);
void warn(const std::string &title, const std::string &description);
// Runs a snippet of UI Lua on the main thread (errors are printed).
void run_lua(const std::string &code);
} // namespace toast
