//Killstreak Rules
#define INVALID_KILLSTREAK_ID							(-1)

#define MISSILE_DRONE_TARGET_ACTIVE						( 2 )
#define MISSILE_DRONE_TARGET_ENABLED					( 1 )
#define MISSILE_DRONE_TARGET_DISABLED					( 0 )

//Hellfire missile
#define REMOTE_MISSILE_FIRED							( 1 )
#define REMOTE_MISSILE_PLAYER_DETONATED					( 2 )
//Hellfire missile Bomblets 
#define REMOTE_MISSILE_BOMBLET_FIRED					( 1 )


//AI_TANK
#define AI_TANK_AGR_NAME 								"ai_tank_drop"
#define AI_TANK_LIFETIME								( 120 * 1000 )
#define AI_TANK_MISSLE_COUNT_AFTER_RELOAD				( 3 )
	
	
//TURRET
#define TURRET_VEHICLE_NAME								"sentry_turret"
#define TURRET_MODEL									"veh_t7_turret_sentry_gun_world_mp"
#define TURRET_VALID_PLACEMENT_MODEL					"veh_t7_turret_sentry_gun_world_yellow"
#define TURRET_INVALID_PLACEMENT_MODEL					"veh_t7_turret_sentry_gun_world_red"
#define TURRET_REMOTE_TEXT								&"MP_REMOTE_USE_TURRET"
#define TURRET_VALID_PLACEMENT_TEXT						&"KILLSTREAK_SENTRY_TURRET_VALID_PLACEMENT"
#define TURRET_INVALID_PLACEMENT_TEXT					&"KILLSTREAK_SENTRY_TURRET_INVALID_PLACEMENT"
#define TURRET_PICKUP_TEXT								&"KILLSTREAK_SENTRY_TURRET_PICKUP"
#define TURRET_MAX_TARGET_DISTANCE						( 2500 )
#define TURRET_TARGET_ANGLE								( 15 )
#define TURRET_DURATION									( 90 * 1000 )
#define TURRET_HEALTH									( 2000 )
#define TURRET_EMP_DAMAGE_PERCENTAGE					( 1 )
#define TURRET_HEADICON_OFFSET							( 0, 0, 70 )
#define TURRET_WATCH_DEATH_DURATION						1.2

//MICROWAVE TURRET
#define MICROWAVE_TURRET_VEHICLE_NAME					"microwave_turret"
#define MICROWAVE_TURRET_MODEL							"veh_t7_turret_guardian"
#define MICROWAVE_TURRET_VALID_PLACEMENT_MODEL			"veh_t7_turret_guardian_yellow"
#define MICROWAVE_TURRET_INVALID_PLACEMENT_MODEL		"veh_t7_turret_guardian_red"
#define MICROWAVE_TURRET_PICKUP_TEXT					&"KILLSTREAK_MICROWAVE_TURRET_PICKUP"
#define MICROWAVE_TURRET_DURATION						( 90 * 1000 )
#define MICROWAVE_TURRET_STOP_SOUND						( 0 )
#define MICROWAVE_TURRET_START_SOUND					( 1 )
#define MICROWAVE_TURRET_HEALTH 						( 1800 )
#define MICROWAVE_TURRET_RADIUS							( 750 )
#define MICROWAVE_TURRET_CONE_ANGLE						( 15 )
#define MICROWAVE_TURRET_DAMAGE							( 15 )
#define MICROWAVE_TURRET_EMP_DAMAGE						( MICROWAVE_TURRET_HEALTH + 1 )
#define MICROWAVE_TURRET_TARGET_OFFSET					( 0, 0, 100 )
#define MICROWAVE_TURRET_HEADICON_OFFSET				( 0, 0, 80 )
#define MICROWAVE_TURRET_KILLCAM_FORWARD_OFFSET			( -100 )
#define MICROWAVE_TURRET_KILLCAM_OFFSET					( 0, 0, 100 )
#define MICROWAVE_TURRET_INITIAL_DAMAGE_DELAY_MAX		( 0.3 )
#define MICROWAVE_TURRET_INITIAL_DAMAGE_DELAY_MIN		( 0.1 )
	
//UAV
#define UAV_MODEL 										"veh_t7_drone_uav_enemy_vista"
#define FX_UAV_LIGHTS 									"killstreaks/fx_uav_lights"
#define FX_UAV_BURNER 									"killstreaks/fx_uav_bunner"
#define FX_UAV_DAMAGE_TRAIL 							"killstreaks/fx_uav_damage_trail"
#define UAV_HEALTH 										( 700 )
#define UAV_LOW_HEALTH 									( UAV_HEALTH * 0.5 )
#define UAV_DURATION 									( 25000 )
#define UAV_DURATION_CHECK 								( UAV_DURATION / 2 )
#define UAV_EXIT_TIME 									( 10 )
#define UAV_Z_OFFSET 									( 2500 )
#define UAV_ROTATION_RADIUS 							( 4000 )
#define UAV_ROTATION_RANDOM_OFFSET 						( 1000 )
	
//CUAV
#define COUNTER_UAV_HEALTH 								( 700 )
#define COUNTER_UAV_LOW_HEALTH 							( COUNTER_UAV_HEALTH * 0.5 )
#define COUNTER_UAV_DURATION_MS 						( 30000 )
#define COUNTER_UAV_DURATION_CHECK 						( COUNTER_UAV_DURATION_MS / 2 )
#define COUNTER_UAV_POSITION_Z_OFFSET 					( 1000 )
#define COUNTER_UAV_MODEL_SCALE 						( 1 )
#define COUNTER_UAV_POSITION_COUNT 						( 20 )
#define COUNTER_UAV_MAP_PERCENTAGE 						( 0.50 )					//Percentage of minimap coords to generate valid points from
#define COUNTER_UAV_LOOP_SOUND 							"veh_uav_engine_loop"
#define COUNTER_UAV_LOCATION_DURATION_MIN 				( 5 )
#define COUNTER_UAV_LOCATION_DURATION_MAX 				( 10 )
#define COUNTER_UAV_SPEED 								( 5 )
#define COUNTER_UAV_ACCELERATION_PERCENTAGE 			( 0.2 )
#define COUNTER_UAV_DECELERATION_PERCENTAGE 			( 0.2 )
#define COUNTER_UAV_ROTATION_DURATION 					( 0.5 )
#define COUNTER_UAV_ROTATION_ACCELERATION_PERCENTAGE 	( 0.2 )
#define COUNTER_UAV_ROTATION_DECELERATION_PERCENTAGE 	( 0.2 )
#define COUNTER_UAV_GROUP_SIZE 							( 3 )
#define COUNTER_UAV_GROUP_OFFSET 						( 450 )
	
//SATELLITE
#define SATELLITE_MODEL 								"veh_t7_drone_srv_blimp"
#define SATELLITE_MODEL_SCALE							( 1 )
#define SATELLITE_DURATION_MS							( 40000 )
#define SATELLITE_Z_OFFSET 								( 5500 )
#define SATELLITE_HEALTH 								( 700 )
#define SATELLITE_LOW_HEALTH 							( SATELLITE_HEALTH * 0.5 )
#define SATELLITE_MISSILES_TO_DESTROY 					( 3 )
#define SATELLITE_ROTATION_DURATION						( 10 )
#define SATELLITE_SPAWN_FROM_ANGLE_MIN					( 90.0 )		// spawns from an angle relative to map center (min)
#define SATELLITE_SPAWN_FROM_ANGLE_MAX					( 180.0 )		// spawns from an angle relative to map center (max)
#define SATELLITE_TRAVEL_DISTANCE_SCALE					( 1.5 )			// the scale of the distance traveled for DURATION_MS
	
//RAPS
#define RAPS_NAME 										"raps"
#define RAPS_DRONE_NAME 								"raps_drone"
#define RAPS_MODEL										"veh_t7_drone_raps"
#define RAPS_VEHICLE									"spawner_bo3_raps_mp"
#define RAPS_HELICOPTER_INFO							"heli_raps_mp"
#define RAPS_HELICOPTER_MODEL 							"veh_t7_mil_vtol_dropship_raps"
#define RAPS_HELICOPTER_MISSILES_TO_DESTROY				( 4 )
#define RAPS_HELICOPTER_MISSILE_DAMAGE					( ( RAPS_HELICOPTER_HEALTH / RAPS_HELICOPTER_MISSILES_TO_DESTROY ) + 1 )
#define RAPS_HELICOPTER_REMOTE_MISSILES_TO_DESTROY		( 1 )
#define RAPS_HELICOPTER_REMOTE_MISSILE_DAMAGE			( ( RAPS_HELICOPTER_HEALTH / RAPS_HELICOPTER_REMOTE_MISSILES_TO_DESTROY ) + 1 )
#define RAPS_HELICOPTER_HACKS_TO_DESTROY				( 2 )
#define RAPS_HELICOTPER_HACKER_TOOL_DAMAGE				( ( RAPS_HELICOPTER_HEALTH / RAPS_HELICOPTER_HACKS_TO_DESTROY ) + 1 )
#define RAPS_HELICOPTER_NAV_RADIUS_MIN 					( 0 )
#define RAPS_HELICOPTER_NAV_RADIUS_MAX 					( 2000 )
#define RAPS_HELICOPTER_NAV_MAX_POINT_COUNT 			( 100 )
#define RAPS_HELICOPTER_NAV_POINT_SPACING 				( 132 )
#define RAPS_HELICOPTER_NAV_SPACIOUS_POINT_BOUNDARY 	( 128 )
#define RAPS_HELICOPTER_NAV_MAP_CENTER_MAX_OFFSET 		( 1024 )
#define RAPS_HELICOPTER_NAV_TRACE_BOX_WIDTH				( 220 )
#define RAPS_HELICOPTER_NAV_ADDITIONAL_TRACE_HEIGHT		( 500 )
#define RAPS_HELICOPTER_NAV_END_POINT_TRACE_OFFSET		( 36 )
#define RAPS_HELICOPTER_NAV_POINT_TRACE_DEBUG			GetDvarInt( "scr_raps_nav_point_trace_debug" )
#define RAPS_HELICOPTER_NAV_DEBUG_VISIT_ALL				GetDvarInt( "scr_raps_debug_visit_all" )
#define RAPS_HELICOPTER_NAV_DEBUG_VISIT_ALL_FAKE_LEAVE	GetDvarInt( "scr_raps_debug_visit_all_fake_leave" )
#define RAPS_HELICOPTER_DEBUG_AUTO_REACTIVATE			GetDvarInt( "scr_raps_debug_auto_reactivate" )
#define RAPS_HELICOPTER_Z_OFFSET 						( 1000 )
#define RAPS_HELICOPTER_Z_OFFSET_PER_HELI				( 400 )
#define RAPS_HELICOPTER_FLY_HEIGHT						INT( airsupport::getMinimumFlyHeight() + RAPS_HELICOPTER_Z_OFFSET )
#define RAPS_HELICOPTER_DROP_LOCATION_COUNT 			( 3 )
#define RAPS_HELICOPTER_DROP_DURATION 					( 2.0 )
#define RAPS_HELICOPTER_DROP_DURATION_DELTA				( 2.0 )
#define RAPS_HELICOPTER_DROP_DURATION_LAST				( 2.0 )
#define RAPS_HELICOPTER_DROP_DURATION_LAST_DELTA		( 1.0 )	
#define RAPS_HELICOPTER_DROP_INTERVAL 					( 1 )
#define RAPS_HELICOPTER_DROP_OFFSET 					( 0, 0, -50 )
#define RAPS_HELICOPTER_DROP_LOCATION_TOLERANCE_SQR		( 5 * 5 )
#define RAPS_HELICOPTER_LEAVE_LOCATION_REACHED_SQR		( 600 * 600 )
#define RAPS_SLEEP_DURATION								( 3 )

//RAPS HELICOPTER DEPLOY POINT SELECTION
#define RAPS_HEDEPS_DEBUG								GetDvarInt( "scr_raps_hedeps_debug" )
#define RAPS_HEDEPS_UNSPECIFIED_AVOID_POINT				( -9999999, -9999999, -9999999 )
#define RAPS_HEDEPS_REDUCE_RADIUS_RETRIES				( 3 )
#define RAPS_HEDEPS_FIRST_POINT_PERCENT_OF_TOTAL		( 66.6 )
#define RAPS_HEDEPS_AVOID_RADIUS						( 1800 )
#define RAPS_HEDEPS_AVOID_RADIUS_SQR					( RAPS_HEDEPS_AVOID_RADIUS * RAPS_HEDEPS_AVOID_RADIUS )
	
//RAPS HELICOPTER AVOIDANCE
#define RAPS_HELAV_DEBUG								GetDvarInt( "scr_raps_helav_debug" )
#define RAPS_HELAV_TIME_BETWEEN_UPDATES					( 0.05 )
#define RAPS_HELAV_MIN_PICK_NEW_GOAL_TIME_MS			( 5000 )
#define RAPS_HELAV_STOP_WAIT_BEFORE_NEXT_DROP_POINT_MS	( 2000 )
#define RAPS_HELAV_MIN_TIME_BETWEEN_FULL_STOPS_MS		(  500 )
#define RAPS_HELAV_FORWARD_OFFSET						(  500 )
#define RAPS_HELAV_OTHER_FORWARD_OFFSET					(  100 )
#define RAPS_HELAV_IN_FRONT_DOT							( 0.707 )
#define RAPS_HELAV_FULL_STOP_DISTANCE					( 1200 )
#define RAPS_HELAV_NEED_NEW_GOAL_DISTANCE				(  200 + RAPS_HELAV_FULL_STOP_DISTANCE )
#define RAPS_HELAV_SLOW_DOWN_DISTANCE					( 1000 + RAPS_HELAV_NEED_NEW_GOAL_DISTANCE )
#define RAPS_HELAV_SLOW_DOWN_SPEED_SCALE_FACTOR			(  0.5 )
#define RAPS_HELAV_SLOW_DOWN_MORE_SCALE_FACTOR			(  0.2 )
#define RAPS_HELAV_SLOW_DOWN_DISTANCE_SQR				( RAPS_HELAV_SLOW_DOWN_DISTANCE * RAPS_HELAV_SLOW_DOWN_DISTANCE )
#define RAPS_HELAV_NEED_NEW_GOAL_DISTANCE_SQR			( RAPS_HELAV_NEED_NEW_GOAL_DISTANCE * RAPS_HELAV_NEED_NEW_GOAL_DISTANCE )
#define RAPS_HELAV_FULL_STOP_DISTANCE_SQR				( RAPS_HELAV_FULL_STOP_DISTANCE * RAPS_HELAV_FULL_STOP_DISTANCE )
#define RAPS_HELAV_DRIVE_MODE_EXPEDIENT					0
#define RAPS_HELAV_DRIVE_MODE_CAUTIOUS					1
#define RAPS_HELAV_DRIVE_MODE_MORE_CAUTIOUS				2
#define RAPS_HELAV_ALWAYS_PICK_NEW_GOAL_POST_BACK_OFF	( true )
#define RAPS_HELAV_BACK_OFF_FACTOR_MIN					( 0.7 )
#define RAPS_HELAV_BACK_OFF_FACTOR_MAX					( 2.5 )
#define RAPS_HELAV_EXPEDIENT_MODE_ACCEL					( 20 )
#define RAPS_HELAV_EXPEDIENT_MODE_DECEL					( 20 )
#define RAPS_HELAV_CAUTIOUS_MODE_ACCEL					( 12 )
#define RAPS_HELAV_CAUTIOUS_MODE_DECEL					( 100 )
#define RAPS_HELAV_FULL_STOP_MODE_ACCEL					( 10 )
#define RAPS_HELAV_FULL_STOP_MODE_DECEL					( 500 )

// RAPS damage/death fx
#define RAPS_HELI_FIRST_EXPLO_FX						"killstreaks/fx_heli_raps_exp_sm"
#define RAPS_HELI_FIRST_EXPLO_FX_TAG					"tag_fx_engine_exhaust_back"
#define RAPS_HELI_DEATH_TRAIL_FX						"killstreaks/fx_heli_raps_exp_trail"
#define RAPS_HELI_DEATH_TRAIL_FX_TAG_A					"tag_fx_engine_exhaust_back"
#define RAPS_HELI_DEATH_TRAIL_FX_TAG_B					"tag_fx_engine_left_front"
#define RAPS_HELI_DEATH_TRAIL_FX_TAG_C					"tag_fx_engine_right_back"
#define RAPS_HELI_DEATH_FX								"killstreaks/fx_heli_raps_exp_lg"
#define RAPS_HELI_DEATH_FX_TAG							"tag_fx_death"


//EMP
#define EMP_MODEL										"wpn_t7_turret_emp_core"
#define EMP_VALID_PLACEMENT_MODEL 						"wpn_t7_turret_emp_core_yellow"
#define EMP_INVALID_PLACEMENT_MODEL 					"wpn_t7_turret_emp_core_red"
#define EMP_FX_TAG										"tag_fx"
#define EMP_DURATION 									( 60 * 1000 )
#define EMP_HEALTH										( 3000 )
#define EMP_LOW_HEALTH									( EMP_HEALTH * 0.5 )
#define EMP_HEADICON_OFFSET								( 0, 0, 90 )
	
//DRONE STRIKE
#define DRONE_STRIKE_MODEL 								"veh_t7_drone_rolling_thunder"
#define DRONE_STRIKE_LOCATION_SELECTOR 					"map_directional_selector"
#define DRONE_STRIKE_COUNT 								( 12 )
#define DRONE_STRIKE_Z_OFFSET 							( 3000 )
#define DRONE_STRIKE_RIGHT_OFFSET 						( 300 )
#define DRONE_STRIKE_LEFT_OFFSET 						( 900 )
#define DRONE_STRIKE_FORWARD_OFFSET 					( 500 )
#define DRONE_STRIKE_SPAWN_INTERVAL 					( 1 )
#define DRONE_STRIKE_START_OFFSET 						( -14000 )
#define DRONE_STRIKE_END_OFFSET 						( -6000 )
#define DRONE_STRIKE_FLIGHT_TIME 						( 1.8 )

//HELICOPTER
#define HELICOPTER_CAMO_STATE_OFF						( 0 )
#define HELICOPTER_CAMO_STATE_ON						( 1 )
#define HELICOPTER_CAMO_STATE_FLICKER					( 2 )
#define HELICOPTER_CAMO_FLICKER_DURATION				( 1 )
#define HELICOPTER_CAMO_DAMAGE_LIMIT					( 100 )
#define HELICOPTER_CAMO_DAMAGE_DURATION					( 10 )
	
//FLAK DRONE
#define FLAK_DRONE_SPAWN_OFFSET							( 0, 0, -50 )			//Spawn offset from parent
#define FLAK_DRONE_VEHICLE_NAME							"veh_flak_drone_mp"
#define FLAK_DRONE_NEAR_GOAL_NOTIFY_DIST				( 40 )
#define FLAK_DRONE_HOVER_RADIUS							( 50.0 )
#define FLAK_DRONE_HOVER_SPEED							( 75.0 )
#define FLAK_DRONE_HOVER_ACCELERATION					( 100.0 )
#define FLAK_DRONE_STUN_DURATION						( 0.1 )
#define FLAK_DRONE_MOVE_SPEED							( 400 )
#define FLAK_DRONE_HOVER_INNER_RADIUS 					( 25 )
#define FLAK_DRONE_HOVER_OUTTER_RADIUS					( 75 )
#define FLAK_DRONE_HOVER_HEIGHT_VARIANCE				( 40 )
#define FLAK_DRONE_HOVER_POINT_SPACING					( 40 )
#define FLAK_DRONE_HOVER_HEIGHT							( -75 )
#define FLAK_DRONE_TIME_AT_SAME_POSITION_MIN			( .1 )
#define FLAK_DRONE_TIME_AT_SAME_POSITION_MAX			( .2 )
	
//HELICOPTER GUNNER
#define HELICOPTER_GUNNER_ANGLE_OFFSET					( 0 )
#define HELICOPTER_GUNNER_FORCED_WAIT 					( 0 )
#define HELICOPTER_GUNNER_INFRARED_VISION 				"remote_mortar_infrared"
#define HELICOPTER_GUNNER_ENHANCED_VISION 				"remote_mortar_enhanced"
#define HELICOPTER_GUNNER_Z_OFFSET 						( 2000 )
#define HELICOPTER_GUNNER_VEHICLE_NAME 					"veh_bo3_mil_gunship_mp"
#define HELICOPTER_GUNNER_DURATION						( 60000 )
#define HELICOPTER_GUNNER_ASSISTANT_SEAT_COUNT 			( 2 )
#define HELICOPTER_GUNNER_ASSISTANT_SEAT_START_INDEX 	( 1 )
#define HELICOPTER_GUNNER_ASSISTANT_0_TARGET_TAG		"tag_gunner_barrel1"
#define HELICOPTER_GUNNER_ASSISTANT_1_TARGET_TAG		"tag_gunner_barrel2"
#define HELICOPTER_GUNNER_HACKER_TOOL_RADIUS			( 50 )
#define HELICOPTER_GUNNER_HACKER_TOOL_DURATION			( 10000 )
#define HELICOPTER_GUNNER_HELICOPTER_HEALTH					( 15000 )
#define HELICOPTER_GUNNER_HELICOPTER_MISSILES_TO_DESTROY	( 6 )
	
#define HELICOPTER_GUNNER_CAMSHAKE_DELAY				( 0 )
#define HELICOPTER_GUNNER_CAMSHAKE_SIZE					( 1 )
#define HELICOPTER_GUNNER_CAMSHAKE_TIME					( 10 )

	
	
//DART
#define DART_VEHICLE_NAME 								"veh_dart_mp"
#define DART_DURATION 									( 30 * 1000 )
#define DART_MAX_SHOT_COUNT 							( 2 )
#define DART_WATCH_TIME 								( 2 )					//Time in seconds to watch the death of the dart
#define DART_EXPLOSION_DAMAGE_MIN						( 25 )
#define DART_EXPLOSION_DAMAGE_MAX						( 350 )
#define DART_EXPLOSION_CAM_DISTANCE						( 300 )
#define DART_MAX_HEIGHT_OFFSET							( 2000 )					//Offset from qrdrone_height node before out of range notification/shutdown
	
#define DART_HACKER_TOOL_RADIUS							( 50 )
#define DART_HACKER_TOOL_DURATION						( 2000 )

//SENTINEL
#define SENTINEL_VEHICLE_NAME 							"veh_sentinel_mp"
#define SENTINEL_RADIUS									( 20 )
#define SENTINEL_DURATION 								( 60000 )
#define SENTINEL_NEAR_GOAL_NOTIFY_DIST					( 40 )
#define SENTINEL_HOVER_RADIUS							( 50.0 )
#define SENTINEL_HOVER_SPEED							( 100.0 )
#define SENTINEL_HOVER_ACCELERATION						( 100.0 )
#define SENTINEL_BEGIN_COMBAT_DISTANCE					( 1500 )				//2D distance between sentinel and enemy to begin combat state
#define SENTINEL_ENGAGEMENT_DISTANCE_MIN				( 50 )					//Minimum distance between sentinel and enemy during combat
#define SENTINEL_ENGAGEMENT_DISTANCE_MAX				( 1000 )				//Maximum distance between sentinel and enemy during combat
#define SENTINEL_TURRET_ENEMEY_DETECT_FREQUENCY			( 0.2 )					//Time to wait after unsuccessful burst fire attempt ( sight line / distance check )
#define SENTINEL_TIME_AT_SAME_POSITION_MIN				( 0.1 )					//Min time to wait after reaching goal in unaware state
#define SENTINEL_TIME_AT_SAME_POSITION_MAX				( 0.2 )					//Max time to wait after reaching goal in unaware state
#define SENTINEL_HOVER_INNER_RADIUS						( 50 )					//Minimum radius from player for unaware target pos selection
#define	SENTINEL_HOVER_OUTTER_RADIUS					( 300 )					//Maximum radius from player for unaware target pos selection
#define SENTINEL_HOVER_POINT_SPACING					( 40 )					//Minimum spacing between valid target pos points
#define SENTINEL_HOVER_HEIGHT							( 50 )					//Height above sentinel owner for unaware target pos selection
#define SENTINEL_HOVER_HEIGHT_VARIANCE					( 40 )					//Variance +/- in height from SENTINEL_HOVER_HEIGHT
#define SENTINEL_ENGAGMENT_HEIGHT_MIN 					( 10 )					//Minimum height for combat position selection
#define SENTINEL_ENGAGMENT_HEIGHT_MAX 					( 100 )					//Maximum height for combat position selection
#define SENTINEL_STUN_DURATION							( 0.1 )					//Time to wait before attacking enemy after being shot
#define SENTINEL_MAX_DISTANCE_FROM_OWNER				( 1200 )				//When in combat, if the owner exceeds this 2d distance, the sentinel will breakaway from combat
#define SENTINEL_MAX_HEIGHT_OFFSET						( 0 )					//Offset from qrdrone_height node before out of range notification/shutdown
#define SENTINEL_SPAWN_Z_OFFSET							( 25 )					//Vertical offset from rcbomb spawn height
#define SENTINEL_EMP_DAMAGE_PERCENTAGE					( 0.5 )					//Percentage of healthDefault ( set in veh gdt ) that each emp grenade will damage
#define SENTINEL_MISSILES_TO_DESTROY					( 1 ) 					//Number of missiles required to destroy
	
//RCBOMB
#define RCBOMB_VEHICLE									"rc_car_mp"
#define RCBOMB_DURATION									( 40 * 1000 )
#define RCBOMB_PLACMENT_FROM_PLAYER						( 70 )
#define RCBOMB_PLACEMENT_STAND_HEIGHT					( 50 )
#define RCBOMB_PLACEMENT_CROUCH_HEIGHT					( 30 )
#define RCBOMB_PLACEMENT_PRONE_HEIGHT					( 15 )
#define RCBOMB_DEATH_HIDE_DELAY							( 0.1 )
#define RCBOMB_SHUTDOWN_DELAY							( 0.2 )
#define RCBOMB_SHUTDOWN_DELAY_ABANDONED					( 10 )
	
//COMBAT ROBOT
#define COMBAT_ROBOT_DURATION							( 90000 )
#define COMBAT_ROBOT_EMP_DURATION						( 750 )

//HELICOPTER COMLINK	
#define HELI_COMLINK_LIGHTS_ON_DELAY					( 0.7 )
	
//HIDE COMPASS (aka minimap)
#define AITANK_HIDE_COMPASS_ON_REMOTE_CONTROL				false
#define DART_HIDE_COMPASS_ON_REMOTE_CONTROL					true
#define RCBOMB_HIDE_COMPASS_ON_REMOTE_CONTROL				false
#define SENTINEL_HIDE_COMPASS_ON_REMOTE_CONTROL				false
#define TURRET_HIDE_COMPASS_ON_REMOTE_CONTROL				true
#define HELICOPTER_GUNNER_HIDE_COMPASS_ON_REMOTE_CONTROL	true

	
#define AI_TANK_VISIONSET_ALIAS					"agr_visionset"	
#define TURRET_VISIONSET_ALIAS					"turret_visionset"
#define DART_VISIONSET_ALIAS					"dart_visionset"
#define MOTHERSHIP_VISIONSET_ALIAS				"mothership_visionset"
#define SENTINEL_VISIONSET_ALIAS 				"sentinel_visionset"
#define REMOTE_MISSILE_VISIONSET_ALIAS			"remote_missile_visionset"


#define MOTHERSHIP_VISIONSET_FILE				"mp_vehicles_mothership"
#define AI_TANK_VISIONSET_FILE					"mp_vehicles_agr"
#define TURRET_VISIONSET_FILE					"mp_vehicles_turret"

	
#define TURRET_VISIONSET_LERP_STEP_COUNT		16
