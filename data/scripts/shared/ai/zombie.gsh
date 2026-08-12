// Melee ranges	and timers 
#define ZM_MELEE_YAW						60
#define ZM_MELEE_DIST						64
#define ZM_MELEE_DIST_SQ					64 * 64

// Vision ranges
#define ZM_SIGHT_DIST_SQ					128 * 128

// used for getting points on the navmesh
#define ZM_SEARCH_RADIUS					48


#define ZM_POSITION_QUERY_RADIUS			20

#define ZM_POSITION_QUERY_MOVE_DIST_MAX		120
	
#define ZM_POSITION_QUERY_LAST_STAND_MOVE_DIST_MIN		12*40
#define ZM_POSITION_QUERY_LAST_STAND_MOVE_DIST_MAX		12*80

// Crawler blocking
#define ZM_CRAWLER_PUSH_DIST_SQ				120 * 120
#define ZM_CRAWLER_PUSH_DISABLE_TIME		2000

#define ASM_ZOMBIE_MELEE_NOTETRACK					"zombie_melee"
#define ASM_ZOMBIE_DOG_MELEE_NOTETRACK				"dog_melee"
#define ASM_ZOMBIE_CRUSH_NOTETRACK					"crushed"

#define ZM_IDGUN_HOLE_PULL_ACC	0.5	
#define ZM_IDGUN_HOLE_PULL_MAX_SPEED	12
	
// Juking
#define ZM_JUKE_TIME_MIN					7500
#define ZM_JUKE_TIME_MAX					9500
#define ZM_JUKE_CHANCE						25
	
// Stumbling
#define ZM_STUMBLE_TIME_MIN					9000
#define ZM_STUMBLE_TIME_MAX					12000
#define ZM_STUMBLE_CHANCE					5
#define ZM_STUMBLE_MIN_DISTANCE_SQ			50000

#define ZM_CAN_STUMBLE						25
	
// Pain
#define ZM_PAIN_DIST_SQ_MAX					4000

	// Suicide
#define ZOMBIE_SUICIDE_RANGE				200
#define	ZOMBIE_SUICIDE_RANGE_SQ				( 200 * 200 )	
	
// Movement checking
#define ZM_MOVE_TIME						1000
#define ZM_MOVE_DIST_SQ						12 * 12	
	
///////////////////////////////////////////////////////////////////////////////
//
//										Clientfields
//
///////////////////////////////////////////////////////////////////////////////

#define ZOMBIE_CLIENTFIELD								"zombie"
#define ZOMBIE_SPECIAL_DAY_EFFECTS_CLIENTFIELD			"zombie_special_day"	
