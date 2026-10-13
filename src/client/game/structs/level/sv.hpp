#pragma once

#include <game/structs/scr/scr.hpp>

#include <game/structs/ai.hpp>
#include <game/structs/core.hpp>
#include <game/structs/level/core.hpp>
#include <game/structs/lobby/core.hpp>
#include <game/structs/phys/core.hpp>
#include <game/structs/user.hpp>
#include <game/structs/vehicle.hpp>

namespace game {

namespace level {

enum class loading_t : uint32_t {
  DONE = 0x0,
  LEVEL = 0x1,
  SAVEGAME = 0x2,
};

inline constexpr const char *serialize(const loading_t state) {
  switch (state) {
  case loading_t::DONE:
    return "LOADING_DONE";
  case loading_t::LEVEL:
    return "LOADING_LEVEL";
  case loading_t::SAVEGAME:
    return "LOADING_SAVEGAME";
  default:
    return "LOADING_UNKNOWN";
  }
}

struct viewClamp {
  vec2_t start;
  vec2_t current;
  vec2_t goal;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"start\": {}, \"current\": {}, \"goal\": {} }}",
                       start.serialize(), current.serialize(),
                       goal.serialize());
  }
};

struct viewClampState {
  viewClamp min;
  viewClamp max;
  float accelTime;
  float decelTime;
  float totalTime;
  float startTime;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"min\": {}, \"max\": {}, \"accelTime\": {}, \"decelTime\": {}, "
        "\"totalTime\": {}, \"startTime\": {} }}",
        min.serialize(), max.serialize(), accelTime, decelTime, totalTime,
        startTime);
  }
};

enum class sessionState_t : int32_t {
  PLAYING = 0x0,
  DEAD = 0x1,
  SPECTATOR = 0x2,
  INTERMISSION = 0x3,
};

inline constexpr const char *serialize(const sessionState_t state) {
  switch (state) {
  case sessionState_t::PLAYING:
    return "SESSION_PLAYING";
  case sessionState_t::DEAD:
    return "SESSION_DEAD";
  case sessionState_t::SPECTATOR:
    return "SESSION_SPECTATOR";
  case sessionState_t::INTERMISSION:
    return "SESSION_INTERMISSION";
  default:
    return "SESSION_UNKNOWN";
  }
}

enum class clientConnected_t : uint32_t {
  DISCONNECTED = 0x0,
  CONNECTING = 0x1,
  CONNECTED = 0x2,
};

inline constexpr const char *serialize(const clientConnected_t conn) {
  switch (conn) {
  case clientConnected_t::DISCONNECTED:
    return "CONNECTED_DISCONNECTED";
  case clientConnected_t::CONNECTING:
    return "CONNECTED_CONNECTING";
  case clientConnected_t::CONNECTED:
    return "CONNECTED_CONNECTED";
  default:
    return "CONNECTED_UNKNOWN";
  }
}

struct playerTeamState_t {
  int32_t location;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"location\": {} }}", location);
  }
};
ASSERT_SIZE(playerTeamState_t, 4);

/*
  Almost entirely unverified as of initial definition, except for
  those fields with a comment indicating otherwise.

  Verify and adjust as needed prior to usage.
*/
constexpr uint32_t CLIENTSESSION_T_SIZE = 0x540;
PACKED(struct clientSession_t {
  sessionState_t sessionState;
  ClientNum_t forceSpectatorClient;
  int32_t isInKillcam;
  int32_t killCamEntity;
  int32_t killCamTargetEntity;
  uint8_t _padding14[4];
  weapon::Weapon killCamWeapon;
  int32_t killCamMod;
  int32_t killCamFlag;
  int32_t archiveTime;
  scr::var::ScrVarIndex_t scriptPersId;
  clientConnected_t connected;
  user::usercmd_t cmd;
  user::usercmd_t oldcmd;
  bool localClient;
  bool predictItemPickup;
  str<18> newnetname;
  int32_t maxHealth; // Verified
  int32_t enterTime;
  playerTeamState_t teamState;
  int32_t voteCount;
  int32_t teamVoteCount;
  float moveSpeedScaleMultiplier;
  int32_t noSpectate;
  qboolean teamInfo;
  uint8_t _unknown127[16];
  clientState_t cs; // Verified
  int32_t psOffsetTime;
  /*
     Unsure if this is the correct length. Array was expanded to ensure
     correct total struct size (0x540) was met.
     Correct as needed.
  */
  int32_t scoreboardColumnCache[51];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"sessionState\": {}, \"forceSpectatorClient\": {}, "
        "\"isInKillcam\": {}, "
        "\"killCamEntity\": {}, \"killCamTargetEntity\": {}, "
        "\"killCamWeapon\": {}, "
        "\"killCamMod\": {}, \"killCamFlag\": {}, \"archiveTime\": {}, "
        "\"scriptPersId\": {}, \"connected\": {}, \"cmd\": {}, \"oldcmd\": {}, "
        "\"localClient\": {}, \"predictItemPickup\": {}, \"newnetname\": {}, "
        "\"maxHealth\": {}, \"enterTime\": {}, \"teamState\": {}, "
        "\"voteCount\": {}, "
        "\"teamVoteCount\": {}, \"moveSpeedScaleMultiplier\": {}, "
        "\"noSpectate\": {}, "
        "\"teamInfo\": {}, \"cs\": {}, \"psOffsetTime\": {}, "
        "\"scoreboardColumnCache\": {} }}",
        game::level::serialize(sessionState), ::serialize(forceSpectatorClient),
        isInKillcam, killCamEntity, killCamTargetEntity,
        killCamWeapon.serialize(), killCamMod, killCamFlag, archiveTime,
        ::serialize(scriptPersId), game::level::serialize(connected),
        cmd.serialize(), oldcmd.serialize(), ::serialize(localClient),
        ::serialize(predictItemPickup), ::serialize(newnetname), maxHealth,
        enterTime, teamState.serialize(), voteCount, teamVoteCount,
        moveSpeedScaleMultiplier, noSpectate, teamInfo.serialize(),
        cs.serialize(), psOffsetTime, ::serialize(scoreboardColumnCache));
  }
});
ASSERT_OFFSET(clientSession_t, localClient, 0xF4);
ASSERT_OFFSET(clientSession_t, _padding14, 0x14);
ASSERT_OFFSET(clientSession_t, cs, 0x138);
ASSERT_OFFSET(clientSession_t, maxHealth, 0x108);
ASSERT_SIZE(clientSession_t, CLIENTSESSION_T_SIZE);

/*
  Almost entirely unverified  as of initial definition, except for
  those fields with a comment indicating otherwise, and total size.

  Verify and adjust as needed prior to usage.
*/
PACKED(struct gclient_s {
  playerState_t ps;
  playerState_t lastPlayerPS;
  uint8_t _padding16ACC[4];
  clientSession_t sess; // Verified
  ClientNum_t spectatorClient;
  uint8_t _unknown[100];
  int32_t flags;
  int32_t lastCmdTime;
  game_button_bits_t button_bits;
  game_button_bits_t oldbutton_bits;
  game_button_bits_t latched_button_bits;
  game_button_bits_t button_bitsSinceLastFrame;
  float fGunPitch;
  float fGunYaw;
  int32_t damage_blood;
  vec3_t damage_from;
  int32_t damage_fromWorld;
  int32_t inactivityTime;
  int32_t inactivityWarning;
  int32_t lastVoiceTime;
  int32_t outWaterTime;
  int32_t switchSeatTime;
  float currentAimSpreadScale;
  int32_t dropWeaponTime;
  EntHandle pLookatEnt;
  vec4_t prevLinkedInvQuat;
  bool prevLinkAnglesSet;
  bool link_doCollision;
  bool link_useTagAnglesForViewAngles;
  bool link_useBaseAnglesForViewClamp;
  float linkAnglesFrac;
  viewClampState link_viewClamp;
  EntHandle useHoldEntity;
  int32_t useHoldTime;
  int32_t useButtonDone;
  int32_t iLastCompassPlayerInfoEnt;
  int32_t compassPingTime;
  int32_t damageTime;
  float v_dmg_roll;
  float v_dmg_pitch;
  vec3_t swayViewAngles;
  vec3_t swayOffset;
  vec3_t swayAngles;
  vec3_t baseAngles;
  vec3_t baseOrigin;
  vec3_t recoilAngles;
  float fLastIdleFactor;
  int32_t weaponIdleTime;
  vec3_t recoilSpeed;
  int32_t previousRecoilTime;
  float previousRecoilRatio;
  int32_t lastServerTime;
  int32_t lastSpawnTime;
  weapon::Weapon lastWeapon;
  bool previouslyFiring;
  bool previouslyUsingNightVision;
  bool previouslyDTP;
  bool previouslyBeganWeaponRaise;
  bool previouslySprinting;
  uint8_t _padding171DD[3];
  qboolean hasSpyplane;
  qboolean hasSatellite;
  qboolean revive;
  int32_t reviveTime;
  qboolean disallowVehicleUsage;
  uint16_t attachShieldTagName;
  uint8_t _padding171F6[2];
  qboolean lastStand;
  int32_t lastStandTime;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"ps\": {}, \"lastPlayerPS\": {}, \"sess\": {}, "
        "\"spectatorClient\": {}, \"flags\": {:08X}, \"lastCmdTime\": {}, "
        "\"button_bits\": {}, \"oldbutton_bits\": {}, \"latched_button_bits\": "
        "{}, \"button_bitsSinceLastFrame\": {}, \"fGunPitch\": {}, "
        "\"fGunYaw\": {}, \"damage_blood\": {}, \"damage_from\": {}, "
        "\"damage_fromWorld\": {}, \"inactivityTime\": {}, "
        "\"inactivityWarning\": {}, \"lastVoiceTime\": {}, \"outWaterTime\": "
        "{}, \"switchSeatTime\": {}, \"currentAimSpreadScale\": {}, "
        "\"dropWeaponTime\": {}, \"pLookAtEnt\": {}, "
        "\"prevLinkedInvQuat\": {}, \"prevLinkAnglesSet\": {}, "
        "\"link_doCollision\": {}, \"link_useTagAnglesForViewAngles\": {}, "
        "\"link_useBaseAnglesForViewClamp\": {}, \"linkAnglesFrac\": {}, "
        "\"link_viewClamp\": {}, \"useHoldEntity\": {}, \"useHoldTime\": {}, "
        "\"useButtonDone\": {}, \"iLastCompassPlayerInfoEnt\": {}, "
        "\"compassPingTime\": {}, \"damageTime\": {}, \"v_dmg_roll\": {}, "
        "\"v_dmg_pitch\": {}, \"swayViewAngles\": {}, \"swayOffset\": {}, "
        "\"swayAngles\": {}, \"baseAngles\": {}, \"baseOrigin\": {}, "
        "\"recoilAngles\": {}, \"fLastIdleFactor\": {}, \"weaponIdleTime\": "
        "{}, \"recoilSpeed\": {}, \"previousRecoilTime\": {}, "
        "\"previousRecoilRatio\": {}, \"lastServerTime\": {}, "
        "\"lastSpawnTime\": {}, \"lastWeapon\": {}, \"previouslyFiring\": {}, "
        "\"previouslyUsingNightVision\": {}, \"previouslyDTP\": {}, "
        "\"previouslyBeganWeaponRaise\": {}, \"previouslySprinting\": {}, "
        "\"hasSpyplane\": {}, \"hasSatellite\": {}, \"revive\": {}, "
        "\"reviveTime\": {}, \"disallowVehicleUsage\": {}, "
        "\"attachShieldTagName\": {:04X}, \"lastStand\": {}, "
        "\"lastStandTime\": {} }}",
        ps.serialize(), lastPlayerPS.serialize(), sess.serialize(),
        ::serialize(spectatorClient), flags, lastCmdTime,
        button_bits.serialize(), oldbutton_bits.serialize(),
        latched_button_bits.serialize(), button_bitsSinceLastFrame.serialize(),
        fGunPitch, fGunYaw, damage_blood, damage_from.serialize(),
        damage_fromWorld, inactivityTime, inactivityWarning, lastVoiceTime,
        outWaterTime, switchSeatTime, currentAimSpreadScale, dropWeaponTime,
        pLookatEnt.serialize(), prevLinkedInvQuat.serialize(),
        ::serialize(prevLinkAnglesSet), ::serialize(link_doCollision),
        ::serialize(link_useTagAnglesForViewAngles),
        ::serialize(link_useBaseAnglesForViewClamp), linkAnglesFrac,
        link_viewClamp.serialize(), useHoldEntity.serialize(), useHoldTime,
        useButtonDone, iLastCompassPlayerInfoEnt, compassPingTime, damageTime,
        v_dmg_roll, v_dmg_pitch, swayViewAngles.serialize(),
        swayOffset.serialize(), swayAngles.serialize(), baseAngles.serialize(),
        baseOrigin.serialize(), recoilAngles.serialize(), fLastIdleFactor,
        weaponIdleTime, recoilSpeed.serialize(), previousRecoilTime,
        previousRecoilRatio, lastServerTime, lastSpawnTime,
        lastWeapon.serialize(), ::serialize(previouslyFiring),
        ::serialize(previouslyUsingNightVision), ::serialize(previouslyDTP),
        ::serialize(previouslyBeganWeaponRaise),
        ::serialize(previouslySprinting), hasSpyplane.serialize(),
        hasSatellite.serialize(), revive.serialize(), reviveTime,
        disallowVehicleUsage.serialize(), attachShieldTagName,
        lastStand.serialize(), lastStandTime);
  }
});
typedef gclient_s gclient_t;
// FIXME: correct size is 0x171F0. This struct needs to be corrected.
ASSERT_SIZE(gclient_s, 0x17200);
ASSERT_OFFSET(gclient_s, sess, 0x16AD0);

PACKED(struct SpawnVar {
  bool spawnVarsValid;
  uint8_t _padding01[3];
  uint32_t numSpawnVars;
  char *spawnVars[100][2];
  uint32_t numSpawnVarChars;
  char spawnVarChars[2048];
  char spawnVarTypes[100];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"spawnVarsValid\": {}, \"numSpawnVars\": {}, \"spawnVars\": {}, "
        "\"numSpawnVarChars\": {}, \"spawnVarChars\": \"{}\", "
        "\"spawnVarTypes\": {} }}",
        ::serialize(spawnVarsValid), numSpawnVars, "TODO", numSpawnVarChars,
        "TODO", ::serialize(spawnVarTypes));
  }
});
ASSERT_SIZE(SpawnVar, 0xEB0);

struct archivedEntity_t;

struct gentity_snd_wait {
  uint32_t notifyString;
  uint32_t index;
  bool stoppable;
  uint8_t _padding09[3];
  int32_t basetime;
  int32_t duration;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"notifyString\": {:08X}, \"index\": {}, \"stoppable\": {}, "
        "\"basetime\": {}, \"duration\": {} }}",
        notifyString, index, ::serialize(stoppable), basetime, duration);
  }
};

struct gclient_s;
static const uint32_t GENTITY_SIZE = 0x4F8;
static const uint32_t GENTITY_MODEL_OFFSET = 0x280;
static const uint32_t GENTITY_SND_WAIT_OFFSET = 0x3D4;

struct gentity_s;
typedef gentity_s gentity_t;

PACKED(struct flame_timed_damage_t {
  gentity_t *attacker;
  int32_t damage;
  float damageDuration;
  float damageInterval;
  int32_t start_timestamp;
  int32_t end_timestamp;
  int32_t lastupdate_timestamp;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"attacker\": {}, \"damage\": {}, \"damageDuration\": {}, "
        "\"damageInterval\": {}, \"start_timestamp\": {}, \"end_timestamp\": "
        "{}, "
        "\"lastupdate_timestamp\": {} }}",
        ::serialize(attacker), damage, damageDuration, damageInterval,
        start_timestamp, end_timestamp, lastupdate_timestamp);
  }
});
ASSERT_SIZE(flame_timed_damage_t, 0x20);

struct item_ent_t {
  uint32_t ammoCount;
  uint32_t clipAmmoCount;
  weapon::Weapon weapon;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"ammoCount\": {}, \"clipAmmoCount\": {}, \"weapon\": {} }}",
        ammoCount, clipAmmoCount, weapon.serialize());
  }
};
ASSERT_SIZE(item_ent_t, 0x10);

PACKED(struct trigger_ent_t {
  int32_t threshold;
  int32_t accumulate;
  int32_t timestamp;
  int32_t singleUserEntIndex;
  uint8_t perk;
  uint8_t _padding11[3];
  int32_t exposureIndex;
  float exposureLerpToLighter;
  float exposureLerpToDarker;
  vec3_t exposureFeather;
  int16_t flags;
  uint8_t playerPrompts[4];
  bool requireAllPlayers;
  uint8_t _padding33[1];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"threshold\": {}, \"accumulate\": {}, \"timestamp\": {}, "
        "\"singleUserEntIndex\": {}, \"perk\": {}, \"exposureIndex\": {}, "
        "\"exposureLerpToLighter\": {}, \"exposureLerpToDarker\": {}, "
        "\"exposureFeather\": {}, \"flags\": {:04X}, \"playerPrompts\": {}, "
        "\"requireAllPlayers\": {} }}",
        threshold, accumulate, timestamp, singleUserEntIndex, perk,
        exposureIndex, exposureLerpToLighter, exposureLerpToDarker,
        exposureFeather.serialize(), flags, ::serialize(playerPrompts),
        ::serialize(requireAllPlayers));
  }
});
ASSERT_SIZE(trigger_ent_t, 0x34);

struct mover_slidedata_t {
  vec3_t mins;
  vec3_t maxs;
  vec3_t velocity;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"mins\": {}, \"maxs\": {}, \"velocity\": {} }}",
                       mins.serialize(), maxs.serialize(),
                       velocity.serialize());
  }
};

struct mover_positions_t {
  float decelTime;
  float speed;
  float midTime;
  vec3_t pos1;
  vec3_t pos2;
  vec3_t pos3;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"decelTime\": {}, \"speed\": {}, \"midTime\": {}, "
                       "\"pos1\": {}, \"pos2\": {}, \"pos3\": {} }}",
                       decelTime, speed, midTime, pos1.serialize(),
                       pos2.serialize(), pos3.serialize());
  }
};
ASSERT_SIZE(mover_positions_t, 0x30);

struct mover_ent_t {
  union {
    mover_positions_t pos;
    mover_slidedata_t slide;
  };
  mover_positions_t angle;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"pos\": {}, \"angle\": {} }}", pos.serialize(),
                       angle.serialize());
  }
};

struct corpse_ent_t {
  int32_t deathAnimStartTime;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"deathAnimStartTime\": {} }}", deathAnimStartTime);
  }
};

PACKED(struct missile_ent_t {
  vec3_t predictLandPos;
  int32_t predictLandTime;
  int32_t timestamp;
  float time;
  int32_t timeOfBirth;
  float travelDist;
  vec3_t surfaceNormal;
  union {
    struct {
      uint8_t forcedDud : 1;
      uint8_t rolling : 1;
    };
    uint8_t _raw;
  };
  uint8_t flags;
  uint8_t _padding2E[2];
  int32_t antilagTimeOffset;
  int32_t timeAlive;
  union {
    struct {
      vec3_t curvature;
      vec3_t targetOffset;
      union {
        struct {
          MissileStage stage;
          MissileFlightMode flightMode;
        } missile;
        struct {
          float pitch;
          float yaw;
          float frequency;
        } grenade;
      };
    };

    int32_t effectIndex;
  };
  float grenadeWobbleCycle;
  float grenadeCurve;
  int32_t destructibleBounceCount;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"predictLandPos\": {}, \"predictLandTime\": {}, "
                       "\"timestamp\": {}, "
                       "\"time\": {}, \"timeOfBirth\": {}, \"travelDist\": {}, "
                       "\"surfaceNormal\": {}, "
                       "\"forcedDud\": {}, \"rolling\": {}, \"flags\": {:02X}, "
                       "\"antilagTimeOffset\": {}, "
                       "\"timeAlive\": {}, \"curvature\": {}, "
                       "\"targetOffset\": {}, \"effectIndex\": {}, "
                       "\"grenadeWobbleCycle\": {}, \"grenadeCurve\": {}, "
                       "\"destructibleBounceCount\": {} }}",
                       predictLandPos.serialize(), predictLandTime, timestamp,
                       time, timeOfBirth, travelDist, surfaceNormal.serialize(),
                       static_cast<bool>(forcedDud), static_cast<bool>(rolling),
                       flags, antilagTimeOffset, timeAlive,
                       curvature.serialize(), targetOffset.serialize(),
                       effectIndex, grenadeWobbleCycle, grenadeCurve,
                       destructibleBounceCount);
  }
});
ASSERT_SIZE(missile_ent_t, 0x68);

struct blend_ent_t {
  vec3_t pos;
  vec3_t vel;
  vec4_t viewQuat;
  bool changed;
  float posAccelTime;
  float posDecelTime;
  float angleAccelTime;
  float angleDecelTime;
  float startTime;
  float posTotalTime;
  float angleTotalTime;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"pos\": {}, \"vel\": {}, \"viewQuat\": {}, \"changed\": {}, "
        "\"posAccelTime\": {}, \"posDecelTime\": {}, \"angleAccelTime\": {}, "
        "\"angleDecelTime\": {}, \"startTime\": {}, \"posTotalTime\": {}, "
        "\"angleTotalTime\": {} }}",
        pos.serialize(), vel.serialize(), viewQuat.serialize(),
        ::serialize(changed), posAccelTime, posDecelTime, angleAccelTime,
        angleDecelTime, startTime, posTotalTime, angleTotalTime);
  }
};

struct actor_ent_t {
  int32_t spawnTime;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"spawnTime\": {} }}", spawnTime);
  }
};

struct spawner_ent_t {
  int32_t timestamp;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"timestamp\": {} }}", timestamp);
  }
};

PACKED(struct zbarrier_piece_t {
  uint8_t state;
  uint8_t scalar;
  uint8_t nextState;
  uint8_t _padding03[1];
  int32_t startTime;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"state\": {}, \"scalar\": {}, \"nextState\": {}, "
                       "\"startTime\": {} }}",
                       state, scalar, nextState, startTime);
  }
});
ASSERT_SIZE(zbarrier_piece_t, 0x8);

struct zbarrier_ent_t {
  zbarrier_piece_t pieces[6];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"pieces\": {} }}", ::serialize(pieces));
  }
};

PACKED(struct tagInfo_t {
  gentity_t *parent;
  gentity_t *next;
  scr::ScrString_t name;
  bool blendToParent;
  bool blendOnlyYaw;
  bool collisionPhysics;
  uint8_t _padding17[1];
  BoneIndex index;
  uint8_t _padding1A[2];
  vec3_t axis[4];
  vec3_t parentInvAxis[4];
  uint8_t _padding7C[4];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"parent\": {}, \"next\": {}, \"name\": {:08X}, "
        "\"blendToParent\": {}, "
        "\"blendOnlyYaw\": {}, \"collisionPhysics\": {}, \"index\": {}, "
        "\"axis\": {}, \"parentInvAxis\": {} }}",
        ::serialize(parent), ::serialize(next), name,
        ::serialize(blendToParent), ::serialize(blendOnlyYaw),
        ::serialize(collisionPhysics), ::serialize(index), ::serialize(axis),
        ::serialize(parentInvAxis));
  }
});

PACKED(struct EntityModelAttachment {
  modelNameIndex_t model;
  uint8_t _padding02[2];
  scr::ScrString_t tag;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"model\": {}, \"tag\": {:08X} }}",
                       ::serialize(model), tag);
  }
});

PACKED(struct gentity_s {
  entityState_t s;
  entityShared_t r;
  gclient_t *client;
  user::actor_t *actor;
  ai::sentient_t *sentient;
  phys::Destructible *destructible;
  vehicle::vehicle_t *vehicle;
  user::bot_t *bot;
  modelNameIndex_t model;
  bool physicsObject;
  bool takedamage;
  bool active;
  bool nopickup;
  bool handler;
  bool avoidHandle;
  scr::ScrString_t classname;
  scr::ScrString_t target;
  scr::ScrString_t targetname;
  scr::ScrString_t script_noteworthy;
  scr::ScrString_t animname;
  scr::ScrString_t script_animname;
  uint32_t attachIgnoreCollision;
  int32_t spawnflags;
  int32_t flags;
  int32_t eventTime;
  qboolean freeAfterEvent;
  qboolean unlinkAfterEvent;
  contents_t clipmask;
  int32_t processedFrame;
  EntHandle parent;
  int32_t nextthink;
  int32_t health;    // Verified
  int32_t maxHealth; // Verified
  int32_t damage;
  uint8_t _padding2D4[4];
  flame_timed_damage_t flame_timed_damage[4];
  int32_t last_timed_radius_damage;
  int32_t count;
  gentity_t *activator;
  union {
    item_ent_t item[2];
    trigger_ent_t trigger;
    mover_ent_t mover;
    corpse_ent_t corpse;
    missile_ent_t missile;
    blend_ent_t blend;
    actor_ent_t actorInfo;
    spawner_ent_t spawner;
    zbarrier_ent_t zbarrier;
  };
  union {
    EntHandle missileTargetEnt;
    EntHandle grenadeOriginalOwner;
  };
  gentity_snd_wait snd_wait;
  gentity_t *tagChildren;
  tagInfo_t *tagInfo;
  anim::animscripted_t *scripted;
  EntityModelAttachment attachments[19];
  anim::XAnimTree *pAnimTree;
  uint16_t disconnectedLinks;
  uint8_t _padding4A2[2];
  int32_t iDisconnectTime;
  phys::PhysObjId physObjId;
  gentity_t *nextFree;
  int32_t birthTime;
  int32_t ikPlayerclipTerrainTime;
  int32_t ikDisableTerrainMappingTime;
  uint32_t shieldAttachmentIndex;
  weapon::DelayedWeaponFire delayedFire;
  HavokObj_t *havokObj;
  PosedEntity_PoseHandle instancePoseHandle;
  int32_t teleportTime;
  int32_t debugRenderTime;
  char tmodeTimeOut;
  uint8_t _padding4F1[7];

  inline constexpr bool dead() const noexcept { return health <= 0; }

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"s\": {}, \"r\": {}, \"client\": {}, \"actor\": {}, "
        "\"sentient\": {:p}, \"destructible\": {}, \"vehicle\": {}, "
        "\"bot\": {}, "
        "\"model\": {}, \"physicsObject\": {}, \"takedamage\": {}, "
        "\"active\": {}, \"nopickup\": {}, \"handler\": {}, \"avoidHandle\": "
        "{}, \"classname\": {:08X}, \"target\": {:08X}, \"targetname\": "
        "{:08X}, \"script_noteworthy\": {:08X}, "
        "\"animname\": {:08X}, \"script_animname\": {:08X}, "
        "\"attachIgnoreCollision\": {:08X}, "
        "\"spawnflags\": {:08X}, \"flags\": {:08X}, \"eventTime\": {}, "
        "\"freeAfterEvent\": {}, "
        "\"unlinkAfterEvent\": {}, \"clipmask\": {:08X}, \"processedFrame\": "
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
        "{}, \"nextFree\": {:p}, "
        "\"birthTime\": {}, \"ikPlayerclipTerrainTime\": {}, "
        "\"ikDisableTerrainMappingTime\": {}, "
        "\"shieldAttachmentIndex\": {:08X}, \"delayedFire\": {}, "
        "\"havokObj\": "
        "{:p}, "
        "\"instancePoseHandle\": {:016X}, \"teleportTime\": {}, "
        "\"debugRenderTime\": {}, \"tmodeTimeOut\": {} }}",
        s.serialize(), r.serialize(), ::serialize(client), ::serialize(actor),
        reinterpret_cast<const void *>(sentient), ::serialize(destructible),
        ::serialize(vehicle), ::serialize(bot), ::serialize(model),
        ::serialize(physicsObject), ::serialize(takedamage),
        ::serialize(active), ::serialize(nopickup), ::serialize(handler),
        ::serialize(avoidHandle), classname, target, targetname,
        script_noteworthy, animname, script_animname, attachIgnoreCollision,
        spawnflags, flags, eventTime, freeAfterEvent.serialize(),
        unlinkAfterEvent.serialize(), clipmask, processedFrame,
        parent.serialize(), nextthink, health, maxHealth, damage,
        ::serialize(flame_timed_damage), last_timed_radius_damage, count,
        ::serialize(activator), missileTargetEnt.serialize(),
        snd_wait.serialize(), reinterpret_cast<const void *>(tagChildren),
        reinterpret_cast<const void *>(tagInfo), ::serialize(scripted),
        ::serialize(attachments), ::serialize(pAnimTree), disconnectedLinks,
        iDisconnectTime, ::serialize(physObjId),
        reinterpret_cast<const void *>(nextFree), birthTime,
        ikPlayerclipTerrainTime, ikDisableTerrainMappingTime,
        shieldAttachmentIndex, delayedFire.serialize(),
        reinterpret_cast<const void *>(havokObj), instancePoseHandle,
        teleportTime, debugRenderTime, tmodeTimeOut);
  }
});

constexpr uint32_t GENTITY_POOL_LEN = 2048;
struct gentity_pool {
  gentity_t pool[GENTITY_POOL_LEN];
};

#ifdef __cplusplus
ASSERT_OFFSET(gentity_s, model, GENTITY_MODEL_OFFSET);
ASSERT_OFFSET(gentity_s, classname, 0x288);
ASSERT_OFFSET(gentity_s, snd_wait, GENTITY_SND_WAIT_OFFSET);
ASSERT_OFFSET(gentity_s, s, 0);
ASSERT_OFFSET(gentity_s, client, 0x250);
ASSERT_OFFSET(gentity_s, vehicle, 0x270);
ASSERT_OFFSET(gentity_s, eventTime, 0x2AC);
ASSERT_OFFSET(gentity_s, r, 0x1F0);
ASSERT_OFFSET(gentity_s, takedamage, 0x283);
ASSERT_OFFSET(gentity_s, health, 0x2C8);
ASSERT_OFFSET(gentity_s, maxHealth, 0x2CC);
ASSERT_SIZE(gentity_s, GENTITY_SIZE);
#endif

// level_locals_t has size 0x23A10 on both client and server
PACKED(struct level_locals_t {
  gclient_t *clients;
  gentity_t *gentities;
  int32_t gentitySize;
  int32_t num_entities;
  gentity_t *firstFreeEnt;
  gentity_t *lastFreeEnt;
  gentity_t *firstFreeFakeEnt;
  gentity_t *lastFreeFakeEnt;
  int32_t numFakeEntities;
  uint8_t _padding3C[4];
  vehicle::vehicle_t *vehicles;
  gentity_t *firstFreeActor;
  gentity_t *lastFreeActor;
  int32_t num_actors;
  uint8_t _padding5C[4];
  ai::sentient_t *sentients;
  user::actor_t *actors;
  int32_t actorCorpseCount;
  uint8_t _padding74[4];
  user::bot_t *bots;
  int32_t maxBots;
  bool initializing;
  bool clientIsSpawning;
  uint8_t _padding86[2];
  int32_t maxclients;
  lobby::LobbyClientPool<int32_t> teamSortedClients;
  uint8_t _paddingD4[12];
  objective_t objectives[128];
  int32_t objectivesClientMask[128][2];
  int32_t framenum;
  int32_t time;
  int32_t previousTime;
  int32_t frametime;
  int32_t startTime;
  uint8_t _padding64F4[4];
  SpawnVar spawnVar;
  EntHandle droppedWeaponCue[32];
  qboolean savepersist;
  float fFogOpaqueDist;
  float fFogOpaqueDistSqrd;
  bool bPlayerIgnoreRadiusDamage;
  bool bPlayerIgnoreRadiusDamageLatched;
  uint8_t _padding7436[2];
  int32_t currentEntityThink;
  qboolean bRegisterLeaderboards;
  phys::cached_tag_mat_t cachedTagMat;
  phys::cached_tag_mat_t cachedEntTargetTagMat;
  phys::trigger_info_t pendingTriggerList[256];
  uint8_t triggerPlayerUsageCount[1024];
  phys::trigger_info_t currentTriggerList[256];
  int32_t pendingTriggerListSize;
  int32_t currentTriggerListSize;
  int32_t scriptPrintChannel;
  vec2_t compassMapUpperLeft;
  vec2_t compassMapLowerRight;
  vec2_t compassMapWorldSize;
  vec2_t compassMapCurrentUpperLeft;
  vec2_t compassMapCurrentLowerRight;
  vec2_t compassMapCurrentWorldSize;
  vec2_t compassNorth;
  int32_t finished;
  bool manualNameChange;
  uint8_t _padding9101[1];
  modelNameIndex_t modelMap[8192];
  uint8_t _paddingD102[2];
  loading_t loading;
  int32_t script_ai_limit;
  uint32_t grenadeHintCount;
  uint8_t specialIndex[1024];
  phys::actor_prone_info_t cgData_actorProneInfo[96];
  uint8_t cgData_actorOnCompass[64];
  uint8_t cgData_actorTeam[64];
  bool bDrawCompassFriendlies;
  bool disable_grenade_suicide;
  uint8_t _paddingDE92[2];
  int32_t numConnectedClients;
  lobby::LobbyClientPool<ClientNum_t> sortedClients;
  qboolean bUpdateScoresForIntermission;
  int32_t numVotingClients;
  uint8_t _paddingDEE8[8];
  MatchState matchState;
  CasterState casterState;
  CasterClientState casterClients;
  int32_t teamScores[21];
  int32_t lastTeammateHealthTime;
  uint32_t teamHasSpyplane[21];
  uint32_t teamHasSatellite[21];
  int32_t teamSpyplaneTime[21];
  int32_t teamSatelliteTime[21];
  char voteString[1024];
  char voteDisplayString[1024];
  int32_t voteTime;
  int32_t voteExecuteTime;
  int32_t voteYes;
  int32_t voteNo;
  int32_t currentActorClone;
  int32_t iSearchFrame;
  uint8_t _paddingEB9C[4];
  phys::sv_FxVisBlock_t fxVisibilityEnts[32];
  int32_t currentPlayerClone;
  qboolean hostMigrationActive;
  int32_t hostMigrationStart;
  phys::actorAntilagFrame_t actorAntilagFrames[20];
  int32_t nextActorAntilagFrame;
  vehicle::vehicleAntilagFrame_t vehicleAntilagFrames[20];
  int32_t nextVehicleAntilagFrame;
  uint8_t _padding1F254[4];
  phys::ZBarrierType *zbarrierTypes[255];
  qboolean checkAnimChange;
  phys::objcamCameraTable objectiveCameras;
  uint8_t _unknown[0x7AC];
});
ASSERT_SIZE(level_locals_t, 0x23A10);

// Verified
PACKED(struct clientSnapshot_t {
  playerState_t ps;
  uint8_t _paddingB56[2];
  team_t clientTeam;
  int32_t destructibleCount;
  int32_t matchStateIndex;
  int32_t entityCount;
  int32_t clientCount;
  int32_t actorCount;
  int32_t casterStateIndex;
  int32_t firstDestructibleIndex;
  int32_t firstCasterClientIndex;
  int32_t firstEntityIndex;
  int32_t firstClientIndex;
  int32_t firstActorIndex;
  int32_t messageSent;
  int32_t messageAcked;
  int32_t messageSize;
  int32_t serverTime;
  int32_t physicsTime;
  int32_t timeDelta;
  qboolean baselineSnap;
  int32_t snapFlags;
});
ASSERT_SIZE(clientSnapshot_t, 0xB5B8);

} // namespace level
} // namespace game
