#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace scr {
namespace var {
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst)> ScrVar_AllocArray{
    0x1412D9480, 0x140165A50};
WEAK symbol<void(scriptInstance_t inst, ScrVarValue_t *value)>
    ScrVar_AddRefValue{0x1412D9350, 0x1401657E0};
WEAK symbol<void(scriptInstance_t inst, ScrVarValue_t *value)>
    ScrVar_ReleaseValue{0x1412E0010, 0x14016C7C0};
WEAK symbol<uint32_t(scriptInstance_t inst, ScrVarIndex_t id)>
    ScrVar_ReleaseVariable{0x1412E0170, 0x14016C950};
WEAK symbol<void(scriptInstance_t inst, ScrVarValue_t *value)>
    ScrVar_CastVector{0x1412DAFA0, 0x140167420};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst)> ScrVar_AllocStruct{
    0x1412D9840, 0x140165D00};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarNameType_t nameType,
                          ScrVarNameIndex_t nameIndex, ScrVarIndex_t parentId)>
    ScrVar_AllocVariable{0x1412D9A60, 0x140165E00};
WEAK symbol<void(scriptInstance_t inst, ScrString_t stringValue)>
    Scr_AddArrayStringIndexed{0x1412E95D0, 0x14016EE50};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t id)> Scr_AddObject{
    0x1412E99A0, 0x14016F290};
WEAK symbol<ScrVarValue_t *(ScrVarValue_t *retstr, scriptInstance_t inst,
                            ScrVarIndex_t id)>
    ScrVar_EvalVariable{0x1412DDF30, 0x14016A5B0};

WEAK symbol<void(scriptInstance_t inst, ScrVarValue_t *value,
                 ScrVarValue_t *index)>
    ScrVar_EvalArray{0x1412DBED0, 0x1401682E0};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarIndex_t parentId,
                          ScrVarValue_t *index)>
    ScrVar_EvalArrayIndex{0x1412DC1D0, 0x140168610};
WEAK symbol<ScrVarValue_t *(ScrVarValue_t *retstr, scriptInstance_t inst,
                            ScrVarIndex_t entId,
                            ScrVarNameIndex_t fieldNameIndex)>
    ScrVar_EvalEntityField{0x1412DCB30, 0x140168FC0};
WEAK symbol<bool(scriptInstance_t inst, ScrVarValue_t *value)> ScrVar_EvalBool{
    0x1412CCE10, 0x14015AEE0};
WEAK symbol<bool(scriptInstance_t inst, ScrVarValue_t *value)>
    ScrVar_EvalFloatBool{0x1412DACF0, 0x140167170};

WEAK symbol<void(scriptInstance_t inst, uint32_t classnum,
                 ScrVarCanonicalName_t canonId, int32_t offset)>
    ScrVar_AddClassFields{0x0, 0x1401652B0};
WEAK
    symbol<void(scriptInstance_t inst, const char *path, const char *extension)>
        ScrVar_AddFields_FastFile{0x1412D8EE0};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t id)>
    ScrVar_AddRefVariable{0x1412D93C0};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t id,
                 ScrVarNameIndex_t nameIndex, ScrVarIndex_t parentId)>
    ScrVar_AddToNameSearchHash{0x1412D9400};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarIndex_t self,
                          ScrVarIndex_t parentId)>
    ScrVar_AllocChildThread{0x1412D9580, 0x140165AC0};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, uint32_t classnum,
                          EntRefUnion entRefUnion, LocalClientNum_t clientNum)>
    ScrVar_AllocEntity{0x1412D9720};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarIndex_t self)>
    ScrVar_AllocThread{0x1412D9940, 0x140165D70};
WEAK symbol<const float *(scriptInstance_t inst, const vec3_t *v)>
    ScrVar_AllocVector{0x1412D9B90};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t parentId,
                 ScrVarIndex_t id)>
    ScrVar_AppendChild{0x1412D9BD0, 0x1401660B0};
WEAK symbol<uint32_t(scriptInstance_t inst, ScrVarIndex_t id)> ScrVar_ArraySize{
    0x1412DAA70};
WEAK symbol<scr_funcptr_t(scriptInstance_t inst, ScrVarIndex_t classInstance,
                          ScrVarCanonicalName_t functionName)>
    ScrVar_ClassVtableLookup{0x1412DB0B0, 0x140167530};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t id, ScrVarValue_t *value)>
    ScrVar_ClearArray{0x0, 0x140168A00};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t id)> ScrVar_ClearVariable{
    0x1412DB690};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t id)> ScrVar_ClearWaitTime{
    0x1412DB770};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, EntRefUnion entRefUnion,
                          uint32_t classnum, LocalClientNum_t clientNum)>
    ScrVar_FindEntityId{0x1412DE1B0};
WEAK symbol<uint32_t(scriptInstance_t inst, ScrVarCanonicalName_t canonId,
                     ScrVarType_t *type)>
    ScrVar_FindField{0x1412DE250};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarIndex_t parentId,
                          ScrVarNameIndex_t nameIndex)>
    ScrVar_FindVariable{0x1412DE290};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarIndex_t parentId)>
    ScrVar_FirstChild{0x1412DE3B0};
WEAK symbol<void(scriptInstance_t inst, EntRefUnion entRefUnion,
                 uint32_t classnum, LocalClientNum_t clientNum)>
    ScrVar_FreeEntityNum{0x1412DE480, 0x14016AB00};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarIndex_t id)>
    ScrVar_GetArrayRef{0x1412DE590};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, EntRefUnion entRefUnion,
                          uint32_t classnum, LocalClientNum_t clientNum)>
    ScrVar_GetEntityId{0x1412DE680};
WEAK symbol<scr_entref_t *(scr_entref_t *retstr, scriptInstance_t inst,
                           ScrVarIndex_t entId)>
    ScrVar_GetEntityIdRef{0x1412DE740, 0x14016ADC0};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarIndex_t parentId,
                          ScrVarNameIndex_t nameIndex)>
    ScrVar_GetVariableByIndex{0x1412DED70};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarIndex_t parentId,
                          ScrVarNameIndex_t nameIndex)>
    ScrVar_GetVariableByWeapon{0x1412DEE80};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, EntRefUnion entRefUnion,
                          LocalClientNum_t clientNum)>
    ScrVar_GetWeaponId{0x1412DEF00};
WEAK symbol<void(scriptInstance_t inst)> ScrVar_Init{0x1412DF210, 0x14016B8F0};
WEAK symbol<void(scriptInstance_t inst)> ScrVar_InitClassMap{0x1412DF300,
                                                             0x14016BA50};
WEAK symbol<void(scriptInstance_t inst)> ScrVar_InitVariables{0x1412DF3C0,
                                                              0x14016BB40};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t parentId,
                 ScrVarIndex_t id, ScrVarIndex_t afterHereId)>
    ScrVar_InsertChild{0x1412DF4A0};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarIndex_t parentId)>
    ScrVar_LastChild{0x1412DF5A0};
WEAK symbol<void(scriptInstance_t inst, mem::MemoryFile *memFile)> ScrVar_Load{
    0x1412D52D0};
WEAK symbol<void(scriptInstance_t inst, mem::MemoryFile *memFile,
                 ScrVarIndex_t id)>
    ScrVar_LoadPortable{0x1412D5570};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, mem::MemoryFile *memFile,
                          ScrVarIndex_t parentId)>
    ScrVar_LoadPortableBranch{0x1412D5790};
WEAK symbol<ScrVarNameIndex_t(scriptInstance_t inst, ScrVarIndex_t id)>
    ScrVar_NameIndex{0x1412DF5D0};
WEAK symbol<ScrString_t(scriptInstance_t inst, ScrVarIndex_t id)>
    ScrVar_NameIndexAsString{0x1412DF610};
WEAK symbol<ScrVarNameType_t(scriptInstance_t inst, ScrVarIndex_t id)>
    ScrVar_NameType{0x1412DF630};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarIndex_t parentId,
                          ScrVarIndex_t nameId)>
    ScrVar_NewVariableById{0x1412DF6C0};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarIndex_t parentId,
                          ScrVarNameIndex_t index)>
    ScrVar_NewVariableByIndex{0x1412DF7A0, 0x14016BF10};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarIndex_t id)>
    ScrVar_NextSibling{0x1412DFA30};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarIndex_t id)>
    ScrVar_Parent{0x1412DFA70};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t parentId,
                 ScrVarIndex_t id)>
    ScrVar_PrependChild{0x1412DFA90};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarIndex_t id)>
    ScrVar_PrevSibling{0x1412DFBD0};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t id)> ScrVar_Prune{
    0x1412DFBF0};
WEAK symbol<void(scriptInstance_t inst, mem::MemoryFile *memFile,
                 ScrVarValue_t *value)>
    ScrVar_ReadValue{0x1412D6F30};
WEAK symbol<void(scriptInstance_t inst)> ScrVar_ReleaseTheFreeEntityList{
    0x1412DE3E0};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t id)>
    ScrVar_ReleaseVariableName{0x1412E0330};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t id)>
    ScrVar_ReleaseVariableValue{0x1412E03E0, 0x14016CBD0};
WEAK symbol<void(scriptInstance_t inst, uint32_t classnum)>
    ScrVar_RemoveClassMap{0x1412E0430};
WEAK symbol<bool(scriptInstance_t inst, ScrVarIndex_t id,
                 ScrVarNameIndex_t nameIndex, ScrVarIndex_t parentId)>
    ScrVar_RemoveFromNameSearchHash{0x1412E04D0, 0x14016CCC0};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t id)>
    ScrVar_RemoveVariable{0x1412E0580, 0x14016CD70};
WEAK symbol<void(scriptInstance_t inst, mem::MemoryFile *memFile,
                 ScrVarIndex_t parentId)>
    ScrVar_SavePortableBranch{0x1412D6830};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarIndex_t id)> ScrVar_Self{
    0x1412E0900, 0x14016D0F0};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t entId,
                 ScrVarNameIndex_t fieldNameIndex, ScrVarValue_t *value)>
    ScrVar_SetEntityFieldValue{0x1412E0920, 0x14016D110};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t id, ScrVarValue_t *value)>
    ScrVar_SetValue{0x1412E0B70, 0x14016D360};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t id, uint32_t waitTime)>
    ScrVar_SetWaitTime{0x1412E0C00};
WEAK symbol<ScrVarType_t(scriptInstance_t inst, ScrVarIndex_t id)> ScrVar_Type{
    0x1412E0CF0, 0x14016D4D0};
WEAK symbol<ScrVarValue_t *(ScrVarValue_t *retstr, scriptInstance_t inst,
                            ScrVarIndex_t id)>
    ScrVar_Value{0x1412E0E00};

WEAK symbol<ScrVarTypePool<const char *>> var_typename{0x1432E6240,
                                                       0x14107C040};
} // namespace var
} // namespace scr
} // namespace game