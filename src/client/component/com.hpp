#pragma once

#include <functional>
namespace com {
using LoadTask = std::function<void(const char *level)>;

void on_level_load(const LoadTask &callback);

void on_level_unload(const LoadTask &callback);
} // namespace com