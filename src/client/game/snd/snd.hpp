#pragma once
#include "game/game.hpp"
#include "game/structs/structs.hpp"

namespace game {
namespace snd {

bool SND_StartTocRead_Impl(SndBankLoad *load, SndAssetBankLoad *assetBank,
                           bool streamed);

void SND_EnqueueLoadedAssets_Impl(SndBankLoad *load);

sd_byte *SD_Alloc_BasicImpl(const char *name, uint32_t size, uint32_t align);
void SD_Free_Impl(sd_byte *ptr);

} // namespace snd
} // namespace game