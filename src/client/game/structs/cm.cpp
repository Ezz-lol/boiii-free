#include <std_include.hpp>

namespace game {
namespace cm {

std::string BulletTraceResults::serialize() const noexcept {
  return std::format("{{ \"trace\": {}, \"hitEnt\": {}, \"hitPos\": {}, "
                     "\"ignoreHitEnt\": {}, "
                     "\"depthSurfaceType\": {} }}",
                     trace.serialize(), ::serialize(hitEnt), hitPos.serialize(),
                     ::serialize(ignoreHitEnt), depthSurfaceType);
}

std::string BulletTrace_Cmd::serialize() const noexcept {
  return std::format(
      "{{ \"localClientNum\": {}, \"bp\": {}, \"weapDef\": {:p}, "
      "\"attacker\": {:p}, "
      "\"br\": {}, \"lastSurfaceType\": {}, \"traceHit\": {}, \"weapon\": "
      "{}, "
      "\"tracerStart\": {}, \"drawTracer\": {}, \"isPlayer\": {}, "
      "\"command_index\": {} }}",
      game::serialize(localClientNum), bp.serialize(),
      reinterpret_cast<const void *>(weapDef),
      reinterpret_cast<const void *>(attacker), br.serialize(), lastSurfaceType,
      ::serialize(traceHit), weapon.serialize(), tracerStart.serialize(),
      ::serialize(drawTracer), ::serialize(isPlayer), command_index);
}

} // namespace cm
} // namespace game
