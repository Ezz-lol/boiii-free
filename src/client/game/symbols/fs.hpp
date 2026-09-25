#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace fs {
WEAK symbol<int> fs_loadStack{0x1579E6410, 0x157A65310, 0x14A39C650};

WEAK symbol<char *(size_t bytes)> FS_AllocMem{0x14224FED0, 0x1422AC9F0,
                                              0x14056C340};
WEAK symbol<fileHandle_t(const char *filename, const char *dir,
                         const char *osbasepath)>
    FS_FOpenFileWriteToDir{0x142246AB0, 0x1422A35D0, 0x0};
WEAK symbol<fileHandle_t(const char *filename, const char *dir,
                         const char *osbasepath)>
    FS_FOpenFileReadFromDir{0x1422469F0, 0x1422A3510, 0x0};
WEAK symbol<void(PathList list)> FS_FreeFileList{0x14227D890, 0x1422EA960,
                                                 0x140582720};
WEAK symbol<const char *(char *final, size_t finalLen, const char *a,
                         const char *b)>
    FS_JoinPath{0x142247030, 0x1422A3B50, 0x1405641D0};

WEAK symbol<FILE *(const char *FileName, const char *Mode, int32_t ShFlag)>
    fsopen{0x142BC34E4, 0x142C3C654, 0x140AB5B44};
WEAK
    symbol<FILE *(const wchar_t *FileName, const wchar_t *Mode, int32_t ShFlag)>
        wfsopen{0x142BD1DCC, 0x142C4B628, 0x140AC7808};
} // namespace fs
} // namespace game
