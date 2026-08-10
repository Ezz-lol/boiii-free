#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace sl {
WEAK symbol<const char *(scr::ScrString_t stringValue)> SL_ConvertToString{
    0x1412D7160, 0x1401632C0};
WEAK symbol<const char *(scr::ScrString_t stringValue)> SL_ConvertToStringSafe{
    0x1412D7180, 0x1401632E0};
WEAK symbol<scr::ScrString_t(const char *str, uint32_t user, int32_t type)>
    SL_GetString{0x1412D7B20, 0x140163D90};
WEAK symbol<scr::ScrString_t(const char *str, uint32_t user, int32_t type)>
    SL_GetLowercaseString{0x1412D7A10, 0x140163C80};
WEAK symbol<void(scr::ScrString_t stringValue, volatile RefString *refStr)>
    SL_RemoveRefToStringFree{0x1412D7E90, 0x140164100};

WEAK symbol<const char *(const char *name)> CopyString{0x1422AC220,
                                                       0x14056BD70};
WEAK symbol<void(const char *str)> FreeString{0x1422AC240, 0x14056BD90};
WEAK symbol<scr::ScrVarCanonicalName_t(const char *name)>
    SL_GenerateCanonicalString{0x1412CB930, 0x140159970};

/*
  No clue what this is. It does not exist in the client, nor in any prior server
  or client engine versions. It is also never instantiated at runtime.

  However, its allocation is explicitly protected from being freed in all
  instances of `SL_RemoveRefToStringFree` usage in dedicated server, so we
  likely need to protect this script string similarly.
*/
WEAK symbol<scr::ScrString_t> sl_protectedScriptString{0x0, 0x142623D04};
} // namespace sl
} // namespace game