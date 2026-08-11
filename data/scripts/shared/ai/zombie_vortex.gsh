#define NUMBER_OF_VORTICES 8

//Server time is smaller than client as we want the vortex fx to play longer than the vortex functionality
#define VORTEX_TIME_SERVER					4
#define VORTEX_TIME_CLIENT					5
#define UPGRADED_VORTEX_TIME_SERVER			9
#define UPGRADED_VORTEX_TIME_CLIENT			10

#define VORTEX_FX 							"zombie/fx_idgun_vortex_zod_zmb" 
#define VORTEX_EXPLOSION_FX					"zombie/fx_idgun_vortex_explo_zod_zmb"
	
#define UPGRADED_VORTEX_FX 					"zombie/fx_idgun_vortex_ug_zod_zmb" 
#define UPGRADED_VORTEX_EXPLOSION_FX		"zombie/fx_idgun_vortex_explo_ug_zod_zmb"

#define VORTEX_SCREEN_EFFECT_NAME			"zm_idgun_vortex"

#define VORTEX_START_CLIENTFIELD			"vortex_start"
#define VORTEX_EFFECT_VERSION_NONE			0
#define VORTEX_EFFECT_VERSION_1			1
#define VORTEX_EFFECT_VERSION_2			2

#define IDGUN_VEH_KILL_DIST_SQR		12.0 * 12.0

// Rumbles
#define VORTEX_RUMBLE_INTERIOR				"zod_idgun_vortex_interior"
#define VORTEX_RUMBLE_SHOCKWAVE				"zod_idgun_vortex_shockwave"
	
// Screen FX
// Prioritized below Beast mode and Parasite/Elemental round screen overlays
#define VORTEX_VISIONSET_PRIORITY			23 
#define VORTEX_VISIONSET_LERP_COUNT			30
#define VORTEX_ENTER_DURATION 				0.25
#define VORTEX_LOOP_DURATION 				2.0
#define VORTEX_EXIT_DURATION 				0.25	

// Screen Blur
#define VORTEX_OVERLAY_PRIORITY				23
#define VORTEX_OVERLAY_LERP_COUNT			1
#define VORTEX_OVERLAY_BLUR_AMOUNT			0.08
#define VORTEX_OVERLAY_INNER_RADIUS			0.75
#define VORTEX_OVERLAY_OUTER_RADIUS			0.9

