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
WEAK symbol<void(scriptInstance_t inst, uint32_t id)> Scr_AddObject{
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

WEAK symbol<ScrVarTypePool<const char *>> var_typename{0x1432E6240,
                                                       0x14107C040};
} // namespace var
} // namespace scr
} // namespace game