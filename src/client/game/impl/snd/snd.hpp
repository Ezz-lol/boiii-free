#pragma once
#include "../../../game/game.hpp"
#include <cstdint>

namespace game {
namespace snd {

bool SND_StartTocRead_Impl(SndBankLoad *load, SndAssetBankLoad *assetBank,
                           bool streamed);

void SND_EnqueueLoadedAssets_Impl(SndBankLoad *load);

uint32_t SND_AssetBankGetFrameRate_Impl(const SndAssetBankEntry *entry);
uint32_t SND_AssetBankGetLengthMs_Impl(const SndAssetBankEntry *entry);
SndStringHash SND_HashName_Impl(const char *name);
int32_t SND_GetPlaybackTimeById(SndAliasId id);
int32_t SND_GetPlaybackTime_Impl(const char *name);

#ifndef NDEBUG
/*
   When boiii is compiled with debug profile, we store a map of hashed sound
   aliases to their input sound alias name for future lookup in case of debug
   logs, printf debugging, assertions, etc.

   When compiled in release mode, values are never stored in this map, and the
   map itself does not exist. As such, this function is only available for use
   when compiled with debug profile. All usage, correspondingly, must also be
   compiled under the condition that the debug profile is used.
*/
const char *SND_AliasLookup(SndAliasId aliasId);
#endif
} // namespace snd
} // namespace game