#include <std_include.hpp>
#include "scr.hpp"

#include <utils/string.hpp>

namespace game {
namespace scr {
void Scr_PlaySoundAtPosition_Impl(scriptInstance_t inst) {
  vec3_t origin;
  Scr_GetVector(SCRIPTINSTANCE_SERVER, 1u, &origin);
  if (!game::bg::BG_ValidateOrigin(&origin, &sv::svs->mapCenter)) {
    Scr_NetworkOriginError("sound", &origin);
  }
  const char *alias = Scr_GetString(SCRIPTINSTANCE_SERVER, 0);
  snd::SndAliasId alias_id = snd::SND_FindAliasId(alias);
  G_PlaySoundAliasAtPoint(&origin, alias_id);
}
} // namespace scr
} // namespace game