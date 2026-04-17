#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

namespace game {
namespace scr {

uint32_t Scr_CanonHash(const char *str);

WEAK symbol<scr::scr_const_t> scr_const{0x14A7290F0, 0x147B91CE0};

WEAK symbol<void(scr::scriptInstance_t inst, int value)> Scr_AddInt{
    0x1412E9870, 0x14016F160};
WEAK symbol<void(scr::scriptInstance_t inst, const char *value)> Scr_AddString{
    0x0, 0x14016F320};
WEAK symbol<const char *(scr::scriptInstance_t inst, uint32_t index)>
    Scr_GetString{0x0, 0x140171490};
WEAK symbol<void(level::gentity_s *ent, ScrVarCanonicalName_t stringValue,
                 uint32_t paramcount)>
    Scr_Notify_Canon{0x0, 0x1402F5FF0};
WEAK symbol<uint32_t(scr::scriptInstance_t inst)> Scr_GetNumParam{0x0,
                                                                  0x140171320};
WEAK symbol<uint32_t(scr::scriptInstance_t inst, const char *filename)>
    Scr_LoadScript{0x1412C83F0, 0x140156610};
WEAK symbol<void(scr::scriptInstance_t inst, int user)> Scr_BeginLoadScripts{
    0x1412C7DF0, 0x140156010};

} // namespace scr
} // namespace game

#endif