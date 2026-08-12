///////////////////////////////////////////////////////////////////////////////
//
// 								THRASHER TUNABLES
//
///////////////////////////////////////////////////////////////////////////////

// Initial thrasher health, NOT USED IN ZOMBIES
#define THRASHER_TOTAL_HEALTH								1000

// Total health is based on level.zombie_health multiplied by this multiplier, increases per round
#define THRASHER_EARLY_ROUND_HEALTH_MULTIPLIER				10
#define THRASHER_LATE_ROUND_HEALTH_MULTIPLIER				3	

// Initial pustule health, NOT USED IN ZOMBIES
#define THRASHER_PUSTULE_HEALTH								100

// Total pustule health is based on level.zombie_health multiplied by this multiplier, increases per round
#define THRASHER_PUSTULE_HEALTH_MULTIPLIER					2

// Initial head health
#define THRASHER_HEAD_HEALTH								10

// Additional health added when enraged
#define THRASHER_RAGE_HEALTH_BONUS							1500

// Amount of rage added when hit in a particular location
#define THRASHER_RAGE_INC_NONVITAL							10
#define THRASHER_RAGE_INC_HEADSHOT							200
#define THRASHER_RAGE_INC_PUSTULE							10

// Amount of rage added per second
#define THRASHER_RAGE_AUTO									1

// Additional rage added on top "THRASHER_RAGE_AUTO" per second multiplied by current level
#define THRASHER_RAGE_AUTO_MULTIPLIER						1

// Amount of rage added when performing behavior
#define THRASHER_RAGE_INC_INGEST_ZOMBIE						50
#define THRASHER_RAGE_INC_INGEST_PLAYER						200

// Amount of rage before pushing the thrasher into berserk mode
#define THRASHER_RAGE_THRESHOLD								200

// How often a thrasher can consume a zombie
#define THRASHER_CONSUME_COOLDOWN							3000

// How many times a thrasher can consume a zombie
#define THRASHER_CONSUME_MAX								2

// How often a thrasher can teleport to close the distance in milliseconds
#define THRASHER_TELEPORT_COOLDOWN							10000

// How close a thrasher can teleport to another thrasher's position
#define THRASHER_TELEPORT_OVERLAP							240

// How far a thrasher will consider a downed player to consume
#define THRASHER_CONSUME_PLAYER_DISTANCE					2400

// How long stuns lasts in milliseconds
#define THRASHER_STUN_TIME									1000

// How much normal damage before the thrasher is stunned
#define THRASHER_STUN_HEALTH								3000

// How long the thrasher stays invulnerable after becoming enraged
#define THRASHER_RAGE_INVULNERABLE_TIME						2.5
	
// Distance from enemy before the thrasher will teleport
#define THRASHER_TELEPORT_MIN_DISTANCE						1200

// How long the thrasher's spore inflate FX will play for.
#define THRASHER_SPORE_INFLATE_TIME							2000

// How long the spore cloud released from a popped pustule lasts
#define THRASHER_SPORE_CLOUD_TIME							5000

// How far away from the center of a spore cloud zombies can be affected
#define THRASHER_SPORE_CLOUD_RADIUS							60

// Maximum number of zombies a single spore cloud can convert to thrashers
#define THRASHER_SPORE_CLOUD_MAX_CONVERT					2

// Maximum number of zombies that can be converted per round into thrashers
#define THRASHER_SPORE_CLOUD_ROUND_MAX_CONVERT				2

// Chance a single zombie will be converted into a thrasher (0, 1)
#define THRASHER_SPORE_CLOUD_CONVERT_CHANCE					0.2

// How often all zombies within a spore cloud will be tested to turn into a thrasher
#define THRASHER_SPORE_CLOUD_EVALUATE						0.5

// How long a player must be in laststand before being able to be eatten
#define THRASHER_LASTSTAND_SAFETY							5000

// How long of a time must pass before a new thrasher can consume a player that was just freed from a thrasher.
#define THRASHER_FREED_SAFETY								10000

///////////////////////////////////////////////////////////////////////////////
//
// 								THRASHER DEFINES
//
///////////////////////////////////////////////////////////////////////////////

#define ASM_THRASHER_MELEE_NOTETRACK						"thrasher_melee"

// Melee
#define THRASHER_MELEE_DIST									96
#define THRASHER_MELEE_DIST_SQ								THRASHER_MELEE_DIST * THRASHER_MELEE_DIST
#define THRASHER_MELEE_YAW									60
#define THRASHER_MELEE_ENRAGED								"thrasher_melee_enraged"

// Consume
#define THRASHER_CONSUME_DIST								48
#define THRASHER_CONSUME_DIST_SQ							THRASHER_CONSUME_DIST * THRASHER_CONSUME_DIST

// Consume Zombie
#define THRASHER_CONSUME_ZOMBIE_HEIGHT						18
#define THRASHER_CONSUME_ZOMBIE_MAX_DIST					96
#define THRASHER_CONSUME_ZOMBIE_MIN_DIST					50

// Navmesh
#define THRASHER_NAVMESH_RADIUS								128
#define THRASHER_NAVMESH_BOUNDARY_DIST						30

#define THRASHER_FOOTSTEP_EARTHQUAKE_MAX_RADIUS				1000

// Spores
#define THRASHER_SPORE_CHEST								"tag_spore_chest"
#define THRASHER_SPORE_BACK									"tag_spore_back"
#define THRASHER_SPORE_LEG									"tag_spore_leg"
#define THRASHER_SPORES										array( THRASHER_SPORE_CHEST, THRASHER_SPORE_BACK, THRASHER_SPORE_LEG )
#define THRASHER_SPORE_DAMAGE_DISTS							array( 12, 18, 12 )

#define THRASHER_SPORE_STATE_DESTROYED						"state_destroyed"
#define THRASHER_SPORE_STATE_HEALTHY						"state_healthly"
#define THRASHER_SPORE_DESTROY_FX1							"fx_thrash_pustule_burst"
#define THRASHER_SPORE_DESTROY_FX1_FILE						"dlc2/island/fx_thrash_pustule_burst"
#define THRASHER_SPORE_DESTROY_FX2							"fx_thrash_pustule_spore_exp"
#define THRASHER_SPORE_DESTROY_FX2_FILE						"dlc2/island/fx_thrash_pustule_spore_exp"

#define THRASHER_SPORE_CLOUD_SM_FX							"fx_spores_cloud_ambient_sm"
#define THRASHER_SPORE_CLOUD_SM_FX_FILE						"dlc2/island/fx_spores_cloud_ambient_sm"
#define THRASHER_SPORE_CLOUD_MD_FX							"fx_spores_cloud_ambient_md"
#define THRASHER_SPORE_CLOUD_MD_FX_FILE						"dlc2/island/fx_spores_cloud_ambient_md"
#define THRASHER_SPORE_CLOUD_LRG_FX							"fx_spores_cloud_ambient_lrg"
#define THRASHER_SPORE_CLOUD_LRG_FX_FILE					"dlc2/island/fx_spores_cloud_ambient_lrg"

#define THRASHER_SPORE_INFLATE_FX							"fx_thrash_pustule_reinflate"
#define THRASHER_SPORE_INFLATE_FX_FILE						"dlc2/island/fx_thrash_pustule_reinflate"

#define THRASHER_SPORE_IMPACT_CF							"thrasher_spore_impact"
#define THRASHER_SPORE_IMPACT_CF_TYPE						"counter"
#define THRASHER_SPORE_IMPACT_CF_BITS						1
#define THRASHER_SPORE_IMPACT_FX							"fx_thrash_pustule_impact"
#define THRASHER_SPORE_IMPACT_FX_FILE						"dlc2/island/fx_thrash_pustule_impact"

#define THRASHER_SPORE_CF									"thrasher_spore_state"
#define THRASHER_SPORE_CF_TYPE								"int"
#define THRASHER_SPORE_CF_BITS								3
#define THRASHER_SPORE_CF_SPORE1							1
#define THRASHER_SPORE_CF_SPORE2							2
#define THRASHER_SPORE_CF_SPORE3							4
#define THRASHER_SPORE_CF_SPORES							array( THRASHER_SPORE_CF_SPORE1, THRASHER_SPORE_CF_SPORE2, THRASHER_SPORE_CF_SPORE3 )

// Thrasher First Person Mouth
#define THRASHER_MOUTH_CF									"thrasher_mouth_cf"
#define THRASHER_MOUTH_CF_TYPE								"int"
// 2 bits per player
#define THRASHER_MOUTH_CF_BITS								8
#define THRASHER_MOUTH_CF_DELETE							0
#define THRASHER_MOUTH_CF_IDLE								1
#define THRASHER_MOUTH_CF_OPEN								2
#define THRASHER_MOUTH_CF_CLOSE								3

#define THRASHER_MOUTH_IDLE_ANIM							"p7_fxanim_zm_island_thrasher_stomach_idle_anim"
#define THRASHER_MOUTH_CLOSE_ANIM							"p7_fxanim_zm_island_thrasher_stomach_close_anim"
#define THRASHER_MOUTH_OPEN_ANIM							"p7_fxanim_zm_island_thrasher_stomach_open_anim"

// Berserk Mode
#define THRASHER_BERSERK_CF									"thrasher_berserk_state"
#define THRASHER_BERSERK_CF_TYPE							"int"
#define THRASHER_BERSERK_CF_BITS							1
#define THRASHER_BERSERK_CF_NORMAL							0
#define THRASHER_BERSERK_CF_BERSERK							1

// Spikes
#define THRASHER_SPIKE_COUNT								24

// FX
#define THRASHER_FOOTSTEP_FX								"fx_mech_foot_step"
#define THRASHER_FOOTSTEP_FX_FILE							"dlc1/castle/fx_mech_foot_step"

#define THRASHER_BERSERK_FX1								"fx_thrash_rage_gas_torso"
#define THRASHER_BERSERK_FX1_TAG							"j_spinelower"
#define THRASHER_BERSERK_FX1_FILE							"dlc2/island/fx_thrash_rage_gas_torso"

#define THRASHER_BERSERK_FX2								"fx_thrash_rage_gas_leg_lft"
#define THRASHER_BERSERK_FX2_TAG							"j_hip_le"
#define THRASHER_BERSERK_FX2_FILE							"dlc2/island/fx_thrash_rage_gas_leg_lft"

#define THRASHER_BERSERK_FX3								"fx_thrash_rage_gas_leg_rgt"
#define THRASHER_BERSERK_FX3_TAG							"j_hip_ri"
#define THRASHER_BERSERK_FX3_FILE							"dlc2/island/fx_thrash_rage_gas_leg_rgt"

#define THRASHER_EYE_TAG									"j_eyeball_le"
#define THRASHER_EYE_FX										"fx_thrash_eye_glow"
#define THRASHER_EYE_FX_FILE								"dlc2/island/fx_thrash_eye_glow"
#define THRASHER_BERSERK_EYE_FX								"fx_thrash_eye_glow_rage"
#define THRASHER_BERSERK_EYE_FX_FILE						"dlc2/island/fx_thrash_eye_glow_rage"
	
#define THRASHER_CONSUMED_PLAYER_FX							"fx_thrash_chest_mouth_drool"
#define THRASHER_CONSUMED_PLAYER_FX_FILE					"dlc2/island/fx_thrash_chest_mouth_drool_1p"
	
#define THRASHER_CONSUMED_PLAYER_POSTFX						"pstfx_thrasher_stomach"
#define THRASHER_CONSUMED_PLAYER_VISIONSET_FILE				"zm_isl_thrasher_stomach"
#define THRASHER_CONSUMED_PLAYER_VISIONSET_ALIAS			"zm_isl_thrasher_stomach_visionset"
#define THRASHER_CONSUMED_PLAYER_VISIONSET_LERP_STEP_COUNT	16
#define THRASHER_CONSUMED_PLAYER_VISIONSET_RAMP_IN_DURATION 2
#define THRASHER_CONSUMED_PLAYER_VISIONSET_PRIORITY			30

// Head Damage
#define THRASHER_HEAD_TAG									"j_head"
#define THRASHER_HEAD_HITLOC								"head"

// Teleport
#define THRASHER_TELERPOT_MIN_DISTANCE_SQ					THRASHER_TELEPORT_MIN_DISTANCE * THRASHER_TELEPORT_MIN_DISTANCE
#define THRASHER_TELEPORT_DESTINATION_SAFE_RADIUS			128
#define THRASHER_TELEPORT_DESTINATION_MAX_RADIUS			256