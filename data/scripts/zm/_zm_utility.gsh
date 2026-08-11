#define IS_DRINKING(_is_drinking) ( _is_drinking > 0 )
#define IS_ZOMBIETEAM(__self) (__self.team == level.zombie_team)

//=============================================================================
// _zm_power power sources

#define ANY_POWER 0
#define LOCAL_POWER_ONLY 1
#define GLOBAL_POWER_ONLY 2

#define STOP_FX_IMMEDIATELY					true

#define DPAD_UP								1
#define DPAD_DOWN							2
#define DPAD_LEFT							3
#define DPAD_RIGHT							4

#define MAX_CLIENTS_ZM 4

#define PATHDIST_APPROXIMATE				0
#define PATHDIST_CORNERPREDICTED			1
#define PATHDIST_ORIGINAL					2

//***** BEGIN MATCH RECORD EVENT DEFINES ****///
#define ZM_MAP_EVENT_INVALID 0
#define ZM_MAP_EVENT_ENTER_BEAST_MODE 1
#define ZM_MAP_EVENT_ENTER_SUPER_BEAST_MODE 2
	
//Optional Param 1 - Gumball the player was holding
//Optional Param 2 - Gumball the player took	
#define ZM_MAP_EVENT_ENTER_BUBBLEGUM_GRABBED 3

//Optional Param 1 -- Gumball index consumed
//Optional Param 2 -- Player Health --Not sure what else to track here.	
#define ZM_MAP_EVENT_ENTER_BUBBLEGUM_USED 4	

//Optional Param 1 -- Door Cost
#define ZM_MAP_EVENT_PURCHASE_DOOR 5

//Optional Param 1 -- Gun Index
//Optional Param 2 -- Gun Cost
#define ZM_MAP_EVENT_PURCHASE_GUN 6

//Optional Param 1 -- Gun Index
//Optional Param 2 -- Gun Cost
#define ZM_MAP_EVENT_PURCHASE_AMMO 7

#define ZM_MAP_EVENT_ROUND_START 8
#define ZM_MAP_EVENT_ROUND_END 9
	
//Optional Param 1 -- Gun Index	
#define ZM_MAP_EVENT_MAGIC_BOX_OFFERED 10
#define ZM_MAP_EVENT_MAGIC_BOX_TAKEN 11
#define ZM_MAP_EVENT_MAGIC_BOX_RETURNED 12

#define ZM_MAP_EVENT_CRAFT_PIECE_PICKEDUP 13
#define ZM_MAP_EVENT_CRAFTABLE_BUILT 14
#define ZM_MAP_EVENT_CRAFTABLE_PLANTED 15
#define ZM_MAP_EVENT_CRAFTABLE_PICKEDUP 16

//Optional Param 1 -- Switch Entity Number	
#define ZM_MAP_EVENT_POWER_ON 17	
#define ZM_MAP_EVENT_POWER_OFF 18
	
//Optional Param 1 -- Gun Index
//Optional Param 2 -- Cost	
#define ZM_MAP_EVENT_PAP_USED 19

//Optional Param 1 -- Gun Index
#define ZM_MAP_EVENT_PAP_NOT_GRABBED 20

//Optional Param 1 -- Ritual Hash
#define ZM_MAP_EVENT_RITUAL_STARTED 21
#define ZM_MAP_EVENT_RITUAL_ENDED 22

//Optional Param 1 -- Powerup Hash
#define ZM_MAP_EVENT_POWERUP_GRABBED 23

//NOTE -- The event position in this case is the world position of the POD, not the player.
//Optional Param 1 -- Pod Level
#define ZM_MAP_EVENT_POD_GRABBED 24

//Optional Param 1 - Weapon Index
//Optional Param 2 - Current AAT Hash
#define ZM_MAP_EVENT_PAP_REPACK_USED 25

//Optional Param 1 - Weapon Index
//Optional Param 2 - Offered AAT Hash
#define ZM_MAP_EVENT_PAP_REPACK_NOT_GRABBED 26
	
//Optional Param 1 - Weapon Index
//Optional Param 2 - AAT Hash
#define ZM_MAP_EVENT_PAP_GRABBED 27

//Optional Param 1 - Weapon Index
//Optional Param 2 - AAT Hash
#define ZM_MAP_EVENT_PAP_REPACK_GRABBED 28	

//NOTE -- The event position in this case is the world position of the PERK MACHINE, not the player. This can be used to identify which perk machine is being accessed.
//Optional Param 1 - Hash Index
#define ZM_MAP_EVENT_PERK_MACHINE_USED 29	
	
//***** END MATCH RECORD EVENT DEFINES ****///
