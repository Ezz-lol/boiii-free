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
} // namespace snd
} // namespace game