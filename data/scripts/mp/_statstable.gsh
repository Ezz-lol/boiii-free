#define STATS_TABLE_MAX_ITEMS	256

#define STATS_TABLE_COL_NUMBERING		0	// unique unsigned int - general numbering of all items
#define STATS_TABLE_COL_GROUP			2	// string - item group name, "primary" "secondary" "inventory" "specialty" "grenades" "special grenades" "stow back" "stow side" "attachment"
#define STATS_TABLE_COL_NAME			3	// string - name of the item, "Extreme Conditioning"
#define STATS_TABLE_COL_REFERENCE		4	// string - reference string of the item, "m203" "svt40" "bulletdamage" "c4"
#define STATS_TABLE_COL_COUNT			5	// signed int - item count, if exists, -1 = has no count
#define STATS_TABLE_COL_IMAGE			6	// string - item's image file name
#define STATS_TABLE_COL_DESC			7	// long string - item's description
#define STATS_TABLE_COL_ATTACHMENTS		8	// long string - item's other string data, reserved
#define STATS_TABLE_COL_SORT_KEY		9	// signed int - item's other number data, used for attachment number representations
#define STATS_TABLE_COL_UNLOCK_LEVEL	10	// unsigned int - represents if item is unlocked by default
#define STATS_TABLE_COL_DEFAULT_CLASS	11	// signed int - item's other number data, used for primary weapon camo skin number representations
#define STATS_TABLE_COL_ALLOCATION		12	// signed int - allocation cost of the item
#define STATS_TABLE_COL_SLOT			13	// string - slot for the given item
#define STATS_TABLE_COL_MOMENTUM		16	// signed int - momentum cost

#define ATTACHMENT_TABLE_COL_NUMBERING   0  // 
#define ATTACHMENT_TABLE_COL_TYPE        1  // 
#define ATTACHMENT_TABLE_COL_BOT_PROB   15  // 

