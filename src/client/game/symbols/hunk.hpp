#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace hunk {

WEAK symbol<uint64_t(size_t size)> Hunk_AllocXAnimClient{0x140932DC0};
WEAK symbol<uint64_t(int64_t size)> Hunk_AllocAlignInternal_32BAlignment{
    0x1422ABE40};
WEAK symbol<void()> Hunk_ClearData{0x1422ABFE0, 0x14056BB50};
WEAK symbol<void()> Hunk_Clear{0x1422AC660, 0x14056C020};
WEAK symbol<void(int64_t mark)> Hunk_ClearToMark{0x1422AC6C0, 0x14056C070};
WEAK symbol<void *(HunkUser *user, size_t size, int32_t alignment,
                   const char *name)>
    Hunk_UserDefaultAlloc{0x1422D3380, 0x14057D6C0};
WEAK symbol<char *(HunkUser *user, const char *in)> Hunk_CopyString{
    0x1422D3790};
WEAK symbol<void *(HunkUser *user, size_t size, int32_t alignment,
                   const char *name)>
    Hunk_UserAlloc{0x1422D3800, 0x14057DB00};
WEAK symbol<HunkUser *(int32_t size, HU_ALLOCATION_SCHEME scheme,
                       uint32_t flags, void *scheme_specific_data,
                       const char *name, int32_t type)>
    Hunk_UserCreate{0x1422D3820, 0x14057DB20};
WEAK symbol<HunkUser *(void *buffer, size_t size, HU_ALLOCATION_SCHEME scheme,
                       HUNKUSER_FLAGS flags, void *scheme_specific_data,
                       const char *name, int32_t type)>
    Hunk_UserCreateFromBuffer{0x1422D38C0, 0x14057DBC0};
WEAK symbol<void(HunkUser *user)> Hunk_UserDestroy{0x1422D3920, 0x14057DC20};
WEAK symbol<void(HunkUser *user, void *ptr)> Hunk_UserFree{0x1422D3940,
                                                           0x14057DC40};
WEAK symbol<void(HunkUser *user)> Hunk_UserDefaultReset{0x1422D3960,
                                                        0x14057D580};
WEAK symbol<HunkUser *(HunkUserNull *user)> Hunk_UserCreateNull{0x1422D38E0,
                                                                0x14057DBE0};

WEAK symbol<HunkUser *> s_localClientHunk{0x14573B200};

WEAK symbol<HunkUserAllocationSchemeMap> g_HunkUserAllocationSchemeMap{
    0x1434418D0, 0x14114BC40};
} // namespace hunk
} // namespace game
#endif