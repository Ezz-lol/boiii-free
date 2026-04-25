#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

namespace game {
namespace scr {

uint32_t Scr_CanonHash(const char *str);

WEAK symbol<scr_const_t> scr_const{0x14A7290F0, 0x147B91CE0};

WEAK symbol<void(scriptInstance_t inst, int value)> Scr_AddInt{0x1412E9870,
                                                               0x14016F160};
WEAK symbol<const char *(scriptInstance_t inst, uint32_t index)> Scr_GetString{
    0x1412EBAA0, 0x140171490};
WEAK symbol<void(scriptInstance_t inst, const char *value)> Scr_AddString{
    0x1412E9A30, 0x14016F320};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst)> ScrVar_AllocStruct{
    0x1412D9840, 0x140165D00};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarNameType_t nameType,
                          ScrVarNameIndex_t nameIndex, ScrVarIndex_t parentId)>
    ScrVar_AllocVariable{0x1412D9A60, 0x140165E00};
WEAK symbol<void(scriptInstance_t inst, ScrString_t stringValue)>
    Scr_AddArrayStringIndexed{0x1412E95D0, 0x14016EE50};
WEAK symbol<void(scriptInstance_t inst, uint32_t id)> Scr_AddObject{
    0x1412E99A0, 0x14016F290};
// Return value is location of allocation as index of VM stack - returned by
// ScrVar_AllocVariable internally.
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst)> Scr_AddStruct{0x1412E9AC0,
                                                                0x14016F3B0};
WEAK symbol<void(scriptInstance_t inst, const vec3_t *value)> Scr_AddVector{
    0x1412E9E90, 0x14016F780};
WEAK symbol<ScrString_t(scriptInstance_t inst, uint32_t index)>
    Scr_GetConstString{0x1412EB1F0, 0x140170BE0};
WEAK symbol<float(scriptInstance_t inst, uint32_t index)> Scr_GetFloat{
    0x1412EB5C0, 0x140170FB0};
WEAK symbol<int(scriptInstance_t inst, uint32_t index)> Scr_GetInt{0x1412EB7F0,
                                                                   0x1401711E0};
WEAK symbol<uint32_t(scriptInstance_t inst)> Scr_GetNumParam{0x1412EB930,
                                                             0x140171320};
WEAK symbol<uint32_t(scriptInstance_t inst, uint32_t index)> Scr_GetObject{
    0x1412EB950, 0x140171340};
WEAK symbol<int(scriptInstance_t inst, uint32_t index)> Scr_GetPointerType{
    0x1412EB9F0, 0x1401713E0};
WEAK symbol<team_t(scriptInstance_t inst, uint32_t index)> Scr_GetTeam{
    0x1412EBB60, 0x140171530};
WEAK symbol<ScrVarType_t(scriptInstance_t inst, uint32_t index)> Scr_GetType{
    0x1412EBD30, 0x140171700};
WEAK symbol<const char *(scriptInstance_t inst, uint32_t index)>
    Scr_GetTypeName{0x1412EBD90, 0x140171760};
WEAK symbol<ScrVarValue_t *(scriptInstance_t inst, uint32_t index)>
    Scr_GetValue{0x1412EBF30, 0x140171900};
WEAK symbol<void(scriptInstance_t inst, uint32_t index, vec3_t *vectorValue)>
    Scr_GetVector{0x1412EBF90, 0x140171960};
WEAK symbol<void(scriptInstance_t inst)> Scr_MakeArray{0x1412EC840,
                                                       0x140172210};
WEAK symbol<void(scriptInstance_t inst, uint32_t index, const char *error)>
    Scr_ParamError{0x1412ECBD0, 0x1401725A0};
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
WEAK symbol<void(scriptInstance_t inst, int user)> Scr_BeginLoadScripts{
    0x1412C7DF0, 0x140156010};
WEAK symbol<void(ScrString_t *to, ScrString_t from)> Scr_SetString{0x1412D8490,
                                                                   0x140164750};
WEAK symbol<void(scriptInstance_t inst, const char *error, bool terminal)>
    Scr_Error{0x1412EA430, 0x14016FDF0};
WEAK symbol<void(level::gentity_s *ent, ScrString_t stringValue,
                 uint32_t paramcount)>
    Scr_Notify_ScrString{0x141B46510, 0x1402F6000};

WEAK symbol<void(scriptInstance_t inst, ScrVarValue_t *value)>
    ScrVar_AddRefValue{0x1412D9350, 0x1401657E0};
WEAK symbol<void(scriptInstance_t inst, ScrVarValue_t *value)>
    ScrVar_ReleaseValue{0x1412E0010, 0x14016C7C0};
WEAK symbol<void(scriptInstance_t inst)> Scr_AddArray{0x1412E9560, 0x14016EDE0};
WEAK symbol<void(scriptInstance_t inst, float value)> Scr_AddFloat{0x1412E9760,
                                                                   0x14016F050};

WEAK symbol<void(scriptInstance_t inst, ScrVarValue_t *value)> Scr_AddValue{
    0x1412E9E00, 0x14016F6F0};

WEAK symbol<void(scriptInstance_t inst, ScrVarValue_t *value)>
    ScrVar_CastVector{0x1412DAFA0, 0x140167420};
WEAK symbol<scr_anim_t(scriptInstance_t inst, uint32_t index,
                       anim::XAnimTree *tree)>
    Scr_GetAnim{0x1412EAB60, 0x140170520};
WEAK
    symbol<scr_animtree_t(scriptInstance_t inst, uint32_t index, uint32_t user)>
        Scr_GetAnimTree{0x1412EACB0, 0x140170670};
WEAK symbol<bool(scriptInstance_t inst, uint32_t index, bool defaultValue)>
    Scr_GetBoolOptional{0x1412EAF20, 0x1401708E0};
WEAK symbol<ScrString_t(scriptInstance_t inst, uint32_t index)>
    Scr_GetConstIString{0x1412EAF40, 0x140170900};
WEAK symbol<scr_entref_t *(scr_entref_t *retstr, scriptInstance_t inst,
                           uint32_t index)>
    Scr_GetEntityRef{0x1412EB490, 0x140170E80};
WEAK symbol<float(scriptInstance_t inst, uint32_t index, float defaultValue)>
    Scr_GetFloatOptional{0x1412EB680, 0x140171070};
WEAK symbol<scr_funcptr_t(scriptInstance_t inst, uint32_t index)> Scr_GetFunc{
    0x1412EB730, 0x140171120};
WEAK symbol<int(scriptInstance_t inst, uint32_t index, int defaultValue)>
    Scr_GetIntOptional{0x1412EB8A0, 0x140171290};
WEAK symbol<uint64_t(scriptInstance_t inst, uint32_t index)> Scr_GetUint64{
    0x1412EBE00, 0x1401717D0};
WEAK symbol<uint64_t(scriptInstance_t inst, uint32_t index,
                     uint64_t defaultValue)>
    Scr_GetUint64Optional{0x1412EBEA0, 0x140171870};
WEAK symbol<void(scriptInstance_t inst, uint32_t index, vec3_t *vectorValue,
                 vec3_t *defaultValue)>
    Scr_GetVectorOptional{0x1412EC040, 0x140171A10};

} // namespace scr
} // namespace game

#endif