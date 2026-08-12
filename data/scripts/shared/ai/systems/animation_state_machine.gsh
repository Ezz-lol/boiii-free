#define ASM_REGISTER_MOCOMP(name,initFunction,updateFunction,terminateFunction) \
	AnimationStateNetwork::RegisterAnimationMocomp(name,initFunction,updateFunction,terminateFunction);
	
#define ASM_REGISTER_NOTETRACK_HANDLER(notetrackname,handlerfunction) \
	AnimationStateNetwork::RegisterNotetrackHandlerFunction(notetrackname,handlerfunction);
	
#define ASM_REGISTER_BLACKBOARD_NOTETRACK_HANDLER(notetrackName,blackboardAttributeName,blackBoardValue) \
	AnimationStateNetwork::RegisterBlackboardNotetrackHandler(notetrackName,blackboardAttributeName,blackBoardValue);


// ASM RUNNING STATUS FLAGS ------------------------------------------------------//
#define ASM_STATE_INVALID_INDEX 			-1
#define ASM_STATE_INACTIVE					"asm_status_inactive"
#define ASM_STATE_RUNNING					"asm_status_running"
#define ASM_STATE_COMPLETE					"asm_status_complete"
#define ASM_STATE_TRANSITION_COMPLETE		"asm_status_transition_complete"
#define ASM_STATE_TERMINATED				"asm_status_terminated"

// TODO - Find a better way to not need to have this hardcoded state name.
#define ARRIVE_AT_COVER_ASM_STATE 			"arrive@locomotion"	

#define ASM_ALIAS_ATTRIBUTE				 "animation"

// ASM NOTETRACK HANDLER DEFINES -----------------------------------------------//
#define ASM_DEATH_NOTIFY					"death"
#define ASM_NOTETRACK_HANDLER_STOP_NOTIFY	"stop_handle_notetracks"

// ASM AIM/SHOOT NODE DEFINES -------------------------------------------------//
#define NOTETRACK_FIRE_BULLET			"fire"

// ASM MOCOMP ATTRIBUTES -----------------------------------------------------//
#define ASM_MOCOMP_NAME					"mocompname"
#define ASM_MOCOMP_START				"asm_mocomp_start"
#define ASM_MOCOMP_UPDATE				"asm_mocomp_update"
#define ASM_MOCOMP_TERMINATE			"asm_mocomp_terminate"
	
// NOTERACKS HANDLED BY ASM ---------------------------------------------------//
#define NOTETRACK_ANIM_END				"end"
#define NOTETRACK_CODE_MOVE				"code_move"
#define NOTETRACK_DROPGUN 				"dropgun"
#define NOTETRACK_DROP_GUN_1			"gun drop"
#define NOTETRACK_DROP_SHIELD			"drop_shield"
#define NOTETRACK_HIDE_WEAPON			"hide_weapon"
#define NOTETRACK_SHOW_WEAPON			"show_weapon"
#define NOTETRACK_HIDE_AI				"hide_ai"
#define NOTETRACK_SHOW_AI				"show_ai"
#define NOTETRACK_ATTACH_KNIFE			"attach_knife"
#define NOTETRACK_DETACH_KNIFE			"detach_knife"
#define NOTETRACK_EXIT_ALIGN			"exit_align"
#define NOTETRACK_GIB_DISABLE			"gib_disable"
#define NOTETRACK_GIB_HEAD				"gib = \"head\""
#define NOTETRACK_GIB_ARM_LEFT			"gib = \"arm_left\""
#define NOTETRACK_GIB_ARM_RIGHT			"gib = \"arm_right\""
#define NOTETRACK_GIB_LEG_LEFT			"gib = \"leg_left\""
#define NOTETRACK_GIB_LEG_RIGHT			"gib = \"leg_right\""
#define NOTETRACK_GRENADE_THROW			"grenade_throw"
#define NOTETRACK_RAGDOLL 				"start_ragdoll"
#define NOTETRACK_RAGDOLL_NODEATH 		"ragdoll_nodeath"
#define NOTETRACK_STANCE_CROUCH			"anim_pose = \"crouch\""
#define NOTETRACK_STANCE_STAND			"anim_pose = \"stand\""

#define NOTETRACK_STANCE_PRONE_FRONT	"anim_pose = \"prone_front\""
#define NOTETRACK_STANCE_PRONE_BACK		"anim_pose = \"prone_back\""
	
#define NOTETRACK_MOVEMENT_STOP			"anim_movement = \"stop\""
#define NOTETRACK_MOVEMENT_RUN			"anim_movement = \"run\""

#define NOTETRACK_START_TURN			"exit_align"
#define NOTETRACK_STOP_TURN				"code_move"
#define NOTETRACK_MELEE_UNSYNC			"unsync"
	
#define NOTETRACK_STAIRS_STEP1			"step1"	
#define NOTETRACK_STAIRS_STEP2			"step2"
	
// AI_ANIM_MODE, DEFINED SAME AS CODE -----------------------------------------//
#define AI_ANIM_UNKNOWN								"none"
#define AI_ANIM_MOVE_CODE							"normal"
#define AI_ANIM_MOVE_CODE_NOGRAVITY					"normal_nogravity"
#define AI_ANIM_USE_POS_DELTAS						"pos deltas"
#define AI_ANIM_USE_ANGLE_DELTAS					"angle deltas"
#define AI_ANIM_USE_ANGLE_DELTAS_NOCLIP				"angle deltas noclip"
#define AI_ANIM_USE_BOTH_DELTAS						"gravity"
#define AI_ANIM_USE_BOTH_DELTAS_NOCLIP				"noclip"
#define AI_ANIM_USE_BOTH_DELTAS_NOGRAVITY			"nogravity"
#define AI_ANIM_USE_BOTH_DELTAS_ZONLY_PHYSICS		"zonly_physics"
#define AI_ANIM_NOPHYSICS							"nophysics"
#define AI_ANIM_POINT_RELATIVE						"point relative"
#define AI_ANIM_SLIDE								"slide"
#define AI_ANIM_NOGRAVITY							"nogravity"
#define AI_ANIM_PRETRAVERSAL						"pre_traversal"
