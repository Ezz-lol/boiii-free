
// return status for BehaviorTreeNodes tick functions - BHTN_STATUS, order is important, BHTN_RUNNING needs to be highest

#define BHTN_INVALID 	0
#define BHTN_TERMINATE  1
#define BHTN_TERMINATED 2
#define BHTN_FAILURE 	3
#define BHTN_SUCCESS 	4
#define BHTN_RUNNING 	5

#define BHTN_UPDATE_FREQUENCY 0.05

#define BHTN_ACTION_START		"bhtn_action_start"
#define BHTN_ACTION_UPDATE		"bhtn_action_update"
#define BHTN_ACTION_TERMINATE	"bhtn_action_terminate"

	