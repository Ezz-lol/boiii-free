#pragma once

#include <cstdarg>
#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace utils::string {
const char *va(const char *fmt, ...);
}

namespace game {
namespace scr {
WEAK symbol<scr_const_t> scr_const{0x14A7290F0, 0x147B91CE0};

WEAK symbol<void(scriptInstance_t inst, int32_t value)> Scr_AddInt{0x1412E9870,
                                                                   0x14016F160};
WEAK symbol<const char *(scriptInstance_t inst, var::ScrVarIndex_t index)>
    Scr_GetString{0x1412EBAA0, 0x140171490};
WEAK symbol<void(scriptInstance_t inst, const char *value)> Scr_AddString{
    0x1412E9A30, 0x14016F320};
// Return value is location of allocation as index of VM stack - returned by
// ScrVar_AllocVariable internally.
WEAK symbol<var::ScrVarIndex_t(scriptInstance_t inst)> Scr_AddStruct{
    0x1412E9AC0, 0x14016F3B0};
WEAK symbol<void(scriptInstance_t inst, const vec3_t *value)> Scr_AddVector{
    0x1412E9E90, 0x14016F780};
WEAK symbol<ScrString_t(scriptInstance_t inst, var::ScrVarIndex_t index)>
    Scr_GetConstString{0x1412EB1F0, 0x140170BE0};
WEAK symbol<float(scriptInstance_t inst, var::ScrVarIndex_t index)>
    Scr_GetFloat{0x1412EB5C0, 0x140170FB0};
WEAK symbol<int(scriptInstance_t inst, var::ScrVarIndex_t index)> Scr_GetInt{
    0x1412EB7F0, 0x1401711E0};
WEAK symbol<uint32_t(scriptInstance_t inst)> Scr_GetNumParam{0x1412EB930,
                                                             0x140171320};
WEAK symbol<uint32_t(scriptInstance_t inst, var::ScrVarIndex_t index)>
    Scr_GetObject{0x1412EB950, 0x140171340};
WEAK symbol<var::ScrVarType_t(scriptInstance_t inst, var::ScrVarIndex_t index)>
    Scr_GetPointerType{0x1412EB9F0, 0x1401713E0};
WEAK symbol<team_t(scriptInstance_t inst, var::ScrVarIndex_t index)>
    Scr_GetTeam{0x1412EBB60, 0x140171530};
WEAK symbol<var::ScrVarType_t(scriptInstance_t inst, var::ScrVarIndex_t index)>
    Scr_GetType{0x1412EBD30, 0x140171700};
WEAK symbol<const char *(scriptInstance_t inst, var::ScrVarIndex_t index)>
    Scr_GetTypeName{0x1412EBD90, 0x140171760};
WEAK symbol<volatile var::ScrVarValue_t *(scriptInstance_t inst,
                                          var::ScrVarIndex_t index)>
    Scr_GetValue{0x1412EBF30, 0x140171900};
WEAK symbol<void(scriptInstance_t inst, var::ScrVarIndex_t index,
                 vec3_t *vectorValue)>
    Scr_GetVector{0x1412EBF90, 0x140171960};
WEAK symbol<void(scriptInstance_t inst)> Scr_MakeArray{0x1412EC840,
                                                       0x140172210};
WEAK symbol<void(scriptInstance_t inst, var::ScrVarIndex_t index,
                 const char *error)>
    _Scr_ParamError{0x1412ECBD0, 0x1401725A0};
inline void Scr_ParamError(scriptInstance_t inst, var::ScrVarIndex_t index,
                           const char *error) {
  return _Scr_ParamError(inst, index, error);
}
template <typename... Args>
inline void Scr_ParamError(scriptInstance_t inst, var::ScrVarIndex_t index,
                           const char *fmt, Args &&...args) {
  const char *error = utils::string::va(fmt, args...);
  return _Scr_ParamError(inst, index, error);
}
WEAK symbol<void(const char *entityName, const vec3_t *origin)>
    Scr_NetworkOriginError{0x141A7C8C0, 0x1402CC4B0};
WEAK symbol<void(scriptInstance_t inst, level::gentity_s *ent,
                 uint32_t classNum)>
    Scr_AddEntity{0x141B2C5D0, 0x1402F4CA0};

WEAK symbol<void(level::gentity_s *ent, ScrVarCanonicalName_t stringValue,
                 uint32_t paramcount)>
    Scr_Notify_Canon{0x0, 0x1402F5FF0};
WEAK symbol<uint32_t(scriptInstance_t inst, const char *filename)>
    Scr_LoadScript{0x1412C83F0, 0x140156610};
WEAK symbol<void(scriptInstance_t inst, int32_t user)> Scr_BeginLoadScripts{
    0x1412C7DF0, 0x140156010};
WEAK symbol<void(ScrString_t *to, ScrString_t from)> Scr_SetString{0x1412D8490,
                                                                   0x140164750};
WEAK symbol<void(scriptInstance_t inst, const char *error, bool terminal)>
    Scr_Error{0x1412EA430, 0x14016FDF0};
WEAK symbol<void(level::gentity_s *ent, ScrString_t stringValue,
                 uint32_t paramcount)>
    Scr_Notify_ScrString{0x141B46510, 0x1402F6000};

WEAK symbol<void(scriptInstance_t inst)> Scr_AddArray{0x1412E9560, 0x14016EDE0};
WEAK symbol<void(scriptInstance_t inst, ScrString_t hash)> Scr_AddConstString{
    0x1412E96B0, 0x14016EEB0};
WEAK symbol<void(scriptInstance_t inst, float value)> Scr_AddFloat{0x1412E9760,
                                                                   0x14016F050};

WEAK symbol<void(scriptInstance_t inst, var::ScrVarValue_t *value)>
    Scr_AddValue{0x1412E9E00, 0x14016F6F0};

WEAK symbol<scr_anim_t(scriptInstance_t inst, var::ScrVarIndex_t index,
                       anim::XAnimTree *tree)>
    Scr_GetAnim{0x1412EAB60, 0x140170520};
WEAK symbol<scr_animtree_t(scriptInstance_t inst, var::ScrVarIndex_t index,
                           uint32_t user)>
    Scr_GetAnimTree{0x1412EACB0, 0x140170670};
// Contents are simply `Scr_GetInt(inst, index) != 0`
WEAK symbol<bool(scriptInstance_t inst, var::ScrVarIndex_t index)> Scr_GetBool{
    0x0, 0x1401708C0};

WEAK symbol<bool(scriptInstance_t inst, var::ScrVarIndex_t index,
                 bool defaultValue)>
    Scr_GetBoolOptional{0x1412EAF20, 0x1401708E0};
WEAK symbol<ScrString_t(scriptInstance_t inst, var::ScrVarIndex_t index)>
    Scr_GetConstIString{0x1412EAF40, 0x140170900};
WEAK symbol<scr_entref_t *(scr_entref_t *retstr, scriptInstance_t inst,
                           var::ScrVarIndex_t index)>
    Scr_GetEntityRef{0x1412EB490, 0x140170E80};
WEAK symbol<level::gentity_t *(var::ScrVarIndex_t index)> Scr_GetEntity{
    0x141B3C000, 0x1402F58A0};
WEAK symbol<float(scriptInstance_t inst, var::ScrVarIndex_t index,
                  float defaultValue)>
    Scr_GetFloatOptional{0x1412EB680, 0x140171070};
WEAK symbol<scr_funcptr_t(scriptInstance_t inst, var::ScrVarIndex_t index)>
    Scr_GetFunc{0x1412EB730, 0x140171120};
WEAK symbol<int(scriptInstance_t inst, var::ScrVarIndex_t index,
                int32_t defaultValue)>
    Scr_GetIntOptional{0x1412EB8A0, 0x140171290};
WEAK symbol<uint64_t(scriptInstance_t inst, var::ScrVarIndex_t index)>
    Scr_GetUint64{0x1412EBE00, 0x1401717D0};
WEAK symbol<uint64_t(scriptInstance_t inst, var::ScrVarIndex_t index,
                     uint64_t defaultValue)>
    Scr_GetUint64Optional{0x1412EBEA0, 0x140171870};
WEAK symbol<void(scriptInstance_t inst, var::ScrVarIndex_t index,
                 vec3_t *vectorValue, vec3_t *defaultValue)>
    Scr_GetVectorOptional{0x1412EC040, 0x140171A10};
WEAK symbol<ScrString_t(scriptInstance_t inst, var::ScrVarIndex_t index)>
    Scr_GetConstLowercaseString{0x1412EAFE0, 0x1401709A0};
WEAK symbol<void()> Scr_UpdateScreen{0x1413E1550};

WEAK symbol<void(scriptInstance_t inst)> Scr_PlaySoundAtPosition{0x141A7EA40,
                                                                 0x1402CC960};

WEAK symbol<void(scriptInstance_t inst, const char *error)> Scr_ObjectError{
    0x1412ECBC0, 0x140172590};

WEAK symbol<void(scriptInstance_t inst, int32_t channel, uint8_t codePos)>
    Scr_PrintPrevCodePos{0x1412D4C50, 0x140160DA0};

WEAK symbol<void(scriptInstance_t inst, uint8_t *pos, const char **filename,
                 int32_t *lineNum, const char **sourceLine)>
    Scr_GetFileAndLineNum{0x1412CBE50, 0x140159E90};
WEAK symbol<void(scriptInstance_t inst, objFileInfo_t *fileInfo)> LoadScriptGDB{
    0x1412CB100, 0x140159360};
// Named LoadScriptGDB in engine - overload
WEAK symbol<void(scriptInstance_t inst)> LoadScriptGDB2{0x1412CAC90,
                                                        0x140158EF0};
WEAK symbol<objFileInfo_t *(scriptInstance_t inst, void *addr)>
    Scr_FindObjFileInfo{0x1412CBD90, 0x140159DD0};
WEAK symbol<void(scriptInstance_t inst, GSC_OBJ *prime_obj,
                 objFileInfo_t *fileInfo, GSC_IMPORT_ITEM *import,
                 char *errorString, int32_t errorStringLength)>
    ReportObjLinkError{0x1412CB4F0, 0x140159750};
// Duplicate. Exactly the same as ReportObjLinkError - not sure why this
// function exists.
WEAK symbol<void(scriptInstance_t inst, GSC_OBJ *prime_obj,
                 objFileInfo_t *fileInfo, GSC_IMPORT_ITEM *import,
                 char *errorString, int32_t errorStringLength)>
    ReportObjLinkError2{0x1412CB710};

WEAK symbol<ObjFileInfoPool> gObjFileInfo{0x1450DC2E0, 0x1425DCA80};
WEAK symbol<ScrPool<uint32_t>> gObjFileInfoCount{0x1450EFB60, 0x1425F0300};

inline objFileInfo_t *get_obj_by_name(scriptInstance_t inst,
                                      const std::string_view &script) {
  for (size_t i = 0; i < gObjFileInfoCount->instance[inst]; ++i) {
    if (std::string_view(
            gObjFileInfo->instance[inst][i].activeVersion->get_name()) ==
        script) {
      return &gObjFileInfo->instance[inst][i];
    }
  }

  return nullptr;
}

} // namespace scr
} // namespace game