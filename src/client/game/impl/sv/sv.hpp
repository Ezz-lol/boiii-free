#pragma once

#include <cstdint>
namespace game {
namespace sv {
void SV_SetConfigString_Impl(const int32_t index, const char *val);
int32_t SV_GetUniqueClientMachineCount_Impl();
} // namespace sv
} // namespace game
