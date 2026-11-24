// Stock Multiplayer only has its join, quit, damage and kill log lines in developer blocks, so they're compiled out

#using scripts\shared\callbacks_shared;
#using scripts\shared\system_shared;

#insert scripts\shared\shared.gsh;

#namespace game_log;

REGISTER_SYSTEM("game_log", &__init__, undefined)

function __init__()
{
  if (IsDedicated()) {
    callback::on_start_gametype(&init);
    callback::on_connect(&on_player_connect);
    callback::on_disconnect(&on_player_disconnect);
  }
}

// callback::on_player_damage and on_player_killed don't pass the attacker or weapon
function init()
{
  level.game_log_player_damage = level.callbackPlayerDamage;
  level.game_log_player_killed = level.callbackPlayerKilled;
  level.callbackPlayerDamage = &callback_player_damage;
  level.callbackPlayerKilled = &callback_player_killed;
}

function string_join(strings, delim = "") {
  result = "";

  if (isarray(strings) && strings.size > 0) {
    result = strings[0];

    for (stringIdx = 1; stringIdx < strings.size; stringIdx++) {
      result += delim + strings[stringIdx];
    }
  }

  return result;
}

function build_game_log(parts) {
  return string_join(parts, ";") + "\n";
}

function log_build_print(parts) {
  return LogPrint(build_game_log(parts));
}

function on_player_connect()
{
  log_build_print(array("J", self GetXuid(), self GetEntityNumber(), self.name));
}

function on_player_disconnect()
{
  log_build_print(array("Q", self GetXuid(), self GetEntityNumber(), self.name));
}

function log_hit(type, attacker, iDamage, sMeansOfDeath, weapon, sHitLoc)
{
  attackerGuid = "";
  attackerNum = -1;
  attackerTeam = "world";
  attackerName = "";
  if (IsPlayer(attacker))
  {
    attackerGuid = attacker GetXuid();
    attackerNum = attacker GetEntityNumber();
    attackerTeam = attacker.team;
    attackerName = attacker.name;
  }

  log_build_print(array(type, self GetXuid(), self GetEntityNumber(), self.team, self.name, attackerGuid, attackerNum, attackerTeam, attackerName, weapon.name, iDamage, sMeansOfDeath, sHitLoc));
}

function callback_player_damage(eInflictor, eAttacker, iDamage, iDFlags, sMeansOfDeath, weapon, vPoint, vDir, sHitLoc, vDamageOrigin, psOffsetTime, boneIndex, vSurfaceNormal)
{
  // The stock callback ignores teammate hits with friendly fire off
  teammateHit = level.teambased && IsPlayer(eAttacker) && eAttacker != self && eAttacker.team == self.team;
  if (self.sessionstate != "dead" && (!teammateHit || level.friendlyfire)) {
    log_hit("D", eAttacker, iDamage, sMeansOfDeath, weapon, sHitLoc);
  }

  self [[ level.game_log_player_damage ]](eInflictor, eAttacker, iDamage, iDFlags, sMeansOfDeath, weapon, vPoint, vDir, sHitLoc, vDamageOrigin, psOffsetTime, boneIndex, vSurfaceNormal);
}

function callback_player_killed(eInflictor, attacker, iDamage, sMeansOfDeath, weapon, vDir, sHitLoc, psOffsetTime, deathAnimDuration, enteredResurrect)
{
  if (game["state"] != "postgame" && self.sessionteam != "spectator") {
    log_hit("K", attacker, iDamage, sMeansOfDeath, weapon, sHitLoc);
  }

  self [[ level.game_log_player_killed ]](eInflictor, attacker, iDamage, sMeansOfDeath, weapon, vDir, sHitLoc, psOffsetTime, deathAnimDuration, enteredResurrect);
}
