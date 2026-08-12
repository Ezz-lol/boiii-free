#define GET_GIB_DATA(entity,field)					(isdefined(entity.gib_data)?entity.gib_data.field:entity.field)

// Server side only
#define GIB_HAT_MODEL(entity)						GET_GIB_DATA(entity,hatmodel)
#define GIB_HEAD_MODEL(entity)						GET_GIB_DATA(entity,head)

// Server side only
#define GIB_TORSO_UNDAMAGED_MODEL(entity)			GET_GIB_DATA(entity,torsoDmg1)
#define GIB_TORSO_RIGHT_ARM_GONE_MODEL(entity)		GET_GIB_DATA(entity,torsoDmg2)
#define GIB_TORSO_LEFT_ARM_GONE_MODEL(entity)		GET_GIB_DATA(entity,torsoDmg3)
#define GIB_TORSO_GUTS_GONE_MODEL(entity)			GET_GIB_DATA(entity,torsoDmg4)
#define GIB_TORSO_HEAD_GONE_MODEL(entity)			GET_GIB_DATA(entity,torsoDmg5)

// Server side only
#define GIB_LEGS_UNDAMAGED_MODEL(entity)			GET_GIB_DATA(entity,legDmg1)
#define GIB_LEGS_RIGHT_LEG_GONE_MODEL(entity)		GET_GIB_DATA(entity,legDmg2)
#define GIB_LEGS_LEFT_LEG_GONE_MODEL(entity)		GET_GIB_DATA(entity,legDmg3)
#define GIB_LEGS_NO_LEGS_MODEL(entity)				GET_GIB_DATA(entity,legDmg4)

#define GIB_UNDAMAGED_FLAG							0
#define GIB_TOGGLE_GIB_MODEL_FLAG					1
#define GIB_ANNIHILATE_FLAG							2
#define GIB_HEAD_HAT_FLAG							4
#define GIB_TORSO_HEAD_FLAG							8
#define GIB_TORSO_RIGHT_ARM_FLAG					16
#define GIB_TORSO_LEFT_ARM_FLAG						32
#define GIB_TORSO_GUTS_FLAG							64
#define GIB_LEGS_RIGHT_LEG_FLAG						128
#define GIB_LEGS_LEFT_LEG_FLAG						256
#define GIB_LEGS_BOTH_LEGS_FLAG						(GIB_LEGS_RIGHT_LEG_FLAG+GIB_LEGS_LEFT_LEG_FLAG)

#define	GIB_FLAG_BITS								9
#define	GIB_DIR_BITS								3
#define	GIB_SCALE_BITS								3

#define GIB_CLIENTFIELD								"gib_state"
// 7 Damage states and 1 toggle bit to determine if gib models should be spawned on the client.
#define GIB_CLIENTFIELD_BITS_ACTOR					(GIB_FLAG_BITS)
#define GIB_CLIENTFIELD_BITS_PLAYER					(GIB_FLAG_BITS+GIB_DIR_BITS+GIB_SCALE_BITS)
#define GIB_CLIENTFIELD_TYPE						"int"
	
#define	GIB_FLAG_STRIPPED(gib_state)						( gib_state & ( ( 1 << GIB_FLAG_BITS ) - 1 ) )
#define	GET_GIB_DIR_BITS(gib_state)							( ( gib_state >> GIB_FLAG_BITS ) & ( ( 1 << GIB_DIR_BITS ) - 1 ) )
#define	GET_GIB_SCALE_BITS(gib_state)						( ( gib_state >> ( GIB_FLAG_BITS + GIB_DIR_BITS ) ) & ( ( 1 << GIB_SCALE_BITS ) - 1 ) )
	
// Less than GIB_TORSO_RIGHT_ARM_FLAG
#define IS_BODY_UNDAMAGED(gib_state)						(gib_state < GIB_TORSO_RIGHT_ARM_FLAG)
#define IS_GIBBED(gib_state,gib_flag)						(gib_state & gib_flag)
#define IS_UNDAMAGED(gib_state)								(gib_state == 0)
#define GET_GIB_FLAG(old_gib_state,current_gib_state)		(old_gib_state ^ current_gib_state)
#define SET_GIBBED(gib_state,gib_flag)						(gib_state | GIB_FLAG_STRIPPED(gib_flag))
#define	SET_GIBBED_PLAYER(gib_state,gib_flag,yawbits)		(GIB_FLAG_STRIPPED(gib_state|gib_flag) + ( yawbits << GIB_FLAG_BITS ) )
#define SHOULD_SPAWN_GIBS(gib_state)						(!(gib_state & GIB_TOGGLE_GIB_MODEL_FLAG))

// Client/Server scriptbundle
#define GET_GIB_BUNDLE(_gib_def)					struct::get_script_bundle("gibcharacterdef",_gib_def)
#define GET_GIB_BUNDLES								struct::get_script_bundles("gibcharacterdef")
