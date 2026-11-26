#using scripts\codescripts\struct;
#using scripts\shared\array_shared;
#using scripts\shared\callbacks_shared;
#using scripts\shared\flag_shared;
#using scripts\shared\laststand_shared;
#using scripts\shared\system_shared;
#using scripts\zm\_zm_magicbox;
#using scripts\zm\_zm_powerups;
#using scripts\zm\_zm_score;
#using scripts\zm\_zm_utility;
#using scripts\zm\_zm_weapons;

#insert scripts\shared\shared.gsh;

#namespace zm_bots;

#define MODE_FOLLOW 0
#define MODE_WANDER 1
#define MODE_CAMP 2

#define BUTTON_SPRINT 1
#define BUTTON_USE 3

#define THREAT_NEAR 200
#define THREAT_FAR 400
#define FOLLOW_RANGE 300

REGISTER_SYSTEM("zm_bots", &__init__, undefined)

function __init__()
{
	callback::on_spawned(&on_spawned);
	level thread add_bots();
	level thread track_doors();
}

function add_bots()
{
	level flag::wait_till("initial_blackscreen_passed");
	count = min(GetDvarInt("zm_bots"), GetDvarInt("com_maxclients") - GetPlayers().size);
	for (i = 0; i < count; i++)
	{
		if (!isdefined(AddTestClient()))
			break;
		wait 0.5;
	}
}

function on_spawned()
{
	if (!self IsTestClient())
		return;
	self.bot_mode = GetDvarInt("zm_bot_mode");
	self.bot_camp_origin = self.origin;
	self.bot_positions = [];
	self.bot_starting_weapon = self GetCurrentWeapon();
	self BotSetGoal(self.origin);
	self thread think();
	self thread watch_path_failed();
}

function think()
{
	self endon("disconnect");
	self endon("death");
	self endon("bled_out");
	level endon("end_game");

	for (;;)
	{
		wait 0.5;
		self BotReleaseButton(BUTTON_SPRINT);
		if (!IS_TRUE(self.bot_reviving))
			self BotReleaseButton(BUTTON_USE);
		self.bot_reviving = false;

		if (self laststand::player_is_in_laststand() || self BotUnderManualControl())
			continue;

		self switch_from_empty_weapon();
		if (self check_stuck())
			continue;

		if (self revive()
			|| self pick_up_powerup()
			|| self pack_a_punch()
			|| self use_magic_box()
			|| self buy_wall_weapon()
			|| self buy_perk()
			|| self open_door()
			|| self repair_barrier())
			continue;

		switch (self.bot_mode)
		{
			case MODE_FOLLOW:
				self follow();
				break;
			case MODE_WANDER:
				self wander();
				break;
			case MODE_CAMP:
				self go_to(self.bot_camp_origin, 72);
				break;
		}
	}
}

function threat_near(radius)
{
	zombie = ArrayGetClosest(self.origin, GetAITeamArray(level.zombie_team));
	return isdefined(zombie) && Distance(self.origin, zombie.origin) < radius;
}

function can_reach(point)
{
	return isdefined(point) && self CanPath(self.origin, point) && zm_utility::check_point_in_enabled_zone(point);
}

function in_range(point)
{
	if (self.bot_mode == MODE_CAMP)
		return Distance(point, self.bot_camp_origin) < THREAT_NEAR;
	if (self.bot_mode == MODE_FOLLOW)
	{
		leader = self leader();
		return !isdefined(leader) || Distance(point, leader.origin) < FOLLOW_RANGE;
	}
	return true;
}

function go_to(point, radius)
{
	if (!isdefined(radius))
		radius = 32;
	if (Distance(self.origin, point) <= radius)
		return true;
	self BotSetGoal(point, radius);
	if (!self threat_near(THREAT_FAR))
		self BotPressButton(BUTTON_SPRINT);
	return false;
}

function use_at(point, trigger, look_at)
{
	if (!self go_to(point, 48))
		return;
	self BotSetGoal(self.origin);
	self BotLookAtPoint(look_at);
	if (!isdefined(self.bot_next_use) || GetTime() > self.bot_next_use)
	{
		trigger notify("trigger", self);
		self.bot_next_use = GetTime() + 1000;
	}
}

function navmesh_point(origin)
{
	return GetClosestPointOnNavMesh(origin, 128, 32);
}

function leader()
{
	players = ArraySortClosest(GetPlayers(), self.origin);
	foreach (player in players)
	{
		if (!player IsTestClient() && player.sessionstate == "playing")
			return player;
	}
	return undefined;
}

function follow()
{
	leader = self leader();
	if (!isdefined(leader))
	{
		self wander();
		return;
	}
	if (Distance(self.origin, leader.origin) < 200)
		return;
	point = leader.origin - AnglesToForward(leader.angles) * 64;
	if (!self can_reach(point))
		point = leader.origin;
	self go_to(point, 64);
}

function wander()
{
	if (!self BotGoalReached() || self threat_near(THREAT_NEAR))
		return;
	if (isdefined(self.bot_next_wander) && GetTime() < self.bot_next_wander)
		return;
	self.bot_next_wander = GetTime() + RandomIntRange(2000, 5000);
	query = PositionQuery_Source_Navigation(self.origin, 128, 1500, 150, 64, self);
	foreach (point in array::randomize(query.data))
	{
		if (self can_reach(point.origin))
		{
			self BotSetGoal(point.origin, 64);
			return;
		}
	}
}

function revive()
{
	foreach (player in ArraySortClosest(GetPlayers(), self.origin))
	{
		if (player == self || !player laststand::player_is_in_laststand())
			continue;
		point = navmesh_point(player.origin);
		if (!self can_reach(point))
			continue;
		if (isdefined(player.revivetrigger) && IS_TRUE(player.revivetrigger.beingrevived) && !IS_TRUE(self.is_reviving_any))
			continue;
		if (Distance2D(self.origin, player.origin) > 48)
		{
			self go_to(point, 24);
			return true;
		}
		self BotSetGoal(self.origin);
		self BotLookAtPoint(player GetCentroid());
		self BotPressButton(BUTTON_USE);
		self.bot_reviving = true;
		return true;
	}
	return false;
}

function pick_up_powerup()
{
	if (self threat_near(THREAT_NEAR))
		return false;
	foreach (powerup in ArraySortClosest(zm_powerups::get_powerups(), self.origin))
	{
		point = powerup.origin - (0, 0, 40);
		if (self in_range(point) && self can_reach(point))
		{
			self go_to(point, 16);
			return true;
		}
	}
	return false;
}

function weapon_score(weapon)
{
	if (!isdefined(weapon) || weapon == level.weaponnone)
		return 0;
	if (IsSubStr(weapon.name, "ray_gun") || IsSubStr(weapon.name, "raygun_mark2"))
		return 9999;
	score = 0;
	if (zm_weapons::is_weapon_upgraded(weapon))
		score += 200;
	if (zm_weapons::is_wonder_weapon(weapon))
		score += 300;
	switch (weapon.weapclass)
	{
		case "spread":
			score += 100 + max(200, weapon.shotcount * 45);
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
	if (weapon.firetype == "Full Auto")
		score += 200;
	if (weapon.isdualwield)
		score *= 1.5;
	return score + floor(weapon.clipsize / max(weapon.reloadtime, 0.1)) * 10 + weapon.maxammo;
}

function weakest_weapon()
{
	weapons = self GetWeaponsListPrimaries();
	if (weapons.size < zm_utility::get_player_weapon_limit(self))
		return level.weaponnone;
	weakest = undefined;
	foreach (weapon in weapons)
	{
		if (!isdefined(weakest) || weapon_score(weapon) < weapon_score(weakest))
			weakest = weapon;
	}
	return weakest;
}

function wants_weapon(weapon)
{
	if (self zm_weapons::has_weapon_or_upgrade(weapon) || zm_utility::is_offhand_weapon(weapon))
		return false;
	weakest = self weakest_weapon();
	return weakest == level.weaponnone || weapon_score(weapon) > weapon_score(weakest) + 100;
}

function hold_weakest_weapon()
{
	weakest = self weakest_weapon();
	if (weakest == level.weaponnone || self GetCurrentWeapon() == weakest)
		return true;
	self SwitchToWeapon(weakest);
	return false;
}

function buy_wall_weapon()
{
	if (self.bot_mode == MODE_CAMP || self threat_near(THREAT_FAR) || !isdefined(level._spawned_wallbuys))
		return false;
	foreach (wallbuy in ArraySortClosest(level._spawned_wallbuys, self.origin))
	{
		weapon = wallbuy.weapon;
		if (!isdefined(level.zombie_weapons[weapon]) || !self zm_score::can_player_purchase(level.zombie_weapons[weapon].cost) || !self wants_weapon(weapon))
			continue;
		stub = wallbuy.trigger_stub;
		point = navmesh_point(stub.origin);
		if (!self in_range(point) || !self can_reach(point))
			continue;
		if (self go_to(point, 48) && self hold_weakest_weapon() && isdefined(stub.playertrigger[self GetEntityNumber()]))
			self use_at(point, stub.playertrigger[self GetEntityNumber()], stub.origin);
		return true;
	}
	return false;
}

function use_magic_box()
{
	if (self.bot_mode == MODE_CAMP || !isdefined(level.chests))
		return false;
	foreach (chest in level.chests)
	{
		if (IS_TRUE(chest.hidden) || IS_TRUE(chest.zbarrier.chest_moving))
			continue;
		point = navmesh_point(chest.zbarrier GetCentroid());
		look_at = chest.zbarrier.origin + (0, 0, 40);
		if (isdefined(chest.chest_user) && chest.chest_user == self)
		{
			if (chest.zbarrier.weapon != level.weaponnone && self wants_weapon(chest.zbarrier.weapon) && self hold_weakest_weapon())
				self use_at(point, chest, look_at);
			return true;
		}
		if (IS_TRUE(chest._box_open) || self threat_near(THREAT_FAR) || !self in_range(point) || !self can_reach(point))
			continue;
		if (!self zm_score::can_player_purchase(chest.zombie_cost + 1000) || self weakest_weapon() != level.weaponnone && weapon_score(self weakest_weapon()) >= 900)
			continue;
		self use_at(point, chest, look_at);
		return true;
	}
	return false;
}

function pack_a_punch()
{
	if (self.bot_mode == MODE_CAMP || !isdefined(level.pack_a_punch) || level.round_number < 5)
		return false;
	foreach (trigger in level.pack_a_punch.triggers)
	{
		if (!IS_TRUE(trigger.powered.power) || trigger.pap_machine.state === "leaving")
			continue;
		point = navmesh_point(trigger.zbarrier GetCentroid());
		if (isdefined(trigger.pack_player) && trigger.pack_player == self)
		{
			if (trigger IsTriggerEnabled())
				self use_at(point, trigger, trigger.origin);
			else
				self go_to(point, 72);
			return true;
		}
		if (isdefined(trigger.pack_player) || self threat_near(THREAT_FAR) || !self in_range(point) || !self can_reach(point))
			continue;
		foreach (weapon in self GetWeaponsListPrimaries())
		{
			if (zm_weapons::is_weapon_upgraded(weapon) || !zm_weapons::can_upgrade_weapon(weapon) || !self zm_score::can_player_purchase(trigger.cost))
				continue;
			if (self GetCurrentWeapon() == weapon)
				self use_at(point, trigger, trigger.origin);
			else
				self SwitchToWeapon(weapon);
			return true;
		}
	}
	return false;
}

function buy_perk()
{
	if (self.bot_mode == MODE_CAMP || self threat_near(THREAT_FAR) || !self zm_utility::can_player_purchase_perk())
		return false;
	machines = GetEntArray("zombie_vending", "targetname");
	foreach (perk in array("specialty_armorvest", "specialty_quickrevive", "specialty_fastreload", "specialty_doubletap2", "specialty_staminup"))
	{
		foreach (trigger in machines)
		{
			if (trigger.script_noteworthy != perk || self HasPerk(perk) || !IS_TRUE(trigger.power_on) || !self zm_score::can_player_purchase(trigger.cost))
				continue;
			point = navmesh_point(trigger.machine GetCentroid());
			if (!self in_range(point) || !self can_reach(point))
				continue;
			self use_at(point, trigger, trigger.origin);
			return true;
		}
	}
	return false;
}

function track_doors()
{
	level flag::wait_till("initial_blackscreen_passed");
	level.bot_doors = [];
	doors = ArrayCombine(GetEntArray("zombie_door", "targetname"), GetEntArray("zombie_debris", "targetname"), false, false);
	doors = ArrayCombine(doors, GetEntArray("zombie_airlock_buy", "targetname"), false, false);
	foreach (door in doors)
	{
		if (door.script_noteworthy === "electric_door" || door.script_noteworthy === "local_electric_door")
			continue;
		foreach (trigger in GetEntArray(door.target, "target"))
		{
			trigger.bot_door = door;
			level.bot_doors[level.bot_doors.size] = trigger;
		}
	}
}

function open_door()
{
	if (self.bot_mode == MODE_CAMP || self threat_near(THREAT_FAR) || !isdefined(level.bot_doors))
		return false;
	foreach (trigger in array::get_all_closest(self.origin, level.bot_doors, undefined, 3))
	{
		door = trigger.bot_door;
		if (!isdefined(door) || !isdefined(door.zombie_cost) || IS_TRUE(door._door_open) || !(door IsTriggerEnabled()))
			continue;
		if (!self zm_score::can_player_purchase(door.zombie_cost + 500))
			continue;
		point = navmesh_point(trigger.origin);
		if (!self in_range(point) || !self can_reach(point))
			continue;
		self BotPressButton(BUTTON_USE);
		self use_at(point, door, trigger.origin);
		return true;
	}
	return false;
}

function repair_barrier()
{
	if (self threat_near(THREAT_NEAR) || !isdefined(level.exterior_goals))
		return false;
	foreach (window in array::get_all_closest(self.origin, level.exterior_goals, undefined, 3, 300))
	{
		if (!isdefined(window.barrier_chunks) || zm_utility::all_chunks_intact(window, window.barrier_chunks) || zm_utility::no_valid_repairable_boards(window, window.barrier_chunks))
			continue;
		point = navmesh_point(window.zbarrier.origin + AnglesToForward(window.zbarrier.angles) * 56);
		if (!self in_range(point) || !self can_reach(point))
			continue;
		self BotPressButton(BUTTON_USE);
		self use_at(point, window, window.zbarrier.origin);
		return true;
	}
	return false;
}

function switch_from_empty_weapon()
{
	weapon = self GetCurrentWeapon();
	if (self IsSwitchingWeapons() || self GetWeaponAmmoClip(weapon) + self GetWeaponAmmoStock(weapon) > 0)
		return;
	foreach (other in self GetWeaponsListPrimaries())
	{
		if (self GetWeaponAmmoClip(other) + self GetWeaponAmmoStock(other) > 0)
		{
			self SwitchToWeapon(other);
			return;
		}
	}
}

function check_stuck()
{
	if (self BotGoalReached() || self threat_near(64))
	{
		self.bot_positions = [];
		return false;
	}
	self.bot_positions[self.bot_positions.size] = self.origin;
	if (self.bot_positions.size < 6)
		return false;
	stuck = Distance(self.bot_positions[0], self.origin) < 32;
	ArrayRemoveIndex(self.bot_positions, 0);
	if (stuck)
		self thread get_unstuck();
	return stuck;
}

function watch_path_failed()
{
	self endon("disconnect");
	for (;;)
	{
		self waittill("bot_path_failed");
		self thread get_unstuck();
	}
}

function get_unstuck()
{
	self endon("disconnect");
	self endon("death");
	if (self BotUnderManualControl())
		return;
	self.bot_positions = [];
	self BotTakeManualControl();
	self BotSetMoveAngle(AnglesToForward((0, self.angles[1] + 180 + RandomIntRange(-45, 45), 0)));
	self BotSetMoveMagnitude(1);
	wait 1;
	self BotReleaseManualControl();
	self BotSetGoal(self.origin);
}
