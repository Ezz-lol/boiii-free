#include <std_include.hpp>

#include "gfx.hpp"

namespace game {
namespace gfx {
bool GfxImage_AssignDefaultTexture_Impl(GfxImage *to, GfxImage *from) {
  if (nonnull(from) && nonnull(from->texture.basemap)) {
    if (nonnull(to->texture.basemap) &&
        !Gfx_TexturePool_Contains(to->texture)) {
      if (game::is_client()) {
        Gfx_TexturePool_ReleaseRef(to->texture, 0);
      }
      to->texture.release();
    }
    to->texture = from->texture;
    to->texture.add_ref();
    if (game::is_client()) {
      Gfx_TexturePool_AddRef(to->texture, 0);
    }
    if (nonnull(to->fallbackTexture.basemap)) {
      if (game::is_client()) {
        Gfx_TexturePool_ReleaseRef(to->fallbackTexture, 0);
      }
      to->fallbackTexture.release();
      to->fallbackTexture.basemap = nullptr;
    }
    if (nonnull(from->fallbackTexture.basemap)) {
      to->fallbackTexture = from->fallbackTexture;
      to->fallbackTexture.add_ref();
      if (game::is_client()) {
        Gfx_TexturePool_AddRef(to->fallbackTexture, 0);
      }
    }
    return true;
  }

  return false;
}
} // namespace gfx
} // namespace game
