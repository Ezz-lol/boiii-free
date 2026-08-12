///////////////////////////////////////////////////////////////////////////////
//
// 									Formations
//
///////////////////////////////////////////////////////////////////////////////

#define PHALANX_COLUMN				"phalanx_column"
#define PHALANX_COLUMN_RIGHT		"phalanx_column_right"
#define PHALANX_FORWARD				"phalanx_forward"
#define PHALANX_DIAGONAL_RIGHT		"phalanx_diagonal_right"
#define PHALANX_DIAGONAL_LEFT		"phalanx_diagonal_left"
#define PHALANX_WEDGE				"phanalx_wedge"

///////////////////////////////////////////////////////////////////////////////
//
// 									Phalanx Tiers
//
///////////////////////////////////////////////////////////////////////////////

#define PHALANX_TIER_ONE				"phalanx_tier1"
#define PHALANX_TIER_TWO				"phalanx_tier2"
#define PHALANX_TIER_THREE				"phalanx_tier3"

///////////////////////////////////////////////////////////////////////////////
//
// 									Formation Positions
//
///////////////////////////////////////////////////////////////////////////////

#define PHALANX_WEDGE_TIER_ONE				array( \
	( 0, 0, 0 ), \
	( -64, -48, 0 ), \
	( 64, -48, 0 ), \
	( -128, -96, 0 ), \
	( 128, -96, 0 ) )

#define PHALANX_WEDGE_TIER_TWO				array( \
	( -32, -96, 0 ), \
	( 32, -96, 0 ) )

#define PHALANX_WEDGE_TIER_THREE			array( )

#define PHALANX_DIAGONAL_LEFT_TIER_ONE		array( \
	( 0, 0, 0 ), \
	( -48, -64, 0 ), \
	( -96, -128, 0 ), \
	( -144, -192, 0 ) )
	
#define PHALANX_DIAGONAL_LEFT_TIER_TWO		array( \
	( 64, 0, 0 ), \
	( 16, -64, 0 ), \
	( -48, -128, 0 ), \
	( -112, -192, 0 ) )
	
#define PHALANX_DIAGONAL_LEFT_TIER_THREE	array( )

#define PHALANX_DIAGONAL_RIGHT_TIER_ONE		array( \
	( 0, 0, 0 ), \
	( 48, -64, 0 ), \
	( 96, -128, 0 ), \
	( 144, -192, 0 ) )
	
#define PHALANX_DIAGONAL_RIGHT_TIER_TWO		array( \
	( -64, 0, 0 ), \
	( -16, -64, 0 ), \
	( 48, -128, 0 ), \
	( 112, -192, 0 ) )
	
#define PHALANX_DIAGONAL_RIGHT_TIER_THREE	array( )

#define PHALANX_FORWARD_TIER_ONE			array( \
	( 0, 0, 0 ), \
	( 64, 0, 0 ), \
	( 128, 0, 0 ), \
	( 192, 0, 0 ) )

#define PHALANX_FORWARD_TIER_TWO			array( \
	( -32, -64, 0 ), \
	( 32, -64, 0 ), \
	( 96, -64, 0 ), \
	( 160, -64, 0 ) )

#define PHALANX_FORWARD_TIER_THREE			array( )

#define PHALANX_COLUMN_TIER_ONE				array( \
	( 0, 0, 0 ), \
	( -64, 0, 0 ), \
	( 0, -64, 0 ), \
	( -64, -64, 0 ) )

#define PHALANX_COLUMN_TIER_TWO				array( \
	( 0, -128, 0 ), \
	( -64, -128, 0 ), \
	( 0, -192, 0 ), \
	( -64, -192, 0 ) )

#define PHALANX_COLUMN_TIER_THREE			array( )

#define PHALANX_COLUMN_RIGHT_TIER_ONE		array( \
	( 0, 0, 0 ), \
	( 0, -64, 0 ), \
	( 0, -128, 0 ), \
	( 0, -192, 0 ) )

#define PHALANX_COLUMN_RIGHT_TIER_TWO		array( )

#define PHALANX_COLUMN_RIGHT_TIER_THREE		array( )

///////////////////////////////////////////////////////////////////////////////
//
// 									Phalanx Defines
//
///////////////////////////////////////////////////////////////////////////////

#define PHALANX_NAVMESH_TOLERANCE			200

#define PHALANX_ROTATION_OFFSET				90

#define PHALANX_MAX_TIER_SIZE				10

#define PHALANX_MIN_TIER_SIZE				1

#define PHALANX_UPDATE_RATE_SEC				1