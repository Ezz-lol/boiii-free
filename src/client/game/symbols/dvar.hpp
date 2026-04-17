#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "macros.hpp"

namespace game {

WEAK symbol<dvar_t *> com_maxclients{0x0, 0x14948EE70};

WEAK symbol<char> s_dvarPool{0x157AC6220, 0x14A3CB620};
WEAK symbol<bool> s_canSetConfigDvars{0x0, 0x14A3CB5D8};

WEAK symbol<int> g_dvarCount{0x157AC61CC, 0x14A3CB5FC};

// Used for e.g. changing in-game client FOV when `cg_fov` is changed,
// queueing mod load when `fs_game` is changed.
WEAK symbol<void(const dvar_t *dvar, modifiedCallback callback)>
    Dvar_SetModifiedCallback{0x1422C8650, 0x140579990};
WEAK symbol<void(const dvar_t *dvar, int flags)> Dvar_AddFlags{0x1422B8260,
                                                               0x140574880};
WEAK symbol<bool(const dvar_t *dvar)> Dvar_IsSessionModeBaseDvar{0x1422C23A0,
                                                                 0x140576890};
WEAK symbol<dvar_t *(const char *dvarName)> Dvar_FindVar{0x1422BCCD0,
                                                         0x140575540};
WEAK symbol<unsigned int(const char *str)> Dvar_GenerateHash{0x14133DBF0,
                                                             0x140185800};
WEAK symbol<dvar_t *(unsigned int hash)> Dvar_FindMalleableVar{0x1422BCC40,
                                                               0x1405754B0};
WEAK symbol<const char *(const dvar_t *dvar)> Dvar_GetDebugName{0x1422BD250};
WEAK symbol<const char *(const dvar_t *dvar)> Dvar_DisplayableValue{
    0x1422BC080};
WEAK symbol<dvar_t *(dvarStrHash_t hash, const char *dvarName, bool value,
                     int flags, const char *description)>
    Dvar_RegisterBool{0x1422D0900, 0x14057B500};
WEAK symbol<dvar_t *(dvarStrHash_t hash, const char *dvarName, int value,
                     int min, int max, unsigned int flags,
                     const char *description)>
    Dvar_RegisterInt{0x1422D0AE0, 0x14057B7B0};
WEAK symbol<dvar_t *(dvarStrHash_t hash, const char *dvarName, float value,
                     float min, float max, unsigned int flags,
                     const char *description)>
    Dvar_RegisterFloat{0x1422D0A60, 0x14057B6B0};
WEAK symbol<dvar_t *(dvarStrHash_t hash, const char *dvarName, bool value,
                     int flags, const char *description)>
    Dvar_SessionModeRegisterBool{0x1422D0D40, 0x14057BAA0};
WEAK symbol<void(dvarStrHash_t hash, bool value, eModes mode)>
    Dvar_SessionModeSetDefaultBool{0x1422D0E90, 0x14057BCE0};
WEAK symbol<dvar_t *(dvarStrHash_t hash, const char *dvarName,
                     const char *value, int flags, const char *description)>
    Dvar_RegisterString{0x1422D0B70, 0x14057B890};
WEAK symbol<void(void (*callback)(const dvar_t *, void *), void *userData)>
    Dvar_ForEach{0x1422BCD00};
WEAK
    symbol<void(const char *dvarName, const char *string, bool createIfMissing)>
        Dvar_SetFromStringByName{0x1422C7500, 0x140579290};
WEAK symbol<dvar_t *(dvar_t *dvar, eModes mode)>
    Dvar_GetSessionModeSpecificDvar{0x1422BF500, 0x140575D90};

WEAK symbol<bool()> Dvar_GetBool{0x1422BCED0, 0x140575740};
WEAK symbol<int32_t(const dvar_t *dvar)> Dvar_GetDomainEnumStringCount{
    0x1422BD340, 0x140575900};
WEAK symbol<float(const dvar_t *dvar)> Dvar_GetDomainFloatMax{0x1422BD3C0,
                                                              0x140575940};
WEAK symbol<float(const dvar_t *dvar)> Dvar_GetDomainFloatMin{0x1422BD400,
                                                              0x140575980};
WEAK symbol<int64_t(const dvar_t *dvar)> Dvar_GetDomainInt64Max{0x1422BD440,
                                                                0x140575A90};
WEAK symbol<int64_t(const dvar_t *dvar)> Dvar_GetDomainInt64Min{0x1422BD480,
                                                                0x1405759C0};
WEAK symbol<int32_t(const dvar_t *dvar)> Dvar_GetDomainIntMax{0x1422BD4C0,
                                                              0x140575A00};
WEAK symbol<int32_t(const dvar_t *dvar)> Dvar_GetDomainIntMin{0x1422BD510,
                                                              0x140575A50};

WEAK symbol<float(const dvar_t *dvar)> Dvar_GetFloat{0x1422BD620, 0x140575B20};
WEAK symbol<int64_t(const dvar_t *dvar)> Dvar_GetInt64{0x1422BDC50,
                                                       0x140575B90};
WEAK symbol<int32_t(const dvar_t *dvar)> Dvar_GetInt{0x1422BE860, 0x140575C20};
WEAK symbol<dvarType_t(const dvar_t *dvar)> Dvar_GetType{0x1422BF5E0,
                                                         0x140575E90};
WEAK symbol<uint64_t(const dvar_t *dvar)> Dvar_GetUInt64{0x1422BF620,
                                                         0x140575ED0};
WEAK symbol<const char *(const dvar_t *dvar)> Dvar_GetString{0x1422BF590,
                                                             0x140575E30};

WEAK symbol<void(const dvar_t *dvar, bool val, DvarSetSource source)>
    Dvar_SetBoolFromSource{0x1422C6290, 0x140578BA0};
WEAK symbol<void(const dvar_t *dvar, float val, DvarSetSource source)>
    Dvar_SetFloatFromSource{0x1422C6DD0, 0x140579190};
WEAK symbol<void(const dvar_t *dvar, int64_t val, DvarSetSource source)>
    Dvar_SetInt64FromSource{0x1422C7B70, 0x140579660};
WEAK symbol<void(const dvar_t *dvar, int32_t val, DvarSetSource source)>
    Dvar_SetIntFromSource{0x1422C7ED0, 0x140579740};
WEAK symbol<void(const dvar_t *dvar, uint64_t val, DvarSetSource source)>
    Dvar_SetUInt64FromSource{0x1422C8AF0, 0x140579B60};

} // namespace game

#endif