
// archetype names here should match the GDT in archetype.gdt and vehicle gdts.

#define ARCHETYPE_HUMAN	    			"human"
#define ARCHETYPE_HUMAN_CLONE			"human_clone"
#define ARCHETYPE_HUMAN_RIOTSHIELD		"human_riotshield"
#define ARCHETYPE_HUMAN_RPG				"human_rpg"
#define ARCHETYPE_ROBOT					"robot"
#define ARCHETYPE_WARLORD				"warlord"	
#define ARCHETYPE_ZOMBIE				"zombie"
#define ARCHETYPE_ZOMBIE_DOG			"zombie_dog"
#define ARCHETYPE_DIREWOLF				"direwolf"
#define ARCHETYPE_CIVILIAN				"civilian"
#define ARCHETYPE_SKELETON				"skeleton"
#define ARCHETYPE_ZOD_COMPANION			"zod_companion"
#define ARCHETYPE_MARGWA				"margwa"
#define ARCHETYPE_MECHZ					"mechz"
#define ARCHETYPE_MANNEQUIN				"mannequin"
#define ARCHETYPE_THRASHER				"thrasher"
#define ARCHETYPE_RAZ					"raz"
#define ARCHETYPE_ZOMBIE_QUAD			"zombie_quad"
#define ARCHETYPE_KEEPER_COMPANION		"keeper_companion"
#define ARCHETYPE_APOTHICON_FURY		"apothicon_fury"
#define ARCHETYPE_KEEPER				"keeper"	
#define ARCHETYPE_MONKEY				"monkey"
#define ARCHETYPE_ASTRONAUT				"astronaut"					
	
	
	
#define ARCHETYPE_AMWS					"amws"
#define ARCHETYPE_HUNTER				"hunter"
#define ARCHETYPE_QUADTANK				"quadtank"
#define ARCHETYPE_RAPS					"raps"
#define ARCHETYPE_SCOUT					"scout"
#define ARCHETYPE_SIEGEBOT				"siegebot"
#define ARCHETYPE_TURRET				"turret"
#define ARCHETYPE_WASP					"wasp"
#define ARCHETYPE_PARASITE				"parasite"
#define ARCHETYPE_GLAIVE				"glaive"
#define ARCHETYPE_SENTINEL_DRONE		"sentinel_drone"
#define ARCHETYPE_SPIDER				"spider"
	
#define IS_ROBOT(__e)					(isdefined(__e.archetype) && ( __e.archetype == ARCHETYPE_ROBOT ))
	
// Following is human will consider "human", "human_riotshield", "human_rpg", "civilian"
#define IS_HUMAN(__e)					(isdefined(__e.archetype) && ((__e.archetype == ARCHETYPE_HUMAN) || (__e.archetype == ARCHETYPE_HUMAN_RIOTSHIELD) || (__e.archetype == ARCHETYPE_HUMAN_RPG) || (__e.archetype == ARCHETYPE_CIVILIAN)))

// same as hkai_navmesh_material.h
#define NMMF_NOVEHICLE					0x00000020
#define NMMF_ALL						0x00ffffff

	
// ----------------------------------------------
// For facial animations on human type archetypes
// ----------------------------------------------
#define	HUMAN_FACIAL_DIALOG_ACTIVE	"facial_dial"	
	
// ----------------------------------------------
// debug helper macro for position query
// ----------------------------------------------
#define ADD_POINT_SCORE(pointStruct,name,point_score) \
	/# \
	if ( !isdefined( pointStruct._scoreDebug ) ) \
	{ \
		pointStruct._scoreDebug = []; \
	} \
	pointStruct._scoreDebug[ name ] = point_score; \
	#/ \
	pointStruct.score += point_score;	