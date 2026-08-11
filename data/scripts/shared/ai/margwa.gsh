///////////////////////////////////////////////////////////////////////////////
//
// 								Margwa Defines
//
///////////////////////////////////////////////////////////////////////////////

#define MARGWA_MODEL_BODY									"c_zom_margwa_body_jaw"

#define MARGWA_MODEL_HEAD_LEFT								"c_zom_margwa_chunks_le"
#define MARGWA_MODEL_HEAD_MID								"c_zom_margwa_chunks_mid"
#define MARGWA_MODEL_HEAD_RIGHT								"c_zom_margwa_chunks_ri"

#define MARGWA_MODEL_GORE_LEFT								"c_zom_margwa_gore_le"
#define MARGWA_MODEL_GORE_MID								"c_zom_margwa_gore_mid"
#define MARGWA_MODEL_GORE_RIGHT								"c_zom_margwa_gore_ri"

// bones used for precision damage
#define MARGWA_TAG_CHUNK_LEFT								"j_chunk_head_bone_le"
#define MARGWA_TAG_JAW_LEFT									"j_jaw_lower_1_le"
#define MARGWA_TAG_HEAD_LEFT								"tag_head_left"

#define MARGWA_TAG_CHUNK_MID								"j_chunk_head_bone"
#define MARGWA_TAG_JAW_MID									"j_jaw_lower_1"
#define MARGWA_TAG_HEAD_MID									"tag_head_mid"

#define MARGWA_TAG_CHUNK_RIGHT								"j_chunk_head_bone_ri"
#define MARGWA_TAG_JAW_RIGHT								"j_jaw_lower_1_ri"
#define MARGWA_TAG_HEAD_RIGHT								"tag_head_right"

#define MARGWA_HEAD_TAG										"j_neck_over"

#define MARGWA_NUM_HEADS									3

#define MARGWA_HEAD_CLEAR									0
#define MARGWA_HEAD_OPEN									1
#define MARGWA_HEAD_CLOSED									2
#define MARGWA_HEAD_SMASH_ATTACK							3

#define MARGWA_ANIM_HEAD_LEFT_OPEN							"ai_margwa_head_l_open_add"
#define MARGWA_ANIM_HEAD_LEFT_CLOSED						"ai_margwa_head_l_closed_add"
#define MARGWA_ANIM_HEAD_MID_OPEN							"ai_margwa_head_m_open_add"
#define MARGWA_ANIM_HEAD_MID_CLOSED							"ai_margwa_head_m_closed_add"
#define MARGWA_ANIM_HEAD_RIGHT_OPEN							"ai_margwa_head_r_open_add"
#define MARGWA_ANIM_HEAD_RIGHT_CLOSED						"ai_margwa_head_r_closed_add"

#define MARGWA_ANIM_HEAD_LEFT_SMASH							"ai_margwa_head_l_smash_attack_1"
#define MARGWA_ANIM_HEAD_MID_SMASH							"ai_margwa_head_m_smash_attack_1"
#define MARGWA_ANIM_HEAD_RIGHT_SMASH						"ai_margwa_head_r_smash_attack_1"

#define MARGWA_ANIM_TENTACLE_LEFT_BASE						"ai_margwa_tentacle_l_0"
#define MARGWA_ANIM_TENTACLE_RIGHT_BASE						"ai_margwa_tentacle_r_0"

#define MARGWA_NUM_TENTACLES_PER_SIDE						7

#define MARGWA_HEAD_HEALTH_BASE								600

// Mouth timing
#define	MARGWA_HEAD_OPEN_MIN								3000
#define	MARGWA_HEAD_OPEN_MAX								5000
#define MARGWA_HEAD_CLOSE_MIN								1500
#define MARGWA_HEAD_CLOSE_MAX								3500

#define MARGWA_SINGLE_HEAD_CLOSE_MIN						500
#define MARGWA_SINGLE_HEAD_CLOSE_MAX						1000

#define MARGWA_PAIN_CLOSE_TIME								5000

#define MARGWA_MOUTH_BLEND_TIME								0.1

#define ASM_MARGWA_SMASH_ATTACK_NOTETRACK					"margwa_smash_attack"
#define ASM_MARGWA_BODYFALL_NOTETRACK						"margwa_bodyfall large"
#define ASM_MARGWA_PAIN_MELEE_NOTETRACK						"margwa_melee_fire"

// Melee ranges
#define MARGWA_MELEE_YAW									45
#define MARGWA_MELEE_DIST									128
#define MARGWA_MELEE_DIST_SQ								MARGWA_MELEE_DIST * MARGWA_MELEE_DIST

#define MARGWA_SWIPE_DIST									128
#define MARGWA_SWIPE_DIST_SQ								MARGWA_SWIPE_DIST * MARGWA_SWIPE_DIST

#define MARGWA_HEAD_EXPLOSION_RANGE							128 * 128

#define MARGWA_HEAD_DAMAGE_RANGE							24 * 24

// Smash attack
#define MARGWA_SMASH_ATTACK_START							160 * 160
#define MARGWA_SMASH_ATTACK_RANGE							144 * 144
#define MARGWA_SMASH_ATTACK_RANGE_LIGHT						192 * 192
#define MARGWA_SMASH_ATTACK_DAMAGE							166
#define MARGWA_SMASH_ATTACK_OFFSET							60
#define MARGWA_SMASH_ATTACK_HEIGHT							48

// Riotshield
#define MARGWA_RIOTSHIELD_DAMAGE							200
#define MARGWA_RIOTSHIELD_FACING_TOLERANCE					0.2

// Mavnesh
#define MARGWA_NAVMESH_RADIUS								64
#define MARGWA_NAVMESH_BOUNDARY_DIST						30

#define MARGWA_LARGE_RADIUS									2048

// FX
#define MARGWA_TELEPORT_FX									"fx_margwa_teleport_zod_zmb"
#define MARGWA_TELEPORT_FX_FILE								"zombie/fx_margwa_teleport_zod_zmb"
#define MARGWA_TELEPORT_TRAVEL_FX							"fx_margwa_teleport_travel_zod_zmb"
#define MARGWA_TELEPORT_TRAVEL_FX_FILE						"zombie/fx_margwa_teleport_travel_zod_zmb"
#define MARGWA_TELEPORT_TRAVEL_TELL_FX						"fx_margwa_teleport_tell_zod_zmb"
#define MARGWA_TELEPORT_TRAVEL_TELL_FX_FILE					"zombie/fx_margwa_teleport_tell_zod_zmb"
#define MARGWA_SPAWN_FX										"fx_margwa_teleport_intro_zod_zmb"
#define MARGWA_SPAWN_FX_FILE								"zombie/fx_margwa_teleport_intro_zod_zmb"

#define MARGWA_IMPACT_FX									"fx_margwa_head_shot_zod_zmb"
#define MARGWA_IMPACT_FX_FILE								"zombie/fx_margwa_head_shot_zod_zmb"
#define MARGWA_ROAR_FX										"fx_margwa_roar_zod_zmb"
#define MARGWA_ROAR_FX_FILE									"zombie/fx_margwa_roar_zod_zmb"
#define MARGWA_SUPER_ROAR_FX								"fx_margwa_roar_purple_zod_zmb"
#define MARGWA_SUPER_ROAR_FX_FILE							"zombie/fx_margwa_roar_purple_zod_zmb"

#define MARGWA_TAG_TELEPORT									"j_spine_1"

#define MARGWA_TELEPORT_OFF									0
#define MARGWA_TELEPORT_ON									1

#define MARGWA_TRAVELER_TIME								0.25
#define MARGWA_TRAVELER_SPEED								600
#define MARGWA_TRAVELER_HEIGHT_OFFSET						60

#define MARGWA_TELL_DIST									1000
#define MARGWA_TELL_DIST_SQ									MARGWA_TELL_DIST * MARGWA_TELL_DIST
	
#define MARGWA_FOOTSTEP_EARTHQUAKE_MAX_RADIUS				GetDvarInt( "scr_margwa_footstep_eq_radius", 1000)

///////////////////////////////////////////////////////////////////////////////
//
//										Clientfields
//
///////////////////////////////////////////////////////////////////////////////
#define MARGWA_HEAD_LEFT_CLIENTFIELD						"margwa_head_left"
#define MARGWA_HEAD_MID_CLIENTFIELD							"margwa_head_mid"
#define MARGWA_HEAD_RIGHT_CLIENTFIELD						"margwa_head_right"
#define MARGWA_HEAD_CLIENTFIELD_TYPE						"int"
#define MARGWA_HEAD_CLIENTFIELD_BITS						2

#define MARGWA_FX_IN_CLIENTFIELD							"margwa_fx_in"
#define MARGWA_FX_OUT_CLIENTFIELD							"margwa_fx_out"
#define MARGWA_FX_TRAVEL_CLIENTFIELD						"margwa_fx_travel"
#define MARGWA_FX_TRAVEL_TELL_CLIENTFIELD					"margwa_fx_travel_tell"
#define MARGWA_FX_CLIENTFIELD_TYPE							"int"
#define MARGWA_FX_CLIENTFIELD_BITS							1

#define MARGWA_FX_SPAWN_CLIENTFIELD							"margwa_fx_spawn"
#define MARGWA_FX_SPAWN_CLIENTFIELD_TYPE					"counter"
#define MARGWA_FX_SPAWN_CLIENTFIELD_BITS					1

#define MARGWA_HEAD_EXPLODE_CLIENTFIELD						"margwa_head_explosion"
#define MARGWA_HEAD_EXPLODE_CLIENTFIELD_TYPE				"counter"
#define MARGWA_HEAD_EXPLODE_CLIENTFIELD_BITS				1

#define MARGWA_SMASH_CLIENTFIELD							"margwa_smash"
#define MARGWA_SMASH_CLIENTFIELD_TYPE						"counter"
#define MARGWA_SMASH_CLIENTFIELD_BITS						1

#define MARGWA_HEAD_LEFT_HIT_CLIENTFIELD					"margwa_head_left_hit"
#define MARGWA_HEAD_MID_HIT_CLIENTFIELD						"margwa_head_mid_hit"
#define MARGWA_HEAD_RIGHT_HIT_CLIENTFIELD					"margwa_head_right_hit"

#define MARGWA_HEAD_KILLED_CLIENTFIELD						"margwa_head_killed"
#define MARGWA_HEAD_KILLED_LEFT								1
#define MARGWA_HEAD_KILLED_MID								2
#define MARGWA_HEAD_KILLED_RIGHT							3

#define MARGWA_JAW_CLIENTFIELD								"margwa_jaw"

#define MARGWA_JAW_BASE_L									"ai_margwa_jaw_l_"
#define MARGWA_JAW_BASE_M									"ai_margwa_jaw_m_"
#define MARGWA_JAW_BASE_R									"ai_margwa_jaw_r_"

#define MARGWA_JAW_CLEAR									0
#define MARGWA_JAW_IDLE										1
#define MARGWA_JAW_HEAD_L_EXPLODE							3
#define MARGWA_JAW_HEAD_M_EXPLODE							4
#define MARGWA_JAW_HEAD_R_EXPLODE							5
#define MARGWA_JAW_REACT_STUN								6
#define MARGWA_JAW_RUN_CHARGE								7
#define MARGWA_JAW_REACT_IDGUN								8
#define MARGWA_JAW_REACT_IDGUN_PACKED						9	
#define MARGWA_JAW_RUN										13
#define MARGWA_JAW_SMASH_ATTACK								14
#define MARGWA_JAW_SWIPE									15
#define MARGWA_JAW_SWIPE_PLAYER								16
#define MARGWA_JAW_TELEPORT_IN								17
#define MARGWA_JAW_TELEPORT_OUT								18
#define MARGWA_JAW_TRV_JUMP_ACROSS_256						19
#define MARGWA_JAW_TRV_JUMP_DOWN_128						20
#define MARGWA_JAW_TRV_JUMP_DOWN_36							21
#define MARGWA_JAW_TRV_JUMP_DOWN_96							22
#define MARGWA_JAW_TRV_JUMP_UP_128							23
#define MARGWA_JAW_TRV_JUMP_UP_36							24
#define MARGWA_JAW_TRV_JUMP_UP_96							25

