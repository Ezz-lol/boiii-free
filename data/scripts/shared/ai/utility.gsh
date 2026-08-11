// node spawnflag macros
#define ISNODEDONTRIGHT(__node) ( SPAWNFLAG( __node, SPAWNFLAG_PATH_DONT_RIGHT ))
#define ISNODEDONTLEFT(__node) ( SPAWNFLAG( __node, SPAWNFLAG_PATH_DONT_LEFT ))

// node type macros	
#define NODE_COVER_LEFT(_node) (_node.type == "Cover Left")
#define NODE_COVER_RIGHT(_node) (_node.type == "Cover Right")
#define NODE_COVER_PILLAR(_node) (_node.type == "Cover Pillar")	
#define NODE_COVER_STAND(_node) (_node.type == "Cover Stand" || _node.type == "Conceal Stand")	
#define NODE_COVER_CROUCH(_node) (_node.type == "Cover Crouch" || _node.type == "Cover Crouch Window" || _node.type == "Conceal Crouch" )	
#define NODE_EXPOSED(_node) (_node.type == "Exposed")	
#define NODE_GUARD(_node) (_node.type == "Guard")	
#define NODE_PATH(_node) (_node.type == "Path")

#define NODE_CONCEALED(_node) (_node.type == "Conceal Crouch" || _node.type == "Conceal Stand")	

#define NODE_TYPE_COVER(_node) ( NODE_COVER_LEFT(_node) || NODE_COVER_RIGHT(_node) || NODE_COVER_PILLAR(_node) || NODE_COVER_STAND(_node) || NODE_COVER_CROUCH(_node) )
	
// node stances macros
#define NODE_SUPPORTS_STANCE_STAND(__node) ( SPAWNFLAG( __node, SPAWNFLAG_PATH_STANCE_STAND ))
#define NODE_SUPPORTS_STANCE_CROUCH(__node) ( SPAWNFLAG( __node, SPAWNFLAG_PATH_STANCE_CROUCH ))
#define NODE_SUPPORTS_STANCE_PRONE(__node) ( SPAWNFLAG( __node, SPAWNFLAG_PATH_STANCE_PRONE ))
	
	
// Yaw and Pitch utility macros
#define GET_YAW(__self,__org) (VectorToAngles(__org-__self.origin)[1])
#define GET_YAW_TO_ORIGIN180(__self,__org) (AngleClamp180(__self.angles[1] - VectorToAngles(__org-__self.origin)[1]))
#define GET_PITCH(__self,__org) (VectorToAngles(__org-__self.origin)[0])
#define GET_YAW2D(__self,__org) (VectorToAngles((__org[0], __org[1], 0)-(__self.origin[0], __self.origin[1], 0))[1])