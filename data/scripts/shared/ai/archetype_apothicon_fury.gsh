// APOTHICON SIZE
#define FURY_RADIUS					30

// APOTHICON JUKE BEHAVIOR
#define FURY_JUKE_MAX_DIST				1500
#define FURY_JUKE_SHORT_DIST 			80
#define FURY_JUKE_MED_DIST 			100
#define FURY_JUKE_LONG_DIST 			130
#define FURY_JUKE_SHORT				"short"
#define FURY_JUKE_MED					"medium"
#define FURY_JUKE_LONG					"long"
#define FURY_TOO_CLOSE_TO_JUKE_DIST 	250
#define FURY_JUKE_GLOBAL_DELAY_MSEC	6000
#define FURY_BAMF_NT_START				"start_effect"
#define FURY_BAMF_NT_STOP				"end_effect"
#define FURY_BAMF_NT_LAND				"bamf_land"	
#define FURY_JUKE_CLIENTFIELD			"juke_active"
#define FURY_JUKE_COOLDOWN_MIN			7000
#define FURY_JUKE_COOLDOWN_MAX			10000
			
#define FURY_FAVOR_FORWARD_JUKE_FOV	0.939 // cos(20)	
#define FURY_JUKE_CHANCE				50						
	
#define FURY_TAUNT_GLOBAL_DELAY_MSEC	9500	
	
// APOTHICON DAMAGE REACTION	
#define FURY_DAMAGE_CLIENTFIELD		"fury_fire_damage"
#define FURY_DAMAGE_EFFECT				"dlc4/genesis/fx_apothicon_fury_impact"	

// BREATH	
#define FURY_BREATH_EFFECT				"dlc4/genesis/fx_apothicon_fury_breath"	
	
// SMOKE AMBIENT	
#define FURY_BODY_SMOKE_EFFECT			"dlc4/genesis/fx_apothicon_fury_smk_body"
	
// FOOT AMBIENT	
#define FURY_FOOTSTEP_AMB_EFFECT		"dlc4/genesis/fx_apothicon_fury_foot_amb"	
	
// ZIGZAG PARAMS	
#define FURY_ZIGZAG_MIN					300
#define FURY_ZIGZAG_MAX					700
	
// FURIOUS MODE
#define FURY_FURIOUS_LEVEL_THRESHOLD		3
#define FURY_FURIOUS_LEVEL_STEP			1
#define FURY_FURIOUS_MODE_CLIENTFIELD		"furious_level"	
#define FURY_FURIOUS_GLOBAL_DELAY_MIN_MSEC	5000
#define FURY_FURIOUS_GLOBAL_DELAY_MAX_MSEC	7000
#define FURY_FURIOUS_CHANCE				40		
	
#define FURY_FURIOUS_MAX_AI				1	

// DEATH
#define FURY_DEATH_START_DISSOLVE_NT	"start_dissolve"
#define FURY_DEATH_DISSOLVED_NT			"dissolved"	
#define FURY_DEATH_CLIENTFIELD			"apothicon_fury_death"	
	
// APOTHICON BAMF ATTACK BEHAVIOR	
#define FURY_BAMF_MELEE_DISTANCE_BB		"_apothicon_bamf_distance"
#define FURY_BAMF_MELEE_NT					"fury_melee"
	
#define FURY_BAMF_MELEE_RANGE				250	
#define FURY_BAMF_MELEE_DAMAGE_MAX			100
#define FURY_BAMF_MELEE_DAMAGE_MIN			15
#define FURY_BAMF_LAND_CLIENTFIELD			"bamf_land"
#define FURY_BAMF_LAND_FX					"dlc4/genesis/fx_apothicon_fury_teleport_impact"	
#define FURY_BAMF_GLOBAL_DELAY_MSEC		4500	
#define FURY_TOO_CLOSE_TO_BAMF_DIST		400
	
#define FURY_BAMF_FOV						0.642 // cos(50)
#define FURY_BAMF_MELEE_DIST_MIN_AFTER_JUKE 250
#define FURY_BAMF_MELEE_DIST_MIN			400
#define FURY_BAMF_MELEE_DIST_MAX			750	
#define FURY_BAMF_APPEAR_DIST_OFFSET		30				
#define FURY_BAMF_APPEAR_DIST_HORIZONTAL	60			
#define FURY_BAMF_APPEAR_DIST_VERTICAL		20
#define FURY_BAMF_NT_EXPLODE				"apothicon_explode"
#define FURY_BAMF_ATTACK_DAMAGE_MIN		50
#define FURY_BAMF_ATTACK_DAMAGE_MAX		120	
#define FURY_BAMF_ATTACK_RADIUS			200	
#define FURY_BAMF_VELOCITY					550
#define FURY_BAMF_COOLDOWN_MIN				4500
#define FURY_BAMF_COOLDOWN_MAX				6000
	
	
#define FURY_MELEE_DIST_SQ					100 * 100
	
	
// impact fx location ids
#define IMPACT_HEAD 	1	
#define IMPACT_CHEST 	2	
#define IMPACT_HIPS 	3	
#define IMPACT_R_ARM 	4	
#define IMPACT_L_ARM 	5	
#define IMPACT_R_LEG 	6	
#define IMPACT_L_LEG 	7		
	
#define FURY_HEALTH_MULTIPLIER_UPTO_ROUND_20		1.2
#define FURY_HEALTH_MULTIPLIER_UPTO_ROUND_50		1.5
#define FURY_HEALTH_MULTIPLIER_UPTO_ROUND_AFTER50	1.7
	
#define FURY_MOVEMENT_VARIANTS	3
	
#define FURY_DEATH_MODEL_SWAP			"c_zom_dlc4_apothicon_fury_dissolve"
#define FURY_DEATH_MODEL_SWAP_EFFECT 	"dlc4/genesis/fx_apothicon_fury_death"