#pragma once

#include "game/structs/scr/builtin/core.hpp"

namespace game {
namespace scr {
namespace builtin {
namespace table {
namespace bg {
union CommonFunctionTable {
  struct {
    const BuiltinFunctionDef ClearAllCharacterTables;
    const BuiltinFunctionDef GetCharacterBodyStyleIndex;
    const BuiltinFunctionDef GetAllCharacterBodies;
    const BuiltinFunctionDef GetAllCharacterHeads;
    const BuiltinFunctionDef GetCharacterBodyModelCount;
    const BuiltinFunctionDef GetCharacterBodyModelColorCount;
    const BuiltinFunctionDef GetCharacterHelmetModelCount;
    const BuiltinFunctionDef GetCharacterHelmetModelColorCount;
    const BuiltinFunctionDef GetAnimForCharacter;
    const BuiltinFunctionDef GetXCamForCharacter;
    const BuiltinFunctionDef GetSpawnStructForCharacter;
    const BuiltinFunctionDef GetWeaponModelForCharacter;
    const BuiltinFunctionDef GetWeaponForCharacter;
    const BuiltinFunctionDef GetCharacterFields;
    const BuiltinFunctionDef GetCharacterDisplayName;
    const BuiltinFunctionDef GetCharacterAssetName;
    const BuiltinFunctionDef GetCharacterBodyModel;
    const BuiltinFunctionDef GetCharacterHelmetModel;
    const BuiltinFunctionDef GetCharacterHelmetHidesHead;
    const BuiltinFunctionDef GetCharacterHeadModel;
    const BuiltinFunctionDef GetCharacterBodyRenderOptions;
    const BuiltinFunctionDef GetCharacterHelmetRenderOptions;
    const BuiltinFunctionDef GetCharacterHeadRenderOptions;
    const BuiltinFunctionDef GetCharacterModeRenderOptions;
    const BuiltinFunctionDef GetXModelCenterOffset;
    const BuiltinFunctionDef GetNextMap;
    const BuiltinFunctionDef GetMapOrder;
    const BuiltinFunctionDef GetMapAtIndex;
    const BuiltinFunctionDef GetMapIntroMovie;
    const BuiltinFunctionDef GetMapOutroMovie;
    const BuiltinFunctionDef GetMapFields;
    const BuiltinFunctionDef SetupFieldOpsKitLoadouts;
    const BuiltinFunctionDef SessionModeIsCampaignZombiesGame;
    const BuiltinFunctionDef IsArchetypeLoaded;
  };
  const BuiltinFunctionDef functions[34];

  static constexpr const char *names[34] = {
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
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(CommonFunctionTable, sizeof(CommonFunctionTable::functions));

union MathFunctionTable {
  struct {
    const BuiltinFunctionDef Log;
    const BuiltinFunctionDef Sin;
    const BuiltinFunctionDef Cos;
    const BuiltinFunctionDef Tan;
    const BuiltinFunctionDef Asin;
    const BuiltinFunctionDef Acos;
    const BuiltinFunctionDef Atan;
    const BuiltinFunctionDef Abs;
    const BuiltinFunctionDef Min;
    const BuiltinFunctionDef Max;
    const BuiltinFunctionDef Floor;
    const BuiltinFunctionDef Ceil;
    const BuiltinFunctionDef Sqrt;
    const BuiltinFunctionDef Pow;
    const BuiltinFunctionDef LerpFloat;
    const BuiltinFunctionDef LerpVector;
    const BuiltinFunctionDef GetMinBitCountForNum;
    const BuiltinFunctionDef MapFloat;
    const BuiltinFunctionDef DiffTrack;
    const BuiltinFunctionDef DiffTrackAngle;
    const BuiltinFunctionDef PackRgba;
    const BuiltinFunctionDef Int;
    const BuiltinFunctionDef CastFloat;
    const BuiltinFunctionDef Distance2DSquared;
    const BuiltinFunctionDef AngleLerp;
    const BuiltinFunctionDef RotatePoint;
    const BuiltinFunctionDef VectorFromLineToPoint;
    const BuiltinFunctionDef PointOnSegmentNearestToPoint;
    const BuiltinFunctionDef Distance;
    const BuiltinFunctionDef Distance2D;
    const BuiltinFunctionDef DistanceSquared;
    const BuiltinFunctionDef Length;
    const BuiltinFunctionDef LengthSquared;
    const BuiltinFunctionDef Closer;
    const BuiltinFunctionDef VectorDot;
    const BuiltinFunctionDef VectorCross;
    const BuiltinFunctionDef VectorNormalize;
    const BuiltinFunctionDef VectorProjection;
    const BuiltinFunctionDef VectorToAngles;
    const BuiltinFunctionDef VectorLerp;
    const BuiltinFunctionDef AnglesToUp;
    const BuiltinFunctionDef AnglesToRight;
    const BuiltinFunctionDef AnglesToForward;
    const BuiltinFunctionDef CombineAngles;
    const BuiltinFunctionDef AngleClamp180;
    const BuiltinFunctionDef AbsAngleClamp180;
    const BuiltinFunctionDef AbsAngleClamp360;
    const BuiltinFunctionDef ForwardRotateAroundUpByAngle;
    const BuiltinFunctionDef RotatePointAroundAxis;
  };
  const BuiltinFunctionDef functions[49];

  static constexpr const char *names[49] = {
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
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(MathFunctionTable, sizeof(MathFunctionTable::functions));

union WeaponFunctionTable {
  struct {
    const BuiltinFunctionDef EnumerateWeapons;
  };
  const BuiltinFunctionDef functions[1];

  static constexpr const char *names[1] = {"EnumerateWeapons"};
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(WeaponFunctionTable, sizeof(WeaponFunctionTable::functions));

union UtilFunctionTable {
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

  static constexpr const char *names[69] = {"GetDvarString",
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
                                            "Sprintf",
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
  static constexpr const frozen::unordered_set<fnv1aHashNull_t,
                                               ARRAYSIZE(names)>
      hashes = make_frozen_set(fnv1a<ARRAYSIZE(names)>(names));
};
ASSERT_SIZE(UtilFunctionTable, sizeof(UtilFunctionTable::functions));
ASSERT_SIZE(UtilFunctionTable, 0x8A0);

} // namespace bg
} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game