#include <std_include.hpp>

#include <game/game.hpp>

namespace game {
namespace db {
namespace xasset {
namespace xmodel {
std::string XModel::serialize() const noexcept {
  return std::format(
      "{{ \"name\": {}, \"numBones\": {}, \"numRootBones\": {}, "
      "\"numCosmeticBones\": {}, "
      "\"boneNames\": {}, \"parentList\": {:p}, \"quats\": {}, "
      "\"trans\": {}, "
      "\"partClassification\": {:p}, \"baseMat\": {:p}, \"lodParams\": {}, "
      "\"highestShadowLod\": {}, "
      "\"highestRenderLod\": {}, \"meshes\": {}, \"meshMaterials\": {}, "
      "\"usage\": {}, "
      "\"collSurfs\": {}, \"numCollSurfs\": {}, \"contents\": 0x{:08X}, "
      "\"boneInfo\": {}, "
      "\"radius\": {}, \"mins\": {}, \"maxs\": {}, \"collLod\": {}, "
      "\"numCollmaps\": {}, \"useMe\": {}, \"flags\": 0x{:08X}, \"_unknown\": "
      "{}, "
      "\"physPreset\": {}, \"collmaps\": {}, \"physConstraints\": {}, "
      "\"numSubmodels\": {}, \"submodels\": {}, \"nameHash\": 0x{:08X}, "
      "\"combinedPartBits\": {} }}",
      ::serialize(name), numBones, numRootBones, numCosmeticBones,
      ::serialize(boneNames, numBones),
      reinterpret_cast<const void *>(parentList), ::serialize(quats),
      ::serialize(trans), reinterpret_cast<const void *>(partClassification),
      reinterpret_cast<const void *>(baseMat), lodParams.serialize(),
      db::xasset::xmodel::serialize(highestShadowLod),
      db::xasset::xmodel::serialize(highestRenderLod), ::serialize(meshes),
      ::serialize(meshMaterials), usage, ::serialize(collSurfs, numCollSurfs),
      numCollSurfs, contents, ::serialize(boneInfo), radius, mins.serialize(),
      maxs.serialize(), ::serialize(collLod), numCollmaps, useMe, flags,
      _unknown, ::serialize(physPreset), ::serialize(collmaps, numCollmaps),
      ::serialize(physConstraints), numSubmodels,
      ::serialize(submodels, numSubmodels), nameHash,
      ::serialize(combinedPartBits));
}
} // namespace xmodel
} // namespace xasset
} // namespace db
} // namespace game
