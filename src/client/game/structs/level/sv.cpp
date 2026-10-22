#include <std_include.hpp>



namespace game {
namespace level {
std::string flame_timed_damage_t::serialize() const noexcept {
  return std::format(
      "{{ \"attacker\": {}, \"damage\": {}, \"damageDuration\": {}, "
      "\"damageInterval\": {}, \"start_timestamp\": {}, \"end_timestamp\": "
      "{}, "
      "\"lastupdate_timestamp\": {} }}",
      ::serialize(attacker), damage, damageDuration, damageInterval,
      start_timestamp, end_timestamp, lastupdate_timestamp);
}

std::string tagInfo_t::serialize() const noexcept {
  return std::format(
      "{{ \"parent\": {}, \"next\": {}, \"name\": 0x{:08X}, "
      "\"blendToParent\": {}, "
      "\"blendOnlyYaw\": {}, \"collisionPhysics\": {}, \"index\": 0x{:04X}, "
      "\"axis\": {}, \"parentInvAxis\": {} }}",
      ::serialize(parent), ::serialize(next), name, ::serialize(blendToParent),
      ::serialize(blendOnlyYaw), ::serialize(collisionPhysics), index,
      ::serialize(axis), ::serialize(parentInvAxis));
}

std::string gentity_s::serialize() const noexcept {
  return std::format(
      "{{ \"s\": {}, \"r\": {}, \"client\": {}, \"actor\": {:p}, "
      "\"sentient\": {:p}, \"destructible\": {}, \"vehicle\": {}, "
      "\"bot\": {}, "
      "\"model\": 0x{:04X}, \"physicsObject\": {}, \"takedamage\": {}, "
      "\"active\": {}, \"nopickup\": {}, \"handler\": {}, \"avoidHandle\": "
      "{}, \"classname\": 0x{:08X}, \"target\": 0x{:08X}, \"targetname\": "
      "{:08X}, \"script_noteworthy\": 0x{:08X}, "
      "\"animname\": 0x{:08X}, \"script_animname\": 0x{:08X}, "
      "\"attachIgnoreCollision\": 0x{:08X}, "
      "\"spawnflags\": 0x{:08X}, \"flags\": 0x{:08X}, \"eventTime\": {}, "
      "\"freeAfterEvent\": {}, "
      "\"unlinkAfterEvent\": {}, \"clipmask\": 0x{:08X}, \"processedFrame\": "
      "{}, \"parent\": {}, "
      "\"nextthink\": {}, \"health\": {}, \"maxHealth\": {}, \"damage\": "
      "{}, "
      "\"flame_timed_damage\": {}, \"last_timed_radius_damage\": {}, "
      "\"count\": {}, "
      "\"activator\": {}, \"missileTargetEnt\": {}, \"snd_wait\": {}, "
      "\"tagChildren\": {:p}, "
      "\"tagInfo\": {:p}, \"scripted\": {}, \"attachments\": {}, "
      "\"pAnimTree\": {}, "
      "\"disconnectedLinks\": {}, \"iDisconnectTime\": {}, \"physObjId\": "
      "0x{:016X}, \"nextFree\": {:p}, "
      "\"birthTime\": {}, \"ikPlayerclipTerrainTime\": {}, "
      "\"ikDisableTerrainMappingTime\": {}, "
      "\"shieldAttachmentIndex\": 0x{:08X}, \"delayedFire\": {}, "
      "\"havokObj\": "
      "{:p}, "
      "\"instancePoseHandle\": 0x{:016X}, \"teleportTime\": {}, "
      "\"debugRenderTime\": {}, \"tmodeTimeOut\": {} }}",
      s.serialize(), r.serialize(), ::serialize(client),
      reinterpret_cast<const void *>(actor),
      reinterpret_cast<const void *>(sentient), ::serialize(destructible),
      ::serialize(vehicle), ::serialize(bot), model, ::serialize(physicsObject),
      ::serialize(takedamage), ::serialize(active), ::serialize(nopickup),
      ::serialize(handler), ::serialize(avoidHandle), classname, target,
      targetname, script_noteworthy, animname, script_animname,
      attachIgnoreCollision, spawnflags, flags, eventTime,
      freeAfterEvent.serialize(), unlinkAfterEvent.serialize(), clipmask,
      processedFrame, parent.serialize(), nextthink, health, maxHealth, damage,
      ::serialize(flame_timed_damage), last_timed_radius_damage, count,
      ::serialize(activator), missileTargetEnt.serialize(),
      snd_wait.serialize(), reinterpret_cast<const void *>(tagChildren),
      reinterpret_cast<const void *>(tagInfo), ::serialize(scripted),
      ::serialize(attachments), ::serialize(pAnimTree), disconnectedLinks,
      iDisconnectTime, physObjId, reinterpret_cast<const void *>(nextFree),
      birthTime, ikPlayerclipTerrainTime, ikDisableTerrainMappingTime,
      shieldAttachmentIndex, delayedFire.serialize(),
      reinterpret_cast<const void *>(havokObj), instancePoseHandle,
      teleportTime, debugRenderTime, tmodeTimeOut);
}

} // namespace level
} // namespace game
