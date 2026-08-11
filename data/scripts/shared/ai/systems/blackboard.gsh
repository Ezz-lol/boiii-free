#define BB_REGISTER_ATTRIBUTE(name,defaultValue,getter) \
	Blackboard::RegisterBlackBoardAttribute(self,name,defaultValue,getter);\
	if( IsActor(self) ) \
	{ \
	}
	
#define ENABLE_BLACKBOARD_DEBUG_TRACKING(self) 

// -------------------------------------------- BLACKBOARD ATTRIBUTES --------------------------//

// Attribute names, keep it lowercase. "_" before the attribute name indicates that its being used by both AST and BT.

// Desired values should have DESIRED and "_desired_" in their definition. There should be a associated BB attribute existing already
// Eg. ATTRIBUTE and DESIRED_ATTRIBUTE and "_attribute" and "_desired_attribute". Try to keep them together please. :)

#define CONTEXT 						"_context"
#define CONTEXT_2						"_context2"
#define DESIRED_STANCE 					"_desired_stance"

#define ARRIVAL_TYPE					"_arrival_type"	
#define ARRIVE_AT_GOAL					"arrive_at_goal"	
#define DONT_ARRIVE_AT_GOAL				"dont_arrive_at_goal"		
	
	
#define ARRIVAL_STANCE					"_arrival_stance"
#define HIT_LOCATION					"_hit_location"
#define FATAL_DAMAGE_LOCATION 			"_fatal_damage_location"	
#define DAMAGE_LOCATION 				"_damage_location"	
#define DAMAGE_DIRECTION 				"_damage_direction"
#define DAMAGE_WEAPON_CLASS 			"_damage_weapon_class"
#define DAMAGE_MOD 						"_damage_mod"
#define DAMAGE_WEAPON 					"_damage_weapon"
#define DAMAGE_TAKEN 					"_damage_taken"
#define TRACKING_TURN_YAW 				"_tracking_turn_yaw"
#define MELEE_DISTANCE 					"_melee_distance"
#define STANCE 							"_stance"
#define THROW_DISTANCE 					"_throw_distance"
#define WEAPON_CLASS 					"_weapon_class"
#define WEAPON_TYPE 					"_weapon_type"
#define FIRE_MODE						"_fire_mode"
#define COVER_MODE						"_cover_mode"
#define PREVIOUS_COVER_MODE				"_previous_cover_mode"

#define YAW_TO_COVER					"_yaw_to_cover"

#define COVER_DIRECTION					"_cover_direction"
#define PREVIOUS_COVER_DIRECTION		"_previous_cover_direction"

#define COVER_BACK_DIRECTION			"cover_back_direction"
#define COVER_FRONT_DIRECTION			"cover_front_direction"
#define COVER_LEFT_DIRECTION			"cover_left_direction"
#define COVER_RIGHT_DIRECTION			"cover_right_direction"
	
#define COVER_TYPE						"_cover_type"
#define CURRENT_LOCATION_COVER_TYPE		"_current_location_cover_type"
#define PREVIOUS_COVER_TYPE				"_previous_cover_type"

#define EXPOSED_TYPE					"_exposed_type"

#define LOCOMOTION_MOTION_ANGLE			"_locomotion_motion_angle"
#define LOCOMOTION_TURN_YAW				"_locomotion_turn_yaw"

#define LOCOMOTION_EXIT_YAW				"_locomotion_exit_yaw"
#define LOCOMOTION_ARRIVAL_YAW			"_locomotion_arrival_yaw"
#define TACTICAL_ARRIVAL_FACING_YAW		"_tactical_arrival_facing_yaw"
#define LOCOMOTION_ARRIVAL_DISTANCE		"_locomotion_arrival_distance"
	
#define LOCOMOTION_FACE_ENEMY_QUADRANT			"_locomotion_face_enemy_quadrant"		
#define LOCOMOTION_FACE_ENEMY_QUADRANT_PREVIOUS "_locomotion_face_enemy_quadrant_previous"		
	
#define LOCOMOTION_FACE_ENEMY_NONE		"locomotion_face_enemy_none"	
#define LOCOMOTION_FACE_ENEMY_FRONT		"locomotion_face_enemy_front"
#define LOCOMOTION_FACE_ENEMY_LEFT		"locomotion_face_enemy_left"
#define LOCOMOTION_FACE_ENEMY_RIGHT		"locomotion_face_enemy_right"
#define LOCOMOTION_FACE_ENEMY_BACK		"locomotion_face_enemy_back"	

#define LOOKAHEAD_ANGLE					"_lookahead_angle"

#define PATROL							"_patrol"
#define PATROL_ENABLED					"patrol_enabled"
#define PATROL_DISABLED					"patrol_disabled"

#define ENEMY							"_enemy"
#define HAS_ENEMY						"has_enemy"
#define NO_ENEMY						"no_enemy"

#define ENEMY_YAW						"_enemy_yaw"
#define PERFECT_ENEMY_YAW				"_perfect_enemy_yaw"
#define REACT_YAW						"_react_yaw"

#define COVER_NONE						"cover_none"
#define COVER_CROUCH					"cover_crouch"
#define COVER_RIGHT						"cover_right"
#define COVER_LEFT						"cover_left"
#define COVER_STAND						"cover_stand"
#define COVER_EXPOSED					"cover_exposed"
#define COVER_PILLAR					"cover_pillar"
#define COVER_GUARD						"Guard"
#define COVER_PATH						"Path"

#define COVER_CONCEALED					"_cover_concealed"
#define COVER_TYPE_CONCEALED			"concealed"
#define COVER_TYPE_UNCONCEALED			"unconcealed"

#define EXPOSED_YES						"exposed_yes"
#define EXPOSED_NO						"exposed_no"

#define COVER_MODE_NONE					"cover_mode_none"
#define COVER_LEAN_MODE					"cover_lean"
#define COVER_BLIND_MODE				"cover_blind"
#define COVER_OVER_MODE					"cover_over"
#define COVER_VANTAGE_MODE				"cover_vantage"
#define COVER_ALERT_MODE				"cover_alert"
#define COVER_SCAN_MODE					"cover_scan"
	
#define TRAVERSAL_TYPE					"_traversal_type"
	
#define VARIANT_TYPE					"_variant_type"
#define LOW_GRAVITY_VARIANT				"_low_gravity_variant"
#define LOW_GRAVITY						"_low_gravity"

#define	LOCOMOTION_PAIN_TYPE			"_locomotion_pain_type"	
	
#define HUMAN_LOCOMOTION_MOVEMENT_TYPE		"_human_locomotion_movement_type"	
#define HUMAN_LOCOMOTION_MOVEMENT_PATROL	"human_locomotion_movement_patrol"
#define HUMAN_LOCOMOTION_MOVEMENT_SPRINT	"human_locomotion_movement_sprint"
#define HUMAN_LOCOMOTION_MOVEMENT_DEFAULT	"human_locomotion_movement_default"

#define HUMAN_LOCOMOTION_VARIATION			"_human_locomotion_variation"

#define SPEED							"_speed"

#define JUKE_DIRECTION					"_juke_direction"
#define JUKE_DISTANCE					"_juke_distance"	

#define GRAPPLE_DIRECTION				"_grapple_direction"
#define LOCOMOTION_SHOULD_TURN			"_locomotion_should_turn"	
#define SHOULD_TURN						"should_turn"
#define SHOULD_NOT_TURN					"should_not_turn"

#define GIB_LOCATION					"_gib_location"

// Robots
#define MIND_CONTROL					"_mind_control"
#define MOVE_MODE						"_move_mode"
#define GIBBED_LIMBS					"_gibbed_limbs"
#define ROBOT_STEP_IN					"_robot_step_in"
#define ROBOT_JUMP_DIRECTION			"_robot_jump_direction"
#define ROBOT_LOCOMOTION_TYPE			"_robot_locomotion_type"
#define ROBOT_WALLRUN_DIRECTION			"_robot_wallrun_direction"
#define ROBOT_TRAVERSAL_TYPE			"_robot_traversal_type"
#define ROBOT_MODE						"_robot_mode"

//Zombies
#define	LOCOMOTION_SPEED_TYPE			"_locomotion_speed"	
#define LOCOMOTION_SPEED_WALK			"locomotion_speed_walk"	
#define LOCOMOTION_SPEED_RUN			"locomotion_speed_run"
#define LOCOMOTION_SPEED_SPRINT			"locomotion_speed_sprint"
#define LOCOMOTION_SPEED_SUPER_SPRINT	"locomotion_speed_super_sprint"
#define LOCOMOTION_SPEED_JUMP_PAD_SUPER_SPRINT	"locomotion_speed_jump_pad_super_sprint"
#define LOCOMOTION_SPEED_BURNED			"locomotion_speed_burned"
#define LOCOMOTION_SPEED_SLIDE			"locomotion_speed_slide"

#define	HAS_LEGS_TYPE					"_has_legs"
#define	HAS_LEGS_YES					"has_legs_yes"
#define	HAS_LEGS_NO						"has_legs_no"
	
#define IDGUN_DAMAGE_DIRECTION			"_idgun_damage_direction"

#define	WHICH_BOARD_PULL_TYPE			"_which_board_pull"
#define BOARD_ATTACK_SPOT				"_board_attack_spot"

#define SHOULD_RUN						"_should_run"
#define SHOULD_RUN_YES					"run"
#define SHOULD_RUN_NO					"walk"

#define SHOULD_HOWL						"_should_howl"
#define SHOULD_HOWL_YES					"howl"
#define SHOULD_HOWL_NO					"dont_howl"

#define LOCOMOTION_SPEED_1				"speed_1"
#define LOCOMOTION_SPEED_2				"speed_2"
#define LOCOMOTION_SPEED_3				"speed_3"	
	
#define ARMS_POSITION					"_arms_position"
#define ARMS_UP							"arms_up"
#define ARMS_DOWN						"arms_down"

#define ZOMBIE_DAMAGEWEAPON_TYPE		"_zombie_damageweapon_type"
#define ZOMBIE_DAMAGEWEAPON_REGULAR		"regular"
#define	ZOMBIE_DAMAGEWEAPON_PACKED		"packed"

#define PARASITE_FIRING_RATE			"_parasite_firing_rate"
	
#define MARGWA_HEAD						"_margwa_head"
#define MARGWA_HEAD_LEFT				"left"
#define MARGWA_HEAD_MIDDLE				"middle"
#define MARGWA_HEAD_RIGHT				"right"

#define MARGWA_TELEPORT					"_margwa_teleport"
#define MARGWA_TELEPORT_IN				"in"
#define MARGWA_TELEPORT_OUT				"out"

#define GETUP_DIRECTION					"_getup_direction"
#define GETUP_BACK						"getup_back"
#define GETUP_BELLY						"getup_belly"

#define KNOCKDOWN_DIRECTION				"_knockdown_direction"
#define PUSH_DIRECTION					"_push_direction"
#define KNOCKDOWN_TYPE					"_knockdown_type"
#define KNOCKDOWN_STUN					"knockdown_stun" //contains all the possible knockdown animations
#define KNOCKDOWN_SHOVED				"knockdown_shoved" //only has the quick to the ground knockdown animations

#define PANIC							"_panic"
#define PANIC_YES						"panic"
#define PANIC_NO						"calm"
	
#define HUMAN_COVER_FLANKABILITY		"_human_cover_flankability"	// will be set to YES if AI will look good if he reacts with flanking
#define HUMAN_COVER_FLANKABLE			"flankable"	
#define HUMAN_COVER_UNFLANKABLE			"unflankable"	

#define	RUN_N_GUN_VARIATION				"_run_n_gun_variation"
#define	RUN_N_GUN_FORWARD				"variation_forward"	
#define	RUN_N_GUN_STRAFE_1				"variation_strafe_1"	
#define	RUN_N_GUN_STRAFE_2				"variation_strafe_2"

#define MECHZ_PART						"_mechz_part"
#define MECHZ_PART_FACEPLATE			"mechz_faceplate"
#define MECHZ_PART_POWERCORE			"mechz_powercore"
#define MECHZ_PART_GUN					"mechz_gun"	

#define WHIRLWIND_SPEED					"_whirlwind_speed"
#define WHIRLWIND_NORMAL				"whirlwind_normal"
#define WHIRLWIND_FAST					"whirlwind_fast"

#define QUAD_WALL_CRAWL					"_quad_wall_crawl"
#define QUAD_WALL_CRAWL_START			"quad_wall_crawl_start"
#define QUAD_WALL_CRAWL_THEATER			"quad_wall_crawl_theater"

#define QUAD_PHASE_DIRECTION			"_quad_phase_direction"
#define QUAD_PHASE_LEFT					"quad_phase_left"
#define QUAD_PHASE_RIGHT				"quad_phase_right"
#define QUAD_PHASE_FORWARD				"quad_phase_forward"

#define QUAD_PHASE_DISTANCE				"_quad_phase_distance"
#define QUAD_PHASE_SHORT				"quad_phase_short"
#define QUAD_PHASE_LONG					"quad_phase_long"
	
#define BLACKHOLEBOMB_PULL_STATE		"_zombie_blackholebomb_pull_state"
#define BLACKHOLEBOMB_PULL_SLOW			"bhb_pull_slow"
#define BLACKHOLEBOMB_PULL_FAST			"bhb_pull_fast"
	
// -------------------------------------------- BLACKBOARD DEFINES --------------------//

// -------------------------------------------- AIM RELATED --------------------------//
#define AIM_RIGHT "aim_right" 
#define AIM_LEFT  "aim_left"
#define AIM_UP    "aim_up"
#define AIM_DOWN  "aim_down"

#define AIM_LIMIT_TABLE_ENTRY_DEFAULT					"default"
#define AIM_LIMIT_TABLE_ENTRY_COVER 	 				"cover"
#define AIM_LIMIT_TABLE_ENTRY_LOCOMOTION 				"locomotion"
#define AIM_LIMIT_TABLE_ENTRY_COVER_LEFT_LEAN			"cover_left_lean"
#define AIM_LIMIT_TABLE_ENTRY_COVER_RIGHT_LEAN			"cover_right_lean"
#define AIM_LIMIT_TABLE_ENTRY_COVER_OVER				"cover_over"
#define AIM_LIMIT_TABLE_ENTRY_COVER_CONCEALED_OVER		"cover_concealed_over"
#define AIM_LIMIT_TABLE_ENTRY_COVER_VANTAGE				"cover_vantage"
#define AIM_LIMIT_TABLE_ENTRY_PILLAR_LEFT_LEAN			"pillar_left_lean"
#define AIM_LIMIT_TABLE_ENTRY_PILLAR_RIGHT_LEAN			"pillar_right_lean"

// -------------------------------------------- STANCES -------------------------------//
#define STANCE_STAND				"stand"
#define STANCE_CROUCH				"crouch"
#define STANCE_PRONE				"prone"
#define STANCE_PRONE_ON_BACK		"prone_back"
#define STANCE_PRONE_ON_FRONT		"prone_front"
#define STANCE_SWIM					"swim"

#define DEFAULT_STANCE				"stand"
#define DEFAULT_MOVEMENT_STANCE		"stand"

// -------------------------------------------- WEAPONS -------------------------------//
#define DEFAULT_WEAPON				"rifle"

// -------------------------------------------- DAMAGE -------------------------------//
#define HEAVY_DAMAGE_RATIO 	0.7
#define HEAVY_CONSECUTIVE_ATTACK_INTERVAL 1 * 1000	
	
#define DAMAGE_HEAVY "heavy"
#define DAMAGE_LIGHT "light"

// -------------------------------------------- TACTICAL WALK -------------------------//
#define RELATIVE_DIR_NONE  0
#define RELATIVE_DIR_FRONT 1
#define RELATIVE_DIR_LEFT  2
#define RELATIVE_DIR_RIGHT 3
#define RELATIVE_DIR_BACK  4
	
// -------------------------------------------- CORNER PREDICTION ----------------------//
#define CORNER_PREDICTOR_STATUS_SUCCESS 0 
#define CORNER_PREDICTOR_STATUS_NO_PATH 1
#define CORNER_PREDICTOR_STATUS_STRAIGHT_LINE_TO_GOAL 2
#define CORNER_PREDICTOR_STATUS_EXITING_COVER	3
#define CORNER_PREDICTOR_STATUS_EXITING_EXPOSED 4
#define CORNER_PREDICTOR_STATUS_ARRIVING_COVER 5
#define CORNER_PREDICTOR_STATUS_ARRIVING_EXPOSED 6
	
#define CORNER_PREDICTION_DISTANCE 100
#define MIN_EXITYAW_DISTANCE_SQ 64 * 64
#define INVALID_EXIT_YAW -1	

// -------------------------------------------- ARRIVAL -------------------------------//
#define MIN_PATH_DIST_FOR_ARRIVAL_PREDICTION_SQ 50 * 50	
#define DEFAULT_DESIRED_ARRIVAL_DISTANCE 0
#define DEFAULT_ARRIVAL_YAW	0	
	
// -------------------------------------------- LOCMOTION PAIN ------------------------//
#define LOCOMOTION_MOVING_PAIN_DIST_SHORT 150
#define LOCOMOTION_MOVING_PAIN_DIST_MED   200
#define LOCOMOTION_MOVING_PAIN_DIST_LONG  300
	
#define LOCOMOTION_MOVING_PAIN_SHORT "locomotion_moving_pain_short"
#define LOCOMOTION_MOVING_PAIN_MED   "locomotion_moving_pain_med"
#define LOCOMOTION_MOVING_PAIN_LONG  "locomotion_moving_pain_long"
	
#define LOCOMOTION_INPLACE_PAIN "locomotion_inplace_pain"

// -------------------------------------------- DAMAGE DIRECTION ------------------------//
#define DAMAGE_DIRECTION_FRONT "front"
#define DAMAGE_DIRECTION_LEFT  "left"
#define DAMAGE_DIRECTION_BACK  "back"
#define DAMAGE_DIRECTION_RIGHT "right"	


// -------------------------------------------- SPECIAL DEATH ------------------------//
#define SPECIAL_DEATH					"_special_death"
#define SPECIAL_DEATH_BALCONY			"balcony"
#define SPECIAL_DEATH_BALCONY_NORAIL	"balcony_norail"
#define SPECIAL_DEATH_NONE				"none"

// -------------------------------------------- AWARENESS ------------------------//
#define AST_AWARENESS					"_awareness"
#define AST_AWARENESS_PREVIOUS			"_awareness_prev"

// -------------------------------- STAIRS ------------------------//
#define STAIRCASE_TYPE					"_staircase_type"
#define STAIRCASE_STATE					"_staircase_state"
#define STAIRCASE_START					"staircase_start"
#define STAIRCASE_LOOP					"staircase_loop"
#define STAIRCASE_END					"staircase_end"
#define STAIRCASE_EXIT_TYPE				"_staircase_exit_type"
#define STAIRCASE_UP_EXIT_L_3_STAIRS	"staircase_up_exit_l_3_stairs"
#define STAIRCASE_UP_EXIT_L_4_STAIRS	"staircase_up_exit_l_4_stairs"
#define STAIRCASE_UP_EXIT_R_3_STAIRS	"staircase_up_exit_r_3_stairs"
#define STAIRCASE_UP_EXIT_R_4_STAIRS	"staircase_up_exit_r_4_stairs"
#define STAIRCASE_DOWN_EXIT_L_2_STAIRS	"staircase_down_exit_l_2_stairs"
#define STAIRCASE_DOWN_EXIT_R_2_STAIRS	"staircase_down_exit_r_2_stairs"
#define STAIRCASE_STEP_SKIP_NUM			"_staircase_skip_num"

#define STAIR_SKIP_3					"staircase_skip_3"
#define STAIR_SKIP_6					"staircase_skip_6"
#define STAIR_SKIP_8					"staircase_skip_8"

// skip4 Not implemented yet
#define STAIR_SKIP_4					"staircase_skip_4" 

	
#define STAIRCASE_DIRECTION				"_staircase_direction"
#define STAIRCASE_UP					"staircase_up"
#define STAIRCASE_DOWN					"staircase_down"
	
#define STAIRCASE_NUM_TOTAL_STEPS		"_staircase_num_total_steps"
#define STAIRCASE_NUM_STEPS				"_staircase_num_steps"
	
#define STAIRCASE_END_THRESHOLD_DISTSQ 16*16
// -------------------------------------------- HITLOCS ------------------------//
// same as hitLocation_t enum in q_shared.h
	
#define HITLOC_HELMET 	  "helmet"
#define HITLOC_HEAD 	  "head"
#define HITLOC_NECK 	  "neck"	
#define HITLOC_TORSO_UPR  "torso_upper"
#define HITLOC_TORSO_MID  "torso_mid"	
#define HITLOC_TORSO_LWR  "torso_lower"	
#define HITLOC_R_ARM_UPR  "right_arm_upper"	
#define HITLOC_L_ARM_UPR  "left_arm_upper"	
#define HITLOC_R_ARM_LWR  "right_arm_lower"	
#define HITLOC_L_ARM_LWR  "left_arm_lower"	
#define HITLOC_R_HAND	  "right_hand"		
#define HITLOC_L_HAND	  "left_hand"	
#define HITLOC_R_LEG_UPR  "right_leg_upper"	
#define HITLOC_L_LEG_UPR  "left_leg_upper"	
#define HITLOC_R_LEG_LWR  "right_leg_lower"	
#define HITLOC_L_LEG_LWR  "left_leg_lower"	
#define HITLOC_R_FOOT	  "right_foot"		
#define HITLOC_L_FOOT	  "left_foot"	
#define HITLOC_GUN		  "gun"	
#define HITLOC_SHIELD	  "riotshield"			
	
#define HITLOC_RIGHT_ARM	"right_arm"
#define HITLOC_LEFT_ARM		"left_arm"
#define HITLOC_CHEST	 	"chest"		
#define HITLOC_HIPS		 	"hips"			
#define HITLOC_LEGS		 	"legs"	
#define HITLOC_GROIN		"groin"


#define MELEE_ENEMY_TYPE	"_melee_enemy_type"
#define MELEE_ENEMY_TYPE_FIREFLY	"fireflyswarm"
	
	
#define IS_HITLOC_HEAD(__sHitLoc)			( IsInArray( array( HITLOC_HELMET, HITLOC_HEAD, HITLOC_NECK ), __sHitLoc ) )
#define IS_HITLOC_CHEST(__sHitLoc)			( IsInArray( array( HITLOC_TORSO_UPR, HITLOC_TORSO_MID ), __sHitLoc ) )
#define IS_HITLOC_HIPS(__sHitLoc)			( IsInArray( array( HITLOC_TORSO_LWR ), __sHitLoc ) )
#define IS_HITLOC_RIGHT_ARM(__sHitLoc)		( IsInArray( array( HITLOC_R_ARM_UPR, HITLOC_R_ARM_LWR, HITLOC_R_HAND, HITLOC_GUN ), __sHitLoc )	)
#define IS_HITLOC_LEFT_ARM(__sHitLoc)		( IsInArray( array( HITLOC_L_ARM_UPR, HITLOC_L_ARM_LWR, HITLOC_L_HAND ), __sHitLoc ) )
#define IS_HITLOC_LEGS(__sHitLoc)			( IsInArray( array( HITLOC_R_LEG_UPR, HITLOC_L_LEG_UPR, HITLOC_R_LEG_LWR, HITLOC_L_LEG_LWR, HITLOC_R_FOOT, HITLOC_L_FOOT ), __sHitLoc )	)	
#define IS_HITLOC_SHIELD(__sHitLoc) 		( IsInArray( array( HITLOC_SHIELD ), __sHitLoc ) )

#define IS_HITLOC_LEFT_LEG(__sHitLoc)		( IsInArray( array( HITLOC_L_LEG_UPR, HITLOC_L_LEG_LWR, HITLOC_L_FOOT ), __sHitLoc ) )
#define IS_HITLOC_RIGHT_LEG(__sHitLoc)		( IsInArray( array( HITLOC_R_LEG_UPR, HITLOC_R_LEG_LWR, HITLOC_R_FOOT ), __sHitLoc ) )
	
#define HITLOC_ALL							array( HITLOC_HELMET, HITLOC_HEAD, HITLOC_NECK, HITLOC_TORSO_UPR, HITLOC_TORSO_MID, HITLOC_TORSO_LWR, HITLOC_R_ARM_UPR, HITLOC_L_ARM_UPR, HITLOC_R_ARM_LWR, HITLOC_L_ARM_LWR, HITLOC_R_HAND, HITLOC_L_HAND, HITLOC_R_LEG_UPR, HITLOC_L_LEG_UPR, HITLOC_R_LEG_LWR, HITLOC_L_LEG_LWR, HITLOC_R_FOOT, HITLOC_L_FOOT, HITLOC_GUN, HITLOC_SHIELD )