#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace playlist {
WEAK symbol<void(const char *buffer)> Playlist_ParsePlaylists{0x1422372C0, 0x0,
                                                              0x14055CC40};
WEAK symbol<int32_t()> Playlist_GetVersionNumber{0x142236FB0, 0x0, 0x0};
} // namespace playlist
} // namespace game
