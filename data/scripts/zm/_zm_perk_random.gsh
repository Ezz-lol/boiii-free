// Machine Lights (ScriptVector0.z)
// Off						0.00
// Gauges/Accessories       0.40
// Logo                     0.70
// Ball                     1.00
#define MACHINE_OFF						0.00
#define MACHINE_POWERED					0.40
#define MACHINE_AVAILABLE				0.70
#define MACHINE_ACTIVATED				1.00

// Artifact Glow (ScriptVector0.x)
// Off						1.00
// On						0.00
#define ARTIFACT_GLOW					0
#define ARTIFACT_DIM					1

// Sequence Timing
#define	DELAY_PRE_PORTAL_EFFECT		0.5 // delay after electrical activation effect starts, until portal effect starts
#define DELAY_UNTIL_BOTTLE_SPAWN	1.0 // delay from triggering the machine until the bottle appears
#define DELAY_UNTIL_BOTTLE_IN_PLACE	3.0 // delay from when the bottle visually spawns to when it becomes available to grab
#define DELAY_POST_PORTAL_EFFECT	0.5	// delay after portal effect ends, until lingering electricity ends
#define DELAY_MACHINE_TIMEOUT		10.0 // delay before the machine time's out

#define ZM_PERK_RANDOM_BALL_ON_OFF_PIECE_INDEX		0
#define ZM_PERK_RANDOM_BODY_ON_OFF_PIECE_INDEX		1
#define ZM_PERK_RANDOM_BODY_AVAILABLE_PIECE_INDEX	2
#define ZM_PERK_RANDOM_BODY_IDLE_PIECE_INDEX		3
#define ZM_PERK_RANDOM_BOTTLE_TAG_PIECE_INDEX		4
#define ZM_PERK_RANDOM_BALL_SPIN_PIECE_INDEX		5
	
#define ZM_PERK_RANDOM_COST					1500
	
#define ZM_PERK_RANDOM_NO_LIGHT_BIT			0
#define ZM_PERK_RANDOM_GREEN_LIGHT_BIT		1
#define ZM_PERK_RANDOM_YELLOW_LIGHT_BIT		2
#define ZM_PERK_RANDOM_RED_LIGHT_BIT		3
	
#define ZM_PERK_RANDOM_STATUS_FX_TAG 	"tag_animate"