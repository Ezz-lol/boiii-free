// Applies the character picked in the lobby (the equipped zombies hero) on stock maps in solo and private matches
#using scripts\shared\callbacks_shared;
#using scripts\shared\system_shared;

#insert scripts\shared\shared.gsh;

#namespace zm_character_select;

REGISTER_SYSTEM("zm_character_select", &__init__, undefined)

function __init__()
{
	// Mods and custom maps keep their own character handling
	fs_game = GetDvarString("fs_game");
	if (IsDedicated() || (fs_game != "" && fs_game != "usermaps") || !is_supported_map(GetDvarString("mapname")))
		return;

	callback::on_start_gametype(&init);
}

// Moon is left out since its Easter egg needs a Richtofen, which its own assignment always hands out
function is_supported_map(mapname)
{
	switch (mapname)
	{
		case "zm_zod":
		case "zm_factory":
		case "zm_castle":
		case "zm_island":
		case "zm_stalingrad":
		case "zm_genesis":
		case "zm_tomb":
		case "zm_prototype":
		case "zm_asylum":
		case "zm_sumpf":
		case "zm_theater":
		case "zm_cosmodrome":
		case "zm_temple":
			return true;
	}
	return false;
}

function init()
{
	if (!IsDefined(level.givecustomcharacters))
		return;

	level.character_select_givecustomcharacters = level.givecustomcharacters;
	level.givecustomcharacters = &give_custom_characters;
}

function give_custom_characters()
{
	// The map only rolls a random character while characterindex is unset
	if (!IsDefined(self.characterindex) && !self IsTestClient())
	{
		hero = self GetSpecialistIndex();
		index = self character_index_for_hero(hero);
		if (IsDefined(index))
		{
			self.characterindex = index;
			// The maps' own assignment flags index 2 when it's given out, or forces it on the last player to join even
			// if it's taken. Some EE steps also check the flag. Zetsubou's flag has no name.
			if (index == 2)
			{
				level.has_richtofen = true;
				if (GetDvarString("mapname") == "zm_island")
					level.var_c7ffdf5 = true;
			}
		}
	}
	self [[level.character_select_givecustomcharacters]]();
}

function character_index_for_hero(hero)
{
	// Shadows of Evil sets body type characterindex + 5, which is the hero index
	if (GetDvarString("mapname") == "zm_zod")
	{
		index = hero - 5;
		if (index < 0 || index > 3 || character_taken(index))
			return undefined;
		return index;
	}

	names = [];
	names[0] = "dempsey";
	names[1] = "nikolai";
	names[2] = "richtofen";
	names[3] = "takeo";
	if (hero < 0 || hero >= names.size)
		return undefined;

	// Maps order their characters differently and their scripts disagree with each other, so check the body model
	// each index actually gives
	for (index = 0; index < 4; index++)
	{
		if (character_taken(index))
			continue;

		self SetCharacterBodyType(index);
		if (IsSubStr(self GetCharacterBodyModel(), names[hero]))
			return index;
	}
	return undefined;
}

function character_taken(index)
{
	foreach (player in GetPlayers())
	{
		if (player != self && IsDefined(player.characterindex) && player.characterindex == index)
			return true;
	}
	return false;
}
