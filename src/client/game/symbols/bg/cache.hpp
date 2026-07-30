#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace bg {
namespace cache {
WEAK symbol<const char *(bgCacheInstance inst, int32_t index)>
    BG_Cache_GetModelNameForIndex{0x1400A7C60, 0x140044430};

WEAK symbol<int32_t(bgCacheInstance inst, const char *name)>
    BG_Cache_GetScriptMenuIndex{0x1400A7DA0, 0x140044570};
inline bool BG_Cache_IsCachedScriptMenuIndex(bgCacheInstance inst,
                                             const char *name) {
  return BG_Cache_GetScriptMenuIndex(inst, name) > 0;
}
WEAK symbol<int32_t(bgCacheInstance inst, const char *name)>
    BG_Cache_GetLocStringIndex{0x1400A7A70, 0x140044270};
inline bool BG_Cache_IsCachedLocalizedString(const char *name) {
  return BG_Cache_GetLocStringIndex(bgCacheInstance::SERVER, name) > 0;
}
inline std::optional<int32_t>
BG_Cache_GetLocStringIndex_Optional(bgCacheInstance inst, const char *name) {
  const int32_t idx = BG_Cache_GetLocStringIndex(inst, name);
  return idx > 0 ? std::optional(idx) : std::nullopt;
}
WEAK symbol<int32_t(ConfigStringStart configStringStart, const char *val)>
    BG_Cache_RegisterAndSetConfigString{0x1400AA290, 0x140046800};
WEAK symbol<void(LocalClientNum_t localClientNum, int32_t index)>
    BG_Cache_HandleConfigStringChange{0x1400A81B0, 0x140047090};

WEAK symbol<volatile BGCacheTypePool<bgCacheTypeInfo>> s_bgCacheTypeInfo{
    0x1432D5720, 0x14106B720};
WEAK symbol<volatile bgCachePool> s_bgCache{0x1436841F0, 0x1413862A};
} // namespace cache
} // namespace bg
} // namespace game