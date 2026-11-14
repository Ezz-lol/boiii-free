#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace playlist {
WEAK symbol<void(const char *buffer)> Playlist_ParsePlaylists{0x1422372C0,
                                                              0x14055CC40};
WEAK symbol<int32_t()> Playlist_GetVersionNumber{0x142236FB0, 0x0};
WEAK symbol<bool(ControllerIndex_t controllerIndex, int32_t playlistId)>
    Playlist_RunRules{0x14223A980, 0x140560360};
} // namespace playlist
} // namespace game
