
#define KNIFE_MODEL 					"t6_wpn_knife_melee"	

#define FRONT_ANGLE_THRESHOLD 			120
#define BACK_ANGLE_THRESHOLD 			60 
#define TAG_SYNC 						"tag_sync"

#define SYNCED_MELEE_DIST_THRESHOLD_SQ 	(30 * 30)
#define MELEE_TRACE_Z_OFFSET 			8.0
#define SYNC_MELEE_SLOPE_THRESHOLD		16.0

#define SYNC_MELEE_BLEND_TIME			0.2
#define SYNC_MELEE_LERP_TIME			0.3
	
#define FRONT_WRESTLE_TYPE 				0	
#define FRONT_FLIP_TYPE 				1
#define BACK_TYPE 						2
#define RIGHT_TYPE						3
#define LEFT_TYPE						4

#define REGULAR_VARIANT					"regular"
#define MELEE_VARIANT					"melee"

#define GET_MELEE_BUNDLES				struct::get_script_bundles( "aiassassination" )
