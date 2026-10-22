#include <std_include.hpp>

namespace game {
namespace db {
namespace xasset {
namespace destructible {

std::string DestructibleStage::serialize() const noexcept {
  return std::format(
      "{{ \"showBone\": 0x{:08X}, \"cosmeticShowBones\": {}, \"breakHealth\": "
      "{}, "
      "\"maxTime\": {}, \"flags\": 0x{:08X}, \"baseEffect\": {:p}, "
      "\"baseEffectTag\": 0x{:08X}, "
      "\"breakAnim\": 0x{:08X}, \"baseEffectTag\": 0x{:08X}, \"breakEffect\": "
      "{:p}, \"breakEffectTag\": 0x{:08X}, "
      "\"breakSound\": {}, \"breakNotify\": {}, \"loopSound\": {}, "
      "\"spawnModel\": {}, "
      "\"physPreset\": {:p} }}",
      showBone, ::serialize(cosmeticShowBones), breakHealth, maxTime, flags,
      reinterpret_cast<const void *>(baseEffect), baseEffectTag, breakAnim,
      baseEffectTag, reinterpret_cast<const void *>(breakEffect),
      breakEffectTag, ::serialize(breakSound), ::serialize(breakNotify),
      ::serialize(loopSound), ::serialize(spawnModel),
      reinterpret_cast<const void *>(physPreset));
}

std::string DestructibleDef::serialize() const noexcept {
  return std::format("{{ \"name\": {}, \"model\": {}, \"pristineModel\": "
                     "{}, \"numPieces\": {}, "
                     "\"pieces\": {}, \"clientOnly\": {}, "
                     "\"syncBaseHealthWithEntity\": {} }}",
                     ::serialize(name), ::serialize(model),
                     ::serialize(pristineModel), numPieces,
                     ::serialize(pieces, numPieces), clientOnly.serialize(),
                     syncBaseHealthWithEntity.serialize());
}

} // namespace destructible
} // namespace xasset
} // namespace db
} // namespace game
