///////////////////////////////////////////////////////////////////////////////
//
// 									Tweakables
//
///////////////////////////////////////////////////////////////////////////////

// Chance that a destructible piece will gib off when the robot is
// killed, regardless of hitlocation, this applies to all pieces. (0, 1)
#define ROBOT_DESTRUCT_DEATH_CHANCE				0.2

// Maximum number of destructible pieces that will randomly gib off
// when the robot is killed.
#define ROBOT_DESTRUCT_MAX_DEATH_PIECES			2

// Percentage of health before robot arms and head can be gibbed off.
// Valid float range (0, 1)
#define ROBOT_GIB_HEALTH_THRESHOLD				0.75

// Chance the robot's head will gib off when killed with a headshot.
// Valid float range (0, 1)
#define ROBOT_GIB_HEAD_HEADSHOT_CHANCE			1

// Chance the robot's head will gib off when damaged with an explosion.
// Valid float range (0, 1)
#define ROBOT_GIB_HEAD_EXPLOSION_CHANCE			0.5

// Chance the robot's head will gib off when dealth a fatal blow.
#define ROBOT_GIB_HEAD_DEATH_CHANCE				0.25

// Percentage of health before the robots leg can be gibbed off. (0, 1)
#define ROBOT_GIB_LEG_HEALTH_THRESHOLD			0.25

// Chance the robot's legs will gib off when killed with a leg shot.
// Valid float range (0, 1)
#define ROBOT_GIB_LEGS_CHANCE					1

// Chance the robot's legs will gib off randomly when killed (0, 1)
#define ROBOT_GIB_LEGS_DEATH_CHANCE				0.25

// Chance the robot's legs will gib off when killed with an explosion.
// Valid float range (0, 1)
#define ROBOT_GIB_LEGS_EXPLOSION_CHANCE			0.5

// Chance the robot's limb will gib off when killed with an explosion. (0, 1)
#define ROBOT_GIB_LIMB_EXPLOSION_CHANCE			0.25

// Chance the robot's limb will gib off when killed with a limb shot. (0, 10
#define ROBOT_GIB_LIMB_DEATH_CHANCE				0.25

// Chance the robot will choose a cover position over an exposed position.
#define ROBOT_CHOOSE_COVER_CHANCE				0.75

// Minimum amount of time in seconds robots will step out of cover for.
#define MIN_EXPOSED_TIME						4

// Maximum amount of time in seconds robots will step out of cover for.
#define MAX_EXPOSED_TIME						8

// Damage modifier when robots are shot in the head.
#define ROBOT_HEADSHOT_MULTIPLIER				0.5

// Damage modifier when robots are shot in the back.
#define ROBOT_BACKSHOT_MULTIPLIER				2

// Time in milliseconds robots will remain exposed after being shot when popping out of cover.
#define ROBOT_RESUME_COVER_TIME					4000

// Robot must be within this distance to their enemy to cause them to juke.
#define ROBOT_JUKE_MAX_DISTANCE					1200

// Robot must be within this distance to their enemy to preemptive juke
#define ROBOT_JUKE_PREEMPTIVE_MAX_DISTANCE		600

// Robot must be within this distance to their attacker to become a crawler.
#define ROBOT_CRAWL_MAX_DISTANCE				(600 * 600)

#define ROBOT_INVALID_COVER_DISTANCE			240

// Robots must be at least this distance away from their enemy to become a rusher.
#define ROBOT_RUSHER_DISTANCE_SQ				(600 * 600)

// Robots within this distance may turn into rushers, if their fellow robot dies.
#define ROBOT_RUSHER_NEIGHBOR_DISTANCE_SQ		(400 * 400)

// Robots' whose enemy is further than this distance will not be considered for rushing.
#define ROBOT_RUSHER_MAX_ENEMY_DISTANCE_SQ		(1200 * 1200)

// Robots will walk to positions even within sprint range if the path is shorter than this.
#define ROBOT_WALK_MIN_DISTANCE					240

// Robots will start to super sprint outside this distance to their target.
#define ROBOT_SUPER_SPRINT_DISTANCE				300

// Amount of time robots will stay inactive after being damaged by an emp.
#define ROBOT_EMP_SHUTDOWN_TIME					2000

///////////////////////////////////////////////////////////////////////////////
//
// 								Robot Grunt Defines
//
///////////////////////////////////////////////////////////////////////////////

#define ROBOT_HEIGHT							72

#define ROBOT_DIAMETER							30

#define ROBOT_NAVMESH_TOLERANCE					200

#define ROBOT_NAVMESH_MAX_TOLERANCE_LEVELS		4

#define ROBOT_OFF_COVER_NODE_MIN_DISTANCE		60

#define ROBOT_OFF_COVER_NODE_MAX_DISTANCE		400

#define ROBOT_DETONATION_RANGE					60

#define ROBOT_DETONATION_INNER_DAMAGE			100

#define ROBOT_DETONATION_OUTER_DAMAGE			50

#define ROBOT_DETONATION_DAMAGE_TYPE			"MOD_EXPLOSIVE"

#define ROBOT_POSITION_QUERY_RADIUS				20

#define ROBOT_POSITION_QUERY_MOVE_DIST_MAX		120

#define ROBOT_MINI_RAPS_MODEL					"veh_t7_drone_raps_mini"

#define ROBOT_MINI_RAPS_SPAWNER					"spawner_bo3_mini_raps"

#define ROBOT_MINI_RAPS_LINK_TO_BONE			"j_spine4"

#define ROBOT_MINI_RAPS_OFFSET_POSITION			( 10, -10, -6 )

#define ROBOT_MINI_RAPS_AUTO_DETONATE_MIN_TIME	20

#define ROBOT_MINI_RAPS_AUTO_DETONATE_MAX_TIME	30

#define ROBOT_PAUSE_TACTICAL_WALK				500

#define ROBOT_JUKE_DISTANCE						90

#define ROBOT_TACTICAL_JUKE_RADIUS				50

#define ROBOT_RUSHER_MIN_RADIUS					150

#define ROBOT_RUSHER_MAX_RADIUS					250

#define ROBOT_ESCORT_MIN_RADIUS					75

#define ROBOT_ESCORT_MAX_RADIUS					150

///////////////////////////////////////////////////////////////////////////////
//
//										Clientfields
//
///////////////////////////////////////////////////////////////////////////////

#define ROBOT_MIND_CONTROL_CLIENTFIELD					"robot_mind_control"
#define ROBOT_MIND_CONTROL_BITS							2
#define ROBOT_MIND_CONTROL_TYPE							"int"
#define ROBOT_MIND_CONTROL_LEVEL_0						0
#define ROBOT_MIND_CONTROL_LEVEL_1						1
#define ROBOT_MIND_CONTROL_LEVEL_2						2
#define ROBOT_MIND_CONTROL_LEVEL_3						3

#define ROBOT_MIND_CONTROL_EXPLOSION_CLIENTFIELD		"robot_mind_control_explosion"
#define ROBOT_MIND_CONTROL_EXPLOSION_BITS				1
#define ROBOT_MIND_CONTROL_EXPLOSION_TYPE				"int"
#define ROBOT_MIND_CONTROL_EXPLOSION_OFF				0
#define ROBOT_MIND_CONTROL_EXPLOSION_ON					1

#define ROBOT_EMP_CLIENTFIELD							"robot_EMP"
#define ROBOT_EMP_BITS									1
#define ROBOT_EMP_TYPE									"int"
#define ROBOT_EMP_OFF									0
#define ROBOT_EMP_ON									1

#define ROBOT_LIGHTS_CLIENTFIELD						"robot_lights"
#define ROBOT_LIGHTS_BITS								3
#define ROBOT_LIGHTS_TYPE								"int"
#define ROBOT_LIGHTS_ON									0
#define ROBOT_LIGHTS_FLICKER							1
#define ROBOT_LIGHTS_OFF								2
#define ROBOT_LIGHTS_HACKED								3
#define ROBOT_LIGHTS_DEATH								4

///////////////////////////////////////////////////////////////////////////////
//
//											FX
//
///////////////////////////////////////////////////////////////////////////////

#define ROBOT_HEAD_GIB_FX							"fx_dest_robot_head_sparks"
#define ROBOT_HEAD_GIB_FX_FILE						"destruct/fx_dest_robot_head_sparks"
#define ROBOT_HEAD_GIB_FX_TAG						"j_neck"

#define ROBOT_MIND_CONTROL_EXPLOSION_FX				"fx_exp_robot_stage3_evb"
#define ROBOT_MIND_CONTROL_EXPLOSION_FX_FILE		"explosions/fx_exp_robot_stage3_evb"
#define ROBOT_MIND_CONTROL_EXPLOSION_FX_TAG			"j_spineupper"

#define ROBOT_EMP_FX								"fx_ability_elec_surge_short_robot"
#define ROBOT_EMP_FX_FILE							"electric/fx_ability_elec_surge_short_robot"
#define ROBOT_EMP_FX_TAG							"j_spine4"

#define ROBOT_FX_BASE(entity)						entity.fxdef
#define ROBOT_FX_BASE_FLICKER(entity)				entity.altfxdef3
#define ROBOT_FX_DEATH(entity)						entity.deathfxdef
#define ROBOT_FX_ROGUE_LEVEL_1(entity)				entity.altfxdef1
#define ROBOT_FX_ROGUE_LEVEL_1_FLICKER(entity)		entity.altfxdef4
#define ROBOT_FX_ROGUE_LEVEL_2(entity)				entity.altfxdef1
#define ROBOT_FX_ROGUE_LEVEL_2_FLICKER(entity)		entity.altfxdef4
#define ROBOT_FX_ROGUE_LEVEL_3(entity)				entity.altfxdef2
#define ROBOT_FX_ROGUE_LEVEL_3_FLICKER(entity)		entity.altfxdef5
