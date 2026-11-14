#pragma once

#include <structs/func.hpp>
namespace com {
typedef fastcallPtr_t<void(const char *level)> LoadTask;

void on_level_load(const LoadTask &callback);

void on_level_unload(const LoadTask &callback);
} // namespace com
