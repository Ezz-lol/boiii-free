#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> join_agreementWindow{
    0x1556CE890, 0x15574D800, 0x148581330};
WEAK symbol<EngineDependentDvarMut> join_completionWindow{
    0x1556CE8A0, 0x15574D810, 0x148581340};
WEAK symbol<EngineDependentDvarMut> join_handshakeWindow{
    0x1556CE898, 0x15574D808, 0x148581338};
WEAK symbol<EngineDependentDvarMut> join_infoprobeWindow{
    0x1556CE8A8, 0x15574D818, 0x148581348};
WEAK symbol<EngineDependentDvarMut> join_ReservationWindow{
    0x1556CE8C0, 0x15574D830, 0x148581360};
WEAK symbol<EngineDependentDvarMut> join_retryCount{0x1556CE8B8, 0x15574D828,
                                                    0x148581358};
WEAK symbol<EngineDependentDvarMut> join_retryDelay{0x1556CE8B0, 0x15574D820,
                                                    0x148581350};
} // namespace game
