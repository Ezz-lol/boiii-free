#pragma once
#include "game/game.hpp"
#include "game/structs/structs.hpp"
#include <cstdint>

namespace game {
namespace snd {

bool SND_StartTocRead_Impl(SndBankLoad *load, SndAssetBankLoad *assetBank,
                           bool streamed);

void SND_EnqueueLoadedAssets_Impl(SndBankLoad *load);

sd_byte *SD_Alloc_BasicImpl(const char *name, uint32_t size, uint32_t align);
void SD_Free_Impl(sd_byte *ptr);

uint32_t SND_AssetBankGetFrameRate_Impl(const SndAssetBankEntry *entry);
uint32_t SND_AssetBankGetLengthMs_Impl(const SndAssetBankEntry *entry);
SndStringHash SND_HashName_Impl(const char *name);
int32_t SND_GetPlaybackTimeById(SndAliasId id);
int32_t SND_GetPlaybackTime_Impl(const char *name);
void G_RegisterSoundWait_Impl(game::level::gentity_s *ent, SndAliasId id,
                              game::scr::ScrString_t notifyString);
} // namespace snd
} // namespace game