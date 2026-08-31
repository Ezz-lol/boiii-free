#pragma once

#include <structs/func.hpp>
namespace com {
using LoadTask = fastcallPtr_t<void(const char *level)>;

void on_level_load(const LoadTask &callback);

void on_level_unload(const LoadTask &callback);
} // namespace com