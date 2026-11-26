#using scripts\shared\flag_shared;
#using scripts\zm\zm_castle_ee_bossfight;

#namespace more_players_castle;

detour zm_castle_ee_bossfight<scripts\zm\zm_castle_ee_bossfight.gsc>::boss_fight_ready()
{
	level endon("boss_fight_begin");
	level.var_b366f2dc = 0;
	areas = GetEntArray("boss_gravity_spike_start_area", "targetname");
	foreach (area in areas)
	{
		area.b_claimed = 0;
		area thread zm_castle_ee_bossfight::function_1c249965();
	}
	while (level.var_b366f2dc < min(level.players.size, areas.size))
		wait 0.05;
	wait 0.75;
	foreach (player in level.players)
		player.var_d725b0f2 = undefined;
	level flag::set("boss_fight_begin");
}
