#pragma once

#include <cstdint>
namespace game {
namespace weapon {
// sizeof=0x8
union Weapon {
  struct {
    uint64_t weaponIdx : 9;
    uint64_t attachment1 : 6;
    uint64_t attachment2 : 6;
    uint64_t attachment3 : 6;
    uint64_t attachment4 : 6;
    uint64_t attachment5 : 6;
    uint64_t attachment6 : 6;
    uint64_t attachment7 : 6;
    uint64_t attachment8 : 6;
    uint64_t padding : 7;
  } _anon_0;

  uint64_t weaponData;
};
static_assert(sizeof(Weapon) == 0x8, "Weapon union size is not 8 bytes");

// sizeof=0x4
union AttachmentCosmeticVariantIndexes {
  struct {
    uint32_t attachmentCosmeticVariantIndex1 : 4;
    uint32_t attachmentCosmeticVariantIndex2 : 4;
    uint32_t attachmentCosmeticVariantIndex3 : 4;
    uint32_t attachmentCosmeticVariantIndex4 : 4;
    uint32_t attachmentCosmeticVariantIndex5 : 4;
    uint32_t attachmentCosmeticVariantIndex6 : 4;
    uint32_t attachmentCosmeticVariantIndex7 : 4;
    uint32_t attachmentCosmeticVariantIndex8 : 4;
  };
  uint32_t data;
};
static_assert(sizeof(AttachmentCosmeticVariantIndexes) == 0x4,
              "AttachmentCosmeticVariantIndexes size must be 4 bytes");
} // namespace weapon
} // namespace game