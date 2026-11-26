#using scripts\codescripts\struct;
#using scripts\shared\callbacks_shared;
#using scripts\shared\clientfield_shared;
#using scripts\shared\laststand_shared;
#using scripts\shared\math_shared;
#using scripts\shared\system_shared;
#using scripts\zm\_zm_laststand;
#using scripts\zm\_zm_utility;

#insert scripts\shared\shared.gsh;

#namespace more_players;

#define STOCK_MAX 4
#define MAX_PLAYERS 18
#define REVIVE_OBJECTIVE 110

REGISTER_SYSTEM("more_players", &__init__, undefined)

function __init__()
{
	level.more_players_missing_fields = [];
	for (i = STOCK_MAX; i < MAX_PLAYERS; i++)
	{
		foreach (name in array("laststand_update" + i, "player" + i + "hasItem", "player" + i + "wearableItem",
			"pillar_challenge_" + i + "_1", "pillar_challenge_" + i + "_2", "pillar_challenge_" + i + "_3", "portal_state_ending_" + i))
			level.more_players_missing_fields[name] = true;
	}

	if (GetDvarInt("com_maxclients") <= STOCK_MAX)
		return;
	callback::on_connect(&on_connect);
	callback::on_spawned(&on_spawned);
}

detour clientfield<scripts\shared\clientfield_shared.gsc>::set(str_field_name, n_value)
{
	if (self == level)
	{
		if (!isdefined(level.more_players_missing_fields) || !isdefined(level.more_players_missing_fields[str_field_name]))
			CodeSetWorldClientField(str_field_name, n_value);
	}
	else
	{
		CodeSetClientField(self, str_field_name, n_value);
	}
}

detour clientfield<scripts\shared\clientfield_shared.gsc>::increment(str_field_name, n_increment_count)
{
	if (!isdefined(n_increment_count))
		n_increment_count = 1;
	if (self == level && isdefined(level.more_players_missing_fields) && isdefined(level.more_players_missing_fields[str_field_name]))
		return;
	for (i = 0; i < n_increment_count; i++)
	{
		if (self == level)
			CodeIncrementWorldClientField(str_field_name);
		else
			CodeIncrementClientField(self, str_field_name);
	}
}

detour zm_score<scripts\zm\_zm_score.gsc>::score_cf_increment_info(name)
{
	if (!isdefined(self.entity_num) || self.entity_num >= STOCK_MAX)
		return;
	foreach (player in level.players)
	{
		if (IS_TRUE(player.ready_for_score_events))
			player clientfield::increment_uimodel("PlayerList.client" + self.entity_num + ".score_cf_" + name);
	}
}

function on_connect()
{
	install_level_hooks();
	level.player_too_many_players_check = false;
	self thread fill_empty_gum_pack();
	self thread mark_when_down();
}

function install_level_hooks()
{
	if (IS_TRUE(level.more_players_hooked))
		return;
	level.more_players_hooked = true;

	level.player_too_many_players_check_func = &no_player_limit;

	alias_slot_structs("targetname", "s_pavlov_player");
	alias_slot_structs("targetname", "dark_arena_teleport_hijack");
	alias_slot_structs("targetname", "past_laboratory_telepoints");
	alias_slot_structs("targetname", "teleport_room_pos");
	for (i = STOCK_MAX; i < MAX_PLAYERS; i++)
	{
		if (!isdefined(level.struct_class_names["targetname"]["gauntlet_bgb_teleport_" + i]))
			level.struct_class_names["targetname"]["gauntlet_bgb_teleport_" + i] = level.struct_class_names["targetname"]["gauntlet_bgb_teleport_" + (i % STOCK_MAX)];
	}

	if (isdefined(level.player_name_directive))
	{
		for (i = STOCK_MAX; i < MAX_PLAYERS; i++)
		{
			if (!isdefined(level.player_name_directive[i]))
				level.player_name_directive[i] = level.player_name_directive[i % STOCK_MAX];
		}
	}

	level.more_players_spawn_delay = level.func_get_zombie_spawn_delay;
	level.func_get_zombie_spawn_delay = &spawn_delay;

	if (isdefined(level.givecustomcharacters))
	{
		level.more_players_give_characters = level.givecustomcharacters;
		level.givecustomcharacters = &give_characters;
	}

	level.more_players_intersection = level.player_intersection_tracker_override;
	level.player_intersection_tracker_override = &intersection_override;

	if (!isdefined(level.bgb_machine_movement_frequency_override_func))
		level.bgb_machine_movement_frequency_override_func = &bgb_machine_movement;
}

function no_player_limit()
{
}

function alias_slot_structs(key, name)
{
	structs = level.struct_class_names[key][name];
	if (!isdefined(structs) || !structs.size || structs.size >= MAX_PLAYERS)
		return;
	size = structs.size;
	for (i = size; i < MAX_PLAYERS; i++)
		level.struct_class_names[key][name][i] = structs[i % size];
}

function spawn_delay(round)
{
	if (level.players.size <= STOCK_MAX && isdefined(level.more_players_spawn_delay))
		return [[ level.more_players_spawn_delay ]](round);

	delay = 0.67;
	for (i = 1; i < min(round, 60); i++)
	{
		delay *= 0.95;
		if (delay <= 0.1)
			return 0.1;
	}
	return delay;
}

function bgb_machine_movement()
{
	switch (level.players.size)
	{
		case 1:
			level.bgb_machine_min_uses_before_move = 1;
			level.bgb_machine_max_uses_before_move = 3;
			break;
		case 2:
			level.bgb_machine_min_uses_before_move = 1;
			level.bgb_machine_max_uses_before_move = 4;
			break;
		case 3:
			level.bgb_machine_min_uses_before_move = 3;
			level.bgb_machine_max_uses_before_move = 5;
			break;
		default:
			level.bgb_machine_min_uses_before_move = 3;
			level.bgb_machine_max_uses_before_move = 6;
			break;
	}
}

function fill_empty_gum_pack()
{
	self endon("disconnect");
	if (self GetEntityNumber() < STOCK_MAX)
		return;

	for (i = 0; !isdefined(self.bgb_pack); i++)
	{
		if (i > 40)
			return;
		wait 0.5;
	}
	if (!isdefined(level.bgb))
		return;
	foreach (gum in self.bgb_pack)
	{
		if (isdefined(gum) && gum != "weapon_null")
			return;
	}

	pack = [];
	foreach (gum in array("zm_bgb_always_done_swiftly", "zm_bgb_arms_grace", "zm_bgb_coagulant", "zm_bgb_in_plain_sight", "zm_bgb_stock_option"))
	{
		if (isdefined(level.bgb[gum]))
			pack[pack.size] = gum;
	}
	if (pack.size)
	{
		self.bgb_pack = pack;
		self.bgb_pack_randomized = [];
	}
}

function mark_when_down()
{
	slot = self GetEntityNumber();
	if (slot < STOCK_MAX || slot >= MAX_PLAYERS)
		return;
	objective = REVIVE_OBJECTIVE + slot;
	self thread unmark_on_disconnect(objective);
	self endon("disconnect");

	marked = false;
	marked_origin = (0, 0, 0);
	revive_start = undefined;
	revive_ms = 3000;
	sent_progress = 0;
	for (;;)
	{
		down = self laststand::player_is_in_laststand();
		if (down && !marked)
		{
			marked_origin = self.origin;
			Objective_Add(objective, "active", marked_origin);
			Objective_SetProgress(objective, 0);
			marked = true;
			revive_start = undefined;
			sent_progress = 0;
		}
		else if (down && DistanceSquared(marked_origin, self.origin) > 16)
		{
			marked_origin = self.origin;
			Objective_Position(objective, marked_origin);
		}
		else if (!down && marked)
		{
			Objective_State(objective, "invisible");
			marked = false;
		}

		if (marked)
		{
			progress = 0;
			if (isdefined(self.revivetrigger) && IS_TRUE(self.revivetrigger.beingrevived))
			{
				if (!isdefined(revive_start))
				{
					revive_start = GetTime();
					revive_ms = self revive_time_ms();
				}
				progress = math::clamp((GetTime() - revive_start) / revive_ms, 0.02, 1);
			}
			else
			{
				revive_start = undefined;
			}
			if (progress != sent_progress)
			{
				Objective_SetProgress(objective, progress);
				sent_progress = progress;
			}
		}
		WAIT_SERVER_FRAME;
	}
}

function revive_time_ms()
{
	foreach (player in GetPlayers())
	{
		if (player != self && player zm_laststand::is_reviving(self))
		{
			seconds = player zm_laststand::revive_get_revive_time(self);
			if (isdefined(seconds) && seconds > 0)
				return seconds * 1000;
		}
	}
	return 3000;
}

function unmark_on_disconnect(objective)
{
	self waittill("disconnect");
	Objective_State(objective, "invisible");
}

function give_characters()
{
	if (!isdefined(self.characterindex))
	{
		index = self least_used_character();
		if (isdefined(index))
		{
			self.characterindex = index;
			self.more_players_shared_character = true;
		}
	}

	rich_sq_player = level.rich_sq_player;
	sndradioa = level.sndradioa;
	self [[ level.more_players_give_characters ]]();
	if (IS_TRUE(self.more_players_shared_character))
	{
		if (isdefined(rich_sq_player))
			level.rich_sq_player = rich_sq_player;
		if (isdefined(sndradioa))
			level.sndradioa = sndradioa;
	}
}

function least_used_character()
{
	uses = array(0, 0, 0, 0);
	foreach (player in GetPlayers())
	{
		if (player != self && isdefined(player.characterindex) && player.characterindex >= 0 && player.characterindex < STOCK_MAX)
			uses[player.characterindex]++;
	}

	lowest = undefined;
	foreach (count in uses)
	{
		if (count == 0)
			return undefined;
		if (!isdefined(lowest) || count < lowest)
			lowest = count;
	}

	candidates = [];
	for (i = 0; i < STOCK_MAX; i++)
	{
		if (uses[i] == lowest)
			candidates[candidates.size] = i;
	}
	return candidates[RandomInt(candidates.size)];
}

function on_spawned()
{
	self thread unstack();
	send_characters();
}

function send_characters()
{
	chars = [];
	for (i = 0; i < MAX_PLAYERS; i++)
		chars[i] = "-";
	foreach (player in GetPlayers())
	{
		if (isdefined(player.characterindex))
			chars[player GetEntityNumber()] = "" + player.characterindex;
	}
	value = "";
	foreach (char in chars)
		value += char;
	foreach (player in GetPlayers())
	{
		if (!player IsTestClient())
			player setclientdvar("more_players_chars " + value);
	}
}

function unstack()
{
	self endon("disconnect");
	self endon("death");
	WAIT_SERVER_FRAME;
	if (!IsAlive(self) || self.sessionstate != "playing" || !self is_stacked())
		return;

	spot = self find_free_spot();
	if (isdefined(spot))
		self SetOrigin(spot);
	else
		self.more_players_stack_grace = GetTime() + 10000;
}

function is_stacked()
{
	foreach (player in GetPlayers())
	{
		if (player != self && player.sessionstate == "playing" && is_near(self.origin, player.origin, 34))
			return true;
	}
	return false;
}

function is_near(a, b, dist)
{
	return abs(a[2] - b[2]) <= 60 && Distance2DSquared(a, b) < dist * dist;
}

function find_free_spot()
{
	query = PositionQuery_Source_Navigation(self.origin, 40, 200, 60, 20);
	if (!isdefined(query) || !isdefined(query.data))
		return undefined;

	check_area = GetEntArray("player_volume", "script_noteworthy").size > 0;
	players = GetPlayers();
	best = undefined;
	best_dist = undefined;
	foreach (point in query.data)
	{
		spot = point.origin;
		dist = Distance2DSquared(self.origin, spot);
		if ((isdefined(best_dist) && dist >= best_dist) || abs(spot[2] - self.origin[2]) > 60)
			continue;

		clear = true;
		foreach (player in players)
		{
			if (player != self && player.sessionstate == "playing" && is_near(spot, player.origin, 44))
			{
				clear = false;
				break;
			}
		}
		if (!clear || PositionWouldTelefrag(spot))
			continue;
		if (check_area && !zm_utility::check_point_in_playable_area(spot))
			continue;
		if (!BulletTracePassed(self.origin + (0, 0, 40), spot + (0, 0, 40), false, self))
			continue;

		best = spot;
		best_dist = dist;
	}
	return best;
}

function intersection_override(other_player)
{
	now = GetTime();
	if ((isdefined(self.more_players_stack_grace) && now < self.more_players_stack_grace) ||
		(isdefined(other_player.more_players_stack_grace) && now < other_player.more_players_stack_grace))
		return true;
	if (isdefined(level.more_players_intersection))
		return self [[ level.more_players_intersection ]](other_player);
	return false;
}
