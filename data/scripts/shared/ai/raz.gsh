///////////////////////////////////////////////////////////////////////////////
//
// 					RAZ (Russian Armored Zombie) Defines
//
///////////////////////////////////////////////////////////////////////////////

//Walk Rumble Distance
#define RAZ_WALK_RUMBLE_DISTANCE						200

//Sprint
#define RAZ_INVOKE_SPRINT_TIME							90000

// Melee
#define RAZ_MELEE_DIST									75
#define RAZ_MELEE_DIST_SQ								RAZ_MELEE_DIST * RAZ_MELEE_DIST
#define RAZ_MELEE_YAW									45
#define RAZ_MELEE_DAMAGE								50
	
// Ground Torpedo
#define RAZ_TORPEDO_ORIGIN_TAG							"tag_weapon_right"
#define RAZ_MIN_TORPEDO_RANGE							150
#define RAZ_MIN_TORPEDO_RANGE_SQ						RAZ_MIN_TORPEDO_RANGE * RAZ_MIN_TORPEDO_RANGE
#define RAZ_MAX_TORPEDO_RANGE							1200
#define RAZ_MAX_TORPEDO_RANGE_SQ						RAZ_MAX_TORPEDO_RANGE * RAZ_MAX_TORPEDO_RANGE
#define RAZ_TORPEDO_COOLDOWN							3000
#define RAZ_TORPEDO_MAX_LAUNCH_ANGLE					0.7	
#define RAZ_TORPEDO_SIGHT_HORIZ_RANGE					50
#define RAZ_TORPEDO_NOTETRACK							"mangler_fire"
#define RAZ_TORPEDO_MODEL								"tag_origin"
#define RAZ_TORPEDO_DETONATION_DIST						64
#define RAZ_TORPEDO_DETONATION_DIST_SQ					RAZ_TORPEDO_DETONATION_DIST * RAZ_TORPEDO_DETONATION_DIST
#define RAZ_TORPEDO_BLAST_RADIUS						128
#define RAZ_TORPEDO_BLAST_INNER_DAMAGE					100
#define RAZ_TORPEDO_BLAST_OUTER_DAMAGE					50
#define RAZ_TORPEDO_DETONATION_CLIENTFIELD				"raz_detonate_ground_torpedo"
#define RAZ_TORPEDO_VELOCITY							500
#define	RAZ_TORPEDO_MOVE_INTERVAL_TIME					0.1
#define RAZ_TORPEDO_TRAIL_CLIENTFIELD					"raz_torpedo_play_trail"
#define	RAZ_TORPEDO_TERRAIN_CHECK_OFFSET				32
#define RAZ_TORPEDO_MAX_YAW_PER_SECOND					135
#define RAZ_TORPEDO_GROUND_OFFSET						16
#define RAZ_TORPEDO_SELF_FX_CLIENTFIELD					"raz_torpedo_play_fx_on_self"
#define RAZ_TORPEDO_SELF_FX_TAG							"tag_origin"

// Navmesh
#define RAZ_NAVMESH_RADIUS								64
#define RAZ_NAVMESH_BOUNDARY_DIST						30

#define RAZ_FOOTSTEP_EARTHQUAKE_MAX_RADIUS				400

// Gun Detach
#define RAZ_GUN_DETACH_CLIENTFIELD						"raz_detach_gun"
#define RAZ_GUN_MODEL									"c_zom_dlc3_raz_s_armcannon"
#define RAZ_GUN_CORE_MODEL								"c_zom_dlc3_raz_s_cannonpowercore"
#define RAZ_R_SHOULDER_WEAKSPOT_TAG						"j_shouldertwist_ri_attach"
#define RAZ_GUN_CORE_HIDE_TAG							"j_shouldertwist_ri_attach"
#define RAZ_GUN_HIDE_TAG								"j_shoulder_ri_attach"
#define RAZ_GUN_DYNENT_LAUNCH_TAG						"j_elbow_ri"
#define RAZ_GUN_CORE_DYNENT_LAUNCH_TAG					"j_shouldertwist_ri_attach"
#define RAZ_GUN_ONGOING_DAMAGE_FX_TAG					"j_shouldertwist_ri_attach"
#define RAZ_GUN_TAG_HIT_RADIUS							9
#define RAZ_GUN_TAG_HIT_RADIUS_SQ						RAZ_GUN_TAG_HIT_RADIUS * RAZ_GUN_TAG_HIT_RADIUS
#define RAZ_GUN_HEALTH_DEFAULT							500
#define RAZ_GUN_WEAKPOINT_HIT_CLIENTFIELD				"raz_gun_weakpoint_hit"
#define RAZ_GUN_WEAKPOINT_HIT_FX_TAG					"j_shoulder_ri"
#define RAZ_GUN_DETACH_DAMAGE_HEALTH_PERCENT			0.33 //when gun is detached, RAZ takes this percentage of max health in damage
#define RAZ_GUN_DETACH_HEALTH_PERCENT_MAX				0.25 //when gun is detached, RAZ will have at most this health percentage
#define RAZ_GUN_DETACH_EXPLOSION_RADIUS					128
#define RAZ_GUN_DETACH_EXPLOSION_DAMAGE_MIN				0.25 //percentage of max health done as damage
#define RAZ_GUN_DETACH_EXPLOSION_DAMAGE_MAX				0.5 //percentage of max health done as damage	
#define RAZ_GUN_DETACH_EXP_PLYR_DAMAGE_MIN				15
#define RAZ_GUN_DETACH_EXP_PLYR_DAMAGE_MAX				50

// FX
#define RAZ_FOOTSTEP_FX                                    "fx_mech_foot_step"
#define RAZ_FOOTSTEP_FX_FILE                            "dlc1/castle/fx_mech_foot_step" //footstep fx for each footfall
#define RAZ_TORPEDO_EXPLOSION_FX                        "fx_raz_mc_shockwave_projectile_impact"
#define RAZ_TORPEDO_EXPLOSION_FX_FILE                    "dlc3/stalingrad/fx_raz_mc_shockwave_projectile_impact" //full filename of fx played when torpedo detonates
#define RAZ_TORPEDO_TRAIL_FX                            "fx_bul_impact_concrete_xtreme" //no longer being used
#define RAZ_TORPEDO_TRAIL_FX_FILE                        "impacts/fx_bul_impact_concrete_xtreme" //no longer being used
#define RAZ_GUN_DETACH_FX                                "fx_raz_dest_weak_point_exp"
#define RAZ_GUN_DETACH_FX_FILE                            "dlc3/stalingrad/fx_raz_dest_weak_point_exp" //fx played at shoulder when gun gets detached
#define RAZ_GUN_ONGOING_DAMAGE_FX                        "fx_raz_dest_weak_point_sparking_loop"
#define RAZ_GUN_ONGOING_DAMAGE_FX_FILE                    "dlc3/stalingrad/fx_raz_dest_weak_point_sparking_loop" //looping fx played at shoulder after gun has been detached
#define RAZ_GUN_WEAKPOINT_HIT_FX                        "fx_raz_dmg_weak_point"
#define RAZ_GUN_WEAKPOINT_HIT_FX_FILE                    "dlc3/stalingrad/fx_raz_dmg_weak_point" //impact fx played at shoulder when gun weakpoint is hit
#define RAZ_TORPEDO_SELF_FX                                "fx_raz_mc_shockwave_projectile"
#define RAZ_TORPEDO_SELF_FX_FILE                        "dlc3/stalingrad/fx_raz_mc_shockwave_projectile" //fx played on torpedo while in flight
#define RAZ_ARMOR_DETACH_FX								"fx_raz_dest_weak_point_exp_generic"
#define RAZ_ARMOR_DETACH_FX_FILE						"dlc3/stalingrad/fx_raz_dest_weak_point_exp_generic" //fx played when generic armor pieces pop off

#define RAZ_ARMOR_DETACH_FX_CANNON_TAG					"TAG_FX_Shoulder_RI_GIB"
#define RAZ_ARMOR_DETACH_FX_LEFT_SHOULDER_TAG			"TAG_FX_Shoulder_LE"
#define RAZ_ARMOR_DETACH_FX_CHEST_TAG					"TAG_FX_ChestPlate"
#define RAZ_ARMOR_DETACH_FX_HELMET_TAG					"TAG_FX_Helmet"
#define RAZ_ARMOR_DETACH_FX_RIGHT_LEG_TAG				"TAG_FX_Thigh_RI"
#define RAZ_ARMOR_DETACH_FX_LEFT_LEG_TAG				"TAG_FX_Thigh_LE"
	
	
// Damage and Armor
#define RAZ_ARMOR_DAMAGE_MODIFIER						0.1
#define RAZ_DEFAULT_HELMET_HEALTH						100
#define RAZ_HELMET_HIT_TAG								"j_head"
#define RAZ_HELMET_HIT_RADIUS							11
#define RAZ_HELMET_HIT_RADIUS_SQ						RAZ_HELMET_HIT_RADIUS * RAZ_HELMET_HIT_RADIUS
#define RAZ_DETACH_HELMET_CLIENTFIELD					"raz_detach_helmet"
#define RAZ_HELMET_MODEL								"c_zom_dlc3_raz_s_helmet"
#define RAZ_HELMET_TAG									"j_head_attach"
#define RAZ_DEFAULT_L_SHOULDER_ARMOR_HEALTH				100
#define RAZ_L_SHOULDER_ARMOR_HIT_TAG					"j_shouldertwist_le_attach"
#define RAZ_L_SHOULDER_ARMOR_MODEL						"c_zom_dlc3_raz_s_leftshoulderpad"
#define RAZ_L_SHOULDER_ARMOR_HIT_RADIUS					9
#define RAZ_L_SHOUDLER_ARMOR_HIT_RADIUS_SQ				RAZ_L_SHOULDER_ARMOR_HIT_RADIUS * RAZ_L_SHOULDER_ARMOR_HIT_RADIUS
#define RAZ_L_SHOULDER_ARMOR_DYNENT_LAUNCH_TAG			"j_shouldertwist_le_attach"
#define RAZ_L_SHOULDER_ARMOR_HIDE_TAG_1					"j_shouldertwist_le_attach"
#define RAZ_L_SHOULDER_ARMOR_HIDE_TAG_2					"j_shoulder_le_attach"
#define RAZ_L_SHOULDER_ARMOR_HIDE_TAG_3					"j_clavicle_le_attach"
#define RAZ_DETACH_L_SHOULDER_ARMOR_CLIENTFIELD			"raz_detach_l_shoulder_armor"
#define RAZ_DEFAULT_CHEST_ARMOR_HEALTH					100
#define RAZ_DETACH_CHEST_ARMOR_CLIENTFIELD				"raz_detach_chest_armor"
#define RAZ_CHEST_ARMOR_HIDE_TAG_1						"j_spine4_attach"
#define RAZ_CHEST_ARMOR_HIDE_TAG_2						"j_spineupper_attach"
#define RAZ_CHEST_ARMOR_HIDE_TAG_3						"j_spinelower_attach"
#define RAZ_CHEST_ARMOR_HIDE_TAG_4						"j_mainroot_attach"
#define RAZ_CHEST_ARMOR_HIDE_TAG_5						"j_clavicle_ri_attachbp"
#define RAZ_CHEST_ARMOR_HIDE_TAG_6						"j_clavicle_le_attachbp"
#define RAZ_CHEST_ARMOR_MODEL							"c_zom_dlc3_raz_s_chestplate"
#define RAZ_CHEST_ARMOR_HIT_RADIUS						12
#define RAZ_CHEST_ARMOR_HIT_RADIUS_SQ					RAZ_CHEST_ARMOR_HIT_RADIUS * RAZ_CHEST_ARMOR_HIT_RADIUS
#define RAZ_CHEST_ARMOR_HIT_TAG							"j_spine4_attach"
#define RAZ_CHEST_ARMOR_DYNENT_LAUNCH_TAG				"j_spine4_attach"
#define RAZ_DEFAULT_THIGH_ARMOR_HEALTH					100
#define RAZ_R_THIGH_ARMOR_HIT_TAG						"j_hiptwist_ri_attach"
#define RAZ_R_THIGH_ARMOR_MODEL							"c_zom_dlc3_raz_s_rightthighpad"
#define RAZ_R_THIGH_ARMOR_HIT_RADIUS					9
#define RAZ_R_THIGH_ARMOR_HIT_RADIUS_SQ					RAZ_R_THIGH_ARMOR_HIT_RADIUS * RAZ_R_THIGH_ARMOR_HIT_RADIUS
#define RAZ_R_THIGH_ARMOR_DYNENT_LAUNCH_TAG				"j_hiptwist_ri_attach"
#define RAZ_R_THIGH_ARMOR_HIDE_TAG_1					"j_hiptwist_ri_attach"
#define RAZ_R_THIGH_ARMOR_HIDE_TAG_2					"j_hip_ri_attach"
#define RAZ_DETACH_R_THIGH_ARMOR_CLIENTFIELD			"raz_detach_r_thigh_armor"
#define RAZ_L_THIGH_ARMOR_HIT_TAG						"j_hiptwist_le_attach"
#define RAZ_L_THIGH_ARMOR_MODEL							"c_zom_dlc3_raz_s_leftthighpad"
#define RAZ_L_THIGH_ARMOR_HIT_RADIUS					9
#define RAZ_L_THIGH_ARMOR_HIT_RADIUS_SQ					RAZ_L_THIGH_ARMOR_HIT_RADIUS * RAZ_L_THIGH_ARMOR_HIT_RADIUS
#define RAZ_L_THIGH_ARMOR_DYNENT_LAUNCH_TAG				"j_hiptwist_le_attach"
#define RAZ_L_THIGH_ARMOR_HIDE_TAG_1					"j_hiptwist_le_attach"
#define RAZ_L_THIGH_ARMOR_HIDE_TAG_2					"j_hip_le_attach"
#define RAZ_DETACH_L_THIGH_ARMOR_CLIENTFIELD			"raz_detach_l_thigh_armor"
	
#define RAZ_ARMOR_PAIN_NONE								"legs"	