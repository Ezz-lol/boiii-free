#pragma once

#include "game/structs/core.hpp"
#include "game/structs/quake/core.hpp"
#include "user.hpp"
#include "phys/core.hpp"
#include "cm.hpp"

namespace game {

namespace level {
struct playerState_s;
typedef playerState_s playerState_t;

struct gentity_s;
typedef gentity_s gentity_t;

struct clientState_s;
typedef clientState_s clientState_t;

struct entityState_s;
typedef entityState_s entityState_t;

struct MatchState;
} // namespace level

namespace vehicle {
struct VehicleDef;
}

namespace move {

enum class pmoveVehAnimState_t : uint32_t {
  PLAYING = 0x0,
  END_STAGE = 0x1,
  COMPLETE = 0x2,
};

typedef phys::colgeom_visitor_inlined_t<300> character_proximity_data_t;

struct pmove_t {
  level::playerState_t *ps;
  user::usercmd_t cmd;
  user::usercmd_t oldcmd;
  contents_t tracemask;
  int numtouch;
  int touchents[32];
  int numGlassTouch;
  int touchGlasses[32];
  vec3_t mins;
  vec3_t maxs;
  float xyspeed;
  float averagePitch;
  int nextPitch;
  float pitchHistory[30];
  qboolean proneChange;
  float maxSprintTimeMultiplier;
  bool mantleStarted;
  uint8_t _padding279[3];
  vec3_t mantleEndPos;
  int mantleDuration;
  vec3_t meleeEntOrigin;
  vec3_t meleeEntAngles;
  vec3_t meleeEntVelocity;
  int viewChangeTime;
  float viewChange;
  vec3_t vehicleAngles;
  pmoveVehAnimState_t vehAnimState;
  uint8_t handler;
  uint8_t _padding2C9[3];
  LocalClientNum_t localClientNum;
  character_proximity_data_t proximity_data;
  bool gravity;
  uint8_t _padding1621[3];
  int gravityOverride;
  int events[18];
};

PACKED(struct pml_t {
  vec3_t forward;
  vec3_t right;
  vec3_t up;
  float frametime;
  int msec;
  int walking;
  int groundPlane;
  int almostGroundPlane;
  cm::trace_t groundTrace;
  float impactSpeed;
  vec3_t previous_origin;
  vec3_t previous_velocity;
  unsigned int holdrand;
});
ASSERT_SIZE(pml_t, 0xA8);

typedef fastcallPtr_t<void(LocalClientNum_t localClientNum, uint32_t,
                           const level::playerState_t *ps, uint32_t)>
    pmove_glassTouch;
typedef fastcallPtr_t<void(const phys::moveclip_t *, cm::trace_t *)>
    pmove_glassClipMoveTrace;
typedef fastcallPtr_t<const level::MatchState *(
    LocalClientNum_t localClientNum)>
    pmove_getMatchState;
typedef fastcallPtr_t<const level::clientState_t *(
    LocalClientNum_t localClientNum, int)>
    pmove_getClientState;
typedef fastcallPtr_t<const weapon::entityGadgetState_t *(
    const LocalClientNum_t localClientNum, int, weapon::gadgetHeldSlot_e)>
    pmove_getGadgetState;
typedef fastcallPtr_t<void(LocalClientNum_t localClientNum, int, vec3_t)>
    pmove_setEntityPosition;
typedef fastcallPtr_t<qboolean(LocalClientNum_t localClientNum, int,
                               scr::ScrString_t, vec3_t *, vec3_t *)>
    pmove_getEntityWorldTagPositionAngles;
typedef fastcallPtr_t<qboolean(LocalClientNum_t localClientNum, int,
                               scr::ScrString_t, vec3_t *)>
    pmove_getEntityWorldTagPosition;
typedef fastcallPtr_t<const level::entityState_t *(
    LocalClientNum_t localClientNum, int)>
    pmove_getEntityState;
typedef fastcallPtr_t<qboolean(const LocalClientNum_t localClientNum,
                               const int)>
    pmove_isEntWalkable;
typedef fastcallPtr_t<void(const LocalClientNum_t localClientNum,
                           level::playerState_t *ps, user::usercmd_t *)>
    pmove_setVehDriverInput;
typedef fastcallPtr_t<void(int, int, const scr::ScrString_t)>
    pmove_entityLinkFromPMove;
typedef fastcallPtr_t<scr::ScrString_t(ClientNum_t, int,
                                       vehicle::VehicleDef *def)>
    pmove_getVehicleTypeString;
typedef fastcallPtr_t<bool(const LocalClientNum_t localClientNum, int,
                           vec3_t *)>
    pmove_getEntityAngularVelocity;
typedef fastcallPtr_t<bool(const LocalClientNum_t localClientNum, int,
                           vec3_t *)>
    pmove_getEntityVelocity;
typedef fastcallPtr_t<bool(const LocalClientNum_t localClientNum, int, vec3_t *,
                           vec3_t *)>
    pmove_getEntityOriginAngles;
typedef fastcallPtr_t<void(int, int)> pmove_PlayerEvent;
typedef fastcallPtr_t<void(cm::trace_t *, const vec3_t *, const vec3_t *,
                           const vec3_t *, const vec3_t *, int, contents_t,
                           phys::col_context_t *)>
    pmove_trace;

} // namespace move
} // namespace game