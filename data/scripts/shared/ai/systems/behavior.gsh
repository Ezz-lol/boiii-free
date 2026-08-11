#define BT_REGISTER_API(name,function) \
	BehaviorTreeNetworkUtility::RegisterBehaviorTreeScriptAPI(name,function);

#define BT_REGISTER_ACTION(name,initFunction,updateFunction,terminateFunction) \
	BehaviorTreeNetworkUtility::RegisterBehaviorTreeAction(name,initFunction,updateFunction,terminateFunction);
	
#define BT_REGISTER_ACTION_SIMPLE(name) \
	BehaviorTreeNetworkUtility::RegisterBehaviorTreeAction(name,undefined,undefined,undefined);	

// at 96 units distance, AI will prefer to walk instead of running to it.
#define TACTICAL_WALK_SHORT_DIST_SQ	 96 * 96

// standing offset at covernodes based on the AI pose	
#define COVER_LEFT_YAW_OFFSET   90.0
#define COVER_RIGHT_YAW_OFFSET -90.0
#define COVER_PILLAR_YAW_OFFSET 180.0
	
// yaws used to check if the AI can see the enemy
#define COVER_CORNER_VALID_YAW_RANGE_MAX 60	
#define COVER_CORNER_VALID_YAW_RANGE_MIN 10

// offsets used to check if the AI can see the enemy	
#define COVER_LEFT_OFFSET   (-36, 7, 63)
#define COVER_RIGHT_OFFSET  (36, 7, 63)
#define COVER_CROUCH_OFFSET (3.5, -12.5, 45)
#define	COVER_STAND_OFFSET	(-3.7, -22, 63)
#define COVER_PILLAR_LEFT_OFFSET (-32, 3.7, 60)
#define COVER_PILLAR_RIGHT_OFFSET (32, 3.7, 60)
	
#define SEEN_ENEMY_RECENTLY_INTERVAL 4 // in sec, if AI has not seen enemy for 4 sec, then he will be considered hidden
	
// maximum time for idle action at cover, ir-respective of the animation length	
#define MAX_IDLE_TIME_MILLIS 2000	
	
// grenade throw ranges	
#define MIN_GRENADE_THROW_DIST_SQ		SQR(500)
#define MAX_GRENADE_THROW_DIST_SQ		SQR(1250)

// grenade throw modifiers
#define MIN_GRENADE_TEAM_TIME			1000				// Lower bound of time before a person of the same team can throw another grenade
#define MAX_GRENADE_TEAM_TIME			2000				// Upper bound of time before a person of the same team can throw another grenade

#define MIN_GRENADE_ANIMSCRIPTED_TIME	3000				// Lower bound of time before a person that finished a scripted animation can throw a grenade
#define MAX_GRENADE_ANIMSCRIPTED_TIME	4000				// Upper bound of time before a person that finished a scripted animation can throw a grenade

#define MIN_GRENADE_THROW_TIME				15000				// Lower bound of time before being able to throw another grenade
#define MAX_GRENADE_THROW_TIME				20000				// Upper bound of time before being able to throw another grenade
#define GRENADE_OVERLAP_DIST_SQ				(600 * 600)			// New grenade throws must land atleast this far from any previous throw, within the GRENADE_THROW_TIME window.
#define ALLIED_GRENADE_SAFE_DIST_SQ			(500 * 500 )		// Don't throw a grenade if you are on team allies and it will land this close to a player (may want to change to any ally) 
#define LASTSTAND_GRENADE_SAFE_DIST_SQ		(500 * 500 )		// Don't throw a grenade if there is a player in laststand this close to the target.

// Melee ranges	and timers 
#define CHARGE_RANGE_SQ_VS_PLAYER	SQR(140)
#define MELEE_RANGE_SQ				SQR(64)
#define BLEND_MELEE_RANGE_SQ		SQR(74)		// allowed 10 units to account for charge blend
#define MELEE_NEAR_RANGE			36
#define MELEE_NEAR_RANGE_SQ 		SQR(MELEE_NEAR_RANGE)
#define MELEE_RANGE_MAX_SQ			CHARGE_RANGE_SQ_VS_PLAYER		// charge distance is the minimum distance required to even attempt melee
	
#define NEXT_CHARGE_MELEE_TIME 		2000
#define MELEE_YAW_THRESHOLD			80
#define MELEE_YAW_THRESHOLDNEAR		40
#define MELEE_ENEMY_DISTANCE_PREDICTION_TIME 0.25
#define MAX_MELEE_PLAYER_ATTACKERS	1

// Arrival
#define MIN_ARRIVAL_DIST_SQ ( 64 * 64 )
#define ARRIVAL_COVER_SPLIT_NOTETRACK "cover_split"

// Distance at which enmey is considered to be nearby
#define CLOSE_ENEMY_DISTANCE_SQ ( 150 * 150	)
	
// Cover modes
#define COVER_MODE_OVER "over"
#define COVER_MODE_LEAN "lean"

//Sprint values
#define SPRINT_GAP_TIME 5000		//delay between two sprints
#define RANDOM_SPRINT_TIME 1000 	//upto one more second delay between sprint
#define ADDITIONAL_SPRINT_GAP 2000	//additional compensation factor
#define SPRINT_PROBABILITY	40		//value between 0 and 100, probability of sprinting

// Cover shoot times
#define COVER_SHOOT_MIN_TIME		800
#define COVER_SHOOT_TAKEDOWN_TIME	3000

	
// *********** SUMEET TODO - Merge seeking and exposed reacquiring behavior together. ***********

// Reacquire/ Cover/Locomotion Seek 
#define COS_60						0.5
#define REACQUIRE_STEP_SIZE			32
#define REACQUIRE_RESET				15
	
#define SEEK_OUT_ENEMY_RADIUS_BIG   400
#define SEEK_OUT_ENEMY_RADIUS_SMALL 256
#define SEEK_OUT_ENEMY_INTERVAL_SUCCESS 5 * 1000
#define SEEK_OUT_ENEMY_INTERVAL_FAILURE 2 * 1000
#define SEEK_OUT_ENEMY_FAILURE_MAX_ATTEMPTS 5