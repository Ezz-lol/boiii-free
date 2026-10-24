#pragma once

#include <std_include.hpp>

namespace gpu {
int get_preference();
void set_preference(int preference);
void check_device(ID3D11Device *device);
} // namespace gpu
