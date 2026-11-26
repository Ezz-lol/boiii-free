#using scripts\codescripts\struct;
#using scripts\shared\array_shared;
#using scripts\shared\flag_shared;
#using scripts\shared\scene_shared;
#using scripts\zm\_zm_bgb;
#using scripts\zm\_zm_utility;
#using scripts\zm\_zm_weapons;
#using scripts\zm\zm_stalingrad_dragon;
#using scripts\zm\zm_stalingrad_vo;

#namespace more_players_stalingrad;

function seat(rider)
{
	if (level.activeplayers.size == 1)
		return "tag_passenger" + level.var_3f756142;
	return "tag_passenger" + ((rider - 1) % 4 + 1);
}

detour dragon<scripts\zm\zm_stalingrad_dragon.gsc>::function_280af5e8()
{
	self endon("disconnect");
	self endon("death");
	if (!array::contains(level.var_163a43e4, self))
	{
		level.var_428b5b88++;
		tag = seat(level.var_428b5b88);
		array::add(level.var_163a43e4, self);
		self thread dragon::function_d1ac3943();
		self bgb::suspend_weapon_cycling();
		self EnableInvulnerability();
		self.var_fa6d2a24 = 1;
		self zm_utility::increment_ignoreme();
		self.var_4222bc21 = 1;
		self thread zm_stalingrad_vo::function_cf8fccfe(1);
		angles = level.var_e29dd7ca GetTagAngles(tag);
		self SetPlayerAngles((0, angles[1], 0));
		self PlayerLinkToDelta(level.var_e29dd7ca, tag, 1, 45, 45, 80, 10, 1, 1);
		if (level.var_163a43e4.size == 1)
		{
			level flag::set("dragon_platform_one_rider");
			level thread dragon::function_51d28faf();
			level thread dragon::function_4e945eab();
		}
	}
	if (level.var_428b5b88 == level.activeplayers.size)
	{
		level flag::set("dragon_full");
		level thread dragon::function_825bb926();
	}
}

detour dragon<scripts\zm\zm_stalingrad_dragon.gsc>::function_3f0f5f61()
{
	spots = struct::get_array("s_pavlov_player", "targetname");
	rider = 0;
	foreach (player in level.var_163a43e4)
	{
		rider++;
		if (array::contains(level.activeplayers, player))
		{
			player scene::stop();
			player Unlink();
			player FreezeControls(0);
			player EnableWeapons();
			player EnableOffhandWeapons();
			player zm_weapons::switch_back_primary_weapon(player.var_de933247, 0);
			tag = seat(rider);
			player PlayerLinkToAbsolute(level.var_e29dd7ca, tag);
			wait 0.05;
			if (isdefined(player))
			{
				player PlayerLinkToDelta(level.var_e29dd7ca, tag, 1, 45, 45, 80, 10, 1, 1);
				player thread dragon::function_900a9386();
			}
		}
	}
	level.var_fc730f22 = [];
	wait 2;
	index = 0;
	foreach (player in level.var_163a43e4)
	{
		if (array::contains(level.activeplayers, player))
		{
			player Unlink();
			if (level.activeplayers.size == 1)
				spot = spots[level.var_3f756142 - 1];
			else
				spot = spots[index % spots.size];
			angles = level.var_e29dd7ca GetTagAngles(seat(index + 1));
			player thread dragon::function_fce6cca8(spot.origin, angles);
			wait 0.4;
		}
		index++;
	}
	level.var_163a43e4 = [];
	level.var_428b5b88 = 0;
	level notify(#"hash_9a634383");
}
