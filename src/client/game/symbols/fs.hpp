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
WEAK symbol<int64_t(const wchar_t *path)> __mkdir{0x142BE85AC, 0x142C62294,
                                                  0x140ADA114};
WEAK symbol<int32_t(const char *fileName, struct _stat64 *stat)> stat64{
    0x142BD3F94, 0x142C4DCB4, 0x140AC6388};
WEAK symbol<int32_t(const char *fileName, struct _stat64i32 *stat)> stat64i32{
    0x142BD15C4, 0x142C4AE20, 0x140ABD25C};

WEAK symbol<void(const char *path, const char *dir, int32_t bLanguageDirectory,
                 int32_t iLanguage)>
    FS_AddGameDirectory{0x142245DB0, 0x1422A28D0, 0x140562F60};
WEAK symbol<void(const char *path, const char *dir)>
    FS_AddLocalizedGameDirectory{0x142245FD0, 0x1422A2AF0, 0x140563180};
WEAK symbol<void(const char *gameName, bool allow_devraw)> FS_Startup{
    0x1422478E0, 0x1422A4400, 0x140564A70};
WEAK symbol<void(const char *base, const char *game, const char *qpath,
                 char *ospath)>
    FS_BuildOSPath{0x142246220, 0x1422A2D40, 0x1405633D0};
WEAK symbol<void(char *ospath, const char *extension, int pathBufferLength,
                 char *game, ZoneType zoneType, const char *zone_internal_id)>
    FS_BuildOSPathForThread{0x1420C9020, 0x1420D57A0, 0x1404E1950};
} // namespace fs
} // namespace game
