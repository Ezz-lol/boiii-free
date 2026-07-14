#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

namespace game {

/*
  Note: all of the below function return and parameter types that are `dvar_t*`
  _should_ be `EngineDependentDvarMut` and those that are `const dvar_t`
  _should_ be `EngineDependentDvar`.

  clang's LLVM IR generation backend for x86-64 Windows MSVC refuses to treat
  `EngineDependentDvarMut` as a PoD value, despite meeting all standard PoD
  criteria and having size equal to that of a 64-bit register.

  Even when attempting to force an `EngineDependentDvar...` to be returned and
  passed by value using the `[[clang::trivial_abi]]` vendor extension attribute
  in its definition, the generated IR always returns the value using a return
  struct, and implicitly passes the value using a pointer to the given argument.

  Usage of a return structure is the incorrect calling convention to use with
  the fastcall ABI when returning a value which fits within a 64-bit register or
  smaller - it is not expected by the engine.

  This erroneous usage of return structures almost consistently
  causes an immediate crash, due to the first argument being implicitly set to a
  pointer to an uninitialized `EngineDependentDvar...`, and proceeding arguments
  each being those explicitly passed to the argument index preceding - each is
  shifted right by one.

  Implicitly passing `EngineDependentDvar...` by reference where it should be
  passed by value similarly causes crashes and undefined behaviour.

  As such, we unfortunately have to use a `dvar_t*` return type for these
  functions. The return value will be implicitly casted to an
  `EngineDependentDvar...` at the callsite, but care must be taken to avoid
  using the `dvar_t*` return value directly.
*/

WEAK symbol<EngineDependentDvar> com_maxclients{0x1568ED850, 0x14948EE70};
// "sv_running" is name of dvar. com_sv_running is name used internally by
// engine to label dvar global
WEAK symbol<EngineDependentDvar> com_sv_running{0x1568ED848, 0x14948EE68};

WEAK symbol<EngineDependentDvar> g_gametype{0x14A399348, 0x147836878};
WEAK symbol<EngineDependentDvar> g_password{0x14A399350, 0x147836880};

WEAK symbol<EngineDependentDvar> sv_hostname{0x1577C5790, 0x14A1797A8};
WEAK symbol<EngineDependentDvar> sv_maprotation{0x1577C5888, 0x14A179948};
WEAK symbol<EngineDependentDvar> sv_maprotationcurrent{0x1577C5890,
                                                       0x14A189980};
// sv_cheats used to enable/disable cheat commands - both in console
// and in SV commands
WEAK symbol<EngineDependentDvar> sv_cheats{0x1576F8698, 0x14A189A18};
// Also named sv_cheats, but global is called "dvar_cheats" in engine.
// Used to enable/disable cheat dvars - controls whether cheat
// protection on a dvar to be modified is checked and respected in internal
// setters. Dvar_Init
WEAK symbol<EngineDependentDvar> dvar_cheats{0x157AC6210, 0x14A3CB610};
WEAK symbol<EngineDependentDvar> sv_gametype{0x1577C57F0, 0x14A179888};
WEAK symbol<qboolean> sv_running{0x1457D9928, 0x142CE04A8};

WEAK symbol<EngineDependentDvar> mapname{0x1577C57C8, 0x14A179858};

WEAK symbol<EngineDependentDvar> live_steam_server_name{0x15142E120,
                                                        0x14837F770};
WEAK symbol<EngineDependentDvar> live_steam_server_description{0x15142E128,
                                                               0x14837F778};
WEAK symbol<EngineDependentDvar> live_steam_server_password{0x15142E130,
                                                            0x14837F780};

WEAK symbol<EngineDependentDvar> net_port{0x157E75320, 0x14A640490};
WEAK symbol<EngineDependentDvar> net_ip{0x157E75318, 0x14A640488};

WEAK symbol<EngineDependentDvar> rcon_password{0x1577B5768, 0x14A178278};

WEAK symbol<EngineDependentDvar> ui_checksum_enabled{0x1568EDC98, 0x14948DC18};
WEAK symbol<EngineDependentDvarMut> ui_error_report{0x1568EEEA8, 0x14948DC20};
WEAK symbol<EngineDependentDvarMut> ui_error_report_delay{0x1568EEEB0,
                                                          0x14948DC28};
WEAK symbol<EngineDependentDvarMut> ui_error_callstack_ship{0x1568EDCA0,
                                                            0x14948DC30};
WEAK symbol<EngineDependentDvarMut> ui_enableAllHeroes{0x1579DF948,
                                                       0x14A30C408};

WEAK symbol<EngineDependentDvar> g_motd{0x14A3D15D0, 0x147836998};
WEAK symbol<EngineDependentDvar> live_motdEnabled{0x15163FC70, 0x148382988};
WEAK symbol<EngineDependentDvar> motd_enabled{0x15120C780, 0x1482DED80};
WEAK symbol<EngineDependentDvar> motddelay{0x15120C778, 0x1482DED78};

WEAK symbol<EngineDependentDvar> testmissingcontentpacks{0x1568EDDF0,
                                                         0x14948DD90};

WEAK symbol<EngineDependentDvar> splitscreen{0x1453D49F0, 0x1428AAEC8};
WEAK symbol<EngineDependentDvar> splitscreen_playerCount{0x1453D4A00,
                                                         0x1428AAED8};

WEAK symbol<EngineDependentDvar> r_vsync{0x14AE95098, 0x147CBC1E0};
WEAK symbol<EngineDependentDvar> r_fullscreen{0x14AE94DF8, 0x147CBBF48};

WEAK symbol<int32_t> g_dvarCount{0x157AC61CC, 0x14A3CB5FC};
WEAK symbol<DvarPool> s_dvarPool{0x0, 0x14A3CB620};
WEAK symbol<EncryptedDvarPool> s_dvarPool_cl{0x157AC6220};
inline EngineDependentDvarPool dvar_pool() {
  if (is_server()) {
    return s_dvarPool.get();
  }
  return s_dvarPool_cl.get();
}

WEAK symbol<int32_t> s_nextFreeCallback{0x157AC620C, 0x14A3CB60C};
WEAK symbol<DvarCallbackPool> s_dvarCallbackPool{0x157C06220, 0x14A4DB620};

WEAK symbol<DvarHashTable> s_dvarHashTable{0x157C07A20, 0x14A4DCE20};

WEAK symbol<bool> s_canSetConfigDvars{0x0, 0x14A3CB5D8};

// Used for e.g. changing in-game client FOV when `cg_fov` is changed,
// queueing mod load when `fs_game` is changed.
WEAK symbol<void(const dvar_t *dvar, modifiedCallback callback)>
    _Dvar_SetModifiedCallback{0x1422C8650, 0x140579990};
inline void Dvar_SetModifiedCallback(EngineDependentDvar dvar,
                                     modifiedCallback callback) {
  _Dvar_SetModifiedCallback(dvar, callback);
}

WEAK symbol<void(const dvar_t *dvar, DvarFlags flags)> _Dvar_AddFlags{
    0x1422B8260, 0x140574880};
inline void Dvar_AddFlags(EngineDependentDvar dvar, DvarFlags flags) {
  return _Dvar_AddFlags(dvar, flags);
}
WEAK symbol<bool(const dvar_t *dvar)> _Dvar_IsSessionModeBaseDvar{0x1422C23A0,
                                                                  0x140576890};
inline bool Dvar_IsSessionModeBaseDvar(EngineDependentDvar dvar) {
  return _Dvar_IsSessionModeBaseDvar(dvar);
}

WEAK symbol<dvar_t *(const char *dvarName)> _Dvar_FindVar{0x1422BCCD0,
                                                          0x140575540};
// See note at top of header
inline EngineDependentDvarMut Dvar_FindVar(const char *dvarName) {
  return _Dvar_FindVar(dvarName);
}

WEAK symbol<game::CanonHash_t(const char *str)> Dvar_GenerateHash{0x14133DBF0,
                                                                  0x140185800};
WEAK symbol<dvar_t *(dvarStrHash_t hash)> _Dvar_FindMalleableVar{0x1422BCC40,
                                                                 0x1405754B0};
// See note at top of header
inline EngineDependentDvarMut Dvar_FindMalleableVar(dvarStrHash_t hash) {
  return _Dvar_FindMalleableVar(hash);
}
WEAK symbol<const char *(const dvar_t *dvar)> _Dvar_GetDebugName{0x1422BD250};
inline const char *Dvar_GetDebugName(EngineDependentDvar dvar) {
  return _Dvar_GetDebugName(dvar);
}
WEAK symbol<const char *(const dvar_t *dvar)> _Dvar_DisplayableValue{
    0x1422BC080, 0x140574F40};
inline const char *Dvar_DisplayableValue(EngineDependentDvar dvar) {
  return _Dvar_DisplayableValue(dvar);
}

WEAK symbol<dvar_t *(dvarStrHash_t hash, const char *dvarName, dvarType_t type,
                     DvarFlags flags, DvarValue *value, DvarLimits *domain,
                     const char *description, bool isSessionModeDvar)>
    _Dvar_RegisterVariant{0x1422C4EA0, 0x140577A90};
// See note at top of header
inline EngineDependentDvarMut
Dvar_RegisterVariant(dvarStrHash_t hash, const char *dvarName, dvarType_t type,
                     DvarFlags flags, DvarValue *value, DvarLimits *domain,
                     const char *description, bool isSessionModeDvar) {
  return _Dvar_RegisterVariant(hash, dvarName, type, flags, value, domain,
                               description, isSessionModeDvar);
}
WEAK symbol<dvar_t *(dvarStrHash_t hash, const char *dvarName, bool value,
                     uint32_t flags, const char *description)>
    _Dvar_RegisterBool{0x1422D0900, 0x14057B500};
// See note at top of header
inline EngineDependentDvarMut Dvar_RegisterBool(dvarStrHash_t hash,
                                                const char *dvarName,
                                                bool value, DvarFlags flags,
                                                const char *description) {
  return _Dvar_RegisterBool(hash, dvarName, value, flags, description);
}
WEAK symbol<dvar_t *(dvarStrHash_t hash, const char *dvarName, int32_t value,
                     int32_t min, int32_t max, uint32_t flags,
                     const char *description)>
    _Dvar_RegisterInt{0x1422D0AE0, 0x14057B7B0};
// See note at top of header
inline EngineDependentDvarMut Dvar_RegisterInt(dvarStrHash_t hash,
                                               const char *dvarName,
                                               int32_t value, int32_t min,
                                               int32_t max, DvarFlags flags,
                                               const char *description) {
  return _Dvar_RegisterInt(hash, dvarName, value, min, max, flags, description);
}
WEAK symbol<dvar_t *(dvarStrHash_t hash, const char *dvarName, float value,
                     float min, float max, uint32_t flags,
                     const char *description)>
    _Dvar_RegisterFloat{0x1422D0A60, 0x14057B6B0};
// See note at top of header
inline EngineDependentDvarMut Dvar_RegisterFloat(dvarStrHash_t hash,
                                                 const char *dvarName,
                                                 float value, float min,
                                                 float max, DvarFlags flags,
                                                 const char *description) {
  return _Dvar_RegisterFloat(hash, dvarName, value, min, max, flags,
                             description);
}
WEAK symbol<dvar_t *(dvarStrHash_t hash, const char *dvarName, bool value,
                     DvarFlags flags, const char *description)>
    _Dvar_SessionModeRegisterBool{0x1422D0D40, 0x14057BAA0};
// See note at top of header
inline EngineDependentDvarMut
Dvar_SessionModeRegisterBool(dvarStrHash_t hash, const char *dvarName,
                             bool value, DvarFlags flags,
                             const char *description) {
  return _Dvar_SessionModeRegisterBool(hash, dvarName, value, flags,
                                       description);
}
WEAK symbol<void(dvarStrHash_t hash, bool value, eModes mode)>
    Dvar_SessionModeSetDefaultBool{0x1422D0E90, 0x14057BCE0};
WEAK
    symbol<dvar_t *(dvarStrHash_t hash, const char *dvarName, const char *value,
                    DvarFlags flags, const char *description)>
        _Dvar_RegisterString{0x1422D0B70, 0x14057B890};
// See note at top of header
inline EngineDependentDvarMut
Dvar_RegisterString(dvarStrHash_t hash, const char *dvarName, const char *value,
                    DvarFlags flags, const char *description) {
  return _Dvar_RegisterString(hash, dvarName, value, flags, description);
}

WEAK symbol<void(forEachCallback callback, void *userData)> _Dvar_ForEach{
    0x1422BCD00, 0x140575570};
extern thread_local engineDependentForEachCallback convertee;
void Converted_ForEachCallback(const dvar_t *dvar, void *userData);
inline void Dvar_ForEach(engineDependentForEachCallback callback,
                         void *userData) {
  convertee = callback;
  return _Dvar_ForEach(Converted_ForEachCallback, userData);
}
WEAK
    symbol<void(const char *dvarName, const char *string, bool createIfMissing)>
        Dvar_SetFromStringByName{0x1422C7500, 0x140579290};
WEAK symbol<dvar_t *(dvar_t *dvar, eModes mode)>
    _Dvar_GetSessionModeSpecificDvar{0x1422BF500, 0x140575D90};
inline EngineDependentDvarMut
Dvar_GetSessionModeSpecificDvar(EngineDependentDvarMut dvar, eModes mode) {
  return _Dvar_GetSessionModeSpecificDvar(dvar, mode);
}
WEAK symbol<bool(const dvar_t *dvar)> _Dvar_GetBool{0x1422BCED0, 0x140575740};
inline bool Dvar_GetBool(EngineDependentDvar dvar) {
  return _Dvar_GetBool(dvar);
}
WEAK symbol<int32_t(const dvar_t *dvar)> _Dvar_GetDomainEnumStringCount{
    0x1422BD340, 0x140575900};
inline int32_t Dvar_GetDomainEnumStringCount(EngineDependentDvar dvar) {
  return _Dvar_GetDomainEnumStringCount(dvar);
}
WEAK symbol<float(const dvar_t *dvar)> _Dvar_GetDomainFloatMax{0x1422BD3C0,
                                                               0x140575940};
inline float Dvar_GetDomainFloatMax(EngineDependentDvar dvar) {
  return _Dvar_GetDomainFloatMax(dvar);
}
WEAK symbol<float(const dvar_t *dvar)> _Dvar_GetDomainFloatMin{0x1422BD400,
                                                               0x140575980};
inline float Dvar_GetDomainFloatMin(EngineDependentDvar dvar) {
  return _Dvar_GetDomainFloatMin(dvar);
}
WEAK symbol<int64_t(const dvar_t *dvar)> _Dvar_GetDomainInt64Max{0x1422BD440,
                                                                 0x140575A90};
inline int64_t Dvar_GetDomainInt64Max(EngineDependentDvar dvar) {
  return _Dvar_GetDomainInt64Max(dvar);
}
WEAK symbol<int64_t(const dvar_t *dvar)> _Dvar_GetDomainInt64Min{0x1422BD480,
                                                                 0x1405759C0};
inline int64_t Dvar_GetDomainInt64Min(EngineDependentDvar dvar) {
  return _Dvar_GetDomainInt64Min(dvar);
}
WEAK symbol<int32_t(const dvar_t *dvar)> _Dvar_GetDomainIntMax{0x1422BD4C0,
                                                               0x140575A00};
inline int32_t Dvar_GetDomainIntMax(EngineDependentDvar dvar) {
  return _Dvar_GetDomainIntMax(dvar);
}
WEAK symbol<int32_t(const dvar_t *dvar)> _Dvar_GetDomainIntMin{0x1422BD510,
                                                               0x140575A50};
inline int32_t Dvar_GetDomainIntMin(EngineDependentDvar dvar) {
  return _Dvar_GetDomainIntMin(dvar);
}

WEAK symbol<float(const dvar_t *dvar)> _Dvar_GetFloat{0x1422BD620, 0x140575B20};
inline float Dvar_GetFloat(EngineDependentDvar dvar) {
  return _Dvar_GetFloat(dvar);
}
WEAK symbol<int64_t(const dvar_t *dvar)> _Dvar_GetInt64{0x1422BDC50,
                                                        0x140575B90};
inline int64_t Dvar_GetInt64(EngineDependentDvar dvar) {
  return _Dvar_GetInt64(dvar);
}
WEAK symbol<int32_t(const dvar_t *dvar)> _Dvar_GetInt{0x1422BE860, 0x140575C20};
inline int32_t Dvar_GetInt(EngineDependentDvar dvar) {
  return _Dvar_GetInt(dvar);
}
WEAK symbol<dvarType_t(const dvar_t *dvar)> _Dvar_GetType{0x1422BF5E0,
                                                          0x140575E90};
inline dvarType_t Dvar_GetType(EngineDependentDvar dvar) {
  return _Dvar_GetType(dvar);
}
WEAK symbol<uint64_t(const dvar_t *dvar)> _Dvar_GetUInt64{0x1422BF620,
                                                          0x140575ED0};
inline uint64_t Dvar_GetUInt64(EngineDependentDvar dvar) {
  return _Dvar_GetUInt64(dvar);
}
WEAK symbol<const char *(const dvar_t *dvar)> _Dvar_GetString{0x1422BF590,
                                                              0x140575E30};
inline const char *Dvar_GetString(EngineDependentDvar dvar) {
  return _Dvar_GetString(dvar);
}

WEAK symbol<void(const dvar_t *dvar, bool val, DvarSetSource source)>
    _Dvar_SetBoolFromSource{0x1422C6290, 0x140578BA0};
// To allow forward declaration for use in struct methods
inline void Dvar_SetBoolFromSource(EngineDependentDvar dvar, bool val,
                                   DvarSetSource source) {
  return _Dvar_SetBoolFromSource(dvar, val, source);
}
WEAK symbol<void(const dvar_t *dvar, float val, DvarSetSource source)>
    _Dvar_SetFloatFromSource{0x1422C6DD0, 0x140579190};
// To allow forward declaration for use in struct methods
inline void Dvar_SetFloatFromSource(EngineDependentDvar dvar, float val,
                                    DvarSetSource source) {
  return _Dvar_SetFloatFromSource(dvar, val, source);
}
WEAK symbol<void(const dvar_t *dvar, int64_t val, DvarSetSource source)>
    _Dvar_SetInt64FromSource{0x1422C7B70, 0x140579660};
// To allow forward declaration for use in struct methods
inline void Dvar_SetInt64FromSource(EngineDependentDvar dvar, int64_t val,
                                    DvarSetSource source) {
  return _Dvar_SetInt64FromSource(dvar, val, source);
}
WEAK symbol<void(const dvar_t *dvar, int32_t val, DvarSetSource source)>
    _Dvar_SetIntFromSource{0x1422C7ED0, 0x140579740};
// To allow forward declaration for use in struct methods
inline void Dvar_SetIntFromSource(EngineDependentDvar dvar, int32_t val,
                                  DvarSetSource source) {
  return _Dvar_SetIntFromSource(dvar, val, source);
}
WEAK symbol<void(const dvar_t *dvar, uint64_t val, DvarSetSource source)>
    _Dvar_SetUInt64FromSource{0x1422C8AF0, 0x140579B60};
// To allow forward declaration for use in struct methods
inline void Dvar_SetUInt64FromSource(EngineDependentDvar dvar, uint64_t val,
                                     DvarSetSource source) {
  return _Dvar_SetUInt64FromSource(dvar, val, source);
}
WEAK symbol<void(const dvar_t *dvar, const char *string, DvarSetSource source)>
    _Dvar_SetStringFromSource{0x1422C8770, 0x140579AB0};
// To allow forward declaration for use in struct methods
inline void Dvar_SetStringFromSource(EngineDependentDvar dvar, const char *val,
                                     DvarSetSource source) {
  return _Dvar_SetStringFromSource(dvar, val, source);
}

WEAK symbol<void(const dvar_t *dvar, const char *val)> _Dvar_SetString{
    0x1422C8720, 0x140579A60};
inline void Dvar_SetString(EngineDependentDvar dvar, const char *val) {
  return _Dvar_SetString(dvar, val);
}
WEAK symbol<void(const dvar_t *dvar, int32_t value)> _Dvar_SetInt{0x1422C7EC0,
                                                                  0x140579730};
inline void Dvar_SetInt(EngineDependentDvar dvar, int32_t value) {
  return _Dvar_SetInt(dvar, value);
}

} // namespace game

#endif