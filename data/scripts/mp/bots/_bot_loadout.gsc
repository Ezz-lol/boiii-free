#using scripts\shared\array_shared;
#using scripts\shared\rank_shared;

#insert scripts\shared\shared.gsh;
#insert scripts\shared\statstable_shared.gsh;

#using scripts\mp\killstreaks\_killstreaks;

#using scripts\mp\bots\_bot;

#define BOT_ALLOCATION_MAX			  100
#define BOT_ALLOCATION_UNLOCK_MAX		3
#define BOT_RANK_ALL_OPTIONS_AVAILABLE 20
#define BOT_RANK_OPTIONS_MULTIPLIER     4
	
#namespace bot_loadout;	

// Item Whitelist
//========================================

function in_whitelist( itemName )
{
	if ( !isdefined( itemName ) )
		return false;
	
	switch( itemName )
	{
		// Secondaries
		case "WEAPON_KNIFE_LOADOUT":
		case "WEAPON_PISTOL_STANDARD":
		case "WEAPON_PISTOL_BURST":
		case "WEAPON_PISTOL_FULLAUTO":
		case "WEAPON_LAUNCHER_STANDARD":
		case "WEAPON_LAUNCHER_LOCKONLY":

        // Primaries
		case "WEAPON_SMG_STANDARD":
		case "WEAPON_SMG_BURST":
		case "WEAPON_SMG_FASTFIRE":
		case "WEAPON_SMG_LONGRANGE":
		case "WEAPON_SMG_VERSATILE":
		case "WEAPON_SMG_CAPACITY":
		case "WEAPON_AR_STANDARD":
		case "WEAPON_AR_ACCURATE":
		case "WEAPON_AR_CQB":
		case "WEAPON_AR_DAMAGE":
		case "WEAPON_AR_FASTBURST":
		case "WEAPON_AR_LONGBURST":
		case "WEAPON_AR_MARKSMAN":
		case "WEAPON_LMG_CQB":
		case "WEAPON_LMG_HEAVY":
		case "WEAPON_LMG_LIGHT":
		case "WEAPON_LMG_SLOWFIRE":
		case "WEAPON_SNIPER_FASTBOLT":
		case "WEAPON_SNIPER_FASTSEMI":
		case "WEAPON_SNIPER_POWERBOLT":
		case "WEAPON_SNIPER_CHARGESHOT":
		case "WEAPON_SHOTGUN_FULLAUTO":
		case "WEAPON_SHOTGUN_PRECISION":
		case "WEAPON_SHOTGUN_PUMP":
		case "WEAPON_SHOTGUN_SEMIAUTO":
		
		// Lethals
		case "WEAPON_FRAGGRENADE":
		case "WEAPON_HATCHET":
		case "WEAPON_STICKY_GRENADE":
		case "WEAPON_SATCHEL_CHARGE":
		case "WEAPON_BOUNCINGBETTY":
		case "WEAPON_INCENDIARY_GRENADE":
		
		// Tacticals
		case "WEAPON_WILLY_PETE":
		case "WEAPON_STUN_GRENADE":
		case "WEAPON_EMPGRENADE":
		case "WEAPON_FLASHBANG":
		case "WEAPON_PROXIMITY_GRENADE":
		case "WEAPON_PDA_HACK":
		case "WEAPON_TROPHY_SYSTEM":
		
		// Killstreaks
		//case "KILLSTREAK_RCBOMB":
		case "KILLSTREAK_RECON":
		case "KILLSTREAK_COUNTER_UAV":
		//case "KILLSTREAK_SUPPLY_DROP":
		//case "KILLSTREAK_MICROWAVE_TURRET":
		case "KILLSTREAK_REMOTE_MISSILE":
		//case "KILLSTREAK_PLANEMORTAR":
		//case "KILLSTREAK_AUTO_TURRET":
		case "KILLSTREAK_AI_TANK_DROP":
		//case "KILLSTREAK_HELICOPTER_COMLINK":
		case "KILLSTREAK_SATELLITE":
		//case "KILLSTREAK_EMP":
		//case "KILLSTREAK_HELICOPTER_GUNNER":
		case "KILLSTREAK_RAPS":
		//case "KILLSTREAK_DRONE_STRIKE":
		//case "KILLSTREAK_DART":
		case "KILLSTREAK_SENTINEL":
		
		// TU Something Weapons
		case "WEAPON_MELEE_KNUCKLES":
		case "WEAPON_MELEE_BUTTERFLY":
		case "WEAPON_MELEE_WRENCH":

		// TU 6 Weapons
		case "WEAPON_PISTOL_SHOTGUN":
		case "WEAPON_AR_GARAND":
		case "WEAPON_SPECIAL_CROSSBOW":
		case "WEAPON_MELEE_CROWBAR":
		case "WEAPON_MELEE_SWORD":
		case "WEAPON_MELEE_BOXING":	
		case "WEAPON_SMG_AK74U":
		case "WEAPON_SMG_MP40":
		case "WEAPON_SMG_RECHAMBER":
		case "WEAPON_SMG_NAILGUN":
		case "WEAPON_AR_AN94":
		case "WEAPON_AR_FAMAS":
		case "WEAPON_SMG_MSMC":
		case "WEAPON_LMG_INFINITE":
		case "WEAPON_AR_PULSE":
		case "WEAPON_AR_M16":
		case "WEAPON_SMG_PPSH":
		case "WEAPON_LAUNCHER_EX41":
		case "WEAPON_SHOTGUN_OLYMPIA":
		case "WEAPON_SNIPER_QUICKSCOPE":
		case "WEAPON_SNIPER_DOUBLE":
		case "WEAPON_SMG_STEN":	
		case "WEAPON_AR_GALIL":	
		case "WEAPON_LMG_RPK":	
		case "WEAPON_AR_M14":
		case "WEAPON_SHOTGUN_ENERGY":
		case "WEAPON_SPECIAL_CROSSBOW_DW":	
		case "WEAPON_AR_PEACEKEEPER":	
		case "WEAPON_MELEE_CHAINSAW":	
		case "WEAPON_SPECIAL_KNIFE_BALLISTIC":	
		case "WEAPON_MELEE_CRESCENT":	
		case "WEAPON_SPECIAL_DISCGUN":	
			return true;
	}
	
	return false;
}

// Classes
//========================================

function build_classes()
{
	primaryWeapons = self get_available_items( undefined, "primary" );
	secondaryWeapons = self get_available_items( undefined, "secondary" );
	lethals = self get_available_items( undefined, "primarygadget" );
	tacticals = self get_available_items( undefined, "secondarygadget" );
	if ( IS_TRUE( level.perksEnabled ) )
	{
		specialties1 = self get_available_items( undefined, "specialty1" );
		specialties2 = self get_available_items( undefined, "specialty2" );
		specialties3 = self get_available_items( undefined, "specialty3" );
	}
	
	foreach( className, classValue in level.classMap )
	{
		if ( !isSubstr( className, "custom" ) )
		{
			continue;
		}
		
		classIndex = int( className[className.size-1] );
		
		pickedItems = [];
		
		pick_item( pickedItems, primaryWeapons );
		
		if ( RandomInt( 100 ) < 95 ) // 5% chance to be a boxer for Scronce
		{
			pick_item( pickedItems, secondaryWeapons );
		}
		
		// Shuffle these selections around a bit so the classes don't all look the same when the allocation is low
		otherItems = Array ( lethals, tacticals, specialties1, specialties2, specialties3 );
		otherItems = array::randomize( otherItems );
		
		for ( i = 0; i < otherItems.size; i ++ )
		{
			pick_item( pickedItems, otherItems[i] );
		}

		// Add items up to the max allocation
		for ( i = 0; i < pickedItems.size && i < level.maxAllocation; i++ )
		{
			self BotClassAddItem( classIndex, pickedItems[i] );
		}
		
		// TODO: Pick primary/secondary attachments, extra perks, extra lethal, extra tactical, overkill
/*		
		primaryWeapon = self GetLoadoutWeapon( classIndex, "primary" );
		
		if ( primaryWeapon != level.weaponNone && primaryWeapon.supportedAttachments.size )
		{
			attachment = array::random( primaryWeapon.supportedAttachments );
			self BotClassAddAttachment( classIndex, primaryWeapon, attachment, "primary" );
		}
		
		secondaryWeapon = self GetLoadoutWeapon( classIndex, "secondary" );
		
		if ( secondaryWeapon != level.weaponNone && secondaryWeapon.supportedAttachments.size  )
		{
			attachment = array::random( secondaryWeapon.supportedAttachments );
			self BotClassAddAttachment( classIndex, secondaryWeapon, attachment, "secondary" );
		}
*/
	}
}

function pick_item( &pickedItems, items )
{
	if ( !isdefined( items ) || items.size <= 0 )
	{
		return;
	}
	
	pickedItems[pickedItems.size] = array::random( items );
}

function pick_classes()
{
	self.loadoutClasses = [];
	self.launcherClassCount = 0;
	
	foreach( className, classValue in level.classMap )
	{
		if ( isSubstr( className, "custom" ) )
		{
			if ( level.disableCAC )
			{
				continue;
			}
			
			classIndex = int( className[className.size-1] );
		}
		else
		{
			// Things bots could use better in the default classes:
			//  C4, Trophy System, Lock on only launcher
			classIndex = level.classToClassNum[ classValue ];
		}
		
		primary = self GetLoadoutWeapon( classIndex, "primary" );
		secondary = self GetLoadoutWeapon( classIndex, "secondary" );
		
		botClass = SpawnStruct();
		botClass.name = className;
		botClass.index = classIndex;
		botClass.value = classValue;
		botClass.primary = primary;
		botClass.secondary = secondary;
		
		if ( botClass.secondary.isRocketLauncher )
		{
			self.launcherClassCount++;
		}
		
		self.loadoutClasses[ self.loadoutClasses.size ] = botClass;
	}
}

function get_current_class()
{
	currValue = self.pers["class"];
	if ( !isdefined( currValue ) )
	{
		return undefined;
	}
	
	foreach( botClass in self.loadoutClasses )
	{
		if ( botClass.value == currValue )
		{
			return botClass;
		}
	}
	
	return undefined;
}

// Specialists
//========================================

function pick_hero_gadget()
{	
	if ( RandomInt( 2 ) < 1 || !self pick_hero_ability() )
	{
		self pick_hero_weapon();
	}
}

function pick_hero_weapon()
{
	heroWeaponRef = self GetHeroWeaponName();
	
	if ( IsItemRestricted( heroWeaponRef ) )
	{
		return false;
	}
	
	heroWeaponName = self get_item_name( heroWeaponRef );
	self BotClassAddItem( 0, heroWeaponName );
	
	return true;
}

function pick_hero_ability()
{
	heroAbilityRef = self GetHeroAbilityName();
	
	if ( IsItemRestricted( heroAbilityRef ) )
	{
		return false;
	}
	
	heroAbilityName = self get_item_name( heroAbilityRef );
	self BotClassAddItem( 0, heroAbilityName );
	
	return true;
}

// Killstreaks
//========================================

function pick_killstreaks()
{
	killstreaks = array::randomize( self get_available_items( "killstreak" ) );
	
	for( i = 0; i < 3 && i < killstreaks.size; i++ )
	{
		self BotClassAddItem( 0, killstreaks[i] );
	}
}


// Get Items
//========================================

function get_available_items( filterGroup, filterSlot )
{
	// Get unlocked and unrestricted items
	items = [];
	
	for( i = 0; i < STATS_TABLE_MAX_ITEMS; i++ )
	{
		row = tableLookupRowNum( level.statsTableID, STATS_TABLE_COL_NUMBERING, i );

		if ( row < 0 )
		{
			continue;
		}
		
		name = tableLookupColumnForRow( level.statsTableID, row, STATS_TABLE_COL_NAME );

		if ( name == "" || !in_whitelist( name ) )
		{
			continue;
		}
		
		allocation = Int( tableLookupColumnForRow( level.statsTableID, row, STATS_TABLE_COL_ALLOCATION ) );

		if ( allocation < 0 )
		{
			continue;
		}
		
		ref = tableLookupColumnForRow( level.statsTableId, row, STATS_TABLE_COL_REFERENCE );
		
		if ( IsItemRestricted( ref ) )
		{
			continue;
		}
			
		number = Int( tableLookupColumnForRow( level.statsTableID, row, STATS_TABLE_COL_NUMBERING ) );
/*		
		if ( SessionModeIsPrivate() && self IsItemLocked( number ) )
		{
			continue;
		}
*/
		if ( isdefined( filterGroup ) )
	    {
			group = tableLookupColumnForRow( level.statsTableID, row, STATS_TABLE_COL_GROUP );
			
			if ( group != filterGroup )
			{
				continue;
			}	
	    }
		
		if ( isdefined( filterSlot ) )
		{
			slot = tableLookupColumnForRow( level.statsTableID, row, STATS_TABLE_COL_SLOT );
		
			if ( slot != filterSlot )
			{
				continue;
			}
		}
		
		items[items.size] = name;
	}
	
	return items;
}

function get_item_name( itemReference )
{
	for( i = 0; i < STATS_TABLE_MAX_ITEMS; i++ )
	{
		row = tableLookupRowNum( level.statsTableID, STATS_TABLE_COL_NUMBERING, i );

		if ( row < 0 )
		{
			continue;
		}
		
		reference = tableLookupColumnForRow( level.statsTableID, row,  STATS_TABLE_COL_REFERENCE );

		if ( reference != itemReference )
		{
			continue;
		}

		name = tableLookupColumnForRow( level.statsTableID, row, STATS_TABLE_COL_NAME );

		return name;
	}
	
	return undefined;
}

// Not in use

function init()
{
	level endon( "game_ended" );

	level.bot_banned_killstreaks = Array ( 	"KILLSTREAK_RCBOMB",
											"KILLSTREAK_QRDRONE",
										/*	"KILLSTREAK_REMOTE_MISSILE",*/
											"KILLSTREAK_REMOTE_MORTAR",
											"KILLSTREAK_HELICOPTER_GUNNER" );
	for ( ;; )
	{
		level waittill( "connected", player );

		if ( !player IsTestClient() )
		{
			continue;
		}

		player thread on_bot_connect();
	}
}

function on_bot_connect()
{
	self endon( "disconnect" );

	if ( isdefined( self.pers[ "bot_loadout" ] ) )
	{
		return;
	}

	wait( 0.10 );

	if ( self GetEntityNumber() % 2 == 0 )
	{
		WAIT_SERVER_FRAME;
	}
		
	self bot::set_rank();
	
	self BotSetRandomCharacterCustomization();

	
	max_allocation = BOT_ALLOCATION_MAX;
/*
	if ( SessionModeIsPrivate() )
	{
		for ( i = 1; i <= BOT_ALLOCATION_UNLOCK_MAX; i++ )
		{
			if ( self IsItemLocked( rank::GetItemIndex( "feature_allocation_slot_" + i ) ) )
			{
				max_allocation--;
			}
		}
	}
*/	
	self construct_loadout( max_allocation );
	self.pers[ "bot_loadout" ] = true;
}

function construct_loadout( allocation_max )
{
/*	if ( SessionModeIsPrivate() && self IsItemLocked( rank::GetItemIndex( "feature_cac" ) ) )
	{
		// cac still locked
		return;
	}
*/
	pixbeginevent( "bot_construct_loadout" );

	item_list = build_item_list();

//	item_list["primary"] = [];
//	item_list["primary"][0] = "WEAPON_RIOTSHIELD";

	construct_class( 0, item_list, allocation_max );
	construct_class( 1, item_list, allocation_max );
	construct_class( 2, item_list, allocation_max );
	construct_class( 3, item_list, allocation_max );
	construct_class( 4, item_list, allocation_max );

	killstreaks = item_list["killstreak1"];

	if ( isdefined( item_list["killstreak2"] ) )
	{
		killstreaks = ArrayCombine( killstreaks, item_list["killstreak2"], true, false );
	}

	if ( isdefined( item_list["killstreak3"] ) )
	{
		killstreaks = ArrayCombine( killstreaks, item_list["killstreak3"], true, false );
	}
		
	if ( isdefined( killstreaks ) && killstreaks.size )
	{
		choose_weapon( 0, killstreaks );
		choose_weapon( 0, killstreaks );
		choose_weapon( 0, killstreaks );
	}

	self.claimed_items = undefined;
	pixendevent();
}

function construct_class( constructclass, items, allocation_max )
{
	allocation = 0;

	claimed_count = build_claimed_list( items );
	self.claimed_items = [];

	// primary
	weapon = choose_weapon( constructclass, items["primary"] );
	claimed_count["primary"]++;
	allocation++;

	// secondary
	weapon = choose_weapon( constructclass, items["secondary"] );
	choose_weapon_option( constructclass, "camo", 1 );
}

function make_choice( chance, claimed, max_claim )
{
	return ( claimed < max_claim && RandomInt( 100 ) < chance );
}

function chose_action( action1, chance1, action2, chance2, action3, chance3, action4, chance4 )
{
	chance1 = Int( chance1 / 10 );
	chance2 = Int( chance2 / 10 );
	chance3 = Int( chance3 / 10 );
	chance4 = Int( chance4 / 10 );

	actions = [];

	for( i = 0; i < chance1; i++ )
	{
		actions[ actions.size ] = action1;
	}

	for( i = 0; i < chance2; i++ )
	{
		actions[ actions.size ] = action2;
	}

	for( i = 0; i < chance3; i++ )
	{
		actions[ actions.size ] = action3;
	}

	for( i = 0; i < chance4; i++ )
	{
		actions[ actions.size ] = action4;
	}

	return array::random( actions );
}

function item_is_claimed( item )
{
	foreach( claim in self.claimed_items )
	{
		if ( claim == item )
		{
			return true;
		}
	}

	return false;
}

function choose_weapon( weaponclass, items )
{
	if ( !isdefined( items ) || !items.size )
	{
		return undefined;
	}

	start = RandomInt( items.size );

	for( i = 0; i < items.size; i++ )
	{
		weapon = items[ start ];

		if ( !item_is_claimed( weapon ) )
		{
			break;
		}

		start = ( start + 1 ) % items.size;
	}
		
	self.claimed_items[ self.claimed_items.size ] = weapon;
	
	self BotClassAddItem( weaponclass, weapon );
	return weapon;
}

function build_weapon_options_list( optionType )
{
	level.botWeaponOptionsId[optionType] = [];
	level.botWeaponOptionsProb[optionType] = [];

	csv_filename = "gamedata/weapons/common/attachmentTable.csv";
	prob = 0;
	for ( row = 0 ; row < 255 ; row++ )
	{
		if ( tableLookupColumnForRow( csv_filename, row, ATTACHMENT_TABLE_COL_TYPE ) == optionType )
		{
			index = level.botWeaponOptionsId[optionType].size;
			level.botWeaponOptionsId[optionType][index] = Int( tableLookupColumnForRow( csv_filename, row, ATTACHMENT_TABLE_COL_NUMBERING ) );
			prob += Int( tableLookupColumnForRow( csv_filename, row, ATTACHMENT_TABLE_COL_BOT_PROB ) );
			level.botWeaponOptionsProb[optionType][index] = prob;
		}
	}
}

function choose_weapon_option( weaponclass, optionType, primary )
{
	if ( !isdefined( level.botWeaponOptionsId ) )
	{
		level.botWeaponOptionsId = [];
		level.botWeaponOptionsProb = [];

		build_weapon_options_list( "camo" );
		build_weapon_options_list( "reticle" );
	}

	// weapon options cannot be set in local matches
	if ( !level.onlineGame && !level.systemLink )
		return;

	// Increase the range of the probability to reduce the chances of picking the option when the bot's level is less than BOT_RANK_ALL_OPTIONS_AVAILABLE
	// (in system link all options are available)
	numOptions = level.botWeaponOptionsProb[optionType].size;
	maxProb = level.botWeaponOptionsProb[optionType][numOptions-1];
	if ( !level.systemLink && self.pers[ "rank" ] < BOT_RANK_ALL_OPTIONS_AVAILABLE )
		maxProb += BOT_RANK_OPTIONS_MULTIPLIER * maxProb * ( ( BOT_RANK_ALL_OPTIONS_AVAILABLE - self.pers[ "rank" ] ) / BOT_RANK_ALL_OPTIONS_AVAILABLE );

	rnd = RandomInt( Int( maxProb ) );
	for (i=0 ; i<numOptions ; i++)
	{
		if ( level.botWeaponOptionsProb[optionType][i] > rnd )
		{
			self BotClassSetWeaponOption( weaponclass, primary, optionType, level.botWeaponOptionsId[optionType][i] );
			break;
		}
	}
}

function choose_primary_attachments( weaponclass, weapon, allocation, allocation_max )
{
	attachments = weapon.supportedAttachments;
	remaining = allocation_max - allocation;

	if ( !attachments.size || !remaining )
	{
		return 0;
	}

	attachment_action = chose_action( "3_attachments", 25, "2_attachments", 65, "1_attachments", 10, "none", 5 );

	if ( remaining >= 4 && attachment_action == "3_attachments" )
	{
		a1 = array::random( attachments );
		self BotClassAddAttachment( weaponclass, weapon, a1, "primaryattachment1" );
		count = 1;

		attachments = GetWeaponAttachments( weapon, a1 );

		if ( attachments.size )
		{
			a2 = array::random( attachments );
			self BotClassAddAttachment( weaponclass, weapon, a2, "primaryattachment2" );
			count++;

			attachments = GetWeaponAttachments( weapon, a1, a2 );

			if ( attachments.size )
			{
				a3 = array::random( attachments );
				self BotClassAddItem( weaponclass, "BONUSCARD_PRIMARY_GUNFIGHTER" );
				self BotClassAddAttachment( weaponclass, weapon, a3, "primaryattachment3" );
				return 4;
			}
		}

		return count;
	}
	else if ( remaining >= 2 && attachment_action == "2_attachments" )
	{
		a1 = array::random( attachments );
		self BotClassAddAttachment( weaponclass, weapon, a1, "primaryattachment1" );

		attachments = GetWeaponAttachments( weapon, a1 );

		if ( attachments.size )
		{
			a2 = array::random( attachments );
			self BotClassAddAttachment( weaponclass, weapon, a2, "primaryattachment2" );
			return 2;
		}

		return 1;
	}
	else if ( remaining >= 1 && attachment_action == "1_attachments" )
	{
		a = array::random( attachments );
		self BotClassAddAttachment( weaponclass, weapon, a, "primaryattachment1" );
		return 1;
	}

	return 0;
}

function choose_secondary_attachments( weaponclass, weapon, allocation, allocation_max )
{
	attachments = weapon.supportedAttachments ;
	remaining = allocation_max - allocation;

	if ( !attachments.size || !remaining )
	{
		return 0;
	}

	attachment_action = chose_action( "2_attachments", 10, "1_attachments", 40, "none", 50, "none", 0 );

	if ( remaining >= 3 && attachment_action == "2_attachments" )
	{
		a1 = array::random( attachments );
		self BotClassAddAttachment( weaponclass, weapon, a1, "secondaryattachment1" );

		attachments = GetWeaponAttachments( weapon, a1 );

		if ( attachments.size )
		{
			a2 = array::random( attachments );
			self BotClassAddItem( weaponclass, "BONUSCARD_SECONDARY_GUNFIGHTER" );
			self BotClassAddAttachment( weaponclass, weapon, a2, "secondaryattachment2" );
			return 3;
		}

		return 1;
	}
	else if ( remaining >= 1 && attachment_action == "1_attachments" )
	{
		a = array::random( attachments );
		self BotClassAddAttachment( weaponclass, weapon, a, "secondaryattachment1" );
		return 1;
	}

	return 0;
}

function build_item_list()
{
	items = [];
	
	for( i = 0; i < STATS_TABLE_MAX_ITEMS; i++ )
	{
		row = tableLookupRowNum( level.statsTableID, STATS_TABLE_COL_NUMBERING, i );

		if ( row > -1 )
		{
			slot = tableLookupColumnForRow( level.statsTableID, row, STATS_TABLE_COL_SLOT );

			if ( slot == "" )
			{
				continue;
			}

			number = Int( tableLookupColumnForRow( level.statsTableID, row, STATS_TABLE_COL_NUMBERING ) );
/*		
			if ( SessionModeIsPrivate() && self IsItemLocked( number ) )
			{
				continue;
			}
*/
			allocation = Int( tableLookupColumnForRow( level.statsTableID, row, STATS_TABLE_COL_ALLOCATION ) );

			if ( allocation < 0 )
			{
				continue;
			}

			name = tableLookupColumnForRow( level.statsTableID, row, STATS_TABLE_COL_NAME );
/*
			if ( item_is_banned( slot, name ) )
			{
				continue;
			}
*/
			if ( !isdefined( items[slot] ) )
			{
				items[slot] = [];
			}

			items[ slot ][ items[slot].size ] = name;
		}
	}

	return items;
}


function build_claimed_list( items )
{
	claimed = [];
	keys = GetArrayKeys( items );

	foreach( key in keys )
	{
		claimed[ key ] = 0;
	}

	return claimed;
}