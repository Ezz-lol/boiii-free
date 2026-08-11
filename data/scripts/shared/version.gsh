// 
// _version.gsh
//
// Contains version #defines for the clientfield system.
//
// To Blame : Dan Laufer.
// 
// Format :
// 
// Add FFOTD updates, as needed in the format of :
// #define <SIGNIFICANT_VERSION>_FFOTD_MMDDYY_VER  <LAST VERSION IN GROUP> + 1
//
// i.e.
//
// #define	VERSION_TU1					1000
// #define	VERSION_TU1_FFOTD_010113_0	1001
// #define	VERSION_TU1_FFOTD_010713_0	1002
//
// #define	VERSION_TU2					2000

// #define	VERSION_TU2_FFOTD_031813_0	2001
// #define	VERSION_TU2_FFOTD_031813_1	2002
// #define	VERSION_TU2_FFOTD_031813_2	2003
// #define	VERSION_TU2_FFOTD_031913_0	2004
//
// The MMDDYY specified is the date you make the change - not the day that the FFOTD is to be posted.
// _VER is just incremented based on additional changes made same day - or be smart, and just use what's already there for the same day - as long as no FFOTD has already
// been posted, using that data.

//=================================================================================================
//IF YOU CHANGE ANY VERSION NUMBERS IN THIS FILE, MAKE SURE TO UPDATE CLIENTFIELDMODELS.LUA AS WELL
//=================================================================================================

// VERSION_SHIP is 1, rather than 0 - so that I can use 0 as a 'wait - we've not recieved anything from the server yet' debug clue.

#define	VERSION_SHIP				1

//--------------------------------------
#define	VERSION_TU1					1000

#define	VERSION_TU2					2000

#define	VERSION_TU3					3000

#define	VERSION_TU4					4000

#define	VERSION_TU5					5000

#define VERSION_TU6					6000		
#define VERSION_TU6_FFOTD_020416_0	6001

#define VERSION_TU7					7000

#define VERSION_TU8					8000

#define VERSION_TU9					9000
#define VERSION_TU9_FFOTD_041916_0	9001	

#define VERSION_TU10				10000

#define VERSION_TU11				11000
#define VERSION_TU11_FFOTD_062416_0	11001	

#define VERSION_TU12				12000		

#define VERSION_TU13				13000

#define VERSION_TU14				14000

#define VERSION_TU15				15000
#define VERSION_TU15_FFOTD_090816_0	15001

#define VERSION_TU16				16000

#define VERSION_TU17				17000

#define VERSION_TU18				18000

#define VERSION_TU19				19000
	
#define VERSION_TU20				20000
	
#define VERSION_TU21				21000


// .... expand as needed.

// DLC Versions should be set to match the current in progress _TU define, and updated to the shipping XBOX TU number at ship.

//--------------------------------------
#define	VERSION_DLC1				VERSION_TU5

#define VERSION_DLC2				VERSION_TU9

#define VERSION_DLC3				VERSION_TU12

#define VERSION_DLC4				VERSION_TU15

#define VERSION_DLC5 				VERSION_TU21

	
#define CF_HOST_ONLY						true
#define CF_CALLBACK_ZERO_ON_NEW_ENT			true

// These are used to pinpoint when a ClientField became obsolete. We use a negative version to signify obsolesence,
// and the absolute value is the version at which obsolesence occurred. This allows us to reenable the ClientField at a
// later date if need be using a new standard (i.e. positive valued) version number

//--------------------------------------

#define VERSION_SHIP_OBSOLETE		(0 - VERSION_SHIP)

#define VERSION_TU1_OBSOLETE		(0 - VERSION_TU1)

#define VERSION_TU2_OBSOLETE		(0 - VERSION_TU2)

#define VERSION_TU3_OBSOLETE		(0 - VERSION_TU3)

#define VERSION_TU4_OBSOLETE		(0 - VERSION_TU4)

#define VERSION_TU5_OBSOLETE		(0 - VERSION_TU5)

#define VERSION_TU6_OBSOLETE		(0 - VERSION_TU6)

#define VERSION_TU7_OBSOLETE		(0 - VERSION_TU7)

#define VERSION_TU8_OBSOLETE		(0 - VERSION_TU8)

#define VERSION_TU9_OBSOLETE		(0 - VERSION_TU9)

#define VERSION_TU10_OBSOLETE		(0 - VERSION_TU10)

#define VERSION_TU11_OBSOLETE		(0 - VERSION_TU11)

#define VERSION_TU12_OBSOLETE		(0 - VERSION_TU12)

#define VERSION_TU13_OBSOLETE		(0 - VERSION_TU13)

#define VERSION_TU14_OBSOLETE		(0 - VERSION_TU14)

#define VERSION_TU15_OBSOLETE		(0 - VERSION_TU15)

#define VERSION_TU16_OBSOLETE		(0 - VERSION_TU16)

#define VERSION_TU17_OBSOLETE		(0 - VERSION_TU17)

#define VERSION_TU18_OBSOLETE		(0 - VERSION_TU18)

#define VERSION_TU19_OBSOLETE		(0 - VERSION_TU19)

