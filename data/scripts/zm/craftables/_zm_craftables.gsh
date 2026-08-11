#define CLIENTFIELD_CRAFTABLE_PIECE_NONE		0

// clientfield name
#define CLIENTFIELD_CRAFTABLE				"craftable"

// whether or not the trigger passed to a craftable_trigger_think call is deleted 
#define DELETE_TRIGGER	1
#define KEEP_TRIGGER	0

// whether or not a craftable lives past being made
#define UNCRAFT				3
#define ONE_USE_AND_FLY		2
#define PERSISTENT    		1
#define ONE_TIME_CRAFT  	0   

#define CRAFTABLE_OBTAINED			1	// used for shared pieces
#define CRAFTABLE_IS_SHARED			1	// piece is a part of shared inventory
#define CRAFTABLE_NEED_ALL_PIECES	1	// must have all pieces before you can craft item

#define CRAFTABLE_INVALID_CHOICE	-1	// No craftable has been chosen
