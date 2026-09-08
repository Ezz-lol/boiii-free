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

  static const std::array<const char *, COUNT> names;

  DECLARE_NAME_MAP(names, hashes);
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

  static const std::array<const char *, COUNT> names;

  DECLARE_NAME_MAP(names, hashes);
  IMPL_TABLE_OPERATORS(functions);
};
ASSERT_SIZE(MathFunctionTable, sizeof(MathFunctionTable::functions));

union WeaponFunctionTable {
  struct {
    BuiltinFunctionDef EnumerateWeapons;
  };

  static inline constexpr size_t COUNT = 1;
  BuiltinFunctionDef functions[COUNT];

  static const std::array<const char *, COUNT> names;

  DECLARE_NAME_MAP(names, hashes);
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

  static const std::array<const char *, COUNT> names;

  DECLARE_NAME_MAP(names, hashes);
  IMPL_TABLE_OPERATORS(functions);
};
ASSERT_SIZE(UtilFunctionTable, sizeof(UtilFunctionTable::functions));
ASSERT_SIZE(UtilFunctionTable, 0x8A0);

} // namespace bg
} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game
