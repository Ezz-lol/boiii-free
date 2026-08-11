#define DESTRUCT_CLIENTFIELD							"destructible_character_state"
#define DESTRUCT_CLIENTFIELD_TYPE						"int"

// 20 possible states and 1 toggle bit to determine if spawning gib models should happen on the client.
// The toggle bit is used to predamage characters when they are spawned in, without creating gib models.
// (20 + 1) states
#define DESTRUCT_CLIENTFIELD_BITS						21

#define DESTRUCT_UNDAMAGED_FLAG							0
#define DESTRUCT_TOGGLE_GIB_MODEL_FLAG					1

#define DESTRUCT_MAX_PIECES								20

#define GET_DESTRUCT_FLAG(destruct_piece_number)							(1 << destruct_piece_number)
#define GET_DESTRUCT_FLAGS(old_destruct_state,current_destruct_state)		(old_destruct_state ^ current_destruct_state)
#define GET_DESTRUCT_BUNDLE(entity)											struct::get_script_bundle( "destructiblecharacterdef", entity.destructibledef )
#define GET_DESTRUCT_BUNDLES												struct::get_script_bundles( "destructiblecharacterdef" )
#define HAS_DESTRUCT_DEF(entity)											IsDefined(entity.destructibledef)
#define IS_DESTRUCTED(destruct_state,destruct_flag)							(destruct_state & destruct_flag)
// Pieces numbers start at 1, and stop at DESTRUCT_MAX_PIECES
#define IS_VALID_PIECE_NUMBER(piece_number)									(1 <= piece_number && piece_number <= 20)
#define SET_DESTRUCTED(destruct_state,destruct_flag)						(destruct_state | destruct_flag)
#define SHOULD_SPAWN_DESTRUCT_GIBS(destruct_state)							(destruct_state & 1)
	