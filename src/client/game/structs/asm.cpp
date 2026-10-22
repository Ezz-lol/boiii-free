#include <std_include.hpp>



namespace game {
namespace anim {
inline std::string animScriptCommand_t::serialize() const noexcept {
  return std::format(
      "{{ \"soundAlias\": {}, \"tagName\": 0x{:08X}, \"bodyPart\": {}, "
      "\"animIndex\": {}, \"animDuration\": {}, \"flags\": 0x{:02X}, "
      "\"motionMatchingEnabled\": {} }}",
      ::serialize(soundAlias), tagName, ::serialize(bodyPart),
      ::serialize(animIndex), ::serialize(animDuration), flags,
      static_cast<bool>(motionMatchingEnabled));
}
} // namespace anim
} // namespace game
