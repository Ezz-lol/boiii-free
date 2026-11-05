#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> counterDownloadInterval{
    0x15118F7C8, 0x15120E748, 0x1482E0B08};
WEAK symbol<EngineDependentDvarMut> counterUploadInterval{
    0x15118F7C0, 0x15120E740, 0x1482E0B00};
} // namespace game
