//Table Names in Vox CSV
#define VOX_TABLE_COL_CATEGORY			0
#define VOX_TABLE_COL_SUBCATEGORY		1
#define VOX_TABLE_COL_SUFFIX			2
#define VOX_TABLE_COL_PERCENTAGE		3
#define VOX_TABLE_COL_RESPONSE			4
#define VOX_TABLE_COL_DELAYBEFOREPLAY	5
	
//Helps Create Response Aliases
#define HERO_APPEND "_resp_hr"
#define RIVAL_APPEND "_resp_riv"
#define RESPOND_APPEND "_resp_"	
	
//Generic Player PRefix
#define PLAYER_PREFIX "vox_plr_"
	
//Character Index Numbers
#define DEMPSEY_CHAR_INDEX 0
#define NIKOLAI_CHAR_INDEX 1
#define TAKEO_CHAR_INDEX 2
#define RICHTOFEN_CHAR_INDEX 3
	
#define DEMPSEY_CHAR_INDEX_R 0
#define NIKOLAI_CHAR_INDEX_R 1
#define TAKEO_CHAR_INDEX_R 3
#define RICHTOFEN_CHAR_INDEX_R 2
	
//Max Distance Away a Player will Play a Response Line
#define RESPONSE_LINE_MAX_DIST 500*500

//Percentage That Response Lines Will Happen
#define RESPONSE_PERCENTAGE 50
	
//Perksacola Definitions
#define PERKSACOLA_WAIT_TIME			randomfloatrange(30,60)
#define PERKSACOLA_PROBABILITY 			(randomintrange(0,100) <= 10) //15%
#define PERKSACOLA_JINGLE				0
#define PERKSACOLA_STINGER				1

//Music System
#define PLAYTYPE_NONE		0
#define PLAYTYPE_REJECT 	1
#define PLAYTYPE_QUEUE		2
#define PLAYTYPE_ROUND 		3	//Clears the queue of lower playtypes, stops previously playing states of lower playtypes
#define PLAYTYPE_SPECIAL 	4	//Clears the queue, stops previously playing states including rounds, prevents all lower playtypes from playing
#define PLAYTYPE_GAMEEND 	5	//Stops all music, immediately plays gameover