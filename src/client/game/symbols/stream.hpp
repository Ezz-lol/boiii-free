#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace stream {
WEAK symbol<int32_t(stream_fh *fh)> Stream_OpenInternal{0x14217BD34,
                                                        0x14051E4E0};
WEAK symbol<stream_fileid(const char *path, int32_t flags)> Stream_OpenFile{
    0x14217AF20, 0x14051D6A0};
WEAK symbol<void(stream_fileid fh)> Stream_CloseFile{0x142179810, 0x14051C020};
} // namespace stream
} // namespace game

#endif