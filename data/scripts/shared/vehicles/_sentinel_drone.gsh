#define SENTINEL_DRONE_RADIUS									35

#define SENTINEL_DRONE_MAX_TIME_AT_SAME_POSITION				1.0
#define SENTINEL_DRONE_CHANGE_POSITION_TOATTACK_TARGET_DELAY	0.5

#define SENTINEL_DRONE_AWAY_FROM_CHARACTER						300

#define SENTINEL_DRONE_ENEMY_TOO_CLOSE_DIST						250
	
//reduce these defines to have the sentinel_drone make more movements close together
#define SENTINEL_DRONE_TOO_CLOSE_TO_SELF_DIST				    150
#define SENTINEL_DRONE_MOVE_DIST_MAX							225
#define SENTINEL_DRONE_JUKE_MOVE_DIST_MAX						300

#define SENTINEL_DRONE_REPATH_RANGE								100
	
#define SENTINEL_DRONE_FIRE_CHANCE								30
#define SENTINEL_DRONE_MELEE_CHANCE								50
#define	SENTINEL_DRONE_MELEE_DIST								64
#define SENTINEL_DRONE_NEARGOAL_DIST							35
	
#define SENTINEL_DRONE_MAX_INSTANCES							32
	
#define SENTINEL_DRONE_BEAM_DAMAGE_PER_SECOND					100
	
#define SENTINEL_DRONE_BEAM_MAX_LENGTH							1200
	
#define SENTINEL_CHARGE_AT_PLAYER_TIME_OUT						4000	
	
#define SENTINEL_DRONE_ARM_RIGHT								1	
#define SENTINEL_DRONE_ARM_LEFT									2
#define SENTINEL_DRONE_ARM_TOP									3
	
#define SENTINEL_DRONE_CAMERA_BROKEN_TAG						"Tag_camera_dead"
#define SENTINEL_DRONE_CAMERA_TURRET_TAG						"tag_turret"
	
#define SENTINEL_DRONE_ARM_LEFT_TAG								"tag_arm_left_01"
#define SENTINEL_DRONE_ARM_RIGHT_TAG							"tag_arm_right_01"
#define SENTINEL_DRONE_ARM_TOP_TAG								"tag_arm_top_01"

#define SENTINEL_DRONE_ARM_LEFT_BROKEN_TAG						"tag_arm_left_01_d1"
#define SENTINEL_DRONE_ARM_RIGHT_BROKEN_TAG						"tag_arm_right_01_d1"
#define SENTINEL_DRONE_ARM_TOP_BROKEN_TAG						"tag_arm_top_01_d1"		
	
#define SENTINEL_DRONE_ARM_LEFT_FX_TAG							"tag_arm_left_03_d1"
#define SENTINEL_DRONE_ARM_RIGHT_FX_TAG							"tag_arm_right_04_d1"
#define SENTINEL_DRONE_ARM_TOP_FX_TAG							"tag_arm_top_03_d1"	

#define SENTINEL_DRONE_FACE_TAG									"tag_faceplate_d0"
#define SENTINEL_DRONE_CORE_TAG									"ag_core_d0"
#define SENTINEL_DRONE_CORE_TAG_2								"tag_center_core"
#define SENTINEL_DRONE_CORE_TAG_3								"tag_core_spin"
	
#define SENTINEL_DRONE_CORE_BLUE_TAG							"tag_center_core_emmisive_blue"
#define SENTINEL_DRONE_CORE_RED_TAG								"tag_center_core_emmisive_red"	

#define SENTINEL_DRONE_DEFAULT_HEALTH_ARM_LEFT					200
#define SENTINEL_DRONE_DEFAULT_HEALTH_ARM_RIGHT					200
#define SENTINEL_DRONE_DEFAULT_HEALTH_ARM_TOP					200
#define SENTINEL_DRONE_DEFAULT_HEALTH_FACE						200	
#define SENTINEL_DRONE_DEFAULT_HEALTH_CAMERA					300	
#define SENTINEL_DRONE_DEFAULT_HEALTH_CORE						100	
	
#define SENTINEL_DRONE_FACE_MODEL		"veh_t7_dlc3_sentinel_drone_faceplate"
#define SENTINEL_DRONE_ARM_MODEL		"veh_t7_dlc3_sentinel_drone_spawn_arm"
#define SENTINEL_DRONE_CLAW_MODEL		"veh_t7_dlc3_sentinel_drone_spawn_claw"