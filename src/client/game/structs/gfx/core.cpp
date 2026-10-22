#include <std_include.hpp>

namespace game {

namespace gfx {
inline std::string GfxSkyBox::serialize() const noexcept {
  return std::format("{{ \"model\": {}, \"rotation\": {}, \"size\": {}, "
                     "\"dynamicCookie\": {}, \"image\": {} }}",
                     ::serialize(model), rotation, size,
                     ::serialize(dynamicCookie), ::serialize(image));
}
} // namespace gfx
} // namespace game
