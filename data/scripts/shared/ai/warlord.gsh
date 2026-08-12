///////////////////////////////////////////////////////////////////////////////
//
// 								Tweakables
//
///////////////////////////////////////////////////////////////////////////////

// Millisecond cooldown time before the warlord can juke again.
// Anything less than 2000 is less than the juke animation playback length.
#define WARLORD_NEXT_JUKE_TIME						3000

// Health pool the warlord uses when in the tug-of-war recharge state.
#define WARLORD_RECHARGE_HEALTH_MAX					1000.0

// The initial health added to the warlord's health pool when entering the recharge state.
// NOTE - This is truncated to WARLORD_RECHARGE_HEALTH_MAX and should be positive.
#define WARLORD_RECHARGE_INITIAL_HEALTH				0.0

// The amount of health to give the warlord per second when recharging.
#define WARLORD_RECHARGE_HEALTH_PER_SECOND			50.0

// Health percentage of the warlord to play the medium damage FX.
#define WARLORD_DAMAGE_STATE_PERCENTAGE				0.5

// Health percentage of the warlord to play the heavy damage FX.
#define WARLORD_HEAVY_DAMAGE_STATE_PERCENTAGE		0.25

// The starting health of the warlord's shield.
#define WARLORD_SHIELD_HEALTH_MAX					1000

// The percent of health the shield has to be to turn orange.
#define WARLORD_SHIELD_MEDIUM_HEALTH_PERCENTAGE	0.6

// The percent of health the shield has to be to turn red.
#define WARLORD_SHIELD_CRITICAL_HEALTH_PERCENTAGE	0.3

// Maximum distance from the player the warlord will try to charge melee them.
#define WARLORD_CHARGE_DISTANCE						280

// How often the warlord will attempt to charge melee.
#define WARLORD_NEXT_CHARGE_TIME					7500

// in seconds
#define WARLORD_SEEN_ENEMY_TOLERENCE				2

#define WARLORD_HUNT_MIN_RADIUS						150

#define WARLORD_HUNT_MAX_RADIUS						250
	
#define WARLORD_HUNT_DURATION						15000
	
#define WARLORD_HUNT_MAX_ACCUMILATED_DAMAGE			350
	
#define WARLORD_ANGRY_ACCUMILATED_DAMAGE_MAX_DUR	1500	
	
#define WARLORD_ANGRY_MAX_ACCUMILATED_DAMAGE		200		
	
#define WARLORD_NEXT_ANGRYATTACK_TIME				13000
	
#define WARLORD_NEAR_JUKE_DISTANCE					300	
	
#define WARLORD_PAIN_COOLDOWN_MIN					500
#define WARLORD_PAIN_COOLDOWN_MAX					2500
	
#define WARLORD_DAMAGE_ACTORS_REDUCTION_MULTIPLIER		0.05	
#define WARLORD_DAMAGE_EXPLOSIVE_REDUCTION_MULTIPLIER	0.25
	
///////////////////////////////////////////////////////////////////////////////
//
// 								Warlord Defines
//
///////////////////////////////////////////////////////////////////////////////

#define WARLORD_RADIUS								18
	
#define WARLORD_TACTICAL_JUKE_RADIUS				50	

#define WARLORD_JUKE_DISTANCE						145
	
#define WARLORD_JUKE_TOO_CLOSE_DIST					250	

#define WARLORD_NAVMESH_THRESHOLD					6

#define WARLORD_LEAP_THRESHOLD						20

#define WARLORD_LEAP_DISTANCES						array( 100, 300 )

#define WARLORD_LEAP_NAMES							array( "short", "long" )

#define WARLORD_HEIGHT								90

#define WARLORD_DIAMETER							36

#define WARLORD_MELEE_RANGE_SQ						SQR(100)
	
#define WARLORD_MELEE_BLAST_RADIUS					96
	
#define WARLORD_MELEE_BLAST_DAMAGE_MIN				50
	
#define WARLORD_MELEE_BLAST_DAMAGE_MAX				50
	
#define WARLORD_REPULSOR_RADIUS						256
	
#define WARLORD_NAVMESH_TOLERANCE					200
	
	
///////////////////////////////////////////////////////////////////////////////
//
//										Clientfields
//
///////////////////////////////////////////////////////////////////////////////

#define WARLORD_DAMAGE_STATE_CLIENTFIELD				"warlord_damage_state"
#define WARLORD_DAMAGE_STATE_UNDAMAGED					0
#define WARLORD_DAMAGE_STATE_DAMAGE						1
#define WARLORD_DAMAGE_STATE_HEAVY_DAMAGE				2
#define WARLORD_DAMAGE_STATE_DEAD						3

#define WARLORD_TYPE_CLIENTFIELD						"warlord_type"
#define WARLORD_TYPE_UKNOWN								0
#define WARLORD_TYPE_DEFAULT							1
#define WARLORD_TYPE_HVT								2

#define WARLORD_THRUSTER_CLIENTFIELD					"warlord_thruster_direction"
#define WARLORD_THRUSTER_OFF							0
#define WARLORD_THRUSTER_FRONT							1
#define WARLORD_THRUSTER_BACK							2
#define WARLORD_THRUSTER_LEFT							3
#define WARLORD_THRUSTER_RIGHT							4
	
#define WARLORD_LIGHTS_CLIENTFIELD						"warlord_lights_state"

///////////////////////////////////////////////////////////////////////////////
//
//									Warlord Spawn Types
//
///////////////////////////////////////////////////////////////////////////////

#define WARLORD_TYPE_NAME_HVT					"actor_spawner_bo3_warlord_enemy_hvt"
#define WARLORD_TYPE_NAME_DEFAULT				"actor_spawner_bo3_warlord_enemy_tool"

///////////////////////////////////////////////////////////////////////////////
//
//											FX
//
///////////////////////////////////////////////////////////////////////////////

#define WARLORD_DAMAGE_STATE_TAG					  "j_spine4"
#define WARLORD_DAMAGE_STATE_FX						  "fx_elec_warlord_damage_1"
#define WARLORD_DAMAGE_STATE_FX_FILE				"electric/fx_elec_warlord_damage_1"
	
#define WARLORD_DAMAGE_STATE_LOWER_TAG				"j_mainroot"
#define WARLORD_DAMAGE_STATE_LOWER_FX	        "fx_elec_warlord_lower_damage_1"
#define WARLORD_DAMAGE_STATE_LOWER_FX_FILE		"electric/fx_elec_warlord_lower_damage_1"

#define WARLORD_DAMAGE_HEAVY_STATE_FX				    "fx_elec_warlord_damage_2"
#define WARLORD_DAMAGE_HEAVY_STATE_FX_FILE			"electric/fx_elec_warlord_damage_2"

#define WARLORD_DAMAGE_HEAVY_STATE_LOWER_FX			  "fx_elec_warlord_lower_damage_2"
#define WARLORD_DAMAGE_HEAVY_STATE_LOWER_FX_FILE	"electric/fx_elec_warlord_lower_damage_2"

#define WARLORD_DEATH_FX							  "fx_exp_warlord_death"
#define WARLORD_DEATH_FX_FILE						"explosions/fx_exp_warlord_death"

#define WARLORD_DEATH_TAG							 "j_spine4"

#define WARLORD_THRUSTER_FX							"fx_exhaust_jetpack_warlord_juke"
#define WARLORD_THRUSTER_FX_FILE				"vehicle/fx_exhaust_jetpack_warlord_juke"

#define WARLORD_THRUSTER_FRONT_TAGS					array( "tag_jets_left_front", "tag_jets_right_front" )
#define WARLORD_THRUSTER_BACK_TAGS					array( "tag_jets_left_back", "tag_jets_right_back" )
#define WARLORD_THRUSTER_LEFT_TAGS					array( "tag_jets_left_side" )
#define WARLORD_THRUSTER_RIGHT_TAGS					array( "tag_jets_right_side" )
	
#define WARLORD_REPULSE_FX							"vehicle/fx_quadtank_airburst"
#define WARLORD_REPULSE_GROUND_FX				"vehicle/fx_quadtank_airburst_ground"
	
#define WARLORD_EYE_FX								  "light/fx_light_eye_glow_warlord"
	
#define WARLORD_HEAD_LIGHT_FX_FILE			"light/fx_light_eye_glow_warlord"		
#define WARLORD_HEAD_LIGHT_FX						"fx_light_eye_glow_warlord"
#define WARLORD_HEAD_LIGHT_TAG					"tag_eye"
	
#define WARLORD_BODY_LIGHT_FX_FILE 			"light/fx_light_body_glow_warlord"
#define WARLORD_BODY_LIGHT_FX						"fx_light_body_glow_warlord"
#define WARLORD_BODY_LIGHT_TAG					"j_spine4"
	
///////////////////////////////////////////////////////////////////////////////
//
//											Sounds
//
///////////////////////////////////////////////////////////////////////////////

#define WARLORD_SOUND_RECHARGE_START			"chr_warlord_field_start"
#define WARLORD_SOUND_RECHARGE_LOOP				"chr_warlord_field_loop"
#define WARLORD_SOUND_RECHARGE_END				"chr_warlord_field_stop"
