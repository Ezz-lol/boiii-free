#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace ui {

namespace lua {
namespace hks {
// Not present in dedicated server.
WEAK symbol<lua_State *> UI_luaVM{0x159C76D88};
WEAK symbol<lua_State *> s_lobbyLuaVM{0x1557588D0, 0x14858C408};
/*
   s_lobbyLuaVM static allocation.
   Is typed as `char s_luaMem[0x400000]` in engine - never used as a `lua_State`
   directly, but `s_lobbyLuaVM` always contains a pointer to `s_luaMem`.
*/
WEAK symbol<bytearray<0x400000>> s_luaMem{0x1557588F0, 0x14858C420};
WEAK symbol<lua_State *> primary_luaVM{0x159C76D88, 0x14858C408};

WEAK symbol<void(lua_State *s, const char *str, hksUint32 l)>
    hksi_lua_pushlstring{0x140A18430, 0x1401DE6F0};

WEAK symbol<void(lua_State *s, const HksObject *tbl, const HksObject *key,
                 const HksObject *val)>
    hks_obj_settable{0x141D4B660, 0x1403F41B0};
WEAK symbol<HksObject *(HksObject *result, lua_State *s, const HksObject *table,
                        const HksObject *key)>
    hks_obj_gettable{0x141D4ABF0, 0x1403F3750};
WEAK symbol<HashTable *(lua_State *s, hksUint32 arraySize, hksUint32 hashSize)>
    Hashtable_Create{0x141D3B5F0, 0x1403E46D0};
WEAK symbol<cclosure *(lua_State *s, lua_function function,
                       hksInt32 num_upvalues, hksInt32 internal_,
                       hksInt32 profilerTreatClosureAsFunc)>
    cclosure_Create{0x141D3B7E0, 0x1403E48C0};
WEAK symbol<int(lua_State *s, hksInt32 t)> hksi_luaL_ref{0x141D4D1A0,
                                                         0x1403F5CF0};
WEAK symbol<void(lua_State *s, hksInt32 t, hksInt32 ref)> hksi_luaL_unref{
    0x141D4D320, 0x1403F5E70};

WEAK symbol<int(lua_State *s, const HksCompilerSettings *options,
                const char *buff, hksSize sz, const char *name)>
    hksi_hksL_loadbuffer{0x141D4BD80, 0x1403F48D0};
WEAK symbol<int(lua_State *s, const char *what, lua_Debug *ar)>
    hksi_lua_getinfo{0x141D4D8D0, 0x1403F64B0};
WEAK symbol<int(lua_State *s, hksInt32 level, lua_Debug *ar)> hksi_lua_getstack{
    0x141D4DB90, 0x1403F6770};
WEAK symbol<void(lua_State *s, const char *fmt, ...)> hksi_luaL_error{
    0x141D4D050, 0x1403F5BA0};
WEAK symbol<const char *(lua_State *s, hksInt32 index, size_t *len)>
    hksi_lua_tolstring{0x141D4B6C0, 0x1403F42B0};
WEAK symbol<ObjectTypePool<const char *>> s_compilerTypeName{0x14337B4B8,
                                                             0x14110B5C8};

// `hks::execute<(HksBytecodeSharingMode)0>`
WEAK symbol<luaReturnCount_e(lua_State *s, const hksInstruction *PC,
                             hksInt32 call_depth)>
    execute0{0x142F82EB0, 0x140DC5EB0};
// `hks::execute<(HksBytecodeSharingMode)1>`
WEAK symbol<luaReturnCount_e(lua_State *s, const hksInstruction *PC,
                             hksInt32 call_depth)>
    execute1{0x142F7D000, 0x140DC0000};
// `hks::execute<(HksBytecodeSharingMode)2>`
WEAK symbol<luaReturnCount_e(lua_State *s, const hksInstruction *PC,
                             hksInt32 call_depth)>
    execute2{0x142F7FEB0, 0x140DC2EB0};

WEAK symbol<void(lua_State *s, hksInt64 nargs, hksInt32 nresults,
                 const hksInstruction *PC)>
    hksi_lua_call{0x141D70FE0, 0x140418E40};

WEAK symbol<hksInt32(lua_State *s, const HksCompilerSettings *options,
                     lua_Reader reader, void *data, lua_Reader debugReader,
                     void *debugData, const char *chunkName)>
    Compiler{0x141D3AFB0, 0x1403E4090};

WEAK symbol<HksRegister> NilValue{0x150BBCC40, 0x14820D720};
WEAK symbol<std::remove_pointer<lua_CFunction>::type> hks_next_placeholder{
    0x141D4A710, 0x1404C6450};

WEAK symbol<Libs> libs{0x143001340, 0x140E420E0};
WEAK symbol<PkgLoaders> pkg_loaders{0x143000990, 0x140E42590};
WEAK symbol<MathFunctions> math_functions{0x1430007C0, 0x140E41D80};

WEAK symbol<HksRegister *(HksRegister *retstr, const lua_State *s,
                          MetaTable *mt, Metamethod m)>
    HKS_METATABLE_GET{0x141D3B8F0, 0x1403E49D0};
WEAK symbol<HksRegister *(HksRegister *retstr, lua_State *s,
                          const HksRegister *table, const HksRegister *key,
                          const hksInstruction *PC, const HksRegister *h,
                          hksUint32 counter)>
    gettable_event_string_outofline_tail{0x141D48470, 0x1403F1010};
WEAK symbol<void(lua_State *s, HksRegister *table, hks::InternString *key,
                 const HksObject *value, const hksInstruction *PC,
                 hksUint32 counter)>
    settable_event_outofline_string{0x141D6B0A0, 0x1404131B0};
WEAK symbol<HksRegister *(HksRegister *retstr, lua_State *s,
                          const HksRegister *table, const HksRegister *key,
                          const hksInstruction *PC, hksUint32 counter)>
    gettable_event_string_outofline_other{0x141D48130, 0x1403F0CD0};
WEAK symbol<HksObject *(lua_State *s, HksObject *func, HksObject *arg_end)>
    non_function_call{0x141D55A10, 0x1403FE4D0};
WEAK symbol<void(lua_State *s, HksObject *lfp)> closePendingUpvalues{
    0x141D3E170, 0x1403E7250};
WEAK symbol<HksRegister *(HksRegister *retstr, lua_State *s,
                          const HksObject *op1, const HksObject *op2,
                          Metamethod event)>
    getcomphandler{0x141D47450, 0x1403EFFF0};
WEAK symbol<hksInt32(InternString *o1, InternString *o2)> compareString{
    0x141D3E270, 0x1403E7350};
WEAK symbol<HksRegister *(HksRegister *retstr, lua_State *s,
                          const HksObject *bv, const HksObject *cv,
                          hks::Metamethod event, const char *op)>
    getrelhandler{0x141D47660, 0x1403F0200};
WEAK symbol<HksRegister *(HksRegister *retstr, lua_State *s,
                          const HksRegister *table, const HksRegister *key,
                          const hksInstruction *PC, hksUint32 counter)>
    gettable_event_string_outofline_table{0x141D48370, 0x1403F0F10};
WEAK symbol<HksRegister *(HksRegister *retstr, lua_State *s,
                          const HksRegister *table, const HksObject *key,
                          const hksInstruction *PC, hksUint32 counter)>
    gettable_event_outofline_other{0x141D47C30, 0x1403F07D0};
WEAK symbol<HksRegister *(HksRegister *retstr, lua_State *s,
                          const HksRegister *table, const HksObject *key,
                          const hksInstruction *PC, const HksRegister *h,
                          hksUint32 counter)>
    gettable_event_outofline_tail{0x141D47F60, 0x1403F0B00};
WEAK symbol<void(lua_State *s, HksRegister *table, const HksObject *key,
                 const HksObject *value, const hksInstruction *PC,
                 hksUint32 counter)>
    settable_event_outofline{0x141D6A830, 0x140412940};

WEAK symbol<void(lua_State *s, const HksObject *bv, const HksObject *cv,
                 hks::Metamethod event, const char *op,
                 const hksInstruction *pc)>
    callBinaryHandler{0x141D3D2C0, 0x1403E63A0};
WEAK symbol<hksUint32(hksByte input)> floatingpointbyte2uint{0x141D44B90,
                                                             0x1403ED7B0};
WEAK symbol<void(lua_State *s, int32_t n)> hksi_lua_concat{0x141D4D430,
                                                           0x1403F5F80};
WEAK symbol<hksInt32(const char *strptr, hksSize len, HksNumber *dd)>
    stringToNumber{0x141D6C9E0, 0x140414AF0};
WEAK symbol<HksObject *(const hks::HashTable *, HksObject *value, hksInt32 key)>
    hks_obj_rawgeti{0x141D451E0, 0x1403EDDF0};
WEAK symbol<HksClosure *(lua_State *s, Method *method, HashTable *globals)>
    createHksClosure{0x141D3F280, 0x1403E8250};
WEAK symbol<void *(lua_State *s, hksSize size, AllocTypes allocType)> getMemory{
    0x141D364C0, 0x1403DF860};

WEAK symbol<std::remove_pointer<lua_CFunction>::type> vm_intern_inext{
    0x141D712E0, 0x140419140};
WEAK symbol<std::remove_pointer<lua_CFunction>::type> vm_intern_next{
    0x141D713A0, 0x140419200};

namespace op {
WEAK symbol<HksRegister *(HksRegister *retstr, lua_State *s, HksObject *left,
                          HksObject *right, const hksInstruction *PC)>
    OpcodeAdd{0x141D36C60, 0x1403E0000};
WEAK symbol<HksRegister *(HksRegister *retstr, lua_State *s, HksObject *left,
                          HksObject *right, const hksInstruction *PC)>
    OpcodeBitAnd{0x141D36DB0, 0x1403E0150};
WEAK symbol<HksRegister *(HksRegister *retstr, lua_State *s, HksObject *left,
                          HksObject *right, const hksInstruction *PC)>
    OpcodeBitOr{0x141D36F10, 0x1403E02B0};
WEAK symbol<HksRegister *(HksRegister *retstr, lua_State *s, HksObject *left,
                          HksObject *right, const hksInstruction *PC)>
    OpcodeDiv{0x141D37070, 0x1403E0410};
WEAK symbol<HksRegister *(HksRegister *retstr, lua_State *s, HksObject *left,
                          HksObject *right, const hksInstruction *PC)>
    OpcodeLShift{0x141D371C0, 0x1403E0560};
WEAK symbol<HksRegister *(HksRegister *retstr, lua_State *s, HksObject *left,
                          HksObject *right, const hksInstruction *PC)>
    OpcodeMod{0x141D37320, 0x1403E06C0};
WEAK symbol<HksRegister *(HksRegister *retstr, lua_State *s, HksObject *left,
                          HksObject *right, const hksInstruction *PC)>
    OpcodeMul{0x141D374A0, 0x1403E0840};
WEAK symbol<HksRegister *(HksRegister *retstr, lua_State *s, HksObject *left,
                          HksObject *right, const hksInstruction *PC)>
    OpcodePow{0x141D375F0, 0x1403E0990};
WEAK symbol<HksRegister *(HksRegister *retstr, lua_State *s, HksObject *left,
                          HksObject *right, const hksInstruction *PC)>
    OpcodeRShift{0x141D37750, 0x1403E0AF0};
WEAK symbol<HksRegister *(HksRegister *retstr, lua_State *s, HksObject *left,
                          HksObject *right, const hksInstruction *PC)>
    OpcodeSub{0x141D378B0, 0x1403E0C50};
WEAK symbol<HksRegister *(HksRegister *retstr, lua_State *s, HksObject *left,
                          HksObject *right, const hksInstruction *PC)>
    OpcodeUnm{0x141D37A00, 0x1403E0DA0};
} // namespace op
} // namespace hks
} // namespace lua
} // namespace ui
} // namespace game

