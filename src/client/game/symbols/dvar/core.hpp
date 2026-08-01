#pragma once

#include "game/symbols/sl.hpp"
#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {

WEAK symbol<EngineDependentDvarMut> com_maxclients{0x1568ED850, 0x14948EE70};
// "sv_running" is name of dvar. com_sv_running is name used internally by
// engine to label dvar global
WEAK symbol<EngineDependentDvarMut> com_sv_running{0x1568ED848, 0x14948EE68};

WEAK symbol<EngineDependentDvarMut> g_gametype{0x14A399348, 0x147836878};
WEAK symbol<EngineDependentDvarMut> g_password{0x14A399350, 0x147836880};

WEAK symbol<EngineDependentDvarMut> sv_hostname{0x1577C5790, 0x14A1797A8};
WEAK symbol<EngineDependentDvarMut> sv_maprotation{0x1577C5888, 0x14A179948};
WEAK symbol<EngineDependentDvarMut> sv_maprotationcurrent{0x1577C5890,
                                                          0x14A189980};
// sv_cheats used to enable/disable cheat commands - both in console
// and in SV commands
WEAK symbol<EngineDependentDvarMut> sv_cheats{0x1576F8698, 0x14A189A18};
// Also named sv_cheats, but global is called "dvar_cheats" in engine.
// Used to enable/disable cheat dvars - controls whether cheat
// protection on a dvar to be modified is checked and respected in internal
// setters. Dvar_Init
WEAK symbol<EngineDependentDvarMut> dvar_cheats{0x157AC6210, 0x14A3CB610};
WEAK symbol<EngineDependentDvarMut> sv_gametype{0x1577C57F0, 0x14A179888};
WEAK symbol<qboolean> sv_running{0x1457D9928, 0x142CE04A8};
// Specific to dedicated server
WEAK symbol<EngineDependentDvarMut> sv_detailedScriptErrors{0x0, 0x14A1797D0};

WEAK symbol<EngineDependentDvarMut> mapname{0x1577C57C8, 0x14A179858};

WEAK symbol<EngineDependentDvarMut> live_steam_server_name{0x15142E120,
                                                           0x14837F770};
WEAK symbol<EngineDependentDvarMut> live_steam_server_description{0x15142E128,
                                                                  0x14837F778};
WEAK symbol<EngineDependentDvarMut> live_steam_server_password{0x15142E130,
                                                               0x14837F780};

WEAK symbol<EngineDependentDvarMut> net_port{0x157E75320, 0x14A640490};
WEAK symbol<EngineDependentDvarMut> net_ip{0x157E75318, 0x14A640488};

WEAK symbol<EngineDependentDvarMut> rcon_password{0x1577B5768, 0x14A178278};

WEAK symbol<EngineDependentDvarMut> ui_checksum_enabled{0x1568EDC98,
                                                        0x14948DC18};
WEAK symbol<EngineDependentDvarMut> ui_error_report{0x1568EEEA8, 0x14948DC20};
WEAK symbol<EngineDependentDvarMut> ui_error_report_delay{0x1568EEEB0,
                                                          0x14948DC28};
WEAK symbol<EngineDependentDvarMut> ui_error_callstack_ship{0x1568EDCA0,
                                                            0x14948DC30};
WEAK symbol<EngineDependentDvarMut> ui_enableAllHeroes{0x1579DF948,
                                                       0x14A30C408};

WEAK symbol<EngineDependentDvarMut> g_log{0x14A3D15E0, 0x147102FA0};
WEAK symbol<EngineDependentDvarMut> g_motd{0x14A3D15D0, 0x147836998};
WEAK symbol<EngineDependentDvarMut> live_motdEnabled{0x15163FC70, 0x148382988};
WEAK symbol<EngineDependentDvarMut> motd_enabled{0x15120C780, 0x1482DED80};
WEAK symbol<EngineDependentDvarMut> motddelay{0x15120C778, 0x1482DED78};

WEAK symbol<EngineDependentDvarMut> testmissingcontentpacks{0x1568EDDF0,
                                                            0x14948DD90};

WEAK symbol<EngineDependentDvarMut> splitscreen{0x1453D49F0, 0x1428AAEC8};
WEAK symbol<EngineDependentDvarMut> splitscreen_playerCount{0x1453D4A00,
                                                            0x1428AAED8};

WEAK symbol<EngineDependentDvarMut> r_vsync{0x14AE95098, 0x147CBC1E0};
WEAK symbol<EngineDependentDvarMut> r_fullscreen{0x14AE94DF8, 0x147CBBF48};

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
WEAK symbol<void(EngineDependentDvar dvar, modifiedCallback callback)>
    Dvar_SetModifiedCallback{0x1422C8650, 0x140579990};

WEAK symbol<void(EngineDependentDvar dvar, DvarFlags flags)> Dvar_AddFlags{
    0x1422B8260, 0x140574880};

WEAK symbol<bool(EngineDependentDvar dvar)> Dvar_IsSessionModeBaseDvar{
    0x1422C23A0, 0x140576890};

WEAK symbol<EngineDependentDvarMut(const char *dvarName)> Dvar_FindVar{
    0x1422BCCD0, 0x140575540};

WEAK symbol<game::CanonHash_t(const char *str)> Dvar_GenerateHash{0x14133DBF0,
                                                                  0x140185800};
WEAK symbol<EngineDependentDvarMut(dvarStrHash_t hash)> Dvar_FindMalleableVar{
    0x1422BCC40, 0x1405754B0};
WEAK symbol<const char *(EngineDependentDvar dvar)> Dvar_GetDebugName{
    0x1422BD250};
WEAK symbol<const char *(EngineDependentDvar dvar)> Dvar_DisplayableValue{
    0x1422BC080, 0x140574F40};

WEAK symbol<EngineDependentDvarMut(
    dvarStrHash_t hash, const char *dvarName, dvarType_t type, DvarFlags flags,
    DvarValue *value, DvarLimits *domain, const char *description,
    bool isSessionModeDvar)>
    Dvar_RegisterVariant{0x1422C4EA0, 0x140577A90};
WEAK symbol<EngineDependentDvarMut(dvarStrHash_t hash, const char *dvarName,
                                   bool value, DvarFlags flags,
                                   const char *description)>
    Dvar_RegisterBool{0x1422D0900, 0x14057B500};
WEAK symbol<EngineDependentDvarMut(dvarStrHash_t hash, const char *dvarName,
                                   int32_t value, int32_t min, int32_t max,
                                   DvarFlags flags, const char *description)>
    Dvar_RegisterInt{0x1422D0AE0, 0x14057B7B0};
WEAK symbol<EngineDependentDvarMut(dvarStrHash_t hash, const char *dvarName,
                                   float value, float min, float max,
                                   DvarFlags flags, const char *description)>
    Dvar_RegisterFloat{0x1422D0A60, 0x14057B6B0};
WEAK symbol<EngineDependentDvarMut(dvarStrHash_t hash, const char *dvarName,
                                   bool value, DvarFlags flags,
                                   const char *description)>
    Dvar_SessionModeRegisterBool{0x1422D0D40, 0x14057BAA0};
WEAK symbol<void(dvarStrHash_t hash, bool value, eModes mode)>
    Dvar_SessionModeSetDefaultBool{0x1422D0E90, 0x14057BCE0};
WEAK symbol<EngineDependentDvarMut(dvarStrHash_t hash, const char *dvarName,
                                   const char *value, DvarFlags flags,
                                   const char *description)>
    Dvar_RegisterString{0x1422D0B70, 0x14057B890};

WEAK symbol<void(forEachCallback callback, void *userData)> Dvar_ForEach{
    0x1422BCD00, 0x140575570};
WEAK
    symbol<void(const char *dvarName, const char *string, bool createIfMissing)>
        Dvar_SetFromStringByName{0x1422C7500, 0x140579290};
WEAK symbol<EngineDependentDvarMut(EngineDependentDvarMut dvar, eModes mode)>
    Dvar_GetSessionModeSpecificDvar{0x1422BF500, 0x140575D90};
WEAK symbol<bool(EngineDependentDvar dvar)> Dvar_GetBool{0x1422BCED0,
                                                         0x140575740};
WEAK symbol<int32_t(EngineDependentDvar dvar)> Dvar_GetDomainEnumStringCount{
    0x1422BD340, 0x140575900};
WEAK symbol<float(EngineDependentDvar dvar)> Dvar_GetDomainFloatMax{
    0x1422BD3C0, 0x140575940};
WEAK symbol<float(EngineDependentDvar dvar)> Dvar_GetDomainFloatMin{
    0x1422BD400, 0x140575980};
WEAK symbol<int64_t(EngineDependentDvar dvar)> Dvar_GetDomainInt64Max{
    0x1422BD440, 0x140575A90};
WEAK symbol<int64_t(EngineDependentDvar dvar)> Dvar_GetDomainInt64Min{
    0x1422BD480, 0x1405759C0};
WEAK symbol<int32_t(EngineDependentDvar dvar)> Dvar_GetDomainIntMax{
    0x1422BD4C0, 0x140575A00};
WEAK symbol<int32_t(EngineDependentDvar dvar)> Dvar_GetDomainIntMin{
    0x1422BD510, 0x140575A50};

WEAK symbol<float(EngineDependentDvar dvar)> Dvar_GetFloat{0x1422BD620,
                                                           0x140575B20};
WEAK symbol<int64_t(EngineDependentDvar dvar)> Dvar_GetInt64{0x1422BDC50,
                                                             0x140575B90};
WEAK symbol<int32_t(EngineDependentDvar dvar)> Dvar_GetInt{0x1422BE860,
                                                           0x140575C20};
WEAK symbol<dvarType_t(EngineDependentDvar dvar)> Dvar_GetType{0x1422BF5E0,
                                                               0x140575E90};
WEAK symbol<uint64_t(EngineDependentDvar dvar)> Dvar_GetUInt64{0x1422BF620,
                                                               0x140575ED0};
WEAK symbol<const char *(EngineDependentDvar dvar)> Dvar_GetString{0x1422BF590,
                                                                   0x140575E30};

WEAK symbol<void(EngineDependentDvar dvar, bool val, DvarSetSource source)>
    _Dvar_SetBoolFromSource{0x1422C6290, 0x140578BA0};
// To allow forward declaration for use in struct methods
inline void Dvar_SetBoolFromSource(EngineDependentDvar dvar, bool val,
                                   DvarSetSource source) {
  return _Dvar_SetBoolFromSource(dvar, val, source);
}
WEAK symbol<void(EngineDependentDvar dvar, float val, DvarSetSource source)>
    _Dvar_SetFloatFromSource{0x1422C6DD0, 0x140579190};
// To allow forward declaration for use in struct methods
inline void Dvar_SetFloatFromSource(EngineDependentDvar dvar, float val,
                                    DvarSetSource source) {
  return _Dvar_SetFloatFromSource(dvar, val, source);
}
WEAK symbol<void(EngineDependentDvar dvar, int64_t val, DvarSetSource source)>
    _Dvar_SetInt64FromSource{0x1422C7B70, 0x140579660};
// To allow forward declaration for use in struct methods
inline void Dvar_SetInt64FromSource(EngineDependentDvar dvar, int64_t val,
                                    DvarSetSource source) {
  return _Dvar_SetInt64FromSource(dvar, val, source);
}
WEAK symbol<void(EngineDependentDvar dvar, int32_t val, DvarSetSource source)>
    _Dvar_SetIntFromSource{0x1422C7ED0, 0x140579740};
// To allow forward declaration for use in struct methods
inline void Dvar_SetIntFromSource(EngineDependentDvar dvar, int32_t val,
                                  DvarSetSource source) {
  return _Dvar_SetIntFromSource(dvar, val, source);
}
WEAK symbol<void(EngineDependentDvar dvar, uint64_t val, DvarSetSource source)>
    _Dvar_SetUInt64FromSource{0x1422C8AF0, 0x140579B60};
// To allow forward declaration for use in struct methods
inline void Dvar_SetUInt64FromSource(EngineDependentDvar dvar, uint64_t val,
                                     DvarSetSource source) {
  return _Dvar_SetUInt64FromSource(dvar, val, source);
}
WEAK symbol<void(EngineDependentDvar dvar, const char *string,
                 DvarSetSource source)>
    _Dvar_SetStringFromSource{0x1422C8770, 0x140579AB0};
// To allow forward declaration for use in struct methods
inline void Dvar_SetStringFromSource(EngineDependentDvar dvar, const char *val,
                                     DvarSetSource source) {
  return _Dvar_SetStringFromSource(dvar, val, source);
}

WEAK symbol<void(EngineDependentDvar dvar, const char *val)> _Dvar_SetString{
    0x1422C8720, 0x140579A60};
// To allow forward declaration for use in struct methods
inline void Dvar_SetString(EngineDependentDvar dvar, const char *val) {
  return _Dvar_SetString(dvar, val);
}
WEAK symbol<void(EngineDependentDvar dvar, int32_t value)> _Dvar_SetInt{
    0x1422C7EC0, 0x140579730};
// To allow forward declaration for use in struct methods
inline void Dvar_SetInt(EngineDependentDvar dvar, int32_t value) {
  return _Dvar_SetInt(dvar, value);
}

WEAK symbol<EngineDependentDvar(EngineDependentDvar base, eModes mode)>
    Dvar_GetSessionModeSpecificDvarInternal{0x1422BF500, 0x140575D90};

WEAK symbol<DvarFlags> g_dvar_modifiedFlags{0x157AC61C8, 0x14A3CB5F8};

template <typename T_DvarValue>
inline dvarCallBack_t *dvar<T_DvarValue>::modifiedCallback() noexcept {
  if (*s_nextFreeCallback > 0) {
    for (int32_t i = 0; i < *s_nextFreeCallback; ++i) {
      dvarCallBack_t *cb = &s_dvarCallbackPool->pool[i];
      if (cb->dvar.name() == name()) {
        return cb;
      }
    }
  }

  return nullptr;
}

template <typename T_DvarValue>
template <typename T>
  requires(!std::same_as<T, const char *> &&
           !std::same_as<T, const std::string_view &> &&
           !std::same_as<T, const std::string &>)
inline std::optional<T> dvar<T_DvarValue>::set(T val) noexcept {
  dvar<T_DvarValue> *sessionModeSpecificDvar = sessionModeSpecific();
  if (sessionModeSpecificDvar) {
    return sessionModeSpecificDvar->set(val);
  }

  if (domain().contains(val)) {
    T prev;
    if (flags().latch) {
      prev = latched().set(val);
    } else {
      *g_dvar_modifiedFlags |= flags();
      prev = current().set(val);
      (void)latched().set(val);
      modified() = true;

      if (flags().modifiedCallback) {
        dvarCallBack_t *cb = modifiedCallback();
        if (cb) {
          cb->needsCallback = true;
        }
      }
    }

    return prev;
  }

  return std::nullopt;
}

template <typename T_DvarValue>
inline std::optional<std::string>
dvar<T_DvarValue>::set(const char *val) noexcept {
  dvar<T_DvarValue> *sessionModeSpecificDvar = sessionModeSpecific();
  if (sessionModeSpecificDvar) {
    return sessionModeSpecificDvar->set(val);
  }

  switch (type()) {
  case dvarType_t::ENUM: {
    if (domain().contains(val)) {
      const char *prev;
      if (flags().latch) {
        prev = latched().set(val);
      } else {
        (void)latched().set(val);
        prev = current().set(val);
        *g_dvar_modifiedFlags |= flags();
        modified() = true;

        if (flags().modifiedCallback) {
          dvarCallBack_t *cb = modifiedCallback();
          if (cb) {
            cb->needsCallback = true;
          }
        }
      }

      return prev;
    }
    break;
  }
  case dvarType_t::STRING: {
    std::string prev;
    if (flags().latch) {
      if (val != latched().string()) {
        const char *prev_alloc = latched().set(sl::CopyString(val), false);
        prev = prev_alloc;
        if (prev_alloc != current().string() &&
            prev_alloc != reset().string()) {
          sl::FreeString(prev_alloc);
        }
      } else {
        break;
      }
    } else if (val != current().string()) {
      *g_dvar_modifiedFlags |= flags();
      const char *new_alloc = sl::CopyString(val);
      const char *prev_alloc = current().set(new_alloc);
      if (new_alloc != latched().string()) {
        latched().string() = new_alloc;
      }
      prev = prev_alloc;
      if (prev_alloc != latched().string() && prev_alloc != reset().string()) {
        sl::FreeString(prev_alloc);
      }
      modified() = true;

      if (flags().modifiedCallback) {
        dvarCallBack_t *cb = modifiedCallback();
        if (cb) {
          cb->needsCallback = true;
        }
      }
    } else {
      break;
    }

    return prev;
  }
  default: {
    break;
  }
  }

  return std::nullopt;
}
template <typename T_DvarValue>
inline std::optional<std::string>
dvar<T_DvarValue>::set(const std::string_view &val) noexcept {
  return set(val.data());
}

template <typename T_DvarValue>
inline std::optional<std::string>
dvar<T_DvarValue>::set(const std::string &val) noexcept {
  return set(val.c_str());
}
} // namespace game