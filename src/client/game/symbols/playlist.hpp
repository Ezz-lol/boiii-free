#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace playlist {
WEAK symbol<void(const char *buffer)> Playlist_ParsePlaylists{0x1422372C0, 0x0,
                                                              0x14055CC40};
} // namespace playlist
} // namespace game
