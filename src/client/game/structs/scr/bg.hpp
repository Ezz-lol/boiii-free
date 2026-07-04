#pragma once

#include <cstdint>
#include "core.hpp"
#include "../asm.hpp"
namespace game {
namespace scr {
namespace bg {
struct bgsClientFieldCallbacks_t {
  clientFieldCallback_t clientFieldCallbacks[4][3072];
  uint32_t numCallbacks[4];
};

struct bgsClientFields_t {
  bool bRegistrationFinished;
  clientFieldSet_t clientFieldSets[15];
  uint32_t fieldHash;
  uint32_t highestVersionUsed;
  bgsClientFieldCallbacks_t *pPendingCallbacks;
};

struct bgsAnim_t {
  anim::animScriptData_t animScriptData;
  struct {
    scr_animtree_t tree;
    scr_anim_t body;
    scr_anim_t main;
    scr_anim_t torso;
    scr_anim_t torso2;
    scr_anim_t legs;
    scr_anim_t motion_matching;
  } generic_human;
  struct {
    scr_animtree_t tree;
  } ai_generic_human;
};

struct bgs_clientfieldapi_t {
  fastcall_t<void(uint32_t)> CompareHashToGameState;
  fastcall_t<bool(void)> AllowVersionReRegistration;
  fastcall_t<bool(clientField_t *, uint32_t)> FieldVersionAllowsRegistration;
  fastcall_t<bool(uint32_t)> ServerVersionAllowsRegistration;
};

union BGScrUtilFunctionTable {
  struct {
    const BuiltinFunctionDef GetDvarString;
    const BuiltinFunctionDef GetDvarInt;
    const BuiltinFunctionDef GetDvarFloat;
    const BuiltinFunctionDef GetDvarVector;
    const BuiltinFunctionDef Throw;
    const BuiltinFunctionDef Array;
    const BuiltinFunctionDef AssociativeArray;
    const BuiltinFunctionDef ArrayRemoveValue;
    const BuiltinFunctionDef ArrayRemoveIndex;
    const BuiltinFunctionDef ArrayInsert;
    const BuiltinFunctionDef ArrayCombine;
    const BuiltinFunctionDef ArrayCopy;
    const BuiltinFunctionDef IsInArray;
    const BuiltinFunctionDef GetArrayKeys;
    const BuiltinFunctionDef GetFirstArrayKey;
    const BuiltinFunctionDef GetNextArrayKey;
    const BuiltinFunctionDef GetLastArrayKey;
    const BuiltinFunctionDef GetPrevArrayKey;
    const BuiltinFunctionDef ArrayIntersect;
    const BuiltinFunctionDef ArrayGetClosest;
    const BuiltinFunctionDef ArrayGetFarthest;
    const BuiltinFunctionDef GetStructField;
    const BuiltinFunctionDef IsFunctionPtr;
    const BuiltinFunctionDef IsString;
    const BuiltinFunctionDef IsArray;
    const BuiltinFunctionDef IsWeapon;
    const BuiltinFunctionDef IsInt;
    const BuiltinFunctionDef IsFloat;
    const BuiltinFunctionDef IsVec;
    const BuiltinFunctionDef IsClass;
    const BuiltinFunctionDef IString;
    const BuiltinFunctionDef IsSubStr;
    const BuiltinFunctionDef GetSubStr;
    const BuiltinFunctionDef ToLower;
    const BuiltinFunctionDef ToUpper;
    const BuiltinFunctionDef StrTok;
    const BuiltinFunctionDef StrTok2;
    const BuiltinFunctionDef StrStartsWith;
    const BuiltinFunctionDef StrEndsWith;
    const BuiltinFunctionDef StrIsNumber;
    const BuiltinFunctionDef StrIsFloat;
    const BuiltinFunctionDef StrIsInt;
    const BuiltinFunctionDef StrStrip;
    const BuiltinFunctionDef MakeLocalizedString;
    const BuiltinFunctionDef Sprintf;
    const BuiltinFunctionDef TableLookup;
    const BuiltinFunctionDef TableLookupIString;
    const BuiltinFunctionDef TableLookupRowNum;
    const BuiltinFunctionDef TableLookupColumnForRow;
    const BuiltinFunctionDef TableLookupFindCoreAsset;
    const BuiltinFunctionDef TableLookupRowCount;
    const BuiltinFunctionDef TableLookupColumnCount;
    const BuiltinFunctionDef TableLookupRow;
    const BuiltinFunctionDef GetItemIndexFromRef;
    const BuiltinFunctionDef GetAttachmentTableIndex;
    const BuiltinFunctionDef OpenFile;
    const BuiltinFunctionDef CloseFile;
    const BuiltinFunctionDef FPrintln;
    const BuiltinFunctionDef FPrintFields;
    const BuiltinFunctionDef FReadLn;
    const BuiltinFunctionDef FGetArg;
    const BuiltinFunctionDef DebugBreak;
    const BuiltinFunctionDef Assert;
    const BuiltinFunctionDef AssertMsg;
    const BuiltinFunctionDef ErrorMsg;
    const BuiltinFunctionDef IsPC;
    const BuiltinFunctionDef IsProfileBuild;
    const BuiltinFunctionDef IsUsingMods;
    const BuiltinFunctionDef Modvar;
  };
  const BuiltinFunctionDef functions[69];
};
ASSERT_SIZE(BGScrUtilFunctionTable, 0x8A0);
} // namespace bg
} // namespace scr
} // namespace game