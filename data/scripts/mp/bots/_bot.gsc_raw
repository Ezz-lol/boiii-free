#using scripts\codescripts\struct;

#using scripts\shared\array_shared;
#using scripts\shared\callbacks_shared;
#using scripts\shared\killstreaks_shared;
#using scripts\shared\math_shared;
#using scripts\shared\rank_shared;
#using scripts\shared\system_shared;
#using scripts\shared\util_shared;
#using scripts\shared\weapons_shared;

#using scripts\shared\weapons\_weapons;

#using scripts\shared\bots\_bot;
#using scripts\shared\bots\_bot_combat;
#using scripts\shared\bots\bot_buttons;
#using scripts\shared\bots\bot_traversals;

#using scripts\mp\bots\_bot_ball;
#using scripts\mp\bots\_bot_clean;
#using scripts\mp\bots\_bot_combat;
#using scripts\mp\bots\_bot_conf;
#using scripts\mp\bots\_bot_ctf;
#using scripts\mp\bots\_bot_dem;
#using scripts\mp\bots\_bot_dom;
#using scripts\mp\bots\_bot_escort;
#using scripts\mp\bots\_bot_hq;
#using scripts\mp\bots\_bot_koth;
#using scripts\mp\bots\_bot_loadout;
#using scripts\mp\bots\_bot_sd;

#using scripts\mp\killstreaks\_ai_tank;
#using scripts\mp\killstreaks\_airsupport;
#using scripts\mp\killstreaks\_combat_robot;
#using scripts\mp\killstreaks\_counteruav;
#using scripts\mp\killstreaks\_dart;
#using scripts\mp\killstreaks\_dogs;
#using scripts\mp\killstreaks\_drone_strike;
#using scripts\mp\killstreaks\_emp;
#using scripts\mp\killstreaks\_flak_drone;
#using scripts\mp\killstreaks\_helicopter;
#using scripts\mp\killstreaks\_helicopter_gunner;
#using scripts\mp\killstreaks\_killstreak_bundles;
#using scripts\mp\killstreaks\_killstreak_detect;
#using scripts\mp\killstreaks\_killstreak_hacking;
#using scripts\mp\killstreaks\_killstreakrules;
#using scripts\mp\killstreaks\_killstreaks;
#using scripts\mp\killstreaks\_microwave_turret;
#using scripts\mp\killstreaks\_planemortar;
#using scripts\mp\killstreaks\_qrdrone;
#using scripts\mp\killstreaks\_raps;
#using scripts\mp\killstreaks\_rcbomb;
#using scripts\mp\killstreaks\_remote_weapons;
#using scripts\mp\killstreaks\_remotemissile;
#using scripts\mp\killstreaks\_satellite;
#using scripts\mp\killstreaks\_sentinel;
#using scripts\mp\killstreaks\_supplydrop;
#using scripts\mp\killstreaks\_turret;
#using scripts\mp\killstreaks\_uav;

#using scripts\mp\teams\_teams;
#using scripts\mp\_util;

#insert scripts\shared\shared.gsh;
#insert scripts\mp\bots\_bot.gsh;

#define MAX_LOCAL_PLAYERS	10
#define MAX_ONLINE_PLAYERS	18
#define MAX_ONLINE_PLAYERS_PER_TEAM	6

#define RESPAWN_DELAY	0.1
#define RESPAWN_INTERVAL 0.1

#namespace bot;

#precache("eventstring", "mpl_killstreak_cruisemissile");
#precache("eventstring", "mpl_killstreak_raps");

REGISTER_SYSTEM("bot_mp", &__init__, undefined)

function __init__()
{
	callback::on_start_gametype(&init);

	level.getBotSettings = &get_bot_settings;

	level.onBotConnect = &on_bot_connect;
	level.onBotSpawned = &on_bot_spawned;
	level.onBotKilled = &on_bot_killed;

	level.botIdle = &bot_idle;

	level.botThreatLost = &bot_combat::chase_threat;

	level.botPreCombat = &bot_combat::mp_pre_combat;
	level.botCombat = &bot_combat::combat_think;
	level.botPostCombat = &bot_combat::mp_post_combat;

	level.botIgnoreThreat = &bot_combat::bot_ignore_threat;

	level.enemyEmpActive = &emp::enemyEmpActive;

/*
/#
	level.botDevguiCmd = &bot_devgui_cmd;
	level thread system_devgui_gadget_think();
#/
*/
	setDvar("bot_enableWallrun", 1);
}

function init()
{
	level endon("game_ended");

	level.botSoak = is_bot_soak();
	if (!init_bot_gametype())
	{
		return;
	}

	wait_for_host();

	level thread populate_bots();
}

// Init Utils
//========================================

function is_bot_soak()
{
	return getDvarInt("sv_botsoak", 0);
}

function wait_for_host()
{
	level endon("game_ended");

	host = util::getHostPlayerForBots();

	while (!isdefined(host))
	{
		wait(0.25);
		host = util::getHostPlayerForBots();
	}
}

function get_host_team()
{
	host = util::getHostPlayerForBots();

	if (!isdefined(host) || host.team == "spectator")
	{
		return "allies";
	}

	return host.team;
}

function is_bot_comp_stomp()
{
	return false;
}

// Bot Events
//========================================

function on_bot_connect()
{
	self endon("disconnect");
	level endon("game_ended");

	if (IS_TRUE(level.disableClassSelection))
	{
		self set_rank();

		// Doesn't work if we don't do it in this order
		self bot_loadout::pick_hero_gadget();
		self bot_loadout::pick_killstreaks();

		return;
	}

	if (!IS_TRUE(self.pers["bot_loadout"]))
	{
		self set_rank();

		// Doesn't work if we don't do it in this order
		self bot_loadout::build_classes();
		self bot_loadout::pick_hero_gadget();
		self bot_loadout::pick_killstreaks();

		self.pers["bot_loadout"] = true;
	}

	self bot_loadout::pick_classes();
	self choose_class();
}

function on_bot_spawned()
{
	self.bot.goalTag = undefined;
/*
/#
	weapon = undefined;

	if (getDvarInt("scr_botsHasPlayerWeapon") != 0)
	{
		player = util::getHostPlayer();
		weapon = player getCurrentWeapon();
	}

	if (getDvarString("devgui_bot_weapon", "") != "")
	{
		weapon = getWeapon(getDvarString("devgui_bot_weapon"));
	}

	if (isdefined(weapon) && level.weaponNone != weapon)
	{
		self weapons::detach_all_weapons();
		self takeAllWeapons();
		self giveWeapon(weapon);
		self switchToWeapon(weapon);
		self setSpawnWeapon(weapon);

		self teams::set_player_model(self.team, weapon);
	}
#/
*/
}

function on_bot_killed()
{
	self endon("disconnect");
	level endon("game_ended");
	self endon("spawned");
	self waittill("death_delay_finished");

	wait RESPAWN_DELAY;

	if (self choose_class() && level.playerForceRespawn)
	{
		return;
	}

	self thread respawn();
}

function respawn()
{
	self endon("spawned");
	self endon("disconnect");
	level endon("game_ended");

	while (1)
	{
		self bot::tap_use_button();

		wait RESPAWN_INTERVAL;
	}
}

function bot_idle()
{
	if (self do_supplydrop())
	{
		return;
	}

	// TODO: Look for an enemy radar blip
	// TODO: Get points on navmesh and feed into the spawn system to see if an enemy is likely to spawn there
	self bot::navmesh_wander();
	self bot::sprint_to_goal();
}

// Crate maxs: 23.1482
#define CRATE_GOAL_RADIUS 39
#define CRATE_USE_RADIUS 62	// Wild guess on usable radius

function do_supplydrop(maxRange = 1400) // A little under minimap width
{
	crates = getEntArray("care_package", "script_noteworthy");

	maxRangeSq = maxRange * maxRange;

	useRadiusSq = CRATE_USE_RADIUS * CRATE_USE_RADIUS;

	closestCrate = undefined;
	closestCrateDistSq = undefined;

	foreach(crate in crates)
	{
		if (!crate isOnGround())
		{
			continue;
		}

		crateDistSq = distance2DSquared(self.origin, crate.origin);

		if (crateDistSq > maxRangeSq)
		{
			continue;
		}

		inUse = isdefined(crate.useEnt) && IS_TRUE(crate.useEnt.inUse);

		if (crateDistSq <= useRadiusSq)
		{
			if (inUse && !self useButtonPressed())
			{
				continue;
			}

			self bot::press_use_button();
			return true;
		}

		if (!self has_minimap() && !self botSightTracePassed(crate))
		{
			continue;
		}

		if (!isdefined(closestCrate) || crateDistSq < closestCrateDistSq)
		{
			closestCrate = crate;
			closestCrateDistSq = crateDistSq;
		}
	}

	if (isdefined(closestCrate))
	{
		randomAngle = (0, randomInt(360), 0);
		randomVec = AnglesToForward(randomAngle);

		point = closestCrate.origin + randomVec * CRATE_GOAL_RADIUS;

		if (self botSetGoal(point))
		{
			self thread watch_crate(closestCrate);
			return true;
		}
	}

	return false;
}

function watch_crate(crate)
{
	self endon("death");
	self endon("bot_goal_reached");
	level endon("game_ended");

	while (isdefined(crate) && !self bot_combat::has_threat())
	{
		wait level.botSettings.thinkInterval;
	}

	self botSetGoal(self.origin);
}

// Bot Team Population
//========================================

function populate_bots()
{
	level endon("game_ended");

	if (level.teambased)
	{
		maxAllies = getDvarInt("bot_maxAllies", 0);
		maxAxis = getDvarInt("bot_maxAxis", 0);

		level thread monitor_bot_team_population(maxAllies, maxAxis);
	}
	else
	{
		maxFree = getDvarInt("bot_maxFree", 0);

		level thread monitor_bot_population(maxFree);
	}
}

function monitor_bot_team_population(maxAllies, maxAxis)
{
	level endon("game_ended");

	if (!maxAllies && !maxAxis)
	{
		return;
	}

	fill_balanced_teams(maxAllies, maxAxis);

	while (1)
	{
		wait 3;

		// TODO: Get a player count that includes 'CON_CONNECTING' players
		allies = getPlayers("allies");
		axis = getPlayers("axis");

		if (allies.size > maxAllies &&
			remove_best_bot(allies))
		{
			continue;
		}

		if (axis.size > maxAxis &&
			remove_best_bot(axis))
		{
			continue;
		}

		if (allies.size < maxAllies || axis.size < maxAxis)
		{
			add_balanced_bot(allies, maxAllies, axis, maxAxis);
		}
	}
}

function fill_balanced_teams(maxAllies, maxAxis)
{
	allies = getPlayers("allies");
	axis = getPlayers("axis");

	while ((allies.size < maxAllies || axis.size < maxAxis) &&
		add_balanced_bot(allies, maxAllies, axis, maxAxis))
	{
		WAIT_SERVER_FRAME;

		allies = getPlayers("allies");
		axis = getPlayers("axis");
	}
}

function add_balanced_bot(allies, maxAllies, axis, maxAxis)
{
	bot = undefined;

	if (allies.size < maxAllies &&
		(allies.size <= axis.size || axis.size >= maxAxis))
	{
		bot = add_bot("allies");
	}
	else if (axis.size < maxAxis)
	{
		bot = add_bot("axis");
	}

	return isdefined(bot);
}

function monitor_bot_population(maxFree)
{
	level endon("game_ended");

	if (!maxFree)
	{
		return;
	}

	// Initial Fill
	players = getPlayers();
	while (players.size < maxFree)
	{
		add_bot();
		WAIT_SERVER_FRAME;
		players = getPlayers();
	}

	while (1)
	{
		wait 3;

		// TODO: Get a player count that includes 'CON_CONNECTING' players
		players = getPlayers();

		if (players.size < maxFree)
		{
			add_bot();
		}
		else if (players.size > maxFree)
		{
			remove_best_bot(players);
		}
	}
}

function remove_best_bot(players)
{
	bots = filter_bots(players);

	if (!bots.size)
	{
		return false;
	}

	// Prefer non-combat bots
	bestBots = [];

	foreach(bot in bots)
	{
		// Don't kick bots in the process of connecting
		if (bot.sessionstate == "spectator")
		{
			continue;
		}

		if (bot.sessionstate == "dead" || !bot bot_combat::has_threat())
		{
			bestBots[bestBots.size] = bot;
		}
	}

	if (bestBots.size)
	{
		remove_bot(bestBots[randomInt(bestBots.size)]);
	}
	else
	{
		remove_bot(bots[randomInt(bots.size)]);
	}

	return true;
}

// Bot Loadouts
//========================================

function choose_class()
{
	if (IS_TRUE(level.disableClassSelection))
	{
		return false;
	}

	currClass = self bot_loadout::get_current_class();

	if (!isdefined(currClass) || randomInt(100) < VAL(level.botSettings.changeClassWeight, 0))
	{
		classIndex = randomInt(self.loadoutClasses.size);
		className = self.loadoutClasses[classIndex].name;
	}

	if (!isdefined(className) || className == currClass)
	{
		return false;
	}

	self notify("menuresponse", MENU_CHANGE_CLASS, className);

	return true;
}

// Killstreaks
//========================================

function use_killstreak()
{
	if (!level.loadoutKillstreaksEnabled ||
		self emp::enemyEmpActive())
	{
		return;
	}

	weapons = self getWeaponsList();
	inventoryWeapon = self getInventoryWeapon();

	foreach(weapon in weapons)
	{
		killstreak = killstreaks::get_killstreak_for_weapon(weapon);

		if (!isdefined(killstreak))
		{
			continue;
		}

		if (weapon != inventoryWeapon && !self getWeaponAmmoClip(weapon))
		{
			continue;
		}

		if (self killstreakrules::isKillstreakAllowed(killstreak, self.team))
		{
			useWeapon = weapon;
			break;
		}
	}

	if (!isdefined(useWeapon))
	{
		return;
	}

	killstreak_ref = killstreaks::get_menu_name(killstreak);

	switch (killstreak_ref)
	{
	case "killstreak_uav":
	case "killstreak_counteruav":
	case "killstreak_satellite":
	case "killstreak_helicopter_player_gunner":
	case "killstreak_raps":
	case "killstreak_sentinel":
	{
		self switchToWeapon(useWeapon);
		break;
	}
	case "killstreak_ai_tank_drop":
	{
		self use_supply_drop(weapon);
		break;
	}
	case "killstreak_remote_missile":
	{
		self switchToWeapon(weapon);
		self waittill("weapon_change_complete");
		wait 1.5;
		self bot::press_attack_button();
		return;
	}
	}
}

function get_closest_enemy(origin, on_radar)
{
	enemies = self get_enemies(on_radar);
	enemies = arraysort(enemies, origin);

	if (enemies.size)
		return enemies[0];

	return undefined;
}

function use_supply_drop(weapon)
{
	if (weapon == "inventory_supplydrop_mp" || weapon == "supplydrop_mp")
	{
		if (gettime() - self.spawntime > 5000)
			return;
	}

	yaw = (0, self.angles[1], 0);
	dir = anglestoforward(yaw);
	dir = vectornormalize(dir);
	drop_point = self.origin + vectorscale(dir, 384);
	end = drop_point + vectorscale((0, 0, 1), 2048.0);

	if (!sighttracepassed(drop_point, end, 0, undefined))
		return;

	if (!sighttracepassed(self.origin, end, 0, undefined))
		return;

	end = drop_point - vectorscale((0, 0, 1), 32.0);

	if (bullettracepassed(drop_point, end, 0, undefined))
		return;

	self addgoal(self.origin, 24, 4, "killstreak");

	if (weapon == "missile_drone_mp" || weapon == "inventory_missile_drone_mp")
		self lookat(drop_point + vectorscale((0, 0, 1), 384.0));
	else
		self lookat(drop_point);

	wait 0.5;

	if (self getCurrentWeapon() != weapon)
	{
		self thread weapon_switch_failsafe();
		self switchToWeapon(weapon);

		self waittill("weapon_change_complete");
	}

	use_item(weapon);
	self switchToWeapon(self.lastnonkillstreakweapon);
	self clearlookat();
	self cancelgoal("killstreak");
}

function use_item(weapon)
{
	self bot::press_attack_button();
	wait 0.5;

	for (i = 0; i < 10; i++)
	{
		if (self getCurrentWeapon() == weapon || self getCurrentWeapon() == "none")
			self bot::press_attack_button();
		else
			return;

		wait 0.5;
	}
}

function killstreak_location(num, weapon)
{
	enemies = get_enemies();

	if (!enemies.size)
		return;

	if (!self switchToWeapon(weapon))
		return;

	self waittill("weapon_change");

	self util::freeze_player_controls(true);
	wait_time = 1;

	while (!isdefined(self.selectinglocation) || self.selectinglocation == 0)
	{
		wait 0.05;
		wait_time -= 0.05;

		if (wait_time <= 0)
		{
			self util::freeze_player_controls(false);
			self switchToWeapon(self.lastnonkillstreakweapon);
			return;
		}
	}

	wait 2;

	for (i = 0; i < num; i++)
	{
		enemies = get_enemies();

		if (enemies.size)
		{
			enemy = randomInt(enemies);
			self notify("confirm_location", enemy.origin, 0);
		}

		wait 0.25;
	}

	self util::freeze_player_controls(false);
}

function weapon_switch_failsafe()
{
	self endon("death");
	self endon("disconnect");
	self endon("weapon_change_complete");
	wait 10;
	self notify("weapon_change_complete");
}

function has_radar()
{
	if (level.teambased)
	{
		return (uav::HasUAV(self.team) || satellite::HasSatellite(self.team));
	}

	return (uav::HasUAV(self.entnum) || satellite::HasSatellite(self.entnum));
}

function has_minimap()
{
	if (self IsEmpJammed())
	{
		return false;
	}

	if (IS_TRUE(level.hardcoreMode))
	{
		return self has_radar();
	}

	return true;
}

function get_enemies(on_radar)
{
	if (!isdefined(on_radar))
	{
		on_radar = false;
	}

	enemies = self GetEnemies();

/*
/#
	for (i = 0; i < enemies.size; i++)
	{
		if (isplayer(enemies[i]) && enemies[i] isInMoveMode("ufo", "noclip"))
		{
			arrayRemoveIndex(enemies, i);
			i--;
		}
	}
#/
*/

	if (on_radar && !self has_radar())
	{
		for (i = 0; i < enemies.size; i++)
		{
			if (!isdefined(enemies[i].lastFireTime))
			{
				arrayRemoveIndex(enemies, i);
				i--;
			}
			else if (GetTime() - enemies[i].lastFireTime > 2000)
			{
				arrayRemoveIndex(enemies, i);
				i--;
			}
		}
	}

	return enemies;
}

function set_rank()
{
	players = getPlayers();

	ranks = [];
	bot_ranks = [];
	human_ranks = [];

	for (i = 0; i < players.size; i++)
	{
		if (players[i] == self)
			continue;

		if (isdefined(players[i].pers["rank"]))
		{
			if (players[i] util::is_bot())
			{
				bot_ranks[bot_ranks.size] = players[i].pers["rank"];
			}
			else
			{
				human_ranks[human_ranks.size] = players[i].pers["rank"];
			}
		}
	}

	if (!human_ranks.size)
		human_ranks[human_ranks.size] = 10;

	human_avg = math::array_average(human_ranks);

	while (bot_ranks.size + human_ranks.size < 5)
	{
		// add some random ranks for better random number distribution
		r = human_avg + randomIntRange(-5, 5);
		rank = math::clamp(r, 0, level.maxRank);
		human_ranks[human_ranks.size] = rank;
	}

	ranks = arrayCombine(human_ranks, bot_ranks, true, false);

	avg = math::array_average(ranks);
	s = math::array_std_deviation(ranks, avg);

	rank = Int(math::random_normal_distribution(avg, s, 0, level.maxRank));

	while (!isdefined(self.pers["codpoints"]))
	{
		wait 0.1;
	}

	self.pers["rank"] = rank;
	self.pers["rankxp"] = rank::getRankInfoMinXP(rank);

	self setRank(rank);
	self rank::syncXPStat();
}

function init_bot_gametype()
{
	switch (level.gameType)
	{
	case "ball":
		bot_ball::init();
		return true;
	case "conf":
		bot_conf::init();
		return true;
	case "ctf":
		bot_ctf::init();
		return true;
	case "dem":
		bot_dem::init();
		return true;
	case "dm":
		return true;
	case "dom":
		bot_dom::init();
		return true;
	case "escort":
		bot_escort::init();
		return true;
//	case "infect":
//		return true;
	case "gun":
		return true;
	case "koth":
		bot_koth::init();
		return true;
	case "sd":
		bot_sd::init();
		return true;
	case "clean":
		bot_clean::init();
		return true;
	case "tdm":
		return true;
	case "sas":
		return true;
	case "prop":
		return true;
	case "sniperonly":
		return true;
	}

	return false;
}

function get_bot_settings()
{
	switch (getDvarInt("bot_difficulty", 1))
	{
	case 0:
		bundleName = "bot_mp_easy";
		break;

	case 1:
		bundleName = "bot_mp_normal";
		break;
	case 2:
		bundleName = "bot_mp_hard";
		break;
	case 3:
	default:
		bundleName = "bot_mp_veteran";
		break;
	}

	return struct::get_script_bundle("botsettings", bundleName);
}

function friend_goal_in_radius(goal_name, origin, radius)
{
	return 0;
}

function friend_in_radius(goal_name, origin, radius)
{
	return false;
}

function get_friends()
{
	return[];
}

function bot_vehicle_weapon_ammo(weaponName)
{
	return false;
}

function navmesh_points_visible(origin, point)
{
	return false;
}

function dive_to_prone(exit_stance)
{

}

/*
/#

// Devgui
//========================================

function bot_devgui_cmd(cmd)
{
	cmdTokens = strtok(cmd, " ");

	if (cmdTokens.size == 0)
	{
		return false;
	}

	host = util::getHostPlayerForBots();
	team = get_host_team();

	switch (cmdTokens[0])
	{
	case "spawn_enemy":
		team = util::getotherteam(team);
	case "spawn_friendly":
		count = 1;
		if (cmdTokens.size > 1)
		{
			count = int(cmdTokens[1]);
		}
		for (i = 0; i < count; i++)
		{
			add_bot(team);
		}
		return true;
	case "remove_enemy":
		team = util::getotherteam(team);
	case "remove_friendly":
		remove_bots(undefined, team);
		return true;
	case "fixed_spawn_enemy":
		team = util::getotherteam(team);
	case "fixed_spawn_friendly":
		bot = add_bot_at_eye_trace(team);
		if (isdefined(bot))
		{
			bot thread fixed_spawn_override();
		}
		return true;

	case "player_weapon":
		players = getPlayers();
		foreach(player in players)
		{
			if (!player util::is_bot())
			{
				continue;
			}

			weapon = host getCurrentWeapon();

			player weapons::detach_all_weapons();
			player takeAllWeapons();
			player giveWeapon(weapon);
			player switchToWeapon(weapon);
			player setSpawnWeapon(weapon);

			player teams::set_player_model(player.team, weapon);
		}
		return true;
	}

	return false;
}

function system_devgui_gadget_think()
{
	setDvar("devgui_bot_gadget", "");

	for (;; )
	{
		wait(1);

		gadget = getDvarString("devgui_bot_gadget");

		if (gadget.size == 0)
		{
			bot_turn_on_gadget(getWeapon(gadget));
			setDvar("devgui_bot_gadget", "");
		}
	}
}

function bot_turn_on_gadget(gadget)
{
	players = getPlayers();

	foreach(player in players)
	{
		if (!player util::is_bot())
		{
			continue;
		}

		host = util::getHostPlayer();
		weapon = host getCurrentWeapon();

		if (!isdefined(weapon) || weapon == level.weaponNone || weapon == level.weaponNull)
		{
			weapon = getWeapon("smg_standard");
		}

		player weapons::detach_all_weapons();
		player takeAllWeapons();
		player giveWeapon(weapon);
		player switchToWeapon(weapon);
		player setSpawnWeapon(weapon);

		player teams::set_player_model(player.team, weapon);

		player giveWeapon(gadget);
		slot = player gadgetGetSlot(gadget);
		player gadgetPowerSet(slot, 100.0);
		player botPressButtonForGadget(gadget);
	}
}

function fixed_spawn_override()
{
	self endon("disconnect");

	spawnOrigin = self.origin;
	spawnAngles = self.angles;

	while (1)
	{
		self waittill("spawned_player");

		self setOrigin(spawnOrigin);
		self setPlayerAngles(spawnAngles);
	}
}

#/
*/
