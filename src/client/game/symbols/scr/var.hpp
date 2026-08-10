#pragma once

#include <game/symbols/scr/mt.hpp>
#include <game/symbols/scr/vm/core.hpp>
#include <game/symbols/sym_include.hpp>
#include <game/symbols/sl.hpp>

namespace game {
namespace scr {
namespace var {
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst)> ScrVar_AllocArray{
    0x1412D9480, 0x140165A50};
WEAK symbol<void(scriptInstance_t inst, volatile ScrVarValue_t *value)>
    ScrVar_AddRefValue{0x1412D9350, 0x1401657E0};
WEAK symbol<void(scriptInstance_t inst, volatile ScrVarValue_t *value)>
    ScrVar_ReleaseValue{0x1412E0010, 0x14016C7C0};
WEAK symbol<uint32_t(scriptInstance_t inst, ScrVarIndex_t id)>
    ScrVar_ReleaseVariable{0x1412E0170, 0x14016C950};
WEAK symbol<void(scriptInstance_t inst, volatile ScrVarValue_t *value)>
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

WEAK symbol<void(scriptInstance_t inst, volatile ScrVarValue_t *value,
                 volatile ScrVarValue_t *index)>
    ScrVar_EvalArray{0x1412DBED0, 0x1401682E0};
WEAK symbol<ScrVarIndex_t(scriptInstance_t inst, ScrVarIndex_t parentId,
                          volatile ScrVarValue_t *index)>
    ScrVar_EvalArrayIndex{0x1412DC1D0, 0x140168610};
WEAK symbol<ScrVarValue_t *(ScrVarValue_t *retstr, scriptInstance_t inst,
                            ScrVarIndex_t entId,
                            ScrVarNameIndex_t fieldNameIndex)>
    ScrVar_EvalEntityField{0x1412DCB30, 0x140168FC0};
WEAK symbol<qboolean(scriptInstance_t inst, volatile ScrVarValue_t *value)>
    Scr_IsTrue{0x1412CCE10, 0x14015AEE0};
WEAK symbol<qboolean(scriptInstance_t inst, volatile ScrVarValue_t *value)>
    ScrVar_CastBool_NonInteger{0x1412DACF0, 0x140167170};

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
    ScrVar_AllocVector{0x1412D9B90, 0x140166070};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t parentId,
                 ScrVarIndex_t id)>
    ScrVar_AppendChild{0x1412D9BD0, 0x1401660B0};
WEAK symbol<uint32_t(scriptInstance_t inst, ScrVarIndex_t id)> ScrVar_ArraySize{
    0x1412DAA70};
WEAK symbol<scr_funcptr_t(scriptInstance_t inst, ScrVarIndex_t classInstance,
                          ScrVarCanonicalName_t functionName)>
    ScrVar_ClassVtableLookup{0x1412DB0B0, 0x140167530};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t id,
                 volatile ScrVarValue_t *value)>
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
                 volatile ScrVarValue_t *value)>
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
                 ScrVarNameIndex_t fieldNameIndex,
                 volatile ScrVarValue_t *value)>
    ScrVar_SetEntityFieldValue{0x1412E0920, 0x14016D110};
WEAK symbol<void(scriptInstance_t inst, ScrVarIndex_t id,
                 volatile ScrVarValue_t *value)>
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
inline constexpr bool valid_scrvar_index(scriptInstance_t inst,
                                         ScrVarIndex_t index) {
  return index < SCRIPTVARIABLE_POOL_SIZE.instance[inst];
}

inline ScrVarIndex_t scrvar_index(scriptInstance_t inst,
                                  volatile ScrVar_t *var) {
  uintptr_t scriptVariablesPtr = reinterpret_cast<uintptr_t>(
      vm::gScrVarGlob->instance[inst].scriptVariables);
  uintptr_t varPtr = reinterpret_cast<uintptr_t>(var);
  return static_cast<ScrVarIndex_t>((varPtr - scriptVariablesPtr) /
                                    sizeof(ScrVar_t));
}

inline bool valid_scrvar_ptr(scriptInstance_t inst, volatile ScrVar_t *var) {
  return valid_engine_ptr(var) // Static or stack allocation
         ||
         valid_scrvar_index(inst, scrvar_index(inst, var)); // Pool allocation
}

inline ScrVarIndex_t scrvarvalue_index(scriptInstance_t inst,
                                       volatile ScrVarValue_t *val) {
  return scrvar_index(inst, val->var());
}

inline bool valid_scrvarvalue_ptr(scriptInstance_t inst,
                                  volatile ScrVarValue_t *val) {
  return valid_engine_ptr(val) // Static or stack allocation
         || valid_scrvar_index(inst,
                               scrvarvalue_index(inst, val)); // Pool allocation
}

inline bool valid_val_allocation_ptr(uintptr_t ptr) {
  return valid_stack_ptr(ptr) ||
         (scr::mt::gScrMemTreePub->mt_buffer &&
          scr::mt::gScrMemTreePub->mt_buffer->contains(ptr));
}

template <typename T> inline bool valid_val_allocation_ptr(volatile T *ptr) {
  return valid_val_allocation_ptr(reinterpret_cast<uintptr_t>(ptr));
}

/*
  Note: this does not work for pointer-typed `ScrVarValue_t` values stored
  in the VM runtime stack, as seen in `Scr_GetValue`'s
  return. Those pointers are an index from the _top_ of the VM runtime stack,
  rather than an absolute index to a value in the `scriptVariables` pool.

  The aforementioned pointer type functions similarly to x86's RIP-relative
  addressing, whereas those handled here function similarly to x86's absolute
  addressing.
*/
inline volatile ScrVar_t *ScrVar_t::deref(scriptInstance_t inst) volatile {
  switch (value.type) {
  case ScrVarType::POINTER:
    if (valid_scrvar_index(inst, value.u.pointerValue)) {
      return &vm::gScrVarGlob->instance[inst]
                  .scriptVariables[value.u.pointerValue];
    }
    break;
  default:
    break;
  }

  return this;
}

inline volatile ScrVarValue_t *
ScrVarValue_t::deref(scriptInstance_t inst) volatile {
  return &this->var()->deref(inst)->value;
}

inline bool ScrVarValue_t::array_like(scriptInstance_t inst) volatile {
  switch (type) {
  case ScrVarType::POINTER: {
    volatile ScrVarValue_t *dereferenced = this->deref(inst);
    return dereferenced && dereferenced->type == ScrVarType::ARRAY;
  }
  case ScrVarType::STRING:
  case ScrVarType::VECTOR: {
    return true;
  }
  default: {
    return false;
  }
  }
}

inline volatile ScrVar_t *
ScrVar_t::next_sibling(scriptInstance_t inst) volatile {
  if (valid_scrvar_index(inst, nextSibling)) {
    return &vm::gScrVarGlob->instance[inst].scriptVariables[nextSibling];
  }

  return nullptr;
}

inline std::optional<float> ScrVarValue_t::cast_float() volatile noexcept {
  switch (type) {
  case ScrVarType::POINTER:
  case ScrVarType::UINT64:
  case ScrVarType::UINTPTR_T:
  case ScrVarType::CODEPOS:
  case ScrVarType::PRECODEPOS:
  case ScrVarType::API_FUNCTION:
  case ScrVarType::FUNCTION:
  case ScrVarType::ANIMATION:
  case ScrVarType::CLASS:
  case ScrVarType::STRUCT:
  case ScrVarType::ENTITY:
  case ScrVarType::ARRAY: {
    /*
      Blindly re-interpret the value as an integer, then cast to float.
      This will only lead to erroneous behaviour if the user unknowingly
      passed a non-floating point value. In this case, we can only assume
      they intended to have the passed value converted to a float, rather
      than throwing an error.
    */
    return static_cast<float>(u.uint64Value);
  }
  case ScrVarType::INT:
  case ScrVarType::ENTITY_OFFSET:
  case ScrVarType::HASH: {
    return static_cast<float>(u.uintValue);
  }
  case ScrVarType::FLOAT: {
    return u.floatValue;
  }

  case ScrVarType::VECTOR: {
    if (u.vectorValue) {
      return *u.vectorValue;
    }
    break;
  }
  case ScrVarType::STRING:
  case ScrVarType::LOCALIZED_STRING: {
    const char *str = sl::SL_ConvertToString(u.stringValue);
    if (str && str[0]) {
      const size_t len = strlen(str);
      float f = 0.0f;
      auto [ptr, ec] = std::from_chars(str, str + len, f);
      if (ec == std::errc()) {
        return f;
      }
    }
    break;
  }
  default:
    break;
  }

  return std::nullopt;
}

inline std::optional<qboolean> ScrVarValue_t::cast_bool() volatile noexcept {

  switch (type) {
  case ScrVarType::FLOAT: {
    return qboolean::from(u.floatValue != 0.0);
  }

  case ScrVarType::INT: {
    return qboolean::from(u.uintValue != 0);
  }
  case ScrVarType::UINT64: {
    return qboolean::from(u.uint64Value != 0);
  }
  case ScrVarType::POINTER:
  case ScrVarType::UINTPTR_T:
  case ScrVarType::CODEPOS:
  case ScrVarType::PRECODEPOS:
  case ScrVarType::API_FUNCTION:
  case ScrVarType::FUNCTION:
  case ScrVarType::ANIMATION:
  case ScrVarType::CLASS:
  case ScrVarType::STRUCT:
  case ScrVarType::ENTITY:
  case ScrVarType::ARRAY: {
    return qboolean::from(u.uintptrValue != 0);
  }
  case ScrVarType::UNDEFINED: {
    return qfalse;
  }
  default: {
    return std::nullopt;
  }
  }
}

} // namespace var
} // namespace scr
} // namespace game