#pragma once

#include "game/structs/scr/builtin/core.hpp"

namespace game {
namespace scr {
namespace builtin {
namespace table {
namespace bg {
struct BGScrCommonFunctionTable {
  const BuiltinFunctionDef functions[34];
};

struct BGScrMathFunctionTable {
  const BuiltinFunctionDef functions[49];
};

union BGScrWeaponFunctionTable {
  struct {
    const BuiltinFunctionDef EnumerateWeapons;
  };
  const BuiltinFunctionDef functions[1];
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
} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game