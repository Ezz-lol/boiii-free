#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace com {
namespace gts {

WEAK symbol<ddl::DDLDef *> s_gametypeSettingsDDL{0x1568EADF8, 0x14948B108};
WEAK symbol<ddl::DDLContext> s_gametypeSettingsDDLCtx{0x1568ED578, 0x14948D888};
WEAK symbol<str8_t> s_downloadedGameType{0x1568ED5C8, 0x14948D8D8};

WEAK symbol<void()> Com_GametypeSettings_GametypeSetting_f{0x1420F4A40,
                                                           0x1404FE210};
WEAK symbol<void()> Com_GametypeSettings_Init{0x1420F4ED0, 0x1404FE6E0};
WEAK symbol<void(const char *gametype, bool loadDefaultSettings)>
    Com_GametypeSettings_SetGametype{0x1420F5980, 0x1404FF120};
WEAK symbol<unsigned int(const char *settingName, bool getDefault)>
    Com_GametypeSettings_GetUInt{0x1420F4E00, 0x1404FE5C0};

} // namespace gts
} // namespace com
} // namespace game