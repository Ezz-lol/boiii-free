#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace fs {
WEAK symbol<int> fs_loadStack{0x157A65310, 0x14A39C650};

WEAK symbol<char *(int bytes)> FS_AllocMem{0x1422AC9F0, 0x14056C340};
WEAK symbol<fileHandle_t(const char *filename, const char *dir,
                         const char *osbasepath)>
    FS_FOpenFileWriteToDir{0x1422A35D0};
WEAK symbol<fileHandle_t(const char *filename, const char *dir,
                         const char *osbasepath)>
    FS_FOpenFileReadFromDir{0x1422A3510};
} // namespace fs
} // namespace game

#endif