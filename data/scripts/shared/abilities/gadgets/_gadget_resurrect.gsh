
#define RESURRECT_WORLD_HINT_FX						"player/fx_plyr_revive"
#define RESURRECT_BODY_DISSAPPEAR_FX				"player/fx_plyr_revive_demat"
#define RESURRECT_SPAWN_FX							"player/fx_plyr_rejack_light"
#define RESURRECT_SMOKE_FX							"player/fx_plyr_rejack_smoke"
#define RESURRECT_SPAWN_SOUND						"mpl_resurrect_npc"
	
#define RESURRECT_HELD_DURATION						500
	
#define RESURRECT_SHELLSHOCK						"resurrect"

#define REJACK_ACTIVATION_DELAY_SECONDS				1.4
#define REJACK_ACTIVATION_WINDOW_SECONDS			4.0
#define REJACK_ENTER_POWER_PENALTY					30
#define REJACK_RESET_SCORESTREAK					true
#define RESURRECT_WEAPON_NAME						"gadget_resurrect"
#define RESURRECT_SMOKE_WEAPON_NAME					"gadget_resurrect_smoke_grenade"
#define REJACK_SMOKE_SIGHT_BLOCK_RADIUS				128
#define REJACK_SMOKE_FX_LIFETIME_SECONDS			5
#define REJACK_SMOKE_SIGHT_BLOCK_TIME				4
#define REJACK_SMOKE_FX_LIFETIME_AFTER_REJACK		.5

#define RESURRECT_VISIONSET							"mp_ability_resurrection"						// Name of the visionset file
#define RESURRECT_VISIONSET_ALIAS					"resurrect"										// alias in script
#define RESURRECT_VISIONSET_PRIORITY				62												// priority vs other visionsets
#define RESURRECT_VISIONSET_STEPS					16												// number of steps when ramping in/out
#define RESURRECT_VISIONSET_RAMP_IN					( REJACK_ACTIVATION_DELAY_SECONDS )				// activation ramp-in time
#define RESURRECT_VISIONSET_RAMP_HOLD				( REJACK_ACTIVATION_WINDOW_SECONDS )			// How long to hold after ramp-in is done and before ramping-out
#define RESURRECT_VISIONSET_RAMP_OUT				( .25 )											// deactivation ramp-out time
	
#define RESURRECT_VISIONSET_UP						"mp_ability_wakeup"								// Name of the visionset file
#define RESURRECT_VISIONSET_UP_ALIAS				"resurrect_up"									// alias in script
#define RESURRECT_VISIONSET_UP_PRIORITY				63												// priority vs other visionsets
#define RESURRECT_VISIONSET_UP_STEPS				16												// number of steps when ramping in/out
#define RESURRECT_VISIONSET_UP_RAMP_IN				( .35 )											// activation ramp-in time
#define RESURRECT_VISIONSET_UP_RAMP_HOLD			( .1 )											// How long to hold after ramp-in is done and before ramping-out
#define RESURRECT_VISIONSET_UP_RAMP_OUT				( .2 )											// deactivation ramp-out time
	
#define RESURRECT_POSTFX_BUNDLE_CLOSE_DURATION		.5
#define RESURRECT_POSTFX_BUNDLE_CLOSE				"pstfx_resurrection_close"
#define RESURRECT_POSTFX_BUNDLE_PUS					"pstfx_resurrection_pus"
#define RESURRECT_POSTFX_BUNDLE_OPEN				"pstfx_resurrection_open"
	
#define RESURRECT_STATE_NONE						0
#define RESURRECT_STATE_DOWN						1
#define RESURRECT_STATE_UP							2
	
#define RESURRECT_STATE_BITS						2