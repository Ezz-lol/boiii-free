#using scripts\codescripts\struct;
#using scripts\shared\scene_shared;
#using scripts\zm\zm_theater_teleporter;

#namespace more_players_theater;

detour zm_theater_teleporter<scripts\zm\zm_theater_teleporter.gsc>::get_array_spots(sname, spots)
{
	for (i = 0; i < GetDvarInt("com_maxclients") || i < 4; i++)
		spots[i] = GetEnt(sname + (i % 4), "targetname");
	return spots;
}

detour zm_theater_teleporter<scripts\zm\zm_theater_teleporter.gsc>::function_7e0ed731(slot, offset)
{
	self endon("disconnect");
	room = struct::get("teleport_room_fx_" + (slot % 4 + 1), "targetname");
	wormhole = struct::spawn(room.origin - offset, room.angles);
	if (isdefined(wormhole))
	{
		waittillframeend;
		wormhole scene::play("p7_fxanim_zm_kino_wormhole_bundle");
		wormhole struct::delete();
	}
}
