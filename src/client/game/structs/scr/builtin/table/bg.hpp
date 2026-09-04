#pragma once

#include <game/structs/scr/builtin/core.hpp>
#include <game/structs/scr/builtin/table/macros.hpp>

namespace game {
namespace scr {
namespace builtin {
namespace table {
namespace bg {
union CommonFunctionTable {
  struct {
    BuiltinFunctionDef ClearAllCharacterTables;
    BuiltinFunctionDef GetCharacterBodyStyleIndex;
    BuiltinFunctionDef GetAllCharacterBodies;
    BuiltinFunctionDef GetAllCharacterHeads;
    BuiltinFunctionDef GetCharacterBodyModelCount;
    BuiltinFunctionDef GetCharacterBodyModelColorCount;
    BuiltinFunctionDef GetCharacterHelmetModelCount;
    BuiltinFunctionDef GetCharacterHelmetModelColorCount;
    BuiltinFunctionDef GetAnimForCharacter;
    BuiltinFunctionDef GetXCamForCharacter;
    BuiltinFunctionDef GetSpawnStructForCharacter;
    BuiltinFunctionDef GetWeaponModelForCharacter;
    BuiltinFunctionDef GetWeaponForCharacter;
    BuiltinFunctionDef GetCharacterFields;
    BuiltinFunctionDef GetCharacterDisplayName;
    BuiltinFunctionDef GetCharacterAssetName;
    BuiltinFunctionDef GetCharacterBodyModel;
    BuiltinFunctionDef GetCharacterHelmetModel;
    BuiltinFunctionDef GetCharacterHelmetHidesHead;
    BuiltinFunctionDef GetCharacterHeadModel;
    BuiltinFunctionDef GetCharacterBodyRenderOptions;
    BuiltinFunctionDef GetCharacterHelmetRenderOptions;
    BuiltinFunctionDef GetCharacterHeadRenderOptions;
    BuiltinFunctionDef GetCharacterModeRenderOptions;
    BuiltinFunctionDef GetXModelCenterOffset;
    BuiltinFunctionDef GetNextMap;
    BuiltinFunctionDef GetMapOrder;
    BuiltinFunctionDef GetMapAtIndex;
    BuiltinFunctionDef GetMapIntroMovie;
    BuiltinFunctionDef GetMapOutroMovie;
    BuiltinFunctionDef GetMapFields;
    BuiltinFunctionDef SetupFieldOpsKitLoadouts;
    BuiltinFunctionDef SessionModeIsCampaignZombiesGame;
    BuiltinFunctionDef IsArchetypeLoaded;
  };

  static inline constexpr size_t COUNT = 34;
  BuiltinFunctionDef functions[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
      "ClearAllCharacterTables",
      "GetCharacterBodyStyleIndex",
      "GetAllCharacterBodies",
      "GetAllCharacterHeads",
      "GetCharacterBodyModelCount",
      "GetCharacterBodyModelColorCount",
      "GetCharacterHelmetModelCount",
      "GetCharacterHelmetModelColorCount",
      "GetAnimForCharacter",
      "GetXCamForCharacter",
      "GetSpawnStructForCharacter",
      "GetWeaponModelForCharacter",
      "GetWeaponForCharacter",
      "GetCharacterFields",
      "GetCharacterDisplayName",
      "GetCharacterAssetName",
      "GetCharacterBodyModel",
      "GetCharacterHelmetModel",
      "GetCharacterHelmetHidesHead",
      "GetCharacterHeadModel",
      "GetCharacterBodyRenderOptions",
      "GetCharacterHelmetRenderOptions",
      "GetCharacterHeadRenderOptions",
      "GetCharacterModeRenderOptions",
      "GetXModelCenterOffset",
      "GetNextMap",
      "GetMapOrder",
      "GetMapAtIndex",
      "GetMapIntroMovie",
      "GetMapOutroMovie",
      "GetMapFields",
      "SetupFieldOpsKitLoadouts",
      "SessionModeIsCampaignZombiesGame",
      "IsArchetypeLoaded",
  };

  DEFINE_NAME_MAP(names, hashes);
  static_assert(hashes.size() == names.size(),
                "name hashmap does not include all names!");

  IMPL_TABLE_OPERATORS(functions);
};
ASSERT_SIZE(CommonFunctionTable, sizeof(CommonFunctionTable::functions));

union MathFunctionTable {
  struct {
    BuiltinFunctionDef Log;
    BuiltinFunctionDef Sin;
    BuiltinFunctionDef Cos;
    BuiltinFunctionDef Tan;
    BuiltinFunctionDef Asin;
    BuiltinFunctionDef Acos;
    BuiltinFunctionDef Atan;
    BuiltinFunctionDef Abs;
    BuiltinFunctionDef Min;
    BuiltinFunctionDef Max;
    BuiltinFunctionDef Floor;
    BuiltinFunctionDef Ceil;
    BuiltinFunctionDef Sqrt;
    BuiltinFunctionDef Pow;
    BuiltinFunctionDef LerpFloat;
    BuiltinFunctionDef LerpVector;
    BuiltinFunctionDef GetMinBitCountForNum;
    BuiltinFunctionDef MapFloat;
    BuiltinFunctionDef DiffTrack;
    BuiltinFunctionDef DiffTrackAngle;
    BuiltinFunctionDef PackRgba;
    BuiltinFunctionDef Int;
    BuiltinFunctionDef CastFloat;
    BuiltinFunctionDef Distance2DSquared;
    BuiltinFunctionDef AngleLerp;
    BuiltinFunctionDef RotatePoint;
    BuiltinFunctionDef VectorFromLineToPoint;
    BuiltinFunctionDef PointOnSegmentNearestToPoint;
    BuiltinFunctionDef Distance;
    BuiltinFunctionDef Distance2D;
    BuiltinFunctionDef DistanceSquared;
    BuiltinFunctionDef Length;
    BuiltinFunctionDef LengthSquared;
    BuiltinFunctionDef Closer;
    BuiltinFunctionDef VectorDot;
    BuiltinFunctionDef VectorCross;
    BuiltinFunctionDef VectorNormalize;
    BuiltinFunctionDef VectorProjection;
    BuiltinFunctionDef VectorToAngles;
    BuiltinFunctionDef VectorLerp;
    BuiltinFunctionDef AnglesToUp;
    BuiltinFunctionDef AnglesToRight;
    BuiltinFunctionDef AnglesToForward;
    BuiltinFunctionDef CombineAngles;
    BuiltinFunctionDef AngleClamp180;
    BuiltinFunctionDef AbsAngleClamp180;
    BuiltinFunctionDef AbsAngleClamp360;
    BuiltinFunctionDef ForwardRotateAroundUpByAngle;
    BuiltinFunctionDef RotatePointAroundAxis;
  };

  static inline constexpr size_t COUNT = 49;
  BuiltinFunctionDef functions[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
      "Log",
      "Sin",
      "Cos",
      "Tan",
      "Asin",
      "Acos",
      "Atan",
      "Abs",
      "Min",
      "Max",
      "Floor",
      "Ceil",
      "Sqrt",
      "Pow",
      "LerpFloat",
      "LerpVector",
      "GetMinBitCountForNum",
      "MapFloat",
      "DiffTrack",
      "DiffTrackAngle",
      "PackRgba",
      "Int",
      "CastFloat",
      "Distance2DSquared",
      "AngleLerp",
      "RotatePoint",
      "VectorFromLineToPoint",
      "PointOnSegmentNearestToPoint",
      "Distance",
      "Distance2D",
      "DistanceSquared",
      "Length",
      "LengthSquared",
      "Closer",
      "VectorDot",
      "VectorCross",
      "VectorNormalize",
      "VectorProjection",
      "VectorToAngles",
      "VectorLerp",
      "AnglesToUp",
      "AnglesToRight",
      "AnglesToForward",
      "CombineAngles",
      "AngleClamp180",
      "AbsAngleClamp180",
      "AbsAngleClamp360",
      "ForwardRotateAroundUpByAngle",
      "RotatePointAroundAxis",
  };

  DEFINE_NAME_MAP(names, hashes);
  static_assert(hashes.size() == names.size(),
                "name hashmap does not include all names!");

  IMPL_TABLE_OPERATORS(functions);
};
ASSERT_SIZE(MathFunctionTable, sizeof(MathFunctionTable::functions));

union WeaponFunctionTable {
  struct {
    BuiltinFunctionDef EnumerateWeapons;
  };

  static inline constexpr size_t COUNT = 1;
  BuiltinFunctionDef functions[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
      "EnumerateWeapons"};

  DEFINE_NAME_MAP(names, hashes);
  static_assert(hashes.size() == names.size(),
                "name hashmap does not include all names!");

  IMPL_TABLE_OPERATORS(functions);
};
ASSERT_SIZE(WeaponFunctionTable, sizeof(WeaponFunctionTable::functions));

union UtilFunctionTable {
  struct {
    BuiltinFunctionDef GetDvarString;
    BuiltinFunctionDef GetDvarInt;
    BuiltinFunctionDef GetDvarFloat;
    BuiltinFunctionDef GetDvarVector;
    BuiltinFunctionDef Throw;
    BuiltinFunctionDef Array;
    BuiltinFunctionDef AssociativeArray;
    BuiltinFunctionDef ArrayRemoveValue;
    BuiltinFunctionDef ArrayRemoveIndex;
    BuiltinFunctionDef ArrayInsert;
    BuiltinFunctionDef ArrayCombine;
    BuiltinFunctionDef ArrayCopy;
    BuiltinFunctionDef IsInArray;
    BuiltinFunctionDef GetArrayKeys;
    BuiltinFunctionDef GetFirstArrayKey;
    BuiltinFunctionDef GetNextArrayKey;
    BuiltinFunctionDef GetLastArrayKey;
    BuiltinFunctionDef GetPrevArrayKey;
    BuiltinFunctionDef ArrayIntersect;
    BuiltinFunctionDef ArrayGetClosest;
    BuiltinFunctionDef ArrayGetFarthest;
    BuiltinFunctionDef GetStructField;
    BuiltinFunctionDef IsFunctionPtr;
    BuiltinFunctionDef IsString;
    BuiltinFunctionDef IsArray;
    BuiltinFunctionDef IsWeapon;
    BuiltinFunctionDef IsInt;
    BuiltinFunctionDef IsFloat;
    BuiltinFunctionDef IsVec;
    BuiltinFunctionDef IsClass;
    BuiltinFunctionDef IString;
    BuiltinFunctionDef IsSubStr;
    BuiltinFunctionDef GetSubStr;
    BuiltinFunctionDef ToLower;
    BuiltinFunctionDef ToUpper;
    BuiltinFunctionDef StrTok;
    BuiltinFunctionDef StrTok2;
    BuiltinFunctionDef StrStartsWith;
    BuiltinFunctionDef StrEndsWith;
    BuiltinFunctionDef StrIsNumber;
    BuiltinFunctionDef StrIsFloat;
    BuiltinFunctionDef StrIsInt;
    BuiltinFunctionDef StrStrip;
    BuiltinFunctionDef MakeLocalizedString;
    BuiltinFunctionDef SPrintf;
    BuiltinFunctionDef TableLookup;
    BuiltinFunctionDef TableLookupIString;
    BuiltinFunctionDef TableLookupRowNum;
    BuiltinFunctionDef TableLookupColumnForRow;
    BuiltinFunctionDef TableLookupFindCoreAsset;
    BuiltinFunctionDef TableLookupRowCount;
    BuiltinFunctionDef TableLookupColumnCount;
    BuiltinFunctionDef TableLookupRow;
    BuiltinFunctionDef GetItemIndexFromRef;
    BuiltinFunctionDef GetAttachmentTableIndex;
    BuiltinFunctionDef OpenFile;
    BuiltinFunctionDef CloseFile;
    BuiltinFunctionDef FPrintln;
    BuiltinFunctionDef FPrintFields;
    BuiltinFunctionDef FReadLn;
    BuiltinFunctionDef FGetArg;
    BuiltinFunctionDef DebugBreak;
    BuiltinFunctionDef Assert;
    BuiltinFunctionDef AssertMsg;
    BuiltinFunctionDef ErrorMsg;
    BuiltinFunctionDef IsPC;
    BuiltinFunctionDef IsProfileBuild;
    BuiltinFunctionDef IsUsingMods;
    BuiltinFunctionDef Modvar;
  };
  static inline constexpr size_t COUNT = 69;
  BuiltinFunctionDef functions[COUNT];

  static inline constexpr std::array<const char *, COUNT> names = {
      "GetDvarString",
      "GetDvarInt",
      "GetDvarFloat",
      "GetDvarVector",
      "Throw",
      "Array",
      "AssociativeArray",
      "ArrayRemoveValue",
      "ArrayRemoveIndex",
      "ArrayInsert",
      "ArrayCombine",
      "ArrayCopy",
      "IsInArray",
      "GetArrayKeys",
      "GetFirstArrayKey",
      "GetNextArrayKey",
      "GetLastArrayKey",
      "GetPrevArrayKey",
      "ArrayIntersect",
      "ArrayGetClosest",
      "ArrayGetFarthest",
      "GetStructField",
      "IsFunctionPtr",
      "IsString",
      "IsArray",
      "IsWeapon",
      "IsInt",
      "IsFloat",
      "IsVec",
      "IsClass",
      "IString",
      "IsSubStr",
      "GetSubStr",
      "ToLower",
      "ToUpper",
      "StrTok",
      "StrTok2",
      "StrStartsWith",
      "StrEndsWith",
      "StrIsNumber",
      "StrIsFloat",
      "StrIsInt",
      "StrStrip",
      "MakeLocalizedString",
      "SPrintf",
      "TableLookup",
      "TableLookupIString",
      "TableLookupRowNum",
      "TableLookupColumnForRow",
      "TableLookupFindCoreAsset",
      "TableLookupRowCount",
      "TableLookupColumnCount",
      "TableLookupRow",
      "GetItemIndexFromRef",
      "GetAttachmentTableIndex",
      "OpenFile",
      "CloseFile",
      "FPrintln",
      "FPrintFields",
      "FReadLn",
      "FGetArg",
      "DebugBreak",
      "Assert",
      "AssertMsg",
      "ErrorMsg",
      "IsPC",
      "IsProfileBuild",
      "IsUsingMods",
      "Modvar"};

  DEFINE_NAME_MAP(names, hashes);
  static_assert(hashes.size() == names.size(),
                "name hashmap does not include all names!");

  IMPL_TABLE_OPERATORS(functions);
};
ASSERT_SIZE(UtilFunctionTable, sizeof(UtilFunctionTable::functions));
ASSERT_SIZE(UtilFunctionTable, 0x8A0);

} // namespace bg
} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game