#using scripts\shared\array_shared;
#using scripts\shared\bots\_bot;
#using scripts\shared\callbacks_shared;
#using scripts\shared\flag_shared;
#using scripts\shared\laststand_shared;
#using scripts\shared\system_shared;
#using scripts\zm\_zm_equipment;
#using scripts\zm\_zm_melee_weapon;
#using scripts\zm\_zm_perks;
#using scripts\zm\_zm_powerups;
#using scripts\zm\_zm_score;
#using scripts\zm\_zm_utility;
#using scripts\zm\_zm_weapons;

#insert scripts\shared\shared.gsh;

#namespace zm_bots;

#define MODE_FOLLOW 0
#define MODE_WANDER 1
#define MODE_CAMP 2

#define THINK_INTERVAL 0.1
#define THREAT_RADIUS_MIN 200
#define THREAT_RADIUS_MAX 400
#define FOLLOW_RANGE 650
#define FOLLOW_AREA 800
#define TELEPORT_RANGE 3000
#define UNREACHABLE_RANGE 1500

REGISTER_SYSTEM("zm_bots", &__init__, undefined)

function __init__()
{
	callback::on_connect(&on_player_connect);
	callback::on_spawned(&on_player_spawned);
	callback::on_player_killed(&on_player_killed);
	level thread start();
}

function start()
{
	level flag::wait_till("initial_blackscreen_passed");
	callback::remove_on_connect(&bot::on_player_connect);
	callback::remove_on_spawned(&bot::on_player_spawned);
	callback::remove_on_player_killed(&bot::on_player_killed);
	init_settings();
	level thread track_doors();
	count = min(GetDvarInt("zm_bots"), GetDvarInt("com_maxclients") - GetPlayers().size);
	for (i = 0; i < count; i++)
	{
		if (!isdefined(AddTestClient()))
			break;
		wait 0.1;
	}
}

function init_settings()
{
	level.zm_bot_melee_range = GetDvarInt("player_meleeRangeDefault");
	SetDvar("bot_AllowMelee", 1);
	SetDvar("bot_AllowGrenades", 1);
	SetDvar("bot_AllowKillstreaks", 0);
	SetDvar("bot_AllowHeroGadgets", 0);
	SetDvar("bot_Fov", 120);
	SetDvar("bot_FovAds", 80);
	SetDvar("bot_PitchSensitivity", 1.2);
	SetDvar("bot_YawSensitivity", 1.2);
	SetDvar("bot_PitchSpeed", 120);
	SetDvar("bot_PitchSpeedAds", 80);
	SetDvar("bot_YawSpeed", 240);
	SetDvar("bot_YawSpeedAds", 160);
	SetDvar("pitchAccelerationTime", 0.05);
	SetDvar("yawAccelerationTime", 0.05);
	SetDvar("pitchDecelerationThreshold", 5);
	SetDvar("yawDecelerationThreshold", 5);
}

function on_player_connect()
{
	if (!self IsTestClient())
		return;
	self.bot = SpawnStruct();
	self.bot.threat = SpawnStruct();
	self.pers["isBot"] = 1;
	self.bot_rank = RandomInt(35);
	self.bot_prestige = RandomInt(12);
}

function on_player_spawned()
{
	if (!self IsTestClient())
		return;
	self notify("zm_bot_spawned");
	self endon("zm_bot_spawned");
	self endon("disconnect");

	self apply_rank();
	if (!isdefined(self.num_perks))
		self.num_perks = 0;

	self.bot.mode = GetDvarInt("zm_bot_mode");
	self.bot.camp_origin = self.origin;
	self.bot.task = "none";
	self.bot.movement = "none";
	self.bot.revive_state = "none";
	self.bot.anchor = self.origin;
	self.bot.look_until = 0;
	self.bot.use_time = GetTime();
	self.bot.melee_time = GetTime();
	self.bot.tactical_time = 0;
	self.bot.lethal_time = 0;
	self.bot.strafing = 0;
	self.bot.threats = [];
	self.bot.roam_time = 0;
	self.bot.gum_round = 0;
	if (enabled("zm_bot_gums"))
		self.bgb_pack = random_gum_pack();
	self.bot.moved_time = GetTime();
	self.bot.next_door_urge = GetTime() + RandomIntRange(20000, 40000);
	self clear_stuck();
	self clear_threat();
	self BotSetGoal(self.origin);

	level flag::wait_till("initial_blackscreen_passed");

	self.bot.starting_weapon = self GetCurrentWeapon();
	self start_spread();
	self thread wait_path_failed_loop();
	self thread wait_goal_reached_loop();
	self thread think_loop();
	self thread task_loop();
	self thread update_loop();
}

function apply_rank()
{
	self.pers["rank"] = self.bot_rank;
	self.pers["plevel"] = self.bot_prestige;
	self.pers["prestige"] = self.bot_prestige;
	self SetRank(self.bot_rank, self.bot_prestige);
}

function start_spread()
{
	leader = self get_leader();
	if (isdefined(leader))
		center = leader.origin;
	else
		center = self.origin;
	self roam_near(center, 100, 350);
}

function roam_near(center, radius_min, radius_max)
{
	query = PositionQuery_Source_Navigation(center, radius_min, radius_max, 100, 48, self);
	foreach (point in array::randomize(query.data))
	{
		if (self CanPath(self.origin, point.origin))
		{
			self BotSetGoal(point.origin, 24);
			self.bot.roam_time = GetTime() + RandomIntRange(2500, 5000);
			return true;
		}
	}
	return false;
}

function on_player_killed()
{
	if (self IsTestClient())
		self BotReleaseManualControl();
}

function think_loop()
{
	self endon("zm_bot_spawned");
	self endon("disconnect");
	self endon("death");
	self endon("bled_out");
	level endon("end_game");

	for (;;)
	{
		self release_buttons();
		self check_stuck();
		self sprint_think();
		self thread combat_think();
		self thread movement_think();
		wait THINK_INTERVAL;
	}
}

function task_loop()
{
	self endon("zm_bot_spawned");
	self endon("disconnect");
	self endon("death");
	self endon("bled_out");
	level endon("end_game");

	for (;;)
	{
		self task_think();
		wait THINK_INTERVAL;
	}
}

function update_loop()
{
	self endon("zm_bot_spawned");
	self endon("disconnect");
	self endon("death");
	self endon("bled_out");
	level endon("end_game");

	wait 1;
	for (;;)
	{
		if (self.pers["rank"] !== self.bot_rank || self.pers["prestige"] !== self.bot_prestige)
			self apply_rank();
		if (self.sessionstate == "playing" && !self laststand::player_is_in_laststand())
		{
			self switch_weapon();
			self reload_weapon();
			self keep_perks();
			if (enabled("zm_bot_gums"))
				self use_gum();
		}
		wait 0.5;
	}
}

function keep_perks()
{
	if (!isdefined(level._custom_perks))
		return;
	if (!isdefined(self.num_perks))
		self.num_perks = 0;
	foreach (perk in array("specialty_armorvest", "specialty_quickrevive", "specialty_fastreload", "specialty_doubletap2"))
	{
		if (isdefined(level._custom_perks[perk]) && !self HasPerk(perk))
			self zm_perks::give_perk(perk, false);
	}
}

function release_buttons()
{
	self BotReleaseButton(0);
	self BotReleaseButton(1);
	self BotReleaseButton(2);
	if (self.bot.revive_state != "revive" && GetTime() > self.bot.use_time + 500)
		self BotReleaseButton(3);
	self BotReleaseButton(4);
	self BotReleaseButton(9);
	self BotReleaseButton(10);
	self BotReleaseButton(11);
	self BotReleaseButton(14);
	self BotReleaseButton(15);
	self BotReleaseButton(24);
}

function sprint_to_goal()
{
	self.bot.sprint_to_goal = 1;
}

function end_sprint_to_goal()
{
	self.bot.sprint_to_goal = 0;
}

function sprint_think()
{
	if (!IS_TRUE(self.bot.sprint_to_goal))
		return;
	if (self BotGoalReached() || (self threat_visible() && IS_TRUE(self.bot.threat.inrange)))
	{
		self end_sprint_to_goal();
		return;
	}
	self BotLookForward();
	self BotPressButton(1);
}

function check_stuck()
{
	if (self BotUnderManualControl() || self BotGoalReached() || self laststand::player_is_in_laststand() || self IsMeleeing() || self has_threat(level.zm_bot_melee_range))
		return;
	if (self GetVelocity() == (0, 0, 0))
	{
		self.bot.stuck_cycles++;
		if (self.bot.stuck_cycles >= 3)
			self thread stuck_resolution();
	}
	else
		self.bot.stuck_cycles = 0;
	if (!self threat_visible())
		self check_stuck_position();
}

function check_stuck_position()
{
	if (GetTime() < self.bot.check_position_time)
		return;
	self.bot.check_position_time = GetTime() + 500;
	self.bot.position_history[self.bot.position_index] = self.origin;
	self.bot.position_index = (self.bot.position_index + 1) % 5;
	if (self.bot.position_history.size < 5)
		return;
	for (i = 0; i < self.bot.position_history.size; i++)
	{
		for (j = i + 1; j < self.bot.position_history.size; j++)
		{
			if (DistanceSquared(self.bot.position_history[i], self.bot.position_history[j]) > 16384)
				return;
		}
	}
	self thread stuck_resolution();
}

function stuck_resolution(direction)
{
	self endon("death");
	self endon("bled_out");
	level endon("end_game");
	self clear_stuck();
	self BotTakeManualControl();
	if (isdefined(direction))
		angle = direction[1] + RandomIntRange(-30, 30);
	else
		angle = self GetAngles()[1] + 180 + RandomIntRange(-45, 45);
	self BotSetMoveAngle(AnglesToForward((0, angle, 0)));
	self BotSetMoveMagnitude(1);
	wait 1;
	self BotReleaseManualControl();
	self BotSetGoal(self.origin);
}

function clear_stuck()
{
	self.bot.stuck_cycles = 0;
	self.bot.position_history = [];
	self.bot.position_index = 0;
	self.bot.check_position_time = 0;
}

function wait_path_failed_loop()
{
	self endon("zm_bot_spawned");
	self endon("disconnect");
	self endon("death");
	self endon("bled_out");
	level endon("end_game");
	for (;;)
	{
		self waittill("bot_path_failed");
		self thread stuck_resolution();
	}
}

function wait_goal_reached_loop()
{
	self endon("zm_bot_spawned");
	self endon("disconnect");
	self endon("death");
	self endon("bled_out");
	level endon("end_game");
	for (;;)
	{
		self waittill("bot_goal_reached");
		self clear_stuck();
	}
}

function alive_players()
{
	players = [];
	foreach (player in level.players)
	{
		if (isdefined(player) && player.sessionstate == "playing")
			players[players.size] = player;
	}
	return players;
}

function fwd_dot(point)
{
	return VectorDot(AnglesToForward(self GetPlayerAngles()), VectorNormalize(point - self GetEye()));
}

function can_reach(point)
{
	return isdefined(point) && self CanPath(self.origin, point) && zm_utility::check_point_in_enabled_zone(point);
}

function can_see(target, from)
{
	if (!isdefined(from))
		from = self GetEye();
	if (!IsSentient(target))
		return BulletTracePassed(from, target GetCentroid(), false, self, target);
	return BulletTracePassed(from, target GetEye(), false, self, target) || BulletTracePassed(from, target GetCentroid(), false, self, target);
}

function at_goal(point)
{
	goal = self BotGetGoalPosition();
	return isdefined(goal) && goal == point && self BotGoalReached();
}

function go_to(point)
{
	if (self at_goal(point))
		return true;
	self BotSetGoal(point);
	return false;
}

function look_at(point)
{
	self.bot.look_until = GetTime() + 500;
	self BotLookAtPoint(point);
}

function use_trigger(trigger, hold_use)
{
	if (!isdefined(trigger) || self IsSwitchingWeapons())
		return;
	if (self zm_utility::has_powerup_weapon())
	{
		weapons = self GetWeaponsListPrimaries();
		self SwitchToWeapon(weapons[0]);
		return;
	}
	if (IS_TRUE(hold_use))
	{
		self BotPressButton(3);
		self.bot.use_time = GetTime();
	}
	if (!isdefined(self.bot.next_use) || self.bot.next_use < GetTime())
	{
		trigger notify("trigger", self);
		self BotLookForward();
		self.bot.next_use = GetTime() + 1000;
	}
}

function interact(point, focus, trigger, hold_use)
{
	if (!self go_to(point))
		return;
	if (self fwd_dot(focus) >= 0.9)
		self use_trigger(trigger, hold_use);
	else
		self look_at(focus);
}

function movement_think()
{
	if (self.bot.task != "none")
		return;
	switch (self.bot.mode)
	{
		case MODE_WANDER:
			self navmesh_wander();
			self.bot.movement = "none";
			break;
		case MODE_CAMP:
			self.bot.movement = self hold_position() ? "hold" : "none";
			break;
		default:
			self.bot.movement = self follow() ? "follow" : "none";
			break;
	}
	if (self GetVelocity() != (0, 0, 0))
		self.bot.moved_time = GetTime();
	else if (GetTime() - self.bot.moved_time > 5000 && !self has_threat())
	{
		leader = self get_leader();
		if (isdefined(leader))
			center = leader.origin;
		else
			center = self.origin;
		if (self roam_near(center, 100, 350))
			self.bot.moved_time = GetTime();
	}
	self idle_look();
}

function follow()
{
	leader = self get_leader();
	if (!isdefined(leader))
		return false;

	distance = Distance(self.origin, leader.origin);
	reachable = self can_reach(leader.origin);
	if (distance > TELEPORT_RANGE || (!reachable && distance > UNREACHABLE_RANGE))
	{
		if (!isdefined(self.bot.far_since))
			self.bot.far_since = GetTime();
		if (GetTime() - self.bot.far_since > 15000 && leader IsOnGround() && self teleport_near(leader))
			self.bot.far_since = undefined;
		else
			self follow_player(leader);
		return true;
	}
	self.bot.far_since = undefined;

	if (distance <= FOLLOW_AREA)
	{
		if (self has_threat(THREAT_RADIUS_MIN))
			return false;
		trace = leader eye_trace(200);
		if (trace["entity"] === self && leader IsMeleeing())
		{
			if (!self BotUnderManualControl())
				self thread stuck_resolution(leader GetAngles());
		}
		else if (!self has_threat() && self BotGoalReached() && GetTime() > self.bot.roam_time)
		{
			self roam_near(leader.origin, 200, 600);
			self.bot.roam_time = GetTime() + RandomIntRange(4000, 8000);
		}
		return true;
	}
	self follow_player(leader);
	return true;
}

function follow_player(player)
{
	goal = self BotGetGoalPosition();
	if (!isdefined(goal) || Distance(goal, player.origin) > 600)
	{
		if (!self roam_near(player.origin, 150, 400))
			self BotSetGoal(player.origin, 128);
	}
	if (Distance(self.origin, player.origin) > 900)
		self sprint_to_goal();
}

function teleport_near(player)
{
	query = PositionQuery_Source_Navigation(player.origin, 48, 160, 24, 32, self);
	foreach (point in query.data)
	{
		closest = ArrayGetClosest(point.origin, alive_players());
		if (zm_utility::check_point_in_enabled_zone(point.origin) && self FindPath(point.origin, player.origin) && isdefined(closest) && Distance(point.origin, closest.origin) >= 48)
		{
			self SetOrigin(point.origin);
			self BotSetGoal(point.origin);
			return true;
		}
	}
	return false;
}

function eye_trace(max_distance)
{
	eye = self GetEye();
	return BulletTrace(eye, eye + AnglesToForward(self GetPlayerAngles()) * max_distance, true, self);
}

function navmesh_wander()
{
	if (self has_threat())
		return;
	if (!isdefined(self.bot.wander_time) || !self BotGoalReached())
	{
		self.bot.wander_time = GetTime() + RandomIntRange(400, 800);
		return;
	}
	if (GetTime() < self.bot.wander_time)
		return;
	fwd = AnglesToForward(self.angles);
	fwd = VectorNormalize((fwd[0], fwd[1], 0));
	query = PositionQuery_Source_Navigation(self.origin, 128, 1500, 150, 64, self);
	best = undefined;
	origin = (self.origin[0], self.origin[1], 0);
	foreach (point in query.data)
	{
		dot = VectorDot(VectorNormalize((point.origin[0], point.origin[1], 0) - origin), fwd);
		point.score = MapFloat(128, 1500, 0, 50, point.disttoorigin2d);
		if (dot > 0.87)
			point.score += RandomFloatRange(30, 50);
		else if (dot > 0)
			point.score += RandomFloatRange(10, 35);
		else
			point.score += RandomFloatRange(0, 15);
		foreach (player in GetPlayers())
		{
			if (player != self && Distance2D(player.origin, point.origin) < 300)
				point.score -= 40;
		}
		if ((!isdefined(best) || point.score > best.score) && self can_reach(point.origin))
			best = point;
	}
	if (isdefined(best))
	{
		self BotSetGoal(best.origin, 128);
		self BotLookForward();
	}
	else
		self thread stuck_resolution();
}

function hold_position()
{
	if ((Distance(self.origin, self.bot.camp_origin) > THREAT_RADIUS_MIN && self has_threat(THREAT_RADIUS_MAX)) || self has_threat(level.zm_bot_melee_range * 2))
		return false;
	if (self can_reach(self.bot.camp_origin))
	{
		closest = ArrayGetClosest(self.bot.camp_origin, alive_players());
		if (closest === self)
			self BotSetGoal(self.bot.camp_origin);
		else
			self BotSetGoal(self.bot.camp_origin, 72);
	}
	return true;
}

function idle_look()
{
	if (self has_threat() || self GetVelocity() != (0, 0, 0))
	{
		self.bot.idle_look_time = undefined;
		return;
	}
	if (!isdefined(self.bot.idle_look_time))
		self.bot.idle_look_time = GetTime() + RandomIntRange(1500, 3000);
	if (GetTime() > self.bot.idle_look_time)
	{
		self BotLookAtPoint(self.origin + (RandomIntRange(-100, 100), RandomIntRange(-100, 100), 64));
		self.bot.idle_look_time = undefined;
	}
}

function get_leader()
{
	leader = undefined;
	foreach (player in GetPlayers())
	{
		if (!isdefined(player) || player == self || isdefined(player.bot))
			continue;
		if (!isdefined(leader) || DistanceSquared(self.origin, player.origin) < DistanceSquared(self.origin, leader.origin))
			leader = player;
	}
	return leader;
}

function task_think()
{
	far = false;
	if (self.bot.mode == MODE_FOLLOW)
	{
		leader = self get_leader();
		if (isdefined(leader))
		{
			if (self.bot.task == "none")
				self.bot.anchor = leader.origin;
			limit = 1200;
			if (self.bot.task == "none")
				limit = 900;
			far = Distance(self.origin, leader.origin) > limit;
		}
	}
	if (self laststand::player_is_in_laststand())
		self.bot.task = "none";
	else if (self gas_mask())
		self.bot.task = "mask";
	else if (enabled("zm_bot_revive") && self revive())
		self.bot.task = self.bot.revive_state == "guard" ? "guard revive" : "revive";
	else if (far && !(enabled("zm_bot_powerups") && self pick_up_powerup()))
		self.bot.task = "none";
	else if (far)
		self.bot.task = "powerup";
	else if (enabled("zm_bot_pap") && self pack_a_punch())
		self.bot.task = "pack a punch";
	else if (enabled("zm_bot_box") && self box_weapon())
		self.bot.task = "box";
	else if (enabled("zm_bot_wallbuys") && self wall_weapon())
		self.bot.task = "wallbuy";
	else if (enabled("zm_bot_perks") && self vending_perk())
		self.bot.task = "perk";
	else if (enabled("zm_bot_wunderfizz") && self wunderfizz())
		self.bot.task = "wunderfizz";
	else if (enabled("zm_bot_gums") && self gum_machine())
		self.bot.task = "gobblegum";
	else if (enabled("zm_bot_powerups") && self pick_up_powerup())
		self.bot.task = "powerup";
	else if (enabled("zm_bot_doors") && self open_doors())
		self.bot.task = "door";
	else if (enabled("zm_bot_power") && self power_switch())
		self.bot.task = "power";
	else if (enabled("zm_bot_shield") && self pick_up_shield())
		self.bot.task = "shield";
	else if (enabled("zm_bot_windows") && self repair_window())
		self.bot.task = "window";
	else
	{
		if (self.bot.mode != MODE_FOLLOW)
			self.bot.anchor = self.origin;
		self.bot.task = "none";
		self.bot.task_target = undefined;
	}
}

function enabled(feature)
{
	return GetDvarString(feature) != "0";
}

function downed_players()
{
	players = [];
	foreach (player in alive_players())
	{
		if (player != self && player laststand::player_is_in_laststand() && player.team == self.team)
			players[players.size] = player;
	}
	return ArraySortClosest(players, self.origin);
}

function reviver_of(player)
{
	reviver = player.bot_reviver;
	if (isdefined(reviver) && reviver != self && IsAlive(reviver) && !reviver laststand::player_is_in_laststand() && isdefined(reviver.bot) && reviver.bot.revive_target === player)
		return reviver;
	return undefined;
}

function revive()
{
	guard = undefined;
	foreach (player in self downed_players())
	{
		if (!self can_reach(player.origin))
			continue;
		if (isdefined(self reviver_of(player)))
		{
			if (!isdefined(guard))
				guard = player;
			continue;
		}
		player.bot_reviver = self;
		self.bot.revive_target = player;
		self revive_player(player);
		return true;
	}
	self.bot.revive_target = undefined;
	if (isdefined(guard))
	{
		self.bot.revive_state = "guard";
		if (Distance(self.origin, guard.origin) > 200)
		{
			self BotSetGoal(guard.origin, 160);
			self sprint_to_goal();
		}
		return true;
	}
	self.bot.revive_state = "none";
	return false;
}

function revive_player(player)
{
	if (self zm_utility::has_powerup_weapon())
	{
		weapons = self GetWeaponsListPrimaries();
		self SwitchToWeapon(weapons[0]);
	}
	if (Distance(player.origin, self.origin) > 72 || !BulletTracePassed(self GetEye(), player GetCentroid(), false, self, player))
	{
		self.bot.revive_state = "reach";
		self BotSetGoal(player.origin);
		self sprint_to_goal();
		return;
	}
	self.bot.revive_state = "revive";
	self BotSetGoal(self.origin);
	self BotSetLookAnglesFromPoint(player GetCentroid());
	self BotPressButton(3);
	self.bot.use_time = GetTime();
}

function gas_mask()
{
	mask = GetWeapon("equip_gasmask");
	if (self zm_equipment::has_player_equipment(mask))
	{
		if (self zm_equipment::is_active(mask))
			return false;
		self SwitchToWeapon(mask);
		return true;
	}
	if (self has_threat(THREAT_RADIUS_MIN))
		return false;
	foreach (equipment in ArraySortClosest(GetEntArray("zombie_equipment_upgrade", "targetname"), self.bot.anchor))
	{
		if (equipment.equipment !== mask)
			continue;
		point = GetClosestPointOnNavMesh(equipment GetCentroid(), 128, 32);
		if (!self can_reach(point))
			continue;
		self interact(point, equipment.origin, equipment);
		self sprint_to_goal();
		return true;
	}
	return false;
}

function blocked(target)
{
	key = self GetEntityNumber();
	if (isdefined(target.bot_blocked) && isdefined(target.bot_blocked[key]) && GetTime() < target.bot_blocked[key])
		return true;
	foreach (player in GetPlayers())
	{
		if (player != self && isdefined(player.bot) && player.bot.task_target === target && !player laststand::player_is_in_laststand())
			return true;
	}
	return false;
}

function track(target)
{
	if (self.bot.task_target !== target)
	{
		self.bot.task_target = target;
		self.bot.task_since = GetTime();
	}
	else if (GetTime() - self.bot.task_since > 15000)
	{
		if (!isdefined(target.bot_blocked))
			target.bot_blocked = [];
		target.bot_blocked[self GetEntityNumber()] = GetTime() + 30000;
		self.bot.task_target = undefined;
	}
}

function closest_task(candidates)
{
	open = [];
	foreach (candidate in candidates)
	{
		if (!self blocked(candidate))
			open[open.size] = candidate;
	}
	if (self.bot.mode == MODE_FOLLOW)
		target = ArrayGetClosest(self.bot.anchor, open, FOLLOW_RANGE);
	else
		target = ArrayGetClosest(self.bot.anchor, open);
	if (isdefined(target))
		self track(target);
	return target;
}

function pack_a_punch()
{
	if (self.bot.mode == MODE_CAMP || !isdefined(level.pack_a_punch) || !isdefined(level.pack_a_punch.triggers))
		return false;
	machines = [];
	foreach (trigger in level.pack_a_punch.triggers)
	{
		if (!isdefined(trigger.zbarrier) || (isdefined(trigger.pap_machine) && trigger.pap_machine.state === "leaving"))
			continue;
		trigger.bot_point = GetClosestPointOnNavMesh(trigger.zbarrier GetCentroid(), 128, 32);
		if (isdefined(trigger.powered) && IS_TRUE(trigger.powered.power) && self can_reach(trigger.bot_point))
			machines[machines.size] = trigger;
	}
	machine = self closest_task(machines);
	if (!isdefined(machine))
		return false;

	selected = undefined;
	if (machine.pack_player === self)
	{
		if (self has_threat(THREAT_RADIUS_MIN))
			return false;
	}
	else
	{
		if (self has_threat(THREAT_RADIUS_MAX))
			return false;
		weapons = self GetWeaponsListPrimaries();
		if (weapons.size < zm_utility::get_player_weapon_limit(self))
			return false;
		foreach (weapon in weapons)
		{
			if (!zm_weapons::is_weapon_upgraded(weapon) && zm_weapons::can_upgrade_weapon(weapon) && self zm_score::can_player_purchase(machine.cost) && self pap_worth(weapon))
			{
				selected = weapon;
				break;
			}
		}
		if (!isdefined(selected))
			return false;
	}

	if (!isdefined(machine.pack_player))
	{
		if (self go_to(machine.bot_point))
		{
			if (self fwd_dot(machine.origin) < 0.9)
				self look_at(machine.origin);
			else if (self GetCurrentWeapon() == selected)
				self use_trigger(machine);
			else
				self SwitchToWeapon(selected);
		}
	}
	else if (machine.pack_player === self)
	{
		if (machine IsTriggerEnabled())
			self interact(machine.bot_point, machine.origin, machine);
		else if (Distance(self.origin, machine.bot_point) > 72)
			self BotSetGoal(machine.bot_point);
		else
		{
			self BotSetGoal(self.origin);
			if (!self has_threat())
				self BotLookAtPoint(machine.origin);
		}
	}
	else if (Distance(self.origin, machine.bot_point) > 128)
		self BotSetGoal(machine.bot_point);
	else
		self BotSetGoal(self.origin);
	self sprint_to_goal();
	return true;
}

function box_weapon()
{
	if (self.bot.mode == MODE_CAMP || !isdefined(level.chests) || level.chests.size == 0 || self has_great_weapons())
		return false;
	chests = [];
	foreach (chest in level.chests)
	{
		if (!isdefined(chest.zbarrier) || IS_TRUE(chest.zbarrier.chest_moving) || IS_TRUE(chest.hidden))
			continue;
		chest.bot_point = GetClosestPointOnNavMesh(chest.zbarrier GetCentroid(), 128, 32);
		if ((self can_reach(chest.bot_point) && self zm_score::can_player_purchase(chest.zombie_cost) && !IS_TRUE(chest._box_open)) || chest.chest_user === self)
			chests[chests.size] = chest;
	}
	chest = self closest_task(chests);
	if (!isdefined(chest))
		return false;

	weakest = self weakest_weapon();
	if (chest.chest_user !== self && (!isdefined(weakest) || self good_for_round(weakest)))
		return false;
	if (chest.chest_user === self)
	{
		if (self has_threat(THREAT_RADIUS_MIN))
			return false;
	}
	else if (self has_threat(THREAT_RADIUS_MAX))
		return false;

	float_height = 40;
	if (isdefined(level.custom_magicbox_float_height))
		float_height = level.custom_magicbox_float_height;
	weapon_point = chest.zbarrier.origin + (0, 0, float_height);
	if (!IS_TRUE(chest._box_open))
		self interact(chest.bot_point, weapon_point, chest);
	else if (chest.chest_user === self)
	{
		offered = chest.zbarrier.weapon;
		if (!isdefined(offered) || offered == level.weaponnone)
		{
			if (Distance(self.origin, chest.bot_point) > 72)
				self BotSetGoal(chest.bot_point);
			else
			{
				self BotSetGoal(self.origin);
				if (!self has_threat())
					self BotLookAtPoint(weapon_point);
			}
		}
		else if (self needs_equipment(offered))
			self interact(chest.bot_point, weapon_point, chest);
		else
		{
			replace = self weakest_weapon(offered);
			if (!isdefined(replace) || replace == offered || (replace != level.weaponnone && !self HasWeapon(replace)))
				return false;
			if (self go_to(chest.bot_point))
			{
				if (self fwd_dot(weapon_point) < 0.9)
					self look_at(weapon_point);
				else if (replace == level.weaponnone || self GetCurrentWeapon() == replace)
					self use_trigger(chest);
				else
					self SwitchToWeapon(replace);
			}
		}
	}
	else
		return false;
	self sprint_to_goal();
	return true;
}

function wall_weapon()
{
	if (self.bot.mode == MODE_CAMP || self has_threat(THREAT_RADIUS_MAX) || !isdefined(level._spawned_wallbuys) || level._spawned_wallbuys.size == 0)
		return false;
	weakest = self weakest_weapon();
	if (!isdefined(weakest) || self good_for_round(weakest))
		return false;
	wallbuys = [];
	foreach (wallbuy in level._spawned_wallbuys)
	{
		if (!isdefined(wallbuy.weapon) || !isdefined(wallbuy.trigger_stub) || !isdefined(level.zombie_weapons[wallbuy.weapon]))
			continue;
		if (self zm_weapons::has_weapon_or_upgrade(wallbuy.weapon) || !self zm_score::can_player_purchase(level.zombie_weapons[wallbuy.weapon].cost))
			continue;
		stub = wallbuy.trigger_stub;
		point = stub.origin;
		if (isdefined(stub.script_length))
			point += AnglesToRight(stub.angles) * stub.script_length * -1;
		wallbuy.bot_point = BulletTrace(point, point - (0, 0, 64), false, self)["position"];
		if (self can_reach(wallbuy.bot_point) && (self good_for_round(wallbuy.weapon) || self needs_equipment(wallbuy.weapon)))
			wallbuys[wallbuys.size] = wallbuy;
	}
	wallbuy = self closest_task(wallbuys);
	if (!isdefined(wallbuy))
		return false;

	stub = wallbuy.trigger_stub;
	if (self go_to(wallbuy.bot_point))
	{
		trigger = isdefined(stub.playertrigger) ? stub.playertrigger[self GetEntityNumber()] : undefined;
		if (self fwd_dot(stub.origin) < 0.9)
			self look_at(stub.origin);
		else if (!isdefined(trigger))
		{
			wallbuy.bot_point = GetClosestPointOnNavMesh(stub.origin, 128, 72);
			self BotSetGoal(wallbuy.bot_point);
		}
		else if (self needs_equipment(wallbuy.weapon))
			self use_trigger(trigger);
		else if (weakest == level.weaponnone || self GetCurrentWeapon() == weakest)
			self use_trigger(trigger);
		else
			self SwitchToWeapon(weakest);
	}
	self sprint_to_goal();
	return true;
}

function vending_perk()
{
	if (self.bot.mode == MODE_CAMP || self has_threat(THREAT_RADIUS_MAX) || !self zm_utility::can_player_purchase_perk())
		return false;
	machines = [];
	foreach (trigger in GetEntArray("zombie_vending", "targetname"))
	{
		perk = trigger.script_noteworthy;
		if (!isdefined(perk) || self HasPerk(perk) || perk == "specialty_tombstone" || perk == "specialty_whoswho")
			continue;
		if (!isdefined(trigger.machine) || !IS_TRUE(trigger.power_on) || !isdefined(trigger.cost) || !self zm_score::can_player_purchase(trigger.cost))
			continue;
		trigger.bot_point = GetClosestPointOnNavMesh(trigger.machine GetCentroid(), 128, 32);
		if (self can_reach(trigger.bot_point))
			machines[machines.size] = trigger;
	}
	machine = self closest_task(machines);
	if (!isdefined(machine))
		return false;
	self interact(machine.bot_point, machine.origin, machine);
	self sprint_to_goal();
	return true;
}

function other_bot_closer(point)
{
	distance = DistanceSquared(self.origin, point);
	foreach (player in GetPlayers())
	{
		if (player != self && isdefined(player.bot) && player.bot.revive_state === "none" && !player laststand::player_is_in_laststand() && DistanceSquared(player.origin, point) < distance)
			return true;
	}
	return false;
}

function random_gum_pack()
{
	mega = array("zm_bgb_aftertaste", "zm_bgb_burned_out", "zm_bgb_dead_of_nuclear_winter", "zm_bgb_immolation_liquidation", "zm_bgb_licensed_contractor", "zm_bgb_phoenix_up", "zm_bgb_respin_cycle", "zm_bgb_unquenchable", "zm_bgb_whos_keeping_score", "zm_bgb_fatal_contraption", "zm_bgb_unbearable");
	rare = array("zm_bgb_cache_back", "zm_bgb_kill_joy", "zm_bgb_on_the_house", "zm_bgb_wall_power", "zm_bgb_temporal_gift", "zm_bgb_crate_power", "zm_bgb_bullet_boost", "zm_bgb_extra_credit", "zm_bgb_soda_fountain");
	ultra = array("zm_bgb_perkaholic", "zm_bgb_head_drama", "zm_bgb_shopping_free", "zm_bgb_near_death_experience", "zm_bgb_profit_sharing", "zm_bgb_round_robbin", "zm_bgb_self_medication", "zm_bgb_power_vacuum");
	pack = [];
	for (i = 0; i < 5; i++)
	{
		roll = RandomInt(100);
		if (roll < 50)
			pool = mega;
		else if (roll < 85)
			pool = rare;
		else
			pool = ultra;
		selected = array::random(pool);
		ArrayRemoveValue(mega, selected);
		ArrayRemoveValue(rare, selected);
		ArrayRemoveValue(ultra, selected);
		pack[pack.size] = selected;
	}
	return pack;
}

function gum_cost(base_cost)
{
	uses = 0;
	if (isdefined(self.bgb_machine_uses_this_round))
		uses = self.bgb_machine_uses_this_round;
	if (uses == 0 && GetDvarInt("scr_firstGumFree") === 1)
		return 0;
	tier = int(min(floor(level.round_number / 10.0), 10));
	switch (uses)
	{
		case 0:
			return base_cost;
		case 1:
			return int(base_cost + 1000 * pow(2, tier)) * 10;
		case 2:
			return int(500 + base_cost + 1000 * pow(2, tier + 1)) * 10;
	}
	return 0;
}

function gum_machine()
{
	if (self.bot.mode == MODE_CAMP || !isdefined(level.bgb_machines) || level.bgb_machines.size == 0 || !isdefined(level.bgb) || !isdefined(self.bgb))
		return false;
	if (self.bgb != "none" && isdefined(level.bgb[self.bgb]))
	{
		limit_type = level.bgb[self.bgb].limit_type;
		if (!((limit_type == "event" || limit_type == "activated") && level.round_number - self.bot.gum_round > 2))
			return false;
	}
	machines = [];
	foreach (machine in level.bgb_machines)
	{
		if (IS_TRUE(machine.hidden) || self blocked(machine))
			continue;
		machine.bot_point = GetClosestPointOnNavMesh(machine GetCentroid(), 128, 32);
		if ((self can_reach(machine.bot_point) && isdefined(machine.base_cost) && self zm_score::can_player_purchase(self gum_cost(machine.base_cost))) || machine.bgb_machine_user === self)
			machines[machines.size] = machine;
	}
	machine = self closest_task(machines);
	if (!isdefined(machine))
		return false;
	if (machine.bgb_machine_user === self)
	{
		if (self has_threat(THREAT_RADIUS_MIN))
			return false;
	}
	else if (self has_threat(THREAT_RADIUS_MAX))
		return false;

	focus = machine.origin + (0, 0, 64);
	if (!IS_TRUE(machine.bgb_machine_open))
		self interact(machine.bot_point, focus, machine);
	else if (machine.bgb_machine_user === self)
	{
		if (IS_TRUE(machine.grab_bgb_hint) && self.selected_bgb !== "")
		{
			if (self go_to(machine.bot_point) && self fwd_dot(focus) >= 0.9)
			{
				self use_trigger(machine);
				self.bot.gum_round = level.round_number;
			}
			else
				self look_at(focus);
		}
		else if (Distance(self.origin, machine.bot_point) > 72)
			self BotSetGoal(machine.bot_point);
		else
		{
			self BotSetGoal(self.origin);
			if (!self has_threat())
				self BotLookAtPoint(focus);
		}
	}
	else
		return false;
	self sprint_to_goal();
	return true;
}

function use_gum()
{
	if (!isdefined(self.bgb) || self.bgb == "none" || !isdefined(level.bgb) || !isdefined(level.bgb[self.bgb]) || IS_TRUE(self.bgb_activation_in_progress))
		return;
	if (level.bgb[self.bgb].limit_type != "activated")
		return;
	double_points = isdefined(level.zombie_vars[self.team]) && IS_TRUE(level.zombie_vars[self.team]["zombie_powerup_double_points_on"]);
	insta_kill = isdefined(level.zombie_vars[self.team]) && IS_TRUE(level.zombie_vars[self.team]["zombie_insta_kill"]);
	zombies = GetAITeamArray(level.zombie_team).size;
	switch (self.bgb)
	{
		case "zm_bgb_dead_of_nuclear_winter":
			use = (level.zombie_total == 0 && zombies == 0) || self downed_players().size > 0;
			break;
		case "zm_bgb_immolation_liquidation":
			use = level.zombie_total == 0 && !IS_TRUE(level.zombie_vars["zombie_powerup_fire_sale_on"]);
			break;
		case "zm_bgb_licensed_contractor":
			use = double_points;
			break;
		case "zm_bgb_phoenix_up":
			use = self downed_players().size > 0;
			break;
		case "zm_bgb_whos_keeping_score":
			use = level.zombie_total > 8 && !double_points;
			break;
		case "zm_bgb_fatal_contraption":
			use = !self zm_utility::has_powerup_weapon() && self get_threats(1000).size > 5;
			break;
		case "zm_bgb_cache_back":
			use = !self has_threat() && !self GetWeaponAmmoStock(self GetCurrentWeapon());
			break;
		case "zm_bgb_kill_joy":
			use = level.zombie_total > 8 && !insta_kill;
			break;
		case "zm_bgb_on_the_house":
		case "zm_bgb_extra_credit":
			use = !self has_threat();
			break;
		case "zm_bgb_bullet_boost":
			use = isdefined(self.aat) && !isdefined(self.aat[self GetCurrentWeapon()]) && zm_weapons::weapon_supports_aat(self GetCurrentWeapon());
			break;
		case "zm_bgb_round_robbin":
			use = level.zombie_total == 0 && zombies < 7;
			break;
		default:
			use = true;
			break;
	}
	if (use && (!isdefined(self.bot.next_gum) || self.bot.next_gum < GetTime()))
	{
		self notify("bgb_activation_request");
		self.bot.next_gum = GetTime() + 10000;
	}
}

function wanted_random_perks()
{
	perks = [];
	if (!isdefined(level._random_perk_machine_perk_list))
		return perks;
	foreach (perk in level._random_perk_machine_perk_list)
	{
		if (!self HasPerk(perk) && perk != "specialty_tombstone" && perk != "specialty_whoswho")
			perks[perks.size] = perk;
	}
	return perks;
}

function perk_bottle_model(perk)
{
	if (isdefined(level._custom_perks[perk]) && isdefined(level._custom_perks[perk].perk_bottle_weapon))
		return level._custom_perks[perk].perk_bottle_weapon.worldmodel;
	if (isdefined(level.machine_assets) && isdefined(level.machine_assets[perk]))
		return level.machine_assets[perk].weapon.worldmodel;
	return undefined;
}

function wunderfizz()
{
	if (self.bot.mode == MODE_CAMP || !isdefined(level.perk_random_machines) || level.perk_random_machines.size == 0 || !self zm_utility::can_player_purchase_perk())
		return false;
	if (self wanted_random_perks().size == 0)
		return false;
	machines = [];
	foreach (machine in level.perk_random_machines)
	{
		if (self blocked(machine))
			continue;
		machine.bot_point = GetClosestPointOnNavMesh(machine GetCentroid(), 128, 32);
		if ((machine.state === "idle" && isdefined(machine.cost) && self can_reach(machine.bot_point) && self zm_score::can_player_purchase(machine.cost)) || (machine.state === "vending" && machine.machine_user === self))
			machines[machines.size] = machine;
	}
	machine = self closest_task(machines);
	if (!isdefined(machine))
		return false;
	if (machine.machine_user === self)
	{
		if (self has_threat(THREAT_RADIUS_MIN))
			return false;
	}
	else if (self has_threat(THREAT_RADIUS_MAX))
		return false;

	focus = machine.origin + (0, 0, 52);
	if (machine.state === "idle")
		self interact(machine.bot_point, focus, machine);
	else if (machine.machine_user === self)
	{
		if (IS_TRUE(machine.grab_perk_hint))
		{
			offered = undefined;
			if (isdefined(level.bottle_spawn_location))
			{
				foreach (perk in level._random_perk_machine_perk_list)
				{
					if (level.bottle_spawn_location.model === perk_bottle_model(perk))
					{
						offered = perk;
						break;
					}
				}
			}
			if (isdefined(offered) && !self HasPerk(offered))
				self interact(machine.bot_point, focus, machine);
			else
				return false;
		}
		else if (Distance(self.origin, machine.bot_point) > 72)
			self BotSetGoal(machine.bot_point);
		else
		{
			self BotSetGoal(self.origin);
			if (!self has_threat())
				self BotLookAtPoint(focus);
		}
	}
	else
		return false;
	self sprint_to_goal();
	return true;
}

function pick_up_shield()
{
	if (self.bot.mode == MODE_CAMP || self HasRiotShield() || self has_threat(THREAT_RADIUS_MAX) || !isdefined(level.a_uts_craftables))
		return false;
	foreach (table in ArraySortClosest(level.a_uts_craftables, self.bot.anchor))
	{
		if (!isdefined(table.weaponname) || !IS_TRUE(table.weaponname.isriotshield) || !IS_TRUE(table.crafted) || self blocked(table))
			continue;
		length = 0;
		if (isdefined(table.script_length))
			length = table.script_length / 2;
		point = table.origin + AnglesToForward(table.angles) * length;
		point = BulletTrace(point, point - (0, 0, 64), false, self)["position"];
		if (!self can_reach(point))
			continue;
		self track(table);
		if (self go_to(point))
		{
			trigger = undefined;
			if (isdefined(table.playertrigger))
				trigger = table.playertrigger[self GetEntityNumber()];
			if (self fwd_dot(table.origin) < 0.9)
				self look_at(table.origin);
			else
				self use_trigger(trigger);
		}
		self sprint_to_goal();
		return true;
	}
	return false;
}

function power_switch()
{
	if (self.bot.mode == MODE_CAMP || !level flag::exists("power_on") || level flag::get("power_on") || self has_threat(THREAT_RADIUS_MAX))
		return false;
	foreach (trigger in ArraySortClosest(GetEntArray("use_elec_switch", "targetname"), self.bot.anchor))
	{
		if (self blocked(trigger))
			continue;
		point = GetClosestPointOnNavMesh(trigger.origin, 128, 72);
		if (!self can_reach(point))
			continue;
		self track(trigger);
		self interact(point, trigger.origin, trigger, true);
		self sprint_to_goal();
		return true;
	}
	return false;
}

function pick_up_powerup()
{
	if (self has_threat(THREAT_RADIUS_MIN))
		return false;
	foreach (powerup in ArraySortClosest(zm_powerups::get_powerups(), self.bot.anchor))
	{
		if (powerup.powerup_name === "free_weapon")
			continue;
		if ((isdefined(powerup.powerup_player) && powerup.powerup_player != self) || self blocked(powerup) || self other_bot_closer(powerup.origin))
			continue;
		point = powerup.origin - (0, 0, 40);
		if (!self can_reach(point))
			continue;
		self track(powerup);
		self BotSetGoal(point);
		self sprint_to_goal();
		return true;
	}
	return false;
}

function track_doors()
{
	level endon("end_game");
	doors = ArrayCombine(GetEntArray("zombie_door", "targetname"), GetEntArray("zombie_debris", "targetname"), false, false);
	doors = ArrayCombine(doors, GetEntArray("zombie_airlock_buy", "targetname"), false, false);
	level.zm_bot_doors = [];
	foreach (door in doors)
	{
		if (door.script_noteworthy === "local_electric_door" || door.script_noteworthy === "electric_door" || !isdefined(door.target))
			continue;
		foreach (trigger in GetEntArray(door.target, "target"))
		{
			trigger.parent = door;
			trigger.nodes = [];
			query = PositionQuery_Source_Navigation(trigger.origin, 16, 192, 80, 16);
			foreach (point in query.data)
			{
				if (point.origin[2] < trigger GetAbsMaxs()[2])
					trigger.nodes[trigger.nodes.size] = point;
			}
			level.zm_bot_doors[level.zm_bot_doors.size] = trigger;
		}
	}
	while (level.zm_bot_doors.size > 0)
	{
		open = [];
		foreach (trigger in level.zm_bot_doors)
		{
			if (isdefined(trigger) && isdefined(trigger.parent) && isdefined(trigger.parent.zombie_cost) && !IS_TRUE(trigger._door_open) && !IS_TRUE(trigger.parent._door_open) && trigger IsTriggerEnabled() && trigger.parent IsTriggerEnabled())
				open[open.size] = trigger;
		}
		level.zm_bot_doors = open;
		wait 0.5;
	}
}

function open_doors()
{
	if (!isdefined(level.zm_bot_doors) || level.zm_bot_doors.size == 0 || self.bot.mode == MODE_CAMP || self has_threat(THREAT_RADIUS_MAX))
		return false;
	if (!IS_TRUE(self.bot.door_urge) && GetTime() > self.bot.next_door_urge)
	{
		self.bot.next_door_urge = GetTime() + RandomIntRange(20000, 40000);
		self.bot.door_urge = RandomInt(100) < 50;
	}
	target = undefined;
	target_point = undefined;
	foreach (trigger in ArraySortClosest(level.zm_bot_doors, self.bot.anchor, 3))
	{
		if (!isdefined(trigger) || !isdefined(trigger.parent) || self blocked(trigger) || !self zm_score::can_player_purchase(trigger.parent.zombie_cost))
			continue;
		if (self.bot.mode == MODE_FOLLOW && !IS_TRUE(self.bot.door_urge) && Distance(trigger.origin, self.bot.anchor) > FOLLOW_RANGE)
			continue;
		foreach (point in trigger.nodes)
		{
			if (self can_reach(point.origin))
			{
				target = trigger;
				target_point = point.origin;
				break;
			}
		}
		if (isdefined(target))
			break;
	}
	if (!isdefined(target))
	{
		self.bot.door_urge = false;
		return false;
	}
	self track(target);
	self interact(target_point, target.origin, target.parent, true);
	self sprint_to_goal();
	return true;
}

function repair_window()
{
	if (!isdefined(level.exterior_goals) || level.exterior_goals.size == 0 || self zm_utility::has_powerup_weapon())
		return false;
	windows = [];
	foreach (window in ArraySortClosest(level.exterior_goals, self.bot.anchor, 3, 0, 300))
	{
		if (!isdefined(window.barrier_chunks) || !isdefined(window.zbarrier) || self blocked(window))
			continue;
		point = window.zbarrier.origin + AnglesToForward(window.zbarrier.angles) * 56;
		window.bot_point = BulletTrace(point, point - (0, 0, 40), false, self)["position"];
		closest = ArrayGetClosest(window.bot_point, alive_players(), 128);
		if (isdefined(closest) && closest != self)
			continue;
		if (self can_reach(window.bot_point) && !zm_utility::all_chunks_intact(window, window.barrier_chunks) && !zm_utility::no_valid_repairable_boards(window, window.barrier_chunks))
			windows[windows.size] = window;
	}
	if (self.bot.mode == MODE_CAMP)
		window = ArrayGetClosest(self.bot.anchor, windows, 200);
	else
		window = ArrayGetClosest(self.bot.anchor, windows);
	if (!isdefined(window) || self has_threat(THREAT_RADIUS_MIN))
		return false;
	self track(window);
	if (!self go_to(window.bot_point))
	{
		if (!self has_threat())
			self BotLookForward();
	}
	else
	{
		self use_trigger(window, true);
		if (!self has_threat())
			self BotLookAtPoint(window.zbarrier.origin);
	}
	return true;
}

function weapon_score(weapon)
{
	if (!isdefined(weapon) || weapon == level.weaponnone)
		return 0;
	if (weapon == self.bot.starting_weapon)
		return 200;
	score = 0;
	if (zm_weapons::is_weapon_upgraded(weapon))
		score += 200;
	if (zm_weapons::is_wonder_weapon(weapon))
		score += 300;
	switch (weapon.weapclass)
	{
		case "spread":
			score += 100;
			break;
		case "smg":
			score += 150;
			break;
		case "rifle":
			score += 200;
			break;
		case "mg":
			score += 250;
			break;
		default:
			score += 50;
			break;
	}
	switch (weapon.firetype)
	{
		case "Burst":
			score += 100;
			break;
		case "Auto Burst":
			score += 150;
			break;
		case "Full Auto":
			score += 200;
			break;
		default:
			score += 50;
			break;
	}
	if (IsSubStr(weapon.weapclass, "spread"))
		score += max(200, weapon.shotcount * 45);
	if (weapon.isdualwield)
		score *= 1.5;
	if (weapon.unlimitedammo)
		score += 300;
	else
	{
		score += floor(weapon.clipsize / max(weapon.reloadtime, 0.1)) * 10;
		if (weapon.islauncher)
			score += weapon.maxammo * 15;
		else
			score += weapon.maxammo;
	}
	return score;
}

function good_for_round(weapon)
{
	if (level.round_number <= 1)
		return true;
	if (zm_utility::is_offhand_weapon(weapon))
		return false;
	return self weapon_score(weapon) >= min(level.round_number * 50, 750);
}

function great_weapon(weapon)
{
	if (IsSubStr(weapon.name, "ray_gun") || IsSubStr(weapon.name, "raygun_mark2"))
		return true;
	return self weapon_score(weapon) >= 900;
}

function pap_worth(weapon)
{
	return level.round_number * 44 >= min(self weapon_score(weapon.rootweapon), 1320);
}

function has_great_weapons()
{
	weapons = self GetWeaponsListPrimaries();
	if (weapons.size < zm_utility::get_player_weapon_limit(self))
		return false;
	foreach (weapon in weapons)
	{
		if (!self great_weapon(weapon))
			return false;
	}
	return true;
}

function weakest_weapon(candidate)
{
	weapons = self GetWeaponsListPrimaries();
	if (weapons.size < zm_utility::get_player_weapon_limit(self))
		return level.weaponnone;
	if (isdefined(candidate))
	{
		if (zm_utility::is_lethal_grenade(candidate) || zm_utility::is_tactical_grenade(candidate) || zm_utility::is_placeable_mine(candidate) || zm_utility::is_hero_weapon(candidate))
			return candidate;
		weapons[weapons.size] = candidate;
	}
	weakest = undefined;
	lowest = 0;
	foreach (weapon in weapons)
	{
		if (zm_weapons::is_weapon_upgraded(weapon))
			continue;
		if (self zm_utility::has_powerup_weapon() && weapon == self GetCurrentWeapon())
			continue;
		if (IsSubStr(weapon.name, "ray_gun") || IsSubStr(weapon.name, "raygun_mark2"))
			score = 9999;
		else
			score = self weapon_score(weapon);
		if (!isdefined(weakest) || score < lowest)
		{
			lowest = score;
			weakest = weapon;
		}
	}
	return weakest;
}

function needs_equipment(weapon)
{
	return (zm_utility::is_lethal_grenade(weapon) && self zm_utility::get_player_lethal_grenade() == level.weaponnone) || (zm_utility::is_tactical_grenade(weapon) && self zm_utility::get_player_tactical_grenade() == level.weaponnone) || (zm_utility::is_placeable_mine(weapon) && self zm_utility::get_player_placeable_mine() == level.weaponnone) || (zm_utility::is_hero_weapon(weapon) && self zm_utility::get_player_hero_weapon() == level.weaponnone);
}

function combat_think()
{
	self update_threat_list();
	if (self has_threat())
	{
		if (self threat_alive())
			self update_threat();
		else
			self drop_threat();
	}
	if (!self has_threat() && !self get_new_threat())
		return;
	if (self has_threat() && (!self threat_visible() || self.bot.threat.last_distance_sq > 20736))
		self get_new_threat(THREAT_RADIUS_MAX);

	if (self threat_visible())
	{
		if (self can_reach(self.bot.threat.last_position) || Distance(self.origin, self.bot.threat.last_position) < THREAT_RADIUS_MIN * 0.75)
			self thread update_threat_goal();
		else if (self.bot.mode == MODE_WANDER && self.bot.task == "none")
		{
			if (Distance(self.origin, self.bot.threat.last_position) > THREAT_RADIUS_MIN)
				self thread chase_threat();
			else
				self BotSetGoal(self.origin, 32);
		}
		self thread engage_threat();
	}
	else if (self.bot.threat.last_visible_time + 3500 < GetTime())
		self clear_threat();
	else if (self.bot.task == "none")
		self BotLookAtPoint(self.bot.threat.last_position + (0, 0, 64));
}

function update_threat_list()
{
	threats = [];
	foreach (threat in GetAITeamArray(level.zombie_team))
	{
		if (isdefined(threat.spawn_time) && threat.spawn_time + 1000 > GetTime())
			continue;
		if (IS_TRUE(threat.nuked) || !self can_see(threat))
			continue;
		distance_sq = DistanceSquared(self GetEye(), threat.origin);
		if (self fwd_dot(threat.origin) >= self BotGetFov() || distance_sq <= 20736 || (threat.talking === 1 && distance_sq <= 640000))
			threats[threats.size] = threat;
	}
	self.bot.threats = threats;
}

function get_threats(max_distance)
{
	if (!isdefined(max_distance))
		max_distance = 2000;
	threats = [];
	foreach (threat in self.bot.threats)
	{
		if (isdefined(threat) && Distance(self GetEye(), threat.origin) <= max_distance)
			threats[threats.size] = threat;
	}
	return threats;
}

function threats_near(center, radius)
{
	threats = [];
	foreach (threat in self.bot.threats)
	{
		if (isdefined(threat) && Distance(center, threat.origin) <= radius)
			threats[threats.size] = threat;
	}
	return threats;
}

function has_threat(max_distance)
{
	if (!isdefined(self.bot) || !isdefined(self.bot.threat) || !isdefined(self.bot.threat.entity))
		return false;
	if (isdefined(max_distance))
		return Distance(self.origin, self.bot.threat.entity.origin) <= max_distance;
	return true;
}

function threat_visible()
{
	return self has_threat() && IS_TRUE(self.bot.threat.visible);
}

function threat_alive()
{
	return !IS_TRUE(self.bot.threat.entity.nuked) && IsAlive(self.bot.threat.entity);
}

function set_threat(entity)
{
	self.bot.threat.entity = entity;
	self.bot.threat.aim_offset = self aim_offset(entity);
	self update_threat(true);
}

function drop_threat()
{
	self.bot.threat.entity = undefined;
	self.bot.strafing = 0;
	self clear_threat_aim();
}

function clear_threat()
{
	self drop_threat();
	self BotLookForward();
}

function update_threat(new_threat)
{
	entity = self.bot.threat.entity;
	if (IS_TRUE(new_threat))
	{
		self.bot.threat.is_new = 1;
		self clear_threat_aim();
	}
	else
		self.bot.threat.is_new = !IS_TRUE(self.bot.threat.visible);

	distance_sq = DistanceSquared(self GetEye(), entity.origin);
	velocity = entity GetVelocity();
	if (distance_sq > 20736)
		position = entity.origin + velocity * THINK_INTERVAL;
	else
		position = entity.origin;
	aim_point = position + self aim_offset(entity);
	dot = self fwd_dot(aim_point);

	if (IS_TRUE(new_threat))
		self.bot.threat.visible = 1;
	else if ((dot < self BotGetFov() && distance_sq > 20736) || !self can_see(entity) || entity.team == self.team)
	{
		self.bot.threat.visible = 0;
		return;
	}

	self.bot.threat.visible = 1;
	self.bot.threat.last_visible_time = GetTime();
	self.bot.threat.last_distance_sq = distance_sq;
	self.bot.threat.last_position = position;
	self.bot.threat.aim_point = aim_point;
	self.bot.threat.dot = dot;
	weapon = self GetCurrentWeapon();
	range = weapon_range(weapon);
	self.bot.threat.inrange = distance_sq < range * range;
	self.bot.threat.in_close_range = distance_sq < 40000;
}

function get_new_threat(max_distance)
{
	entity = ArrayGetClosest(self.origin, self get_threats(max_distance));
	if (isdefined(entity) && entity !== self.bot.threat.entity && (!self has_threat() || DistanceSquared(self GetEye(), entity.origin) + 16384 < DistanceSquared(self GetEye(), self.bot.threat.entity.origin)))
	{
		self set_threat(entity);
		return true;
	}
	return false;
}

function engage_threat()
{
	self end_sprint_to_goal();
	entity = self.bot.threat.entity;
	if (!isdefined(entity))
		return;

	if (enabled("zm_bot_grenades") && IS_TRUE(self.bot.threat.is_new) && !self IsThrowingGrenade() && !self FragButtonPressed() && !self SecondaryOffhandButtonPressed() && !self IsSwitchingWeapons() && !self IsReloading())
	{
		group = self threats_near(self.bot.threat.last_position, 500).size;
		distance_sq = self.bot.threat.last_distance_sq;
		tactical = self zm_utility::get_player_tactical_grenade();
		if (RandomInt(100) < 66 && distance_sq >= 160000 && distance_sq <= 1000000 && self GetWeaponAmmoStock(tactical) && group > 5 && self.bot.tactical_time < GetTime())
		{
			self clear_threat_aim();
			self throw_grenade(self.bot.threat.last_position, tactical);
			self.bot.tactical_time = GetTime() + 90000;
			return;
		}
		lethal = self zm_utility::get_player_lethal_grenade();
		if (RandomInt(100) < 75 && distance_sq >= 160000 && distance_sq <= 1000000 && self GetWeaponAmmoStock(lethal) && group > 3 && self.bot.lethal_time < GetTime())
		{
			self clear_threat_aim();
			self throw_grenade(self.bot.threat.last_position, lethal);
			self.bot.lethal_time = GetTime() + 30000;
			return;
		}
		self.bot.threat.aim_offset = self aim_offset(entity);
	}

	if (self IsThrowingGrenade())
	{
		self throw_grenade(self.bot.threat.last_position);
		return;
	}
	if (self.bot.revive_state == "revive")
		return;
	if (self.bot.task != "none" && self.bot.task != "guard revive" && (!IS_TRUE(self.bot.threat.inrange) || self.bot.look_until > GetTime()))
		return;

	self update_weapon_aim();
	if (self IsSwitchingWeapons() || self IsThrowingGrenade() || self FragButtonPressed() || self SecondaryOffhandButtonPressed() || self IsMeleeing())
		return;
	if (self melee_attack() || entity.targetname === "avogadro")
		return;
	if (self IsReloading())
		return;
	self update_weapon_ads();
	self fire_weapon();
}

function update_threat_goal()
{
	if (self BotUnderManualControl() || self.bot.task != "none" || self.bot.movement != "none")
	{
		self.bot.strafing = 0;
		return;
	}
	if (self BotGoalSet() && self.bot.strafing && (!IS_TRUE(self.bot.threat.is_new) || !IS_TRUE(self.bot.threat.visible)))
		return;
	radius = self threat_goal_radius();
	if (Distance2DSquared(self.origin, self.bot.threat.last_position) < radius * radius)
	{
		self.bot.strafing = 1;
		self combat_strafe();
		return;
	}
	self.bot.strafing = 0;
	self BotSetGoal(self.bot.threat.last_position, radius);
}

function threat_goal_radius()
{
	weapon = self GetCurrentWeapon();
	if (weapon.weapclass == "melee" || weapon.weapclass == "item" || self.bot.threat.entity.targetname === "avogadro")
		return level.zm_bot_melee_range;
	return RandomIntRange(THREAT_RADIUS_MIN, THREAT_RADIUS_MAX);
}

function chase_threat()
{
	if (!IS_TRUE(self.bot.threat.is_new) && self BotGoalSet())
		return;
	query = PositionQuery_Source_Navigation(self.origin, 0, 384, 64, 32, self);
	best = undefined;
	fwd = AnglesToForward(self GetPlayerAngles());
	foreach (point in query.data)
	{
		closest = ArrayGetClosest(point.origin, level.players);
		if (closest != self && Distance(closest.origin, point.origin) <= 64)
			continue;
		if (self can_reach(point.origin) && self can_see(self.bot.threat.entity, point.origin + (0, 0, self GetEye()[2] - self.origin[2])) && VectorDot(fwd, VectorNormalize(point.origin - self.origin)) > 0)
			best = point;
	}
	if (isdefined(best))
		self BotSetGoal(best.origin);
}

function fire_weapon()
{
	if (!IS_TRUE(self.bot.threat.inrange))
		return;
	if (self PlayerAds() > 0 && self PlayerAds() < 1)
		return;
	weapon = self GetCurrentWeapon();
	if (weapon == level.weaponnone || weapon.weapclass == "melee" || weapon.weapclass == "item")
		return;
	if (!self GetWeaponAmmoClip(weapon) && weapon.clipsize > 0 && !self zm_utility::has_powerup_weapon())
		return;
	if (self.bot.threat.dot < self fire_dot())
		return;
	if ((weapon.firetype == "Single Shot" || weapon.firetype == "Burst" || weapon.firetype == "Charge Shot") && self AttackButtonPressed())
		return;
	self BotPressButton(0);
	if (weapon.isdualwield)
		self BotPressButton(24);
}

function melee_attack()
{
	if (GetTime() < self.bot.melee_time + 1500 || self zm_utility::has_powerup_weapon())
		return false;
	if (self.bot.threat.dot < 0.965 || DistanceSquared(self.origin, self.bot.threat.last_position) > level.zm_bot_melee_range * level.zm_bot_melee_range)
		return false;
	weapon = self GetCurrentWeapon();
	if (self IsReloading() && weapon.reloadtime > 3)
		return false;
	melee = self zm_utility::get_player_melee_weapon();
	nearby = self get_threats(level.zm_bot_melee_range * 1.5).size;
	entity = self.bot.threat.entity;
	strong_melee = isdefined(melee) && melee.meleedamage >= entity.health && nearby <= 3 * (self.health / self.maxhealth);
	if (!(weapon.weapclass == "melee" || weapon.weapclass == "item" || entity.targetname === "avogadro" || strong_melee || self laststand::player_is_in_laststand() || !self GetWeaponAmmoClip(weapon)))
		return false;
	self BotPressButton(2);
	self.bot.melee_time = GetTime();
	return true;
}

function aim_offset(entity)
{
	weapon = self GetCurrentWeapon();
	half_range = weapon_range(weapon) / 2;
	if (entity.animname === "quad_zombie")
		return entity GetTagOrigin("tag_origin") + AnglesToUp(entity GetTagAngles("tag_origin")) * 12 - entity.origin;
	if (entity.animname === "monkey_zombie")
		return entity GetCentroid() - entity.origin;
	if (entity.targetname === "zombie")
	{
		if (Distance(self.origin, entity.origin) <= half_range && weapon.isbulletweapon)
			return entity GetTagOrigin("J_Head") - entity.origin;
		return entity GetTagOrigin("J_SpineUpper") - entity.origin;
	}
	if (entity.targetname === "zombie_dog")
		return entity GetCentroid() + AnglesToUp(entity GetAngles()) * -12 - entity.origin;
	if (entity.targetname === "zombie_wasp")
		return entity GetEye() + AnglesToUp(entity GetAngles()) * -6 - entity.origin;
	if (IsSentient(entity) && Distance(self.origin, entity.origin) <= half_range && weapon.isbulletweapon)
		return entity GetEye() - entity.origin;
	return entity GetCentroid() - entity.origin;
}

function update_weapon_aim()
{
	if (!isdefined(self.bot.threat.aim_start_time))
		self start_threat_aim();
	aim_time = GetTime() - self.bot.threat.aim_start_time;
	if (aim_time < 0)
		return;
	if (aim_time >= self.bot.threat.aim_duration || !isdefined(self.bot.threat.aim_error))
	{
		self BotLookAtPoint(self.bot.threat.aim_point);
		return;
	}
	target = VectorToAngles(self.bot.threat.aim_point - self GetEye());
	angles = VectorLerp(target + self.bot.threat.aim_error, target, aim_time / self.bot.threat.aim_duration);
	self BotSetLookAngles(AnglesToForward(angles));
}

function start_threat_aim()
{
	switch (difficulty())
	{
		case 0:
			self.bot.threat.aim_start_time = GetTime() + 300;
			self.bot.threat.aim_duration = 300;
			self.bot.threat.aim_error = (angle_error(4, 8), angle_error(8, 14), 0);
			break;
		case 2:
			self.bot.threat.aim_start_time = GetTime() + 75;
			self.bot.threat.aim_duration = 75;
			self.bot.threat.aim_error = (angle_error(1, 2.5), angle_error(2, 5), 0);
			break;
		case 3:
			self.bot.threat.aim_start_time = GetTime() + 40;
			self.bot.threat.aim_duration = 40;
			self.bot.threat.aim_error = (angle_error(0.5, 1), angle_error(0.5, 2), 0);
			break;
		default:
			self.bot.threat.aim_start_time = GetTime() + 100;
			self.bot.threat.aim_duration = 100;
			self.bot.threat.aim_error = (angle_error(2, 4), angle_error(4, 8), 0);
			break;
	}
}

function difficulty()
{
	if (GetDvarString("zm_bot_difficulty") == "")
		return 1;
	return GetDvarInt("zm_bot_difficulty");
}

function fire_dot()
{
	switch (difficulty())
	{
		case 0:
			return 0.95;
		case 2:
			return 0.97;
		case 3:
			return 0.98;
	}
	return 0.965;
}

function angle_error(angle_min, angle_max)
{
	angle = (angle_max - angle_min) * RandomFloatRange(-1, 1);
	if (angle < 0)
		return angle - angle_min;
	return angle + angle_min;
}

function clear_threat_aim()
{
	self.bot.threat.aim_start_time = undefined;
	self.bot.threat.aim_duration = undefined;
	self.bot.threat.aim_error = undefined;
}

function update_weapon_ads()
{
	if (!IS_TRUE(self.bot.threat.inrange) || IS_TRUE(self.bot.threat.in_close_range))
		return;
	weapon = self GetCurrentWeapon();
	if (weapon == level.weaponnone || weapon.isdualwield || weapon.weapclass == "melee" || weapon.weapclass == "item" || self zm_utility::has_powerup_weapon() || self GetWeaponAmmoClip(weapon) <= 0)
		return;
	if (weapon.isrocketlauncher)
		ads_dot = 0.766;
	else
		ads_dot = 0.94;
	if (self.bot.threat.dot < ads_dot)
		return;
	self BotPressButton(11);
}

function weapon_range(weapon)
{
	if (weapon.issniperweapon)
		return 1500;
	if (weapon.isrocketlauncher)
		return 1000;
	if (weapon.weapclass == "spread" && weapon.shotcount == 1)
		return 800;
	switch (weapon.weapclass)
	{
		case "mg":
		case "rifle":
			return 1000;
		case "smg":
			return 800;
		case "spread":
			return 400;
	}
	return 700;
}

function switch_weapon()
{
	current = self GetCurrentWeapon();
	if (self IsSwitchingWeapons() || self IsMeleeing() || self IsThrowingGrenade() || self zm_utility::is_player_revive_tool(current) || self zm_utility::is_hero_weapon(current) || self zm_utility::has_powerup_weapon())
		return;
	weapons = self GetWeaponsListPrimaries();
	if (current == level.weaponnone)
	{
		if (weapons.size > 0)
			self SwitchToWeapon(weapons[0]);
		else
			self zm_melee_weapon::give_fallback_weapon(1);
		return;
	}
	stock = self GetWeaponAmmoStock(current);
	if (weapons.size == 1 && !stock)
	{
		self GiveMaxAmmo(current);
		return;
	}
	if (stock || self GetWeaponAmmoClip(current))
		return;
	next = undefined;
	for (i = 0; i < weapons.size; i++)
	{
		if (weapons[i] == current)
		{
			next = weapons[(i + 1) % weapons.size];
			break;
		}
	}
	if (isdefined(next))
		self SwitchToWeapon(next);
	self GiveMaxAmmo(current);
	self SetWeaponAmmoClip(current, current.clipsize);
}

function reload_weapon()
{
	if (GetAITeamArray(level.zombie_team).size > 0 || self IsReloading() || self IsSwitchingWeapons() || self IsThrowingGrenade() || self IsMeleeing())
		return;
	weapon = self GetCurrentWeapon();
	if (weapon.weapclass == "melee" || weapon.weapclass == "item" || !self GetWeaponAmmoStock(weapon))
		return;
	if (self clip_frac(weapon) < 0.6)
		self BotTapButton(4);
}

function clip_frac(weapon)
{
	if (weapon.clipsize <= 0)
		return 1;
	return self GetWeaponAmmoClip(weapon) / weapon.clipsize;
}

function throw_grenade(target, weapon)
{
	if (isdefined(weapon))
	{
		if (zm_utility::is_lethal_grenade(weapon))
			self BotPressButton(14);
		else if (zm_utility::is_tactical_grenade(weapon))
			self BotPressButton(15);
		return;
	}
	self BotSetLookAnglesFromPoint(target + (0, 0, Distance(self.origin, target) / 7));
}

function combat_strafe()
{
	fwd = AnglesToForward(self.angles);
	query = PositionQuery_Source_Navigation(self.origin, 0, 400, 64, 32, self);
	best = undefined;
	foreach (point in query.data)
	{
		if (!self can_reach(point.origin) || !BulletTracePassed(self.origin + (0, 0, 32), point.origin + (0, 0, 32), true, self))
			continue;
		dot = VectorDot(VectorNormalize(point.origin - self.origin), fwd);
		if (dot <= 0.5)
		{
			if (Distance2D(self.origin, point.origin) >= 200)
			{
				if (Closer(point.origin, self.origin, self.bot.threat.last_position))
					point.score = MapFloat(200, 400, 100, 200, point.disttoorigin2d) + RandomFloatRange(0, 50);
				else
					point.score = MapFloat(200, 400, 50, 100, point.disttoorigin2d) + RandomFloatRange(0, 25);
			}
			else
				point.score = MapFloat(200, 400, 25, 50, point.disttoorigin2d) + RandomFloatRange(0, 10);
		}
		else
			point.score = MapFloat(200, 400, 0, 25, point.disttoorigin2d) + RandomFloatRange(0, 5);
		if (!isdefined(best) || point.score > best.score)
			best = point;
	}
	if (isdefined(best))
	{
		self BotSetGoal(best.origin, 32);
		self end_sprint_to_goal();
	}
}
