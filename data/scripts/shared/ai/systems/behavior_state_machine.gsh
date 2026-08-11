#define BSM_STATE_FUNC_START 		"bsm_state_start"
#define BSM_STATE_FUNC_UPDATE 		"bsm_state_update"
#define BSM_STATE_FUNC_TERMINATE 	"bsm_state_terminate"

// These enums	must match with StateMachineStatus in behavior_state_machine_db.h
#define BSM_INVALID 0
#define BSM_DEFAULT BSM_INVALID
#define	BSM_UNUSED1	1			// HACK - just to match status codes with the behavior tree, ease of use!
#define BSM_UNUSED2	2			// HACK - just to match status codes with the behavior tree, ease of use!
#define BSM_FAILURE 3
#define BSM_SUCCESS 4
#define BSM_RUNNING 5

#define BSM_REGISTER_CONDITION(name,scriptFunction) \
	BehaviorStateMachine::RegisterBSMScriptAPIInternal(name,scriptFunction);
	
#define BSM_REGISTER_API(name,scriptFunction) \
	BehaviorStateMachine::RegisterBSMScriptAPIInternal(name,scriptFunction);	

