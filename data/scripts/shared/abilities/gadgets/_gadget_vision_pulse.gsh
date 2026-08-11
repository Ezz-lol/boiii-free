#define DETECTED_LOOP_ALIAS "gdt_vision_pulse_loop"

#define REVEAL_MATERIAL_ENEMY "mc/hud_outline_model_z_red" // no alpha to differentiate from view model in shader
#define REVEAL_MATERIAL_SELF "mc/hud_outline_model_z_red_alpha" 
	
//	SCRIPT VECTOR USAGE
//		scriptVector0.x : pulse position in the world (1.0 near, 0.0 far)
//		scriptVector0.y : pulse width (percentage of value in material)
//		scriptVector0.z : pulse edge width
//		scriptVector0.w : iris appearance and character glow fade
//		scriptVector1.x : highlight enemeies (0: no, 1: yes)
//		scriptVector1.y : viewmodel pulse width
//		scriptVector1.z : viewmodel pulse edge width
//		scriptVector1.w : viewmodel pulse max radius
//		scriptVector2.xy: viewmodel pulse origin
//		scriptVector2.z : viewmodel pulse position
//		scriptVector2.w : max distance to pulse.
	
#define VISION_PULSE_CONSTID_RADIUS 						0
#define VISION_PULSE_CONSTID_PULSE_WIDTH 					1
#define VISION_PULSE_CONSTID_EDGE_WIDTH						2
#define VISION_PULSE_CONSTID_IRIS_FADE						3
#define VISION_PULSE_CONSTID_HIGHLIGHT_ENEMIES				4

#define VISION_PULSE_CONSTID_VIEWMODEL_PULSE_WIDTH 			5
#define VISION_PULSE_CONSTID_VIEWMODEL_EDGE_WIDTH 			6
#define VISION_PULSE_CONSTID_VIEWMODEL_IS_PULSED			7
#define VISION_PULSE_CONSTID_VIEWMODEL_ORIGIN_X				8
#define VISION_PULSE_CONSTID_VIEWMODEL_ORIGIN_Y				9
#define VISION_PULSE_CONSTID_VIEWMODEL_PULSE_POSITION		10
	
#define VISION_PULSE_CONSTID_MAX_RADIUS						11
	
#define VISION_PULSE_DURATION								2000
#define VISION_PULSE_REVEAL_TIME							4000
#define VISION_PULSE_RAMP_OUT_TIME							500 // Must  be smaller than VISION_PULSE_REVEAL_TIME
	
#define VISION_PULSE_FADE_RAMP_IN							0.1		// How far from 0 ( start )  - 1 ( end ) it takes for it to ramp in all the way
#define VISION_PULSE_FADE_RAMP_OUT							.5		// How far from VISION_PULSE_FADE_RAMP_IN ( start )  - 1 ( end ) it takes for it to ramp out all the way
	
#define VISION_PULSE_ENT_SCRIPT_VECTOR						"scriptVector7"

#define VISION_PULSE_VISIONSET					"vision_puls_bw"							// Name of the visionset file
#define VISION_PULSE_VISIONSET_ALIAS			"vision_pulse"								// alias in script
#define VISION_PULSE_VISIONSET_PRIORITY			61											// priority vs other visionsets
#define VISION_PULSE_VISIONSET_STEPS			12											// number of steps when ramping in/out
#define VISION_PULSE_VISIONSET_RAMP_IN			( .25 )										// activation ramp-in time
#define VISION_PULSE_VISIONSET_RAMP_HOLD		( VISION_PULSE_REVEAL_TIME / 1000 + 1 )		// How long to hold after ramp-in is done and before ramping-out
#define VISION_PULSE_VISIONSET_RAMP_OUT			( .75 )										// deactivation ramp-out time
	
// Ramp in + ramp out + ramp hold == VISION_PULSE_DURATION + VISION_PULSE_REVEAL_TIME
