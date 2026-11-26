#using scripts\shared\exploder_shared;
#using scripts\shared\util_shared;
#using scripts\zm\zm_factory_teleporter;

#namespace more_players_factory;

detour zm_factory_teleporter<scripts\zm\zm_factory_teleporter.gsc>::teleport_players()
{
	players = GetPlayers();
	core_pos = [];
	image_room = [];
	occupied = [];
	for (i = 0; i < 4; i++)
	{
		core_pos[i] = GetEnt("origin_teleport_player_" + i, "targetname");
		image_room[i] = GetEnt("teleport_room_" + i, "targetname");
		occupied[i] = false;
	}

	touching = [];
	for (i = 0; i < players.size; i++)
	{
		player = players[i];
		room = image_room[i % 4];
		if (!isdefined(player) || !self zm_factory_teleporter::player_is_near_pad(player))
			continue;
		touching[touching.size] = player;
		if (!isdefined(room))
			continue;
		player DisableOffhandWeapons();
		player DisableWeapons();
		offset = (0, 0, 0);
		if (player GetStance() == "prone")
			offset = (0, 0, 49);
		else if (player GetStance() == "crouch")
			offset = (0, 0, 20);
		player.teleport_origin = Spawn("script_origin", player.origin);
		player.teleport_origin.angles = player.angles;
		player LinkTo(player.teleport_origin);
		player.teleport_origin.origin = room.origin + offset;
		player FreezeControls(true);
		util::wait_network_frame();
		if (isdefined(player))
		{
			util::setClientSysState("levelNotify", "black_box_start", player);
			player.teleport_origin.angles = room.angles;
		}
	}

	wait 2;
	core = GetEnt("trigger_teleport_core", "targetname");
	core thread zm_factory_teleporter::teleport_nuke(undefined, 300);
	foreach (player in players)
	{
		if (!isdefined(player))
			continue;
		for (j = 0; j < 4; j++)
		{
			if (!occupied[j] && Distance2D(core_pos[j].origin, player.origin) < 16)
				occupied[j] = true;
		}
		util::setClientSysState("levelNotify", "black_box_end", player);
	}
	util::wait_network_frame();

	for (i = 0; i < touching.size; i++)
	{
		player = touching[i];
		if (!isdefined(player))
			continue;
		slot = i % 4;
		for (tries = 0; occupied[slot] && tries < 4; tries++)
			slot = (slot + 1) % 4;
		occupied[slot] = true;
		player Unlink();
		if (isdefined(player.teleport_origin))
			player.teleport_origin Delete();
		player.teleport_origin = undefined;
		player EnableWeapons();
		player EnableOffhandWeapons();
		player SetOrigin(core_pos[slot].origin);
		player SetPlayerAngles(core_pos[slot].angles);
		player FreezeControls(false);
		if (i >= 4)
			player.more_players_stack_grace = GetTime() + 10000;
		player thread zm_factory_teleporter::teleport_aftereffects();
	}

	exploder::exploder_duration("mainframe_arrival", 1.7);
	exploder::exploder_duration("mainframe_steam", 14.6);
}
