///////////////////////////////////////////////////////////////////////////////
//
// 								Mechz Defines
//
///////////////////////////////////////////////////////////////////////////////

#define ASM_MECHZ_MELEE_NOTETRACK							"melee_soldat"
#define ASM_MECHZ_GRENADE_NOTETRACK							"fire_chaingun"

// Melee ranges
#define MECHZ_MELEE_YAW										45
#define MECHZ_MELEE_DIST									112
#define MECHZ_MELEE_DIST_SQ									MECHZ_MELEE_DIST * MECHZ_MELEE_DIST

// Mavnesh
#define MECHZ_NAVMESH_RADIUS								64
#define MECHZ_NAVMESH_BOUNDARY_DIST							30

#define MECHZ_FOOTSTEP_EARTHQUAKE_MAX_RADIUS				1000

// Models
#define MECHZ_MODEL_BODY									"c_zom_mech_body"
#define MECHZ_MODEL_ARMOR_KNEE_LEFT							"c_zom_mech_armor_knee_left"
#define MECHZ_MODEL_ARMOR_KNEE_RIGHT						"c_zom_mech_armor_knee_right"
#define MECHZ_MODEL_ARMOR_SHOULDER_LEFT						"c_zom_mech_armor_shoulder_left"
#define MECHZ_MODEL_ARMOR_SHOULDER_RIGHT					"c_zom_mech_armor_shoulder_right"
#define MECHZ_MODEL_CLAW									"c_zom_mech_gun_barrel"
#define MECHZ_MODEL_FACEPLATE								"c_zom_mech_faceplate"
#define MECHZ_MODEL_POWERSUPPLY								"c_zom_mech_powersupply_cap"

// Tags
#define MECHZ_TAG_ARMOR_KNEE_LEFT							"j_knee_attach_le"
#define MECHZ_TAG_ARMOR_KNEE_RIGHT							"j_knee_attach_ri"
#define MECHZ_TAG_ARMOR_SHOULDER_LEFT						"j_shoulderarmor_le"
#define MECHZ_TAG_ARMOR_SHOULDER_RIGHT						"j_shoulderarmor_ri"
#define MECHZ_TAG_CLAW										"tag_gun_spin"
#define MECHZ_TAG_FACEPLATE									"j_faceplate"
#define MECHZ_TAG_POWERSUPPLY								"tag_powersupply"
#define MECHZ_TAG_POWERCORE									"tag_powersupply_hit"

// FX
#define MECHZ_FT_FX											"fx_mech_wpn_flamethrower"
#define MECHZ_FT_FX_FILE									"dlc1/castle/fx_mech_wpn_flamethrower"
#define MECHZ_FACEPLATE_OFF_FX								"fx_mech_dmg_armor_face"
#define MECHZ_FACEPLATE_OFF_FX_FILE							"dlc1/castle/fx_mech_dmg_armor_face"
#define MECHZ_POWERCAP_OFF_FX								"fx_mech_dmg_armor"
#define MECHZ_POWERCAP_OFF_FX_FILE							"dlc1/castle/fx_mech_dmg_armor"
#define MECHZ_CLAW_OFF_FX									"fx_mech_dmg_armor"
#define MECHZ_CLAW_OFF_FX_FILE								"dlc1/castle/fx_mech_dmg_armor"
#define MECHZ_115_GUN_MUZZLE_FLASH_FX						"fx_wpn_115_muz"
#define MECHZ_115_GUN_MUZZLE_FLASH_FX_FILE					"dlc1/castle/fx_wpn_115_muz"
#define MECHZ_RKNEE_ARMOR_OFF_FX							"fx_mech_dmg_armor"
#define MECHZ_RKNEE_ARMOR_OFF_FX_FILE						"dlc1/castle/fx_mech_dmg_armor"
#define MECHZ_LKNEE_ARMOR_OFF_FX							"fx_mech_dmg_armor"
#define MECHZ_LKNEE_ARMOR_OFF_FX_FILE						"dlc1/castle/fx_mech_dmg_armor"
#define MECHZ_RSHOULDER_AMOR_OFF_FX							"fx_mech_dmg_armor"
#define MECHZ_RSHOULDER_AMOR_OFF_FX_FILE					"dlc1/castle/fx_mech_dmg_armor"
#define MECHZ_LSHOULDER_AMOR_OFF_FX							"fx_mech_dmg_armor"
#define MECHZ_LSHOULDER_AMOR_OFF_FX_FILE					"dlc1/castle/fx_mech_dmg_armor"
#define MECHZ_HEADLIGHT_FX									"fx_mech_head_light"
#define MECHZ_HEADLIGHT_FX_FILE								"dlc1/castle/fx_mech_head_light"
#define MECHZ_ARMOR_OFF_SPARKS_FX							"fx_mech_dmg_sparks"
#define MECHZ_ARMOR_OFF_SPARKS_FX_FILE						"dlc1/castle/fx_mech_dmg_sparks"
#define MECHZ_KNEE_ARMOR_OFF_SPARKS_FX						"fx_mech_dmg_knee_sparks"
#define MECHZ_KNEE_ARMOR_OFF_SPARKS_FX_FILE					"dlc1/castle/fx_mech_dmg_knee_sparks"
#define MECHZ_GUN_OFF_SPARKS_FX								"fx_mech_dmg_sparks"
#define MECHZ_GUN_OFF_SPARKS_FX_FILE						"dlc1/castle/fx_mech_dmg_sparks"
#define MECHZ_FOOTSTEP_FX									"fx_mech_foot_step"
#define MECHZ_FOOTSTEP_FX_FILE								"dlc1/castle/fx_mech_foot_step"
#define MECHZ_HEADLAMP_DESTROYED_FX							"fx_mech_light_dmg"
#define MECHZ_HEADLAMP_DESTROYED_FX_FILE					"dlc1/castle/fx_mech_light_dmg"
#define MECHZ_FOOTSTEP_STEAM_FX								"fx_mech_foot_step_steam"
#define MECHZ_FOOTSTEP_STEAM_FX_FILE						"dlc1/castle/fx_mech_foot_step_steam"
#define MECHZ_POWERCORE_FX									"fx_mech_dmg_body_light"
#define MECHZ_POWERCORE_FX_FILE								"dlc1/castle/fx_mech_dmg_body_light"

#define MECHZ_FT_OFF										0
#define MECHZ_FT_ON											1

#define MECHZ_FT_CLIENTFIELD								"mechz_ft"
#define MECHZ_FACE_CLIENTFIELD								"mechz_face"

// Face anims
#define MECHZ_FACE_CLEAR									0
#define MECHZ_FACE_ATTACK									1
#define MECHZ_FACE_DEATH									2
#define MECHZ_FACE_IDLE										3
#define MECHZ_FACE_PAIN										4

// Grenade
#define MECHZ_GRENADE_TYPE									"electroball_grenade"	
#define MECHZ_GRENADE_TAG									"tag_gun_barrel2"

#define MECHZ_GRENADE_MAX									9
#define MECHZ_GRENADE_BURST_SIZE							3

#define MECHZ_GRENADE_DELAY									6000

#define MECHZ_GRENADE_DIST_SQ_MIN							250 * 250							
#define MECHZ_GRENADE_DIST_SQ_MAX							1200 * 1200

#define MECHZ_GRENADE_DEVIATION_YAW_MIN						-10
#define MECHZ_GRENADE_DEVIATION_YAW_MAX						10
#define MECHZ_GRENADE_DEVIATION_PITCH_MIN					10
#define MECHZ_GRENADE_DEVIATION_PITCH_MAX					20
#define MECHZ_GRENADE_DEVIATION_RADIUS						32
#define MECHZ_GRENADE_TARGET_PREDICTION_TIME				1.5

// Flamethrower
#define MECHZ_FT_RIGHT_OFFSET								26

#define MECHZ_FT_DIST_SQ_MIN								96 * 96							
#define MECHZ_FT_DIST_SQ_MAX								225 * 225

#define MECHZ_FT_DELAY										7500
#define	MECHZ_FT_RUN_DURATION								2500

#define MECHZ_FT_BLEND_TIME									0.3

#define MECHZ_FT_TAG										"tag_flamethrower_fx"

#define MECHZ_FT_SWEEP_PLAYER_DIST_SQ						100 * 100
#define MECHZ_FT_SWEEP_CHANCE								10

// Aiming
#define MECHZ_AIM_PITCH_MAX									60
#define MECHZ_AIM_YAW_COS									0.5

// Damage
#define MECHZ_PROJECTILE_DAMAGE_SCALE						0.1
#define MECHZ_PROJECTILE_SPLASH_DAMAGE_SCALE				0.2
#define MECHZ_BODY_DAMAGE_SCALE								0.1
#define MECHZ_POWERCORE_DAMAGE_SCALE						0.5
	
// Detachable Part Health
#define MECHZ_FACEPLATE_HEALTH								50
#define MECHZ_POWERCAP_COVER_HEALTH							50
#define MECHZ_POWERCAP_HEALTH								50
#define MECHZ_ARMOR_KNEE_RIGHT_HEALTH						50
#define MECHZ_ARMOR_KNEE_LEFT_HEALTH						50
#define MECHZ_ARMOR_SHOULDER_RIGHT_HEALTH					50
#define MECHZ_ARMOR_SHOULDER_LEFT_HEALTH					50

#define MECHZ_FT_PLAYER_DAMAGE								30
#define MECHZ_FT_PLAYER_DAMAGE_JUGG							20
#define MECHZ_FT_PLAYER_DAMAGE_DELAY						0.5
#define MECHZ_FT_PLAYER_BURN_TIME							1.5

#define MECHZ_VISIONSET_BURN								"mechz_player_burn"	

//	Berserk
#define MECHZ_BERSERK_TIME									10000

// Stun
#define MECHZ_STUN_TIME										500
#define MECHZ_STUMBLE_TIME									500


// React
#define MECHZ_REACT_ELEMENTAL_BOW							"elemental_bow"
#define MECHZ_REACT_ELEMENTAL_BOW_CHARGED					"elemental_bow4"
#define MECHZ_STUN_STUMBLE_COOLDOWN							10000
