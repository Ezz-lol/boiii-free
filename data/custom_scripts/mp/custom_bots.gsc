// Multiplayer bot AI on top of the stock bot scripts: combat movement, hearing, grenades, scorestreaks, care packages,
// objective play and getting unstuck. Aim and reactions are left to the bot_difficulty preset. Only runs with
// boiii_advancedBots on, read at match start.
#using scripts\codescripts\struct;

#using scripts\shared\callbacks_shared;
#using scripts\shared\system_shared;
#using scripts\shared\array_shared;
#using scripts\shared\util_shared;
#using scripts\shared\gameobjects_shared;
#using scripts\shared\bots\_bot;
#using scripts\shared\bots\_bot_combat;
#using scripts\shared\bots\bot_buttons;
#using scripts\mp\bots\_bot;
#using scripts\mp\bots\_bot_loadout;
#using scripts\mp\killstreaks\_killstreaks;
#using scripts\mp\killstreaks\_killstreakrules;

#insert scripts\shared\shared.gsh;

#namespace custom_bots;

REGISTER_SYSTEM("custom_bots", &__init__, undefined)

function __init__()
{
	callback::on_start_gametype(&init);
	callback::on_connect(&on_player_connect);
	callback::on_spawned(&on_player_spawned);
}

function init()
{
	if (!GetDvarInt("boiii_advancedBots"))
		return;

	InitBotConfig();
	level thread WrapBotScripts();
	level thread BotFootstepLoop();
}

// Chances are % (per think unless noted), distances are in units (about 39 per meter), times in ms
function InitBotConfig()
{
	level.mv_cfg = SpawnStruct();
	level.mv_cfg.combatJump = 6;			// Thrust jump in a fight
	level.mv_cfg.dropshotBots = 30;			// Bots that dropshot at all
	level.mv_cfg.combatDropshot = 35;		// Fights those bots dropshot in
	level.mv_cfg.combatSlide = 4;			// Slide in a fight
	level.mv_cfg.moveSlide = 25;			// Slide while sprinting somewhere far
	level.mv_cfg.shootStreaks = 5;			// Shoot at an enemy air scorestreak
	level.mv_cfg.crateRadius = 1500;		// Go for a care package this close
	level.mv_cfg.crateStealRadius = 1500;	// Same for an enemy's
	level.mv_cfg.crateTeamWait = 0;			// Wait before taking a teammate's
	level.mv_cfg.crateLate = 45000;			// A crate this old is worth coming from three times as far
	level.mv_cfg.cleanDeposit = 2;			// Fracture: data carried before heading to a deposit
	level.mv_cfg.bombGuard = 60;			// S&D / Demolition: attackers guarding a planted bomb
	level.mv_cfg.ctfDefend = 25;			// CTF: share of a team defending its flag
	level.mv_cfg.strafeHold = 25;			// Strafe steps spent standing still to aim
	level.mv_cfg.strafeRange = 3000;		// Strafe at enemies this close
	level.mv_cfg.hearGunfire = 1500;
	level.mv_cfg.hearSuppressed = 300;
	level.mv_cfg.hearFootsteps = 400;		// Sprinting and sliding, doubled with Awareness
	level.mv_cfg.hearInvestigate = 35;		// Go check out a noise
	level.mv_cfg.hearDefend = 800;			// Objective modes: fights this close to the objective draw bots in
	level.mv_cfg.hunt = 75;					// Go after an enemy firing on the minimap
	level.mv_cfg.huntRange = 2500;
	level.mv_cfg.huntSlow = 400;			// Slow down to face the noise this close to it
	level.mv_cfg.routeVary = 40;			// Take a detour to a far goal
	level.mv_cfg.routeMin = 1200;			// Only for goals this far
	level.mv_cfg.nades = 50;				// Throw a grenade where an enemy disappeared or at a noise
	level.mv_cfg.objectiveNades = 25;		// Grenade an enemy held objective on the way to it
	level.mv_cfg.holdAngle = 1500;			// Hold the angle an enemy ducked behind this long
	level.mv_cfg.repeekReaction = 40;		// % of the first-sight aim delay for an enemy peeking back out there
}

// The stock bot scripts set up their combat and connect functions and the bot_difficulty preset at game start too,
// change them once that's done
function WrapBotScripts()
{
	waittillframeend;
	ApplyBotSettings();
	WrapBotCombat();
	WrapBotConnect();
}

function ApplyBotSettings()
{
	if (!isDefined(level.botSettings))
		return;

	// How far each weapon class fires: the presets stop SMGs at 20 m and rifles at 38 m, well short of how players use
	// them
	level.botSettings.smgRange = 2500;
	level.botSettings.rifleRange = 3000;
	level.botSettings.pistolRange = 2000;
	level.botSettings.defaultRange = 1500;

	// Stock bots throw a grenade the moment they see an enemy, see BotThreatLost (the Combat Axe keeps it, BotRunCombat)
	level.mv_stockLethalWeight = (isDefined(level.botSettings.lethalWeight) ? level.botSettings.lethalWeight : 0);
	level.botSettings.lethalWeight = 0;
	level.botSettings.tacticalWeight = 0;

	// A bot opening a care package stretches the shared think interval, this is the one to go back to
	level.mv_thinkInterval = level.botSettings.thinkInterval;
}

function on_player_connect()
{
	if (!isDefined(level.mv_cfg))
		return;

	self thread BotWatchGunfire();
}

function on_player_spawned()
{
	if (!isDefined(level.mv_cfg) || !self util::is_bot())
		return;

	self endon("disconnect");
	level endon("game_ended");

	// State left over if the bot died in the middle of something
	self.mv_usingStreak = undefined;
	self.mv_capturingCrate = undefined;
	self.mv_holdThink = undefined;
	self.mv_holdThinkApplied = undefined;
	self.mv_holdButtons = undefined;
	self.mv_jumping = undefined;
	self.mv_sliding = undefined;
	self.mv_maneuvering = undefined;
	self.mv_airTarget = undefined;
	self.mv_airNext = undefined;
	self.mv_guardZone = undefined;
	self.mv_ctfDefender = undefined;
	self.mv_ctfRoleUntil = undefined;
	self.mv_defSpot = undefined;
	self.mv_defNext = undefined;
	self.mv_chaseNext = undefined;
	self.mv_idleSince = undefined;
	self.mv_idleSpot = undefined;
	self.mv_idleTries = undefined;
	self.mv_settleUntil = undefined;
	self.mv_kothNext = undefined;
	self.mv_dropshot = undefined;
	self.mv_dropshotPlanned = undefined;
	self.mv_dropshotThreat = undefined;
	self.mv_strafeNext = undefined;
	self.mv_strafeSide = undefined;
	self.mv_closeNext = undefined;
	self.mv_hearing = undefined;
	self.mv_hearUntil = undefined;
	self.mv_detourGoal = undefined;
	self.mv_routePoint = undefined;
	self.mv_routeGoal = undefined;
	self.mv_defending = undefined;
	self.mv_noiseSource = undefined;
	self.mv_noisePos = undefined;
	self.mv_hurt = undefined;
	self.mv_throwUntil = undefined;
	self.mv_nadeNext = undefined;
	self.mv_nadeHoldUntil = undefined;
	self BotReleaseManualControl();

	// Let the bot scripts finish setting up this bot first
	waittillframeend;
	self BotSetCtfFlags();
	BotApplyOutfit();
	self thread BotWallrunWatch();
	self thread BotObjectiveNadeWatch();
	self thread BotTurnToDamage();
	if (level.gametype == "koth")
	{
		WrapKothBots();
		self thread BotKothZoneWatch();
	}
	WrapCleanBots();
	WrapBombBots();
	WrapCtfBots();
	WrapBotStreaks();

	// Last, so it wraps the mode's idle (and our CTF / KotH / bomb ones)
	WrapBotRoutes();
}

// Bot loadouts: the stock one only picks from a short whitelist, so bots kept using the same few guns and only
// scorestreaks that activate on selection
function WrapBotConnect()
{
	if (!isDefined(level.onBotConnect) || isDefined(level.mv_onBotConnect))
		return;

	level.mv_onBotConnect = level.onBotConnect;
	level.onBotConnect = &BotConnectWithStreaks;
}

// Same as the stock mp on_bot_connect, with our weapons and scorestreaks
function BotConnectWithStreaks()
{
	self endon("disconnect");
	level endon("game_ended");

	self BotSetCtfFlags();

	if (IS_TRUE(level.disableClassSelection))
	{
		self bot::set_rank();
		self bot_loadout::pick_hero_gadget();
		self PickBotKillstreaks();
		return;
	}

	if (!IS_TRUE(self.pers["bot_loadout"]))
	{
		self bot::set_rank();
		self BuildBotClasses();
		self bot_loadout::pick_hero_gadget();
		self PickBotKillstreaks();
		self.pers["bot_loadout"] = true;
	}

	self bot_loadout::pick_classes();
	self bot::choose_class();
}

// CTF: the stock connect step stores the bot's home and enemy flags, which all its CTF logic reads. This script
// replaces that step, so do it here too (and on spawn, in case the bot changed teams).
function BotSetCtfFlags()
{
	if (level.gametype != "ctf" || !isDefined(level.flags) || !isDefined(self.bot))
		return;

	foreach (flag in level.flags)
	{
		if (flag gameobjects::get_owner_team() == self.team)
			self.bot.flag = flag;
		else
			self.bot.enemyFlag = flag;
	}
}

// CTF: stock bots only get new orders without a goal, so a carrier keeps walking to an old waiting spot after its
// flag comes home. Send it straight to its base to score.
function BotCtfCarrierCheck()
{
	if (level.gametype != "ctf" || !isDefined(self.bot.flag) || !isDefined(self.bot.enemyFlag))
		return;

	carrier = self.bot.enemyFlag gameobjects::get_carrier();
	if (!isDefined(carrier) || carrier != self || self bot_combat::has_threat())
		return;

	// Own flag still away: the stock logic waits near the base for it
	if (self.bot.flag gameobjects::is_object_away_from_home())
		return;

	base = self.bot.flag.flagBase.trigger;
	radius = self bot::get_trigger_radius(base) + 64;
	if (!self BotGoalSet() || Distance2DSquared(self BotGetGoalPosition(), base.origin) > radius * radius)
	{
		self bot::approach_goal_trigger(base);
		self bot::sprint_to_goal();
	}
}

// CTF roles. Stock bots roll 80% "defend" near their base, often right on the flag, so they bunch up there. Instead up
// to ctfDefend % of a team patrols 6-22 m around its flag, the rest attack or escort. Roles rotate every 20-45 s.
function WrapCtfBots()
{
	if (level.gametype != "ctf" || isDefined(level.mv_botIdleCtf) || !isDefined(level.botIdle))
		return;

	level.mv_botIdleCtf = level.botIdle;
	level.botIdle = &BotCtfIdle;
}

// Never all of a team's bots: there's always at least one attacker
function BotCtfPickDefender()
{
	bots = 0;
	defenders = 0;
	foreach (player in level.players)
	{
		if (!player util::is_bot() || player.team != self.team)
			continue;
		bots++;
		if (player != self && IS_TRUE(player.mv_ctfDefender))
			defenders++;
	}

	maxDefenders = int(bots * level.mv_cfg.ctfDefend / 100 + 0.5);
	if (maxDefenders > bots - 1)
		maxDefenders = bots - 1;

	return defenders < maxDefenders;
}

// Defender patrol: hold a spot 6-22 m from the flag for 3-8 s, then another (stock sends a bot in range straight onto
// the flag). Runs every think, the stock idle only runs without a goal.
function BotCtfDefendTick()
{
	if (level.gametype != "ctf" || !isDefined(self.bot.flag) || !isDefined(self.bot.enemyFlag))
		return;

	carrier = self.bot.enemyFlag gameobjects::get_carrier();
	if (!IS_TRUE(self.mv_ctfDefender) || self bot_combat::has_threat() || (isDefined(carrier) && carrier == self) ||
		self.bot.flag gameobjects::is_object_away_from_home())
	{
		if (isDefined(self.mv_defSpot))
		{
			self.mv_defSpot = undefined;
			self.mv_defNext = undefined;
			if (!self bot_combat::has_threat())
				self BotLookForward();
		}
		return;
	}

	// Crate, killstreak or a fight near our flag: let it finish, it comes back after
	if (IS_TRUE(self.mv_capturingCrate) || IS_TRUE(self.mv_usingStreak) || isDefined(self.mv_airTarget) ||
		IS_TRUE(self.mv_defending))
		return;

	base = self.bot.flag.flagBase.trigger.origin;
	holding = isDefined(self.mv_defNext);

	if (!isDefined(self.mv_defSpot) || (holding && GetTime() > self.mv_defNext) ||
		(!holding && Distance2DSquared(self.origin, self.mv_defSpot) > 80 * 80 &&
		 (!self BotGoalSet() || Distance2DSquared(self BotGetGoalPosition(), self.mv_defSpot) > 64 * 64)))
	{
		query = PositionQuery_Source_Navigation(base, 250, 900, 150, 128, self);
		if (!isDefined(query) || !query.data.size)
			return;

		self.mv_defSpot = array::random(query.data).origin;
		self.mv_defNext = undefined;
		self BotLookForward();
		self BotSetGoal(self.mv_defSpot, 32);
		if (Distance2DSquared(self.origin, self.mv_defSpot) > 1200 * 1200)
			self bot::sprint_to_goal();
		return;
	}

	if (Distance2DSquared(self.origin, self.mv_defSpot) < 80 * 80)
	{
		if (!isDefined(self.mv_defNext))
		{
			self.mv_defNext = GetTime() + RandomIntRange(3000, 8000);
			if (RandomInt(100) < 65)
				self.mv_defLook = self.bot.enemyFlag.trigger.origin;
			else
				self.mv_defLook = self.origin + FlatDirection(base, self.origin) * 600;
		}
		// Turned toward a noise: let it look that way for the moment
		if (!IS_TRUE(self.mv_hearing))
			self BotLookAtPoint((self.mv_defLook[0], self.mv_defLook[1], self.origin[2] + 60));
	}
}

// CTF, our flag taken: stock bots near the base still walk to the empty stand and only re-plan at their goal, so
// chasers run to where the carrier was. Everyone but our carrier chases it (updated every second) or returns the flag.
function BotCtfChaseTick()
{
	if (level.gametype != "ctf" || !isDefined(self.bot.flag) || !isDefined(self.bot.enemyFlag))
		return;

	if (!(self.bot.flag gameobjects::is_object_away_from_home()) || self bot_combat::has_threat())
		return;

	// Our carrier waits near the base (stock logic)
	ourCarrier = self.bot.enemyFlag gameobjects::get_carrier();
	if (isDefined(ourCarrier) && ourCarrier == self)
		return;

	if (IS_TRUE(self.mv_capturingCrate) || IS_TRUE(self.mv_usingStreak) || isDefined(self.mv_airTarget))
		return;

	if (isDefined(self.mv_chaseNext) && GetTime() < self.mv_chaseNext)
		return;
	self.mv_chaseNext = GetTime() + 1000;

	enemyCarrier = self.bot.flag gameobjects::get_carrier();
	if (isDefined(enemyCarrier))
		target = enemyCarrier.origin;
	else
		target = self.bot.flag.trigger.origin;

	self BotSetGoal(target, 48);
	self bot::sprint_to_goal();
}

// Runs when the bot has no enemy and no goal
function BotCtfIdle()
{
	if (!isDefined(self.bot.flag) || !isDefined(self.bot.enemyFlag))
	{
		self [[level.mv_botIdleCtf]]();
		return;
	}

	carrier = self.bot.enemyFlag gameobjects::get_carrier();

	// Carrying the enemy flag, or our own flag is away: stock logic (score / wait near base / return it)
	if ((isDefined(carrier) && carrier == self) || self.bot.flag gameobjects::is_object_away_from_home())
	{
		self [[level.mv_botIdleCtf]]();
		return;
	}

	// An attacker in the enemy half keeps attacking until it dies or gets back
	if (isDefined(self.mv_ctfRoleUntil) && GetTime() > self.mv_ctfRoleUntil)
	{
		pushing = !IS_TRUE(self.mv_ctfDefender) &&
			Distance2DSquared(self.origin, self.bot.enemyFlag.trigger.origin) < Distance2DSquared(self.origin, self.bot.flag.flagBase.trigger.origin);
		if (!pushing)
			self.mv_ctfDefender = undefined;
	}
	if (!isDefined(self.mv_ctfDefender))
	{
		self.mv_ctfDefender = self BotCtfPickDefender();
		self.mv_ctfRoleUntil = GetTime() + RandomIntRange(20000, 45000);
	}

	if (self.mv_ctfDefender)
	{
		self BotCtfDefendTick();
		return;
	}

	if (!isDefined(carrier))
		self bot::approach_goal_trigger(self.bot.enemyFlag.trigger);
	else
		self bot::approach_point(carrier.origin, 150, 500);
	self bot::sprint_to_goal();
}

// Any primary and secondary in the game (minus restricted ones)
function BuildBotClasses()
{
	if (!isDefined(level.mv_botPrimaries))
	{
		level.mv_botPrimaries = GetBotWeaponPool("primary");
		level.mv_botSecondaries = GetBotWeaponPool("secondary");
	}
	if (!level.mv_botPrimaries.size)
	{
		self bot_loadout::build_classes();
		return;
	}

	others = [];
	others[others.size] = self bot_loadout::get_available_items(undefined, "primarygadget");
	others[others.size] = self bot_loadout::get_available_items(undefined, "secondarygadget");
	if (IS_TRUE(level.perksEnabled))
	{
		others[others.size] = self bot_loadout::get_available_items(undefined, "specialty1");
		others[others.size] = self bot_loadout::get_available_items(undefined, "specialty2");
		others[others.size] = self bot_loadout::get_available_items(undefined, "specialty3");
	}

	foreach (className, classValue in level.classMap)
	{
		if (!isSubstr(className, "custom"))
			continue;

		classIndex = int(className[className.size - 1]);

		picked = [];
		picked[picked.size] = array::random(level.mv_botPrimaries);
		if (level.mv_botSecondaries.size && RandomInt(100) < 95)
			picked[picked.size] = array::random(level.mv_botSecondaries);

		others = array::randomize(others);
		foreach (items in others)
		{
			if (isDefined(items) && items.size)
				picked[picked.size] = array::random(items);
		}

		for (i = 0; i < picked.size && i < level.maxAllocation; i++)
			self BotClassAddItem(classIndex, picked[i]);
	}
}

// Every weapon in the game's item table for a slot ("primary" / "secondary"). Columns from statstable_shared.gsh:
// 0 numbering, 3 name, 4 reference, 12 allocation, 13 slot
function GetBotWeaponPool(slot)
{
	items = [];
	for (i = 0; i < 256; i++)
	{
		row = tableLookupRowNum(level.statsTableID, 0, i);
		if (row < 0)
			continue;

		name = tableLookupColumnForRow(level.statsTableID, row, 3);
		if (name == "" || tableLookupColumnForRow(level.statsTableID, row, 13) != slot)
			continue;

		// Melee weapons: bots would only pull them out when out of ammo
		if (isSubstr(name, "MELEE"))
			continue;

		if (Int(tableLookupColumnForRow(level.statsTableID, row, 12)) < 0)
			continue;

		ref = tableLookupColumnForRow(level.statsTableID, row, 4);
		if (ref == "" || IsItemRestricted(ref) || GetWeapon(ref) == level.weaponNone)
			continue;

		items[items.size] = name;
	}
	return items;
}

// Stock bots only get a random specialist, always in the default outfit. Pick one outfit per bot and keep it; bodies
// and helmets are listed in the same order, so the same index is a matching set.
function BotApplyOutfit()
{
	character = self GetCharacterBodyType();
	if (!isDefined(self.pers["mv_outfit"]) || self.pers["mv_outfit"]["character"] != character)
	{
		mode = CurrentSessionMode();
		bodies = GetCharacterBodyModelCount(character, mode);
		helmets = GetCharacterHelmetModelCount(character, mode);

		count = bodies;
		if (helmets < count)
			count = helmets;
		index = count > 0 ? RandomInt(count) : 0;

		outfit = [];
		outfit["character"] = character;
		outfit["body"] = index;
		outfit["helmet"] = index;
		self.pers["mv_outfit"] = outfit;
	}

	self SetCharacterBodyStyle(self.pers["mv_outfit"]["body"]);
	self SetCharacterHelmetStyle(self.pers["mv_outfit"]["helmet"]);
}

// Three of the scorestreaks BotPostCombatWithStreaks knows how to use
function PickBotKillstreaks()
{
	if (!isDefined(level.mv_botStreaks))
		level.mv_botStreaks = GetBotStreakPool();
	if (!level.mv_botStreaks.size)
	{
		self bot_loadout::pick_killstreaks();
		return;
	}

	killstreaks = array::randomize(level.mv_botStreaks);
	for (i = 0; i < 3 && i < killstreaks.size; i++)
		self BotClassAddItem(0, killstreaks[i]);
}

function GetBotStreakPool()
{
	usable = array("killstreak_uav", "killstreak_counteruav", "killstreak_satellite", "killstreak_raps",
		"killstreak_sentinel", "killstreak_remote_missile", "killstreak_helicopter_player_gunner", "killstreak_rcbomb",
		"killstreak_dart", "killstreak_planemortar", "killstreak_drone_strike", "killstreak_helicopter_comlink",
		"killstreak_supply_drop", "killstreak_ai_tank_drop", "killstreak_auto_turret", "killstreak_microwave_turret",
		"killstreak_emp");

	items = [];
	for (i = 0; i < 256; i++)
	{
		row = tableLookupRowNum(level.statsTableID, 0, i);
		if (row < 0)
			continue;

		name = tableLookupColumnForRow(level.statsTableID, row, 3);
		ref = tableLookupColumnForRow(level.statsTableID, row, 4);
		if (name == "" || !IsInArray(usable, ref) || IsItemRestricted(ref) ||
			Int(tableLookupColumnForRow(level.statsTableID, row, 12)) < 0)
			continue;

		items[items.size] = name;
	}
	return items;
}

function WrapBotStreaks()
{
	if (isDefined(level.mv_botPostCombatStreaks) || !isDefined(level.botPostCombat))
		return;

	level.mv_botPostCombatStreaks = level.botPostCombat;
	level.botPostCombat = &BotPostCombatWithStreaks;
}

function BotPostCombatWithStreaks()
{
	// Crates, killstreaks, Fracture deposits and Kill Confirmed tags all set new goals, wait until the bot lands
	if (self BotTraversing())
		return;

	self [[level.mv_botPostCombatStreaks]]();

	self BotCtfCarrierCheck();
	self BotCtfDefendTick();
	self BotCtfChaseTick();

	if (isDefined(self.carryObject))
		return;

	if (IS_TRUE(self.mv_usingStreak) || IS_TRUE(self.mv_capturingCrate) || self BotSeesThreat() ||
		isDefined(self.selectingLocation))
		return;

	// Mid slide, dropshot or jump: a fight slide gives the bot's movement back at the end, which would cut off an RC-XD
	// or Dart started meanwhile, and placing or throwing needs the buttons free
	if (IS_TRUE(self.mv_maneuvering))
		return;

	// Still chasing an enemy it lost sight of: only a crate right there is worth stopping for
	chasing = self bot_combat::has_threat();
	self BotLookForCrate(chasing);
	if (chasing || IS_TRUE(self.mv_capturingCrate) || !level.loadoutKillstreaksEnabled)
		return;

	inventoryWeapon = self GetInventoryWeapon();
	foreach (weapon in self GetWeaponsList())
	{
		killstreak = killstreaks::get_killstreak_for_weapon(weapon);
		if (!isDefined(killstreak))
			continue;

		if (weapon != inventoryWeapon && !self GetWeaponAmmoClip(weapon))
			continue;

		if (!self killstreakrules::isKillstreakAllowed(killstreak, self.team))
			continue;

		// The stock code only tries the first streak it finds, so one it can't use (a crate's Sentry Gun) blocked the
		// rest
		switch (killstreaks::get_menu_name(killstreak))
		{
		case "killstreak_planemortar":
			self thread BotUseLocationStreak(weapon, 3, false);
			return;
		case "killstreak_drone_strike":
			self thread BotUseLocationStreak(weapon, 1, true);
			return;
		case "killstreak_helicopter_comlink":
			self thread BotUseLocationStreak(weapon, 1, false);
			return;
		case "killstreak_supply_drop":
		case "killstreak_ai_tank_drop":
			self thread BotUseCarePackage(weapon);
			return;
		case "killstreak_auto_turret":
		case "killstreak_microwave_turret":
		case "killstreak_emp":
			self thread BotUsePlaceable(weapon);
			return;
		// Activate on selection (Hellstorm then falls where the bot is looking)
		case "killstreak_uav":
		case "killstreak_counteruav":
		case "killstreak_satellite":
		case "killstreak_raps":
		case "killstreak_sentinel":
		case "killstreak_remote_missile":
		// Mothership flies itself, the bot only mans the gun with its normal aiming and firing
		case "killstreak_helicopter_player_gunner":
			self SwitchToWeapon(weapon);
			return;
		// Remote controlled, the script drives them toward enemies
		case "killstreak_rcbomb":
			self thread BotUseRCXD(weapon);
			return;
		case "killstreak_dart":
			self thread BotUseDart(weapon);
			return;
		}
	}
}

// Location streaks wait for map clicks ("confirm_location"), send enemy positions instead. Lightning Strike takes 3,
// the AI helicopter 1 (the area it guards), Rolling Thunder 1 plus a direction.
function BotUseLocationStreak(weapon, count, withYaw)
{
	self endon("death");
	self endon("disconnect");
	level endon("game_ended");

	enemies = self GetEnemies();
	if (enemies.size == 0)
		return;

	self.mv_usingStreak = true;
	self SwitchToWeapon(weapon);

	for (i = 0; i < 40 && !isDefined(self.selectingLocation); i++)
		wait 0.05;

	if (isDefined(self.selectingLocation))
	{
		enemies = array::randomize(self GetEnemies());
		for (i = 0; i < count; i++)
		{
			if (enemies.size > 0)
				target = enemies[i % enemies.size].origin;
			else
				target = self.origin + (RandomIntRange(-1000, 1000), RandomIntRange(-1000, 1000), 0);

			target += (RandomIntRange(-150, 150), RandomIntRange(-150, 150), 0);
			if (withYaw)
				self notify("confirm_location", target, VectorToAngles(target - self.origin)[1]);
			else
				self notify("confirm_location", target);
			wait 0.05;
		}
	}

	wait 1;
	self.mv_usingStreak = undefined;
}

// RC-XD and Dart are driven with the player's own input: take manual control and steer at the nearest enemy. The bot's
// body stays put while driving, so measure from the vehicle and steer with world directions from it.
function BotNearestEnemyTo(point)
{
	nearest = undefined;
	nearestDistSq = undefined;
	foreach (enemy in self GetEnemies())
	{
		if (!IsAlive(enemy))
			continue;

		distSq = DistanceSquared(point, enemy.origin);
		if (!isDefined(nearest) || distSq < nearestDistSq)
		{
			nearest = enemy;
			nearestDistSq = distSq;
		}
	}
	return nearest;
}

function BotWaitForRemoteControl(maxWait)
{
	for (i = 0; i < maxWait * 20 && !self IsRemoteControlling(); i++)
		wait 0.05;
	return self IsRemoteControlling();
}

function FlatDirection(from, to)
{
	return VectorNormalize((to[0] - from[0], to[1] - from[1], 0));
}

function TurnDirection(dir, yawOffset)
{
	return AnglesToForward((0, VectorToAngles(dir)[1] + yawOffset, 0));
}

// RC-XD: drive at the nearest enemy (backing up and turning when stuck) and blow it up next to them
function BotUseRCXD(weapon)
{
	self endon("death");
	self endon("disconnect");
	level endon("game_ended");

	if (!isDefined(self BotNearestEnemyTo(self.origin)) || !self IsOnGround())
		return;

	self.mv_usingStreak = true;
	bodyPos = self.origin;
	self SwitchToWeapon(weapon);

	if (self BotWaitForRemoteControl(4))
	{
		car = self GetVehicleOccupied();
		self BotTakeManualControl();

		lastPos = car.origin;
		reverseFrames = 0;
		detourFrames = 0;
		detourDir = undefined;
		detonate = false;

		for (i = 0; i < 400 && self IsRemoteControlling() && isDefined(car); i++)
		{
			target = self BotNearestEnemyTo(car.origin);
			if (isDefined(target))
				dir = FlatDirection(car.origin, target.origin);
			else
				dir = AnglesToForward((0, car.angles[1], 0));

			if (reverseFrames > 0)
			{
				reverseFrames--;
				self BotSetLookAngles(dir);
				self BotSetMoveAngle(dir * -1);
			}
			else
			{
				if (detourFrames > 0)
				{
					detourFrames--;
					dir = detourDir;
				}
				self BotSetLookAngles(dir);
				self BotSetMoveAngle(dir);
			}
			self BotSetMoveMagnitude(1);

			if (isDefined(target) && DistanceSquared(car.origin, target.origin) < 150 * 150)
			{
				detonate = true;
				break;
			}

			// Stuck check every second: back up for half a second, then go around for a second
			if (i % 20 == 19)
			{
				if (reverseFrames == 0 && DistanceSquared(car.origin, lastPos) < 40 * 40)
				{
					reverseFrames = 10;
					side = 1;
					if (RandomInt(2))
						side = -1;
					detourDir = TurnDirection(dir, side * RandomIntRange(70, 110));
					detourFrames = 20;
				}
				lastPos = car.origin;
			}
			wait 0.05;
		}

		// Out of time: only blow it up if it's well away from the bot itself, otherwise let it time out on its own
		if (!detonate && isDefined(car) && DistanceSquared(car.origin, bodyPos) > 400 * 400)
			detonate = true;

		if (detonate)
		{
			for (i = 0; i < 10 && self IsRemoteControlling(); i++)
			{
				self bot::press_attack_button();
				wait 0.05;
			}
			self bot::release_attack_button();
		}
		self BotReleaseManualControl();
	}

	wait 1;
	self.mv_usingStreak = undefined;
}

// Dart: throw it like a marker (needs open sky), then fly at the nearest enemy and fire when close
function BotUseDart(weapon)
{
	self endon("death");
	self endon("disconnect");
	level endon("game_ended");

	if (!isDefined(self BotNearestEnemyTo(self.origin)))
		return;

	self.mv_usingStreak = true;
	self SwitchToWeapon(weapon);

	for (i = 0; i < 40 && self GetCurrentWeapon() != weapon; i++)
		wait 0.05;

	if (self GetCurrentWeapon() == weapon)
	{
		wait 0.3;
		self BotSetLookAngles(AnglesToForward((-30, self.angles[1], 0)));
		self bot::tap_attack_button();

		if (self BotWaitForRemoteControl(4))
		{
			dart = self GetVehicleOccupied();
			self BotTakeManualControl();
			for (i = 0; i < 600 && self IsRemoteControlling() && isDefined(dart); i++)
			{
				target = self BotNearestEnemyTo(dart.origin);
				if (isDefined(target))
				{
					dir = VectorNormalize((target.origin + (0, 0, 40)) - dart.origin);
					self BotSetLookAngles(dir);
					self BotSetMoveAngle(dir);
					self BotSetMoveMagnitude(1);

					if (i % 10 == 0 && DistanceSquared(dart.origin, target.origin) < 1200 * 1200)
						self bot::tap_attack_button();
				}
				wait 0.05;
			}
			self BotReleaseManualControl();
		}
	}

	wait 1;
	self.mv_usingStreak = undefined;
}

// Sentry Gun, Guardian and Power Core are placed with fire where the spot in front is valid. Carrying disables the
// bot's weapons, so look around for a spot and cancel (keeping the streak) if there's none.
function BotUsePlaceable(weapon)
{
	self endon("death");
	self endon("disconnect");
	level endon("game_ended");

	self.mv_usingStreak = true;
	self SwitchToWeapon(weapon);

	for (i = 0; i < 40 && !isDefined(self.holding_placeable); i++)
		wait 0.05;

	if (isDefined(self.holding_placeable))
	{
		yaw = self.angles[1];
		for (i = 0; i < 120 && isDefined(self.holding_placeable); i++)
		{
			if (i % 20 == 19)
				yaw += RandomIntRange(60, 180);
			self BotSetLookAngles(AnglesToForward((15, yaw, 0)));

			// Placing needs fire released and then pressed
			if (i % 2 == 0)
				self bot::press_attack_button();
			else
				self bot::release_attack_button();
			wait 0.05;
		}
		self bot::release_attack_button();

		if (isDefined(self.holding_placeable) && IS_TRUE(self.holding_placeable.cancelable))
			self.holding_placeable notify("cancelled");
	}

	wait 0.5;
	self.mv_usingStreak = undefined;
}

function BotUseCarePackage(weapon)
{
	self endon("death");
	self endon("disconnect");
	level endon("game_ended");

	self.mv_usingStreak = true;
	self SwitchToWeapon(weapon);

	for (i = 0; i < 40 && self GetCurrentWeapon() != weapon; i++)
		wait 0.05;

	// The marker goes where the bot looks and only takes fire on open ground in range (markerPosition is set while it
	// does), so look at a spot like that first. Try a few, the marker's own check is stricter than BotCarePackageSpot.
	for (attempt = 0; attempt < 4 && self GetCurrentWeapon() == weapon && !self BotSeesThreat(); attempt++)
	{
		spot = self BotCarePackageSpot();
		if (!isDefined(spot))
		{
			wait 0.5;
			continue;
		}

		for (i = 0; i < 30 && !isDefined(self.markerPosition) && !self BotSeesThreat(); i++)
		{
			self BotLookAtPoint(spot);
			wait 0.05;
		}
		if (!isDefined(self.markerPosition))
			continue;

		self bot::tap_attack_button();
		for (i = 0; i < 20 && self GetCurrentWeapon() == weapon; i++)
		{
			self BotLookAtPoint(spot);
			wait 0.05;
		}
	}

	// Nowhere it would land, or an enemy showed up: the marker can't shoot, so put it away (the streak is kept)
	if (self GetCurrentWeapon() == weapon)
	{
		self BotSwitchToGun();
	}
	if (!self BotSeesThreat())
		self BotLookForward();

	wait 1;
	self.mv_usingStreak = undefined;
}

// Open ground 5-15m away (within the marker's range) that the bot can see, with nothing overhead. Ahead first, then
// further around.
function BotCarePackageSpot()
{
	minRange = 200;
	maxRange = 600;
	if (isDefined(level.killstreakCoreBundle))
	{
		if (isDefined(level.killstreakCoreBundle.ksMinAirdropTargetRange))
			minRange = level.killstreakCoreBundle.ksMinAirdropTargetRange + 64;
		if (isDefined(level.killstreakCoreBundle.ksMaxAirdropTargetRange) &&
			level.killstreakCoreBundle.ksMaxAirdropTargetRange - 64 < maxRange)
			maxRange = level.killstreakCoreBundle.ksMaxAirdropTargetRange - 64;
	}
	if (maxRange < minRange)
		maxRange = minRange;

	eye = self GetEye();
	foreach (offset in array(0, 45, -45, 90, -90, 135, -135, 180))
	{
		dir = AnglesToForward((0, self.angles[1] + offset, 0));
		spot = GetClosestPointOnNavMesh(self.origin + dir * RandomFloatRange(minRange, maxRange), 96, 48);
		if (!isDefined(spot) || Distance2DSquared(spot, self.origin) < minRange * minRange)
			continue;

		if (BulletTracePassed(spot + (0, 0, 16), spot + (0, 0, 4000), false, undefined) &&
			SightTracePassed(eye, spot + (0, 0, 4), false, self))
			return spot;
	}

	return undefined;
}

// Back to the gun the bot had before the scorestreak
function BotSwitchToGun()
{
	weapon = self.lastNonKillstreakWeapon;
	if (!isDefined(weapon) || weapon == level.weaponNone || !self HasWeapon(weapon))
	{
		primaries = self GetWeaponsListPrimaries();
		if (!primaries.size)
			return;
		weapon = primaries[0];
	}

	self SwitchToWeapon(weapon);
}

// Care packages: players grab them fast, so any bot that isn't fighting goes for one nearby, dropping a noise for it.
// Crates expire after 90 s, so once one has sat a while bots come for it from further. One bot per crate. The crate
// checks use is held every frame but the bot think releases buttons, so trigger it from script and stretch the think.
function BotLookForCrate(chasing)
{
	radius = level.mv_cfg.crateRadius;
	if (radius <= 0)
		return;
	if (IS_TRUE(chasing) && radius > 400)
		radius = 400;

	// CTF: our flag is away, getting it back comes first
	if (level.gametype == "ctf" && isDefined(self.bot.flag) && self.bot.flag gameobjects::is_object_away_from_home())
		return;

	// Fracture: a bot carrying enough data to deposit goes to the deposit first
	if (level.gametype == "clean" && isDefined(self.carriedTacos) && level.mv_cfg.cleanDeposit > 0 &&
		self.carriedTacos >= level.mv_cfg.cleanDeposit)
		return;

	hearing = IS_TRUE(self.mv_hearing);
	nearest = undefined;
	nearestKind = undefined;
	nearestSpot = undefined;
	nearestDistSq = undefined;
	foreach (crate in GetEntArray("care_package", "script_noteworthy"))
	{
		// Bots look every think, so this is about when it landed
		if (!isDefined(crate.mv_seenTime))
			crate.mv_seenTime = GetTime();
		age = GetTime() - crate.mv_seenTime;

		// Another bot is already on its way
		if (isDefined(crate.mv_claimer) && crate.mv_claimer != self && isAlive(crate.mv_claimer) &&
			IS_TRUE(crate.mv_claimer.mv_capturingCrate))
			continue;

		reach = radius;
		late = level.mv_cfg.crateLate >= 0 && age >= level.mv_cfg.crateLate;
		if (late && !IS_TRUE(chasing))
			reach = radius * 3;
		kind = "own";
		if (!isDefined(crate.owner) || crate.owner != self)
		{
			// A crate whose owner left is anyone's
			kind = "abandoned";
			if (isDefined(crate.owner) && (!isDefined(crate.team) || crate.team == self.team))
			{
				kind = "teammate's";
				if (level.mv_cfg.crateTeamWait < 0 || (!late && age < level.mv_cfg.crateTeamWait))
					continue;
			}
			else if (isDefined(crate.owner))
			{
				kind = "enemy";
				if (reach > level.mv_cfg.crateStealRadius)
					reach = level.mv_cfg.crateStealRadius;
			}
		}

		if (isDefined(crate.useEnt) && IS_TRUE(crate.useEnt.inUse))
			continue;

		if (isDefined(self.mv_badCrate) && self.mv_badCrate == crate && GetTime() < self.mv_badCrateUntil)
			continue;

		distSq = DistanceSquared(self.origin, crate.origin);
		if ((isDefined(nearestDistSq) && distSq >= nearestDistSq) || distSq > reach * reach)
			continue;

		// Crates often land a little off the walkable area: walk to the floor next to it (they open from a step away)
		spot = GetClosestPointOnNavMesh(crate.origin, 128, 100);
		if (isDefined(spot))
		{
			nearest = crate;
			nearestKind = kind;
			nearestSpot = spot;
			nearestDistSq = distSq;
		}
	}

	if (!isDefined(nearest))
		return;

	if (hearing)
		self BotDropNoise();
	self thread BotCaptureCrate(nearest, nearestSpot);
}

function BotCaptureCrate(crate, spot)
{
	self endon("death");
	self endon("disconnect");
	level endon("game_ended");

	self.mv_capturingCrate = true;
	crate.mv_claimer = self;
	self thread BotWatchCratePath(crate);
	self BotSetGoal(spot, 32);
	self bot::sprint_to_goal();

	// Time to get there at a sprint (about 250 units/s) and some, 45s at most
	frames = 300 + int(Distance2D(self.origin, spot) / 250 * 20);
	if (frames > 900)
		frames = 900;
	for (i = 0; i <= frames; i++)
	{
		if (i == frames)
		{
			self.mv_capturingCrate = undefined;
			self notify("mv_crate_done");
			return;
		}

		if (!isDefined(crate) || self BotSeesThreat() || (isDefined(self.mv_badCrate) && self.mv_badCrate == crate))
		{
			self.mv_capturingCrate = undefined;
			self notify("mv_crate_done");
			return;
		}
		if (Distance2DSquared(self.origin, crate.origin) < 100 * 100 && self IsOnGround())
			break;

		// Objective modes keep giving bots new goals, keep putting the crate back as the goal
		if (i % 5 == 0 && !self BotTraversing() && (!self BotGoalSet() || Distance2DSquared(self BotGetGoalPosition(), spot) > 48 * 48))
		{
			self BotSetGoal(spot, 32);
			self bot::sprint_to_goal();
		}
		wait 0.05;
	}

	if (isDefined(crate) && Distance2DSquared(self.origin, crate.origin) < 100 * 100)
	{
		if (isDefined(crate.owner) && crate.owner == self)
			useTime = level.crateOwnerUseTime / 1000;
		else
			useTime = level.crateNonOwnerUseTime / 1000;

		// Stretch the think interval so the bot doesn't release use mid capture, wait for it to apply
		self.mv_holdThink = useTime + 0.5;
		for (i = 0; i < 10 && !IS_TRUE(self.mv_holdThinkApplied); i++)
			wait 0.05;

		self BotLookAtPoint(crate.origin);
		self bot::press_use_button();
		crate notify("trigger", self);

		for (i = 0; i < (useTime + 0.5) * 20 && isDefined(crate); i++)
		{
			self bot::press_use_button();
			wait 0.05;
		}
		self bot::release_use_button();
	}

	self.mv_holdThink = undefined;
	self.mv_holdThinkApplied = undefined;
	self.mv_capturingCrate = undefined;
	self notify("mv_crate_done");
}

// No path to the crate: skip it for a while. Not for the bot being off the walkable area a moment, that sorts itself
// out.
function BotWatchCratePath(crate)
{
	self endon("death");
	self endon("disconnect");
	self endon("mv_crate_done");

	for (;;)
	{
		self waittill("bot_path_failed", reason);
		if (isDefined(reason) && reason != 1)
			break;
	}
	self.mv_badCrate = crate;
	self.mv_badCrateUntil = GetTime() + 30000;
}

// The stock approach_goal_trigger sends a bot over 1500 units out to a random spot up to 1500 out, and on arrival it's
// often still "too far" (3D distance), so far bots circled and never reached the hardpoint. Step closer instead (one
// long path straight across the map runs bots into walls).
function WrapKothBots()
{
	if (level.gametype != "koth" || isDefined(level.mv_botIdleKoth) || !isDefined(level.botIdle))
		return;

	level.mv_botIdleKoth = level.botIdle;
	level.botIdle = &BotKothIdle;
}

// Goal at one of the spots a navmesh query finds around a point (BotSetGoal fails without a path). preferNear: closest
// of 6, else random. inZoneOnly: inside the hardpoint if any is. True once set.
function BotTryGoalNear(point, radiusMin, radiusMax, height, spacing, preferNear, inZoneOnly)
{
	queryResult = PositionQuery_Source_Navigation(point, radiusMin, radiusMax, height, spacing, self);
	points = array::randomize(queryResult.data);

	if (IS_TRUE(inZoneOnly))
	{
		inside = [];
		foreach (spot in points)
		{
			if (BotPointInZone(spot.origin))
				inside[inside.size] = spot;
		}
		if (inside.size > 0)
			points = inside;
	}

	tried = [];
	for (tries = 0; tries < 3; tries++)
	{
		best = undefined;
		bestDistSq = undefined;
		for (i = 0; i < points.size && i < 6; i++)
		{
			if (IS_TRUE(tried[i]))
				continue;
			distSq = DistanceSquared(self.origin, points[i].origin);
			if (!isDefined(best) || (preferNear && distSq < bestDistSq))
			{
				best = i;
				bestDistSq = distSq;
			}
		}
		if (!isDefined(best))
			return false;
		if (self BotSetGoal(points[best].origin, 24))
			return true;
		tried[best] = true;
	}
	return false;
}

// Whether a spot on the floor is inside the hardpoint's box, height included (a roof above the hardpoint isn't in it)
function BotPointInZone(point)
{
	return BotPointInTrigger(point, level.zone.gameobject.trigger);
}

// Same for any objective area (hardpoint, Domination flag)
function BotPointInTrigger(point, trigger)
{
	mins = trigger GetMins();
	maxs = trigger GetMaxs();
	low = trigger.origin + mins;
	high = trigger.origin + maxs;
	return point[0] >= low[0] - 8 && point[0] <= high[0] + 8 && point[1] >= low[1] - 8 && point[1] <= high[1] + 8 &&
		point[2] >= low[2] - 32 && point[2] <= high[2];
}

// Walkable spot inside the hardpoint closest to its center. The closest walkable spot overall can be on another floor
// (on Nuketown, the roof above a hardpoint in a house).
function BotKothCenter()
{
	if (isDefined(level.zone.mv_navCenter))
		return level.zone.mv_navCenter;

	trigger = level.zone.gameobject.trigger;
	origin = level.zone.origin;
	if (!isDefined(origin))
		origin = trigger.origin;

	center = GetClosestPointOnNavMesh(origin, 300);
	if (!isDefined(center) || !BotPointInZone(center))
	{
		mins = trigger GetMins();
		maxs = trigger GetMaxs();
		middle = (origin[0], origin[1], trigger.origin[2] + (mins[2] + maxs[2]) / 2);
		queryResult = PositionQuery_Source_Navigation(middle, 0, 400, (maxs[2] - mins[2]) / 2 + 32, 32, self);

		best = undefined;
		bestDistSq = undefined;
		foreach (spot in queryResult.data)
		{
			if (!BotPointInZone(spot.origin))
				continue;
			distSq = Distance2DSquared(spot.origin, origin);
			if (!isDefined(best) || distSq < bestDistSq)
			{
				best = spot.origin;
				bestDistSq = distSq;
			}
		}

		if (isDefined(best))
			center = best;
	}

	if (!isDefined(center))
		center = GetClosestPointOnNavMesh(trigger.origin, 300);
	level.zone.mv_navCenter = center;
	return center;
}

// Head for the hardpoint by its center: the stock way works out spots from the trigger's shape, and on some maps none
// is reachable, leaving bots with no goal. From further than 1800 units, first a spot within 1100. True if in it.
function BotKothGo()
{
	trigger = level.zone.gameobject.trigger;
	inZone = self IsTouching(trigger);

	center = self BotKothCenter();
	if (isDefined(center))
	{
		if (DistanceSquared(self.origin, center) > 1800 * 1800)
		{
			if (self BotTryGoalNear(center, 400, 1100, 300, 128, true, false))
				return inZone;
		}
		else if (self BotTryGoalNear(center, 0, 160, 100, 32, false, true))
			return inZone;

		if (self BotSetGoal(center, 48))
			return inZone;
	}

	self bot::path_to_point_in_trigger(trigger);
	return inZone;
}

// Runs when the bot has no enemy and no goal (and from the idle watchdog)
function BotKothIdle()
{
	if (!isDefined(level.zone) || !isDefined(level.zone.gameobject) || !isDefined(level.zone.gameobject.trigger))
	{
		self [[level.mv_botIdleKoth]]();
		return;
	}

	// No way there worked: don't redo the navmesh searches every think for every bot
	if (isDefined(self.mv_kothNext) && GetTime() < self.mv_kothNext)
		return;

	if (!self BotKothGo())
		self bot::sprint_to_goal();

	if (!self BotGoalSet())
		self.mv_kothNext = GetTime() + 1000;
}

// Hardpoint moved: the stock script gives every bot a goal where it stands, which counts as reached and is never
// replaced, so whole groups froze. Send them toward the new hardpoint right after.
function BotKothZoneWatch()
{
	self notify("mv_koth_watch");
	self endon("mv_koth_watch");
	self endon("death");
	self endon("disconnect");
	level endon("game_ended");

	for (;;)
	{
		level waittill("zone_moved");
		waittillframeend; // after the stock handler

		// The new zone may take a moment to be set up
		for (i = 0; i < 50 && !isDefined(level.zone); i++)
			wait 0.1;

		if (!isDefined(level.zone) || self bot_combat::has_threat() || self BotTraversing() ||
			IS_TRUE(self.mv_usingStreak) || IS_TRUE(self.mv_capturingCrate))
			continue;

		oldGoal = undefined;
		if (self BotGoalSet())
			oldGoal = self BotGetGoalPosition();
		self BotKothGo();
		self bot::sprint_to_goal();

		// The whole team heads to the new hardpoint at once: not all the same way
		self BotMaybeDetour(oldGoal);
	}
}

// S&D / Demolition: after a plant, attackers wander off (stock only guards it within ~19 m, Demolition has no bot
// script). Some of the attackers hold a spot around the bomb while it ticks.
function WrapBombBots()
{
	if ((level.gametype != "dem" && level.gametype != "sd") || isDefined(level.mv_botIdle) || !isDefined(level.botIdle) ||
		!isDefined(level.bombZones))
		return;

	level.mv_botIdle = level.botIdle;
	level.botIdle = &BotBombIdle;
}

// Runs when the bot has no enemy and no goal
function BotBombIdle()
{
	if (self BotGuardPlantedBomb())
		return;

	self [[level.mv_botIdle]]();
}

function BotGuardPlantedBomb()
{
	foreach (zone in level.bombZones)
	{
		if (!IS_TRUE(zone.isPlanted) || !isDefined(zone.trigger))
			continue;

		// The zone belongs to the defending team, they defuse (stock logic)
		if (zone gameobjects::get_owner_team() == self.team)
			continue;

		if (!isDefined(self.mv_guardZone) || self.mv_guardZone != zone)
		{
			self.mv_guardZone = zone;
			self.mv_guarding = RandomInt(100) < level.mv_cfg.bombGuard;
		}
		if (!self.mv_guarding)
			return false;

		far = Distance2DSquared(self.origin, zone.trigger.origin) > 800 * 800;
		self bot::approach_point(zone.trigger.origin, 150, 600);
		if (far)
			self bot::sprint_to_goal();
		return true;
	}

	self.mv_guardZone = undefined;
	return false;
}

// Fracture: stock bots only deposit when idle (~10% per data piece), so they hoard it. Bots carrying enough go straight to
// the nearest deposit, even mid-fight.
function WrapCleanBots()
{
	if (level.gametype != "clean" || isDefined(level.mv_botPostCombat) || !isDefined(level.botPostCombat))
		return;

	level.mv_botPostCombat = level.botPostCombat;
	level.botPostCombat = &BotCleanPostCombat;
}

function BotCleanPostCombat()
{
	self [[level.mv_botPostCombat]]();

	if (!isDefined(self.carriedTacos) || !isDefined(level.cleanDepositHubs))
		return;

	// Already on the way to a care package (picked up before it had enough data): let it finish, then deposit
	if (IS_TRUE(self.mv_capturingCrate))
		return;

	if (self.carriedTacos < level.mv_cfg.cleanDeposit)
		return;

	// Already heading to a deposit: if a fight or chase changed its goal, send it back once the fight is over
	if (isDefined(self.targetHub))
	{
		if (!self bot_combat::has_threat() &&
			(!self BotGoalSet() || Distance2DSquared(self BotGetGoalPosition(), self.targetHub.trigger.origin) > 300 * 300))
		{
			self bot::path_to_point_in_trigger(self.targetHub.trigger);
			self bot::sprint_to_goal();
		}
		return;
	}

	nearest = undefined;
	nearestDistSq = undefined;
	foreach (hub in level.cleanDepositHubs)
	{
		if (hub.interactTeam != "any")
			continue;

		distSq = Distance2DSquared(self.origin, hub.trigger.origin);
		if (!isDefined(nearest) || distSq < nearestDistSq)
		{
			nearest = hub;
			nearestDistSq = distSq;
		}
	}

	if (!isDefined(nearest))
		return;

	self.targetHub = nearest;
	self.targetTaco = undefined;
	self bot::path_to_point_in_trigger(nearest.trigger);
	self bot::sprint_to_goal();
}

// Dropshot: hold prone (button 8) for a moment while shooting. The think releases all buttons every think, so press
// it every frame (and at the start of each think in BotCombat).
#define BOT_BUTTON_PRONE	8
#define BOT_BUTTON_SPRINT	1
#define BOT_BUTTON_ADS		11

function BotDropShot()
{
	self endon("death");
	self endon("disconnect");

	self.mv_dropshot = true;
	self.mv_maneuvering = true;
	self.mv_holdButtons = array(BOT_BUTTON_PRONE);

	proneTime = RandomFloatRange(0.8, 1.6);
	for (t = 0; t < proneTime; t += 0.05)
	{
		self BotPressButton(BOT_BUTTON_PRONE);
		wait 0.05;
	}

	self.mv_holdButtons = undefined;
	self.mv_maneuvering = undefined;
	self BotReleaseButton(BOT_BUTTON_PRONE);

	wait RandomFloatRange(1.5, 3);
	self.mv_dropshot = undefined;
}

function BotCanManeuver()
{
	return !IS_TRUE(self.mv_jumping) && !IS_TRUE(self.mv_dropshot) && !IS_TRUE(self.mv_sliding) &&
		!isDefined(self.mv_airTarget) && !IS_TRUE(self.mv_usingStreak) && !IS_TRUE(self.mv_capturingCrate) && !self IsRemoteControlling() &&
		self IsOnGround() && !self BotTraversing() && !self IsPlayerSwimming() && !self BotThrowingNade();
}

// Mid-throw: a slide, jump or dropshot now would cancel it
function BotThrowingNade()
{
	return (isDefined(self.mv_throwUntil) && GetTime() < self.mv_throwUntil) || self IsThrowingGrenade();
}

function BotSpeed2D()
{
	velocity = self GetVelocity();
	return Sqrt(velocity[0] * velocity[0] + velocity[1] * velocity[1]);
}

function BotSlide(inCombat)
{
	self endon("death");
	self endon("disconnect");
	level endon("game_ended");

	self.mv_sliding = true;
	self.mv_maneuvering = true;
	manual = false;

	// Bots face the enemy in fights and can only sprint forward: slide at the enemy, a bit off to one side
	if (inCombat)
	{
		dir = TurnDirection(FlatDirection(self.origin, self.bot.threat.lastPosition), RandomIntRange(-35, 36));
		if (!IsPointOnNavMesh(self.origin + dir * 250, self))
		{
			self.mv_maneuvering = undefined;
			wait 1;
			self.mv_sliding = undefined;
			return;
		}
		self BotTakeManualControl();
		manual = true;
		self BotSetMoveAngle(dir);
		self BotSetMoveMagnitude(1);
	}
	else
	{
		// Only with floor ahead: a slide off the walkable area leaves the bot off its path and the stock code turns it
		// around
		velocity = self GetVelocity();
		if (!IsPointOnNavMesh(self.origin + VectorNormalize((velocity[0], velocity[1], 0)) * 250, self))
		{
			self.mv_maneuvering = undefined;
			wait 1;
			self.mv_sliding = undefined;
			return;
		}
	}

	// Slides only start from a full sprint (~210 units/s). Fire and sights stop a sprint, so let go of them, briefly in
	// fights.
	sprintFrames = 16;
	if (inCombat)
		sprintFrames = 10;
	for (i = 0; i < sprintFrames && (!self IsSprinting() || self BotSpeed2D() < 230); i++)
	{
		self bot::press_sprint_button();
		if (inCombat)
		{
			self bot::release_attack_button();
			self bot::release_ads_button();
		}
		wait 0.05;
	}

	sprinted = self IsSprinting();

	// Crouching while sprinting only crouches a bot, the game has its own bot slide (notifies "slide_begin")
	self.mv_slideSeen = false;
	self thread BotWatchSlideBegin();

	if (sprinted && self IsOnGround() && !self BotTraversing())
	{
		cancelAt = -1;
		if (RandomInt(100) < 50)
			cancelAt = RandomIntRange(4, 8);

		self.mv_holdButtons = array(BOT_BUTTON_SPRINT);
		self PressSlideButton();
		for (i = 0; i < 16; i++)
		{
			// In a fight let go of sprint once sliding, it keeps the weapon lowered
			if (inCombat && self.mv_slideSeen)
			{
				if (isDefined(self.mv_holdButtons))
				{
					self.mv_holdButtons = undefined;
					self BotReleaseButton(BOT_BUTTON_SPRINT);
				}
			}
			else
				self BotPressButton(BOT_BUTTON_SPRINT);
			wait 0.05;

			if (!self.mv_slideSeen && i >= 4)
				break;
			if (self.mv_slideSeen && !self IsSliding() && i >= 4)
				break;

			if (i == cancelAt && self.mv_slideSeen)
			{
				self.mv_holdButtons = undefined;
				self BotReleaseButton(BOT_BUTTON_SPRINT);
				self bot::tap_jump_button();
				break;
			}
		}
		self.mv_holdButtons = undefined;
	}
	self notify("mv_slide_done");

	if (manual)
		self BotReleaseManualControl();
	self.mv_maneuvering = undefined;


	if (inCombat)
		wait RandomFloatRange(2, 4);
	else
		wait RandomFloatRange(1, 2.5);
	self.mv_sliding = undefined;
}

function BotWatchSlideBegin()
{
	self endon("death");
	self endon("disconnect");
	self endon("mv_slide_done");

	self waittill("slide_begin");
	self.mv_slideSeen = true;
}

function BotCombatJump()
{
	self endon("death");
	self endon("disconnect");
	level endon("game_ended");

	self.mv_jumping = true;
	self.mv_maneuvering = true;
	self bot::tap_jump_button();
	wait 0.2;
	for (i = 0; i < RandomIntRange(4, 10); i++)
	{
		// A thrust while on a wall or mantling would knock the bot off it
		if (self IsWallRunning() || self IsMantling())
			break;
		self bot::press_doublejump_button();
		wait 0.05;
	}
	self bot::release_doublejump_button();
	self.mv_maneuvering = undefined;

	wait RandomFloatRange(1.5, 3);
	self.mv_jumping = undefined;
}

function WrapBotCombat()
{
	// Domination, Hardpoint and Fracture set their own (objective first, then the stock one), so wrap whatever is set
	if (isDefined(level.botUpdateThreatGoal) && !isDefined(level.mv_botUpdateThreatGoal))
	{
		level.mv_botUpdateThreatGoal = level.botUpdateThreatGoal;
		level.botUpdateThreatGoal = &BotUpdateThreatGoal;
	}

	if (isDefined(level.botPreCombat) && !isDefined(level.mv_botPreCombat))
	{
		level.mv_botPreCombat = level.botPreCombat;
		level.botPreCombat = &BotPreCombatTraversal;
	}

	if (isDefined(level.botThreatLost) && !isDefined(level.mv_botThreatLost))
	{
		level.mv_botThreatLost = level.botThreatLost;
		level.botThreatLost = &BotThreatLost;
	}

	if (isDefined(level.botThreatEngage) && !isDefined(level.mv_botThreatEngage))
	{
		level.mv_botThreatEngage = level.botThreatEngage;
		level.botThreatEngage = &BotEngageThreat;
	}

	if (!isDefined(level.botCombat) || isDefined(level.mv_botCombat))
		return;

	level.mv_botCombat = level.botCombat;
	level.botCombat = &BotCombat;
}

// Wallrunning, mantling or a traversal jump: the stock traversal script steers these, and anything that changes the
// goal or takes the controls in the middle drops the bot off the wall
function BotTraversing()
{
	if (self IsWallRunning() || self IsMantling())
		return true;

	// Traversal jumps run under manual control, the bot's own combat jumps don't
	return !self IsOnGround() && self BotUnderManualControl() && !IS_TRUE(self.mv_usingStreak);
}

function BotNoOp()
{
}

// The stock stuck check (3 still thinks = stuck, then a turn-around) was made for 0.25s thinks: at 0.1s a bot stopping
// for 0.3s counted as stuck and spun. Hold the count back to ~0.75s of standing still.
function BotStuckCheckByTime()
{
	if (!isDefined(self.bot) || !isDefined(self.bot.stuckCycles))
		return;

	if (self.bot.stuckCycles <= 0)
	{
		self.mv_stillSince = undefined;
		return;
	}

	if (!isDefined(self.mv_stillSince))
		self.mv_stillSince = GetTime();

	allowed = int((GetTime() - self.mv_stillSince) / 250);
	if (self.bot.stuckCycles > allowed)
		self.bot.stuckCycles = allowed;
}

// While traversing only reload: the rest of the stock pre combat (turn to damage, switch weapons, killstreaks) waits
function BotPreCombatTraversal()
{
	if (isDefined(self.mv_airTarget))
		return;

	// Domination stops bots whose goal is in a flag we own; going to a fight on our own flag is exactly that
	if (IS_TRUE(self.mv_defending))
		self.bot.goalFlag = undefined;

	if (!self BotTraversing())
	{
		if (self BotPutAwayLauncher())
			return;
		self [[level.mv_botPreCombat]]();
		return;
	}

	if (!self IsReloading() && !self IsSwitchingWeapons() && !self IsThrowingGrenade() && !self IsMeleeing())
		self bot_combat::reload_weapon();
}

// Launcher out (left from shooting a scorestreak, or spawned with it) and an enemy in sight: the stock code only
// switches on an empty clip, and a Blackcell can't fire at players, so the bot just stared. Switch to a gun.
function BotPutAwayLauncher()
{
	current = self GetCurrentWeapon();
	if (!isDefined(current) || !current.isRocketLauncher || !self bot_combat::has_threat() || self IsSwitchingWeapons() ||
		self IsThrowingGrenade() || self IsMeleeing())
		return false;

	// A scorestreak (UAV, turret, AGR...) is what the launcher is for
	if (isDefined(self.bot.threat.entity.killstreakType))
		return false;

	best = undefined;
	bestRank = 0;
	foreach (weapon in self GetWeaponsListPrimaries())
	{
		if (weapon == current || weapon.isRocketLauncher || (!self GetWeaponAmmoClip(weapon) && !self GetWeaponAmmoStock(weapon)))
			continue;

		rank = 3;
		if (weapon.weapClass == "pistol")
			rank = 2;
		else if (weapon.weapClass == "melee")
			rank = 1;
		if (rank > bestRank)
		{
			best = weapon;
			bestRank = rank;
		}
	}
	if (!isDefined(best))
		return false;

	self BotSwitchToWeapon(best);
	return true;
}

// Aim down sights still held from the last think drops a bot off a wall it just landed on
function BotWallrunWatch()
{
	self notify("mv_wallrun_watch");
	self endon("mv_wallrun_watch");
	self endon("death");
	self endon("disconnect");
	level endon("game_ended");

	for (;;)
	{
		self waittill("wallrun_begin");
		self bot::release_ads_button();
	}
}

// The Combat Axe flies straight and kills in one hit, so the stock throw on first sight suits it: on for this bot only
function BotRunCombat()
{
	axe = isDefined(level.botSettings) && isDefined(level.mv_stockLethalWeight) &&
		self BotHasNade(self.grenadeTypePrimary, array("hatchet"));
	if (axe)
		level.botSettings.lethalWeight = level.mv_stockLethalWeight;

	self BotRunCombatStep();

	if (axe)
		level.botSettings.lethalWeight = 0;
}

// While traversing keep shooting, but don't change the goal, throw grenades or melee (it drops the bot off the wall)
function BotRunCombatStep()
{
	if (!self BotTraversing())
	{
		self [[level.mv_botCombat]]();
		return;
	}

	// The combat step starts these as threads that run right away until their first wait, so the swap only affects this
	// bot
	engage = level.botThreatEngage;
	updateGoal = level.botUpdateThreatGoal;
	lost = level.botThreatLost;
	level.botThreatEngage = &BotTraversalEngage;
	level.botUpdateThreatGoal = &BotNoOp;
	level.botThreatLost = &BotNoOp;

	self [[level.mv_botCombat]]();

	level.botThreatEngage = engage;
	level.botUpdateThreatGoal = updateGoal;
	level.botThreatLost = lost;
}

function BotTraversalEngage()
{
	// Aiming down sights drops a wallrunning player off the wall, hip fire only
	self bot::release_ads_button();

	// New target: pick head or body like the stock code does
	if (!self.bot.threat.wasVisible && self.bot.threat.visible)
		self.bot.threat.aimOffset = self bot_combat::get_aim_offset(self.bot.threat.entity);

	// Don't turn while jumping onto a wall, the bot has to face along it to start the run
	if (self IsWallRunning())
		self bot_combat::update_weapon_aim();

	if (self IsReloading() || self IsSwitchingWeapons() || self IsThrowingGrenade() || self IsMeleeing())
		return;

	self bot_combat::fire_weapon();
}

function BotCombat()
{
	self BotStuckCheckByTime();

	// Dropshot / slide in progress: the think just let go of every button, press the stance button again right away
	if (isDefined(self.mv_holdButtons))
	{
		foreach (button in self.mv_holdButtons)
			self BotPressButton(button);
	}

	// The think reads thinkInterval right after this with no wait, so set this bot's value and leave it
	if (isDefined(level.botSettings))
	{
		if (isDefined(self.mv_holdThink))
		{
			// Capturing a care package: don't think (and release use) until the capture is done
			level.botSettings.thinkInterval = self.mv_holdThink;
			self.mv_holdThinkApplied = true;
		}
		else if (isDefined(level.mv_thinkInterval))
			level.botSettings.thinkInterval = level.mv_thinkInterval;
	}

	self BotRunCombat();

	// Every bot's first think lands in the same moment at match start (a crash happened then): spread out the first
	// slide, jump or dropshot over 1.5-5 s
	if (!isDefined(self.mv_settleUntil))
		self.mv_settleUntil = GetTime() + RandomIntRange(1500, 5000);
	settling = GetTime() < self.mv_settleUntil;

	if (!settling && !IS_TRUE(self.mv_jumping) && !IS_TRUE(self.mv_dropshot) && !IS_TRUE(self.mv_sliding) && !IS_TRUE(self.mv_dropshotPlanned) &&
		!isDefined(self.mv_airTarget) && !self BotThrowingNade() &&
		!IS_TRUE(self.mv_usingStreak) && !self IsRemoteControlling() && self bot_combat::has_threat() && self IsOnGround() &&
		RandomInt(100) < level.mv_cfg.combatJump)
	{
		self thread BotCombatJump();
	}

	// Only some bots dropshot, in some of their fights, once the enemy is close
	if (!isDefined(self.pers["mv_dropshotter"]))
	{
		self.pers["mv_dropshotter"] = RandomInt(100) < level.mv_cfg.dropshotBots;
	}

	if (!settling && self.pers["mv_dropshotter"] && self bot_combat::has_threat())
	{
		if (!isDefined(self.mv_dropshotThreat) || self.mv_dropshotThreat != self.bot.threat.entity)
		{
			self.mv_dropshotThreat = self.bot.threat.entity;
			self.mv_dropshotPlanned = RandomInt(100) < level.mv_cfg.combatDropshot;
		}

		if (IS_TRUE(self.mv_dropshotPlanned) && !IS_TRUE(self.mv_jumping) && !IS_TRUE(self.mv_dropshot) && !IS_TRUE(self.mv_sliding) &&
			!IS_TRUE(self.mv_usingStreak) && !self IsRemoteControlling() && self IsOnGround() && !self BotTraversing() && !self IsPlayerSwimming() &&
			!self BotThrowingNade() && IS_TRUE(self.bot.threat.visible) && self.bot.threat.lastDistanceSq < 700 * 700)
		{
			self.mv_dropshotPlanned = undefined;
			self thread BotDropShot();
		}
	}

	// Slides: in fights to close in or get across, otherwise while sprinting somewhere far
	if (!settling && self BotCanManeuver())
	{
		if (self bot_combat::has_threat())
		{
			if (!IS_TRUE(self.mv_dropshotPlanned) && IS_TRUE(self.bot.threat.visible) &&
				self.bot.threat.lastDistanceSq > 250 * 250 && self.bot.threat.lastDistanceSq < 1500 * 1500 &&
				self BotSpeed2D() > 100 && RandomInt(100) < level.mv_cfg.combatSlide)
			{
				self thread BotSlide(true);
			}
		}
		// Already sprinting (the stock "sprint to goal" flag isn't set for most plain movement, so check the sprint
		// itself)
		else if ((self IsSprinting() || IS_TRUE(self.bot.sprintToGoal)) && self BotGoalSet() &&
			Distance2DSquared(self.origin, self BotGetGoalPosition()) > 600 * 600 && self BotSpeed2D() > 150 &&
			RandomInt(100) < level.mv_cfg.moveSlide)
		{
			self thread BotSlide(false);
		}
	}

	if (!settling)
		self BotConsiderAirStreaks();

	if (!settling)
		self BotIdleWatchdog();
}

// Combat movement. The stock threat goal only moves a bot on first sight (toward the enemy, one step sideways once
// close), players strafe the whole fight. Within strafeRange step left or right every 0.4-1s (some steps spent standing
// to aim), only to spots that still see the enemy so a bot doesn't strafe behind a wall, and on the objective only
// inside it. Out of weapon range it closes in first, strafing out there can't hit anything.
function BotUpdateThreatGoal()
{
	if (!self BotStrafeAllowed())
	{
		self.mv_strafeNext = undefined;
		self [[level.mv_botUpdateThreatGoal]]();
		return;
	}

	threat = self.bot.threat;
	if (!IS_TRUE(threat.visible) || !isDefined(threat.aimPoint) || !isDefined(threat.lastPosition) ||
		!isDefined(threat.lastDistanceSq))
	{
		self.mv_strafeNext = undefined;
		self [[level.mv_botUpdateThreatGoal]]();
		return;
	}

	area = self BotObjectiveArea();
	if (!isDefined(area) && !IS_TRUE(threat.inRange))
	{
		self.mv_strafeNext = undefined;
		if (!BotFreeRoamMode() || !self BotCloseIn(threat))
			self [[level.mv_botUpdateThreatGoal]]();
		return;
	}

	if (threat.lastDistanceSq > level.mv_cfg.strafeRange * level.mv_cfg.strafeRange)
	{
		self.mv_strafeNext = undefined;
		self [[level.mv_botUpdateThreatGoal]]();
		return;
	}

	if (isDefined(self.mv_strafeNext) && GetTime() < self.mv_strafeNext)
		return;
	self.mv_strafeNext = GetTime() + RandomIntRange(400, 1000);

	if (RandomInt(100) < level.mv_cfg.strafeHold)
	{
		self BotSetGoal(self.origin);
		self bot::end_sprint_to_goal();
		return;
	}

	if (!self BotStrafeStep(threat, area))
	{
		self.mv_strafeNext = undefined;
		self [[level.mv_botUpdateThreatGoal]]();
	}
}

// Modes without objectives, enemy in sight but out of weapon range: walk to where it is now every second or so. The
// stock code sets that goal once, so the bot stood at the spot staring at an enemy it couldn't hit.
function BotCloseIn(threat)
{
	if (isDefined(self.mv_closeNext) && GetTime() < self.mv_closeNext)
		return true;
	self.mv_closeNext = GetTime() + RandomIntRange(750, 1250);

	// Well inside the weapon's range, not the stock 5-14 m fighting distance (a rifle doesn't need to get that close)
	range = self BotWeaponRange();
	if (!isDefined(range))
		return false;
	radius = int(range * RandomFloatRange(0.5, 0.75));

	if (Distance2DSquared(self.origin, threat.lastPosition) <= radius * radius)
		return true;
	return self BotSetGoal(threat.lastPosition, radius);
}

// Not mid jump, slide, dropshot, wallrun, killstreak or crate, nor tied to a spot (Fracture hub/taco, carrying,
// planting, defusing)
function BotStrafeAllowed()
{
	if (self BotUnderManualControl() || IS_TRUE(self.mv_maneuvering) || IS_TRUE(self.mv_usingStreak) ||
		IS_TRUE(self.mv_capturingCrate) || isDefined(self.mv_airTarget) || self IsRemoteControlling() ||
		!self IsOnGround() || self BotTraversing() || self IsPlayerSwimming() ||
		IS_TRUE(self.isPlanting) || IS_TRUE(self.isDefusing) || isDefined(self.carryObject))
		return false;

	if (isDefined(self.targetHub) || isDefined(self.targetTaco))
		return false;

	// Knife or out of ammo: the stock code runs at the enemy for a melee
	weapon = self GetCurrentWeapon();
	if (weapon.weapClass == "melee" || (!self GetWeaponAmmoClip(weapon) && !self GetWeaponAmmoStock(weapon)))
		return false;

	return true;
}

function BotWeaponRange()
{
	range = bot_combat::weapon_range(self GetCurrentWeapon());
	if (!isDefined(range) || range <= 0)
		return undefined;
	return range;
}

// One strafe step, mostly the other way from the last. area: stay inside it (on the objective). True once it has a
// spot.
function BotStrafeStep(threat, area)
{
	toThreat = FlatDirection(self.origin, threat.lastPosition);
	side = (0 - toThreat[1], toThreat[0], 0);

	dir = 1;
	if (isDefined(self.mv_strafeSide))
		dir = 0 - self.mv_strafeSide;
	if (RandomInt(100) < 30)
		dir = 0 - dir;

	// Edge in on an enemy near the end of the weapon's range, back off a bit from one that's very close, else stay about
	// as far as it is
	forward = RandomIntRange(-30, 31);
	range = self BotWeaponRange();
	if (isDefined(range) && threat.lastDistanceSq > range * range * 0.75 * 0.75)
		forward = RandomIntRange(40, 121);
	else if (isDefined(level.botSettings.threatRadiusMinSq) && threat.lastDistanceSq < level.botSettings.threatRadiusMinSq)
		forward = RandomIntRange(-100, -39);

	// Holding the objective: stay on it rather than closing in or backing off
	if (isDefined(area))
		forward = RandomIntRange(-30, 31);

	// Each side a full step, then each side a short one (room for it on a small hardpoint or flag)
	eyeOffset = self GetEye() - self.origin;
	for (i = 0; i < 4; i++)
	{
		distance = RandomIntRange(64, 161);
		if (i >= 2)
			distance = RandomIntRange(40, 81);

		point = self.origin + side * (dir * distance) + toThreat * forward;
		point = GetClosestPointOnNavMesh(point, 48, 32);
		if (isDefined(point) && Distance2DSquared(point, self.origin) > 32 * 32 &&
			(!isDefined(area) || BotPointInTrigger(point, area)) &&
			SightTracePassed(point + eyeOffset, threat.aimPoint, false, self) && self BotSetGoal(point, 16))
		{
			self.mv_strafeSide = dir;
			self bot::end_sprint_to_goal();
			return true;
		}

		// No room, off the objective or no view of the enemy that way: try the other side
		dir = 0 - dir;
	}

	return false;
}

// Hearing. Stock bots only notice enemies in their field of view or ones that hit them. Gunfire (quieter suppressed)
// and sprinting or sliding (not with Dead Silence, further with Awareness) turn a bot roughly toward the noise, so its
// own eyes pick the enemy up. Unsuppressed fire also shows on the minimap: most bots in modes without objectives go
// after it, in objective modes only fights near the bot's objective draw it in. One noise at a time, only one under
// half as far takes over, never back to an old one.
function BotWatchGunfire()
{
	self notify("mv_gunfire_watch");
	self endon("mv_gunfire_watch");
	self endon("disconnect");
	level endon("game_ended");

	for (;;)
	{
		self waittill("weapon_fired", weapon);

		// One noise per shooter every 0.4s is plenty (automatic fire notifies every shot)
		if (isDefined(self.mv_lastShotHeard) && GetTime() - self.mv_lastShotHeard < 400)
			continue;
		self.mv_lastShotHeard = GetTime();

		if (!isDefined(level.mv_cfg))
			continue;

		if (isDefined(weapon) && WeaponHasAttachment(weapon, "suppressed"))
		{
			BotsHearNoise(self, level.mv_cfg.hearSuppressed, false, false);
			continue;
		}

		self.mv_pingTime = GetTime();
		self.mv_pingPos = self.origin;
		BotsHearNoise(self, level.mv_cfg.hearGunfire, false, true);
	}
}

function BotFootstepLoop()
{
	level endon("game_ended");

	for (;;)
	{
		wait 0.5;
		if (!isDefined(level.mv_cfg) || level.mv_cfg.hearFootsteps <= 0)
			continue;

		foreach (player in level.players)
		{
			if (isAlive(player) && (player IsSprinting() || player IsSliding()) && !player HasPerk("specialty_quieter"))
				BotsHearNoise(player, level.mv_cfg.hearFootsteps, true, false);
		}
	}
}

// Every bot close enough to hear the source, or to see it on the minimap (ping): alive, an enemy of the source
function BotsHearNoise(source, radius, footsteps, ping)
{
	if (IS_TRUE(level.inPrematchPeriod) || IS_TRUE(level.gameEnded) || !isAlive(source))
		return;
	if (radius < 0)
		radius = 0;
	pingRange = 0;
	if (ping)
		pingRange = level.mv_cfg.huntRange;
	if (radius <= 0 && pingRange <= 0)
		return;

	foreach (bot in level.players)
	{
		if (bot == source || !isAlive(bot) || !isDefined(bot.bot) || !bot util::is_bot())
			continue;
		if (level.teambased && bot.team == source.team)
			continue;

		hearing = radius;
		if (footsteps && bot HasPerk("specialty_loudenemies"))
			hearing = radius * 2;
		distSq = DistanceSquared(bot.origin, source.origin);
		heard = distSq <= hearing * hearing;
		seen = distSq <= pingRange * pingRange;
		if (heard || seen)
			bot thread BotHearNoise(source, heard, seen);
	}
}

function BotHearNoise(source, heard, seen)
{
	self endon("death");
	self endon("disconnect");
	level endon("game_ended");

	// Still reacting to a noise or turned toward one just now: only one much closer gets its attention. Not while
	// fighting.
	if ((IS_TRUE(self.mv_hearing) || (isDefined(self.mv_hearUntil) && GetTime() < self.mv_hearUntil)) &&
		!self BotNoiseCloser(source))
		return;
	if (self bot_combat::has_threat() || !self BotCanLook())
		return;
	self.mv_hearUntil = GetTime() + 4000;

	origin = source.origin;

	wait RandomFloatRange(0.25, 0.7);
	if (self bot_combat::has_threat() || !self BotCanLook())
		return;

	// Drop the old noise for good. If the bot was on its way to it, it gets new orders after this one unless this one
	// sends it somewhere.
	newOrders = IS_TRUE(self.mv_defending);
	self notify("mv_noise");
	self endon("mv_noise");

	guess = self BotGuessPosition(origin);
	self.mv_hearing = true;
	self.mv_defending = undefined;
	self.mv_noiseSource = source;
	self.mv_noisePos = guess;
	self.mv_hurt = undefined;
	self thread BotWatchHurt();
	if (heard)
	{
		self BotLookAtPoint(guess + (0, 0, 40));
	}

	if (BotFreeRoamMode())
	{
		if (seen && RandomInt(100) < level.mv_cfg.hunt && self BotHunt(guess, source))
			return;

		if (heard && RandomInt(100) < level.mv_cfg.hearInvestigate)
		{
			point = GetClosestPointOnNavMesh(guess, 200, 100);
			if (self BotNoiseSpot(point) && self BotSetGoal(point, 96))
			{
				self BotRespondTo(point, "checking the noise out", source);
				return;
			}
		}
	}
	else
	{
		// Objective modes: only a fight at the bot's own objective draws it in. On the objective it moves within it.
		area = self BotObjectiveArea();
		if (isDefined(area))
		{
			if (self BotClearObjectiveFrom(guess, area))
				return;
		}
		else
		{
			objective = self BotDefendObjective(guess);
			if (isDefined(objective) && self BotDefendObjectiveFrom(guess, objective, source))
				return;
		}
	}

	if (!heard)
	{
		self BotEndNoise(newOrders);
		return;
	}

	self BotWatchNoise(RandomIntRange(15, 31), newOrders);
}

// A noise less than half as far as the one the bot is reacting to, from someone else
function BotNoiseCloser(source)
{
	if (!isDefined(self.mv_noisePos) || (isDefined(self.mv_noiseSource) && source == self.mv_noiseSource))
		return false;

	return Distance2DSquared(self.origin, source.origin) * 4 < Distance2DSquared(self.origin, self.mv_noisePos);
}

// Getting shot ends the reaction, the bot turns to deal with whoever is shooting it
function BotWatchHurt()
{
	self endon("death");
	self endon("disconnect");
	self endon("mv_noise");
	self endon("mv_noise_end");

	self waittill("damage");
	self.mv_hurt = true;
}

// Shot by an enemy it can't see: turn toward them (roughly, like a hit marker), then it fights them as usual if they're
// there. Stock only reacts with no enemy in mind, and then just wanders that way.
function BotTurnToDamage()
{
	self endon("death");
	self endon("disconnect");
	level endon("game_ended");

	for (;;)
	{
		self waittill("damage", amount, attacker);
		if (!isDefined(attacker) || !isPlayer(attacker) || attacker == self ||
			(level.teambased && attacker.team == self.team))
			continue;
		if (self BotSeesThreat() || !self BotCanLook())
			continue;

		self BotDropNoise();
		guess = self BotGuessPosition(attacker.origin) + (0, 0, 40);
		for (i = 0; i < 20 && !self BotSeesThreat() && self BotCanLook(); i++)
		{
			self BotLookAtPoint(guess);
			wait 0.05;
		}
		if (!self BotSeesThreat() && self BotCanLook())
			self BotLookForward();
	}
}

// Something more important came up: drop the noise without new orders (the caller gives the bot its own)
function BotDropNoise()
{
	self notify("mv_noise");
	self notify("mv_noise_end");
	self.mv_hearing = undefined;
	self.mv_defending = undefined;
	self.mv_noiseSource = undefined;
	self.mv_noisePos = undefined;
}

// Done with the noise: look ahead again, and if the bot went somewhere for it, new orders (unless fighting or shot)
function BotEndNoise(newOrders)
{
	self notify("mv_noise_end");
	self.mv_hearing = undefined;
	self.mv_defending = undefined;
	self.mv_noiseSource = undefined;
	self.mv_noisePos = undefined;
	if (self bot_combat::has_threat())
		return;

	if (self BotCanLook())
		self BotLookForward();
	if (!newOrders || IS_TRUE(self.mv_hurt) || IS_TRUE(level.inPrematchPeriod) || !isDefined(level.botIdle))
		return;

	self [[level.botIdle]]();
}

// Only roughly where it is, the further away the less sure
function BotGuessPosition(origin)
{
	spread = Distance2D(self.origin, origin) * 0.15;
	if (spread < 16)
		spread = 16;
	else if (spread > 250)
		spread = 250;

	return origin + (RandomFloatRange(0 - spread, spread), RandomFloatRange(0 - spread, spread), 0);
}

// Modes without objectives: go after an enemy seen firing on the minimap. False if there's no way there.
function BotHunt(guess, source)
{
	point = GetClosestPointOnNavMesh(guess, 200, 100);
	if (!self BotNoiseSpot(point) || !self BotSetGoal(point, 128))
		return false;

	self BotRespondTo(point, "enemy on the minimap", source);
	return true;
}

// Not right where the bot already is: whoever made it is about to show up, so just turn that way and carry on
function BotNoiseSpot(point)
{
	return isDefined(point) && Distance2DSquared(self.origin, point) > 200 * 200;
}

// The source is still showing on the minimap: fired unsuppressed in the last 3s
function BotPinged(source)
{
	return isDefined(source) && isAlive(source) && isDefined(source.mv_pingTime) && GetTime() - source.mv_pingTime < 3000;
}

// Keep looking toward the noise (frames x 0.1s) until the bot sees someone or gets shot
function BotWatchNoise(frames, newOrders)
{
	for (i = 0; i < frames && !self bot_combat::has_threat() && !IS_TRUE(self.mv_hurt); i++)
		wait 0.1;

	self BotEndNoise(newOrders);
}

// The objective area a bot is standing on: the hardpoint it's in, or the Domination flag it's capturing
function BotObjectiveArea()
{
	if (level.gametype == "koth" && isDefined(level.zone) && isDefined(level.zone.gameobject) &&
		isDefined(level.zone.gameobject.trigger) && self IsTouching(level.zone.gameobject.trigger))
		return level.zone.gameobject.trigger;

	if (isDefined(self.bot.capturingFlag) && isDefined(self.bot.capturingFlag.trigger))
		return self.bot.capturingFlag.trigger;

	return undefined;
}

// The objective the bot is going to or holding (the real goal while on a detour), if the noise is within
// hearDefend of it. Undefined for a carrier or a bot with no goal.
function BotDefendObjective(guess)
{
	if (level.mv_cfg.hearDefend <= 0 || isDefined(self.carryObject) || !self BotGoalSet())
		return undefined;

	objective = self BotGetGoalPosition();
	if (isDefined(self.mv_routePoint) && DistanceSquared(objective, self.mv_routePoint) < 1)
		objective = self.mv_routeGoal;

	if (Distance2DSquared(guess, objective) > level.mv_cfg.hearDefend * level.mv_cfg.hearDefend)
		return undefined;

	return objective;
}

// Go toward a fight near the objective, no further than hearDefend from it. False if there's no way there.
function BotDefendObjectiveFrom(guess, objective, source)
{
	leash = level.mv_cfg.hearDefend + 200;
	point = GetClosestPointOnNavMesh(guess, 200, 100);
	if (!self BotNoiseSpot(point) || Distance2DSquared(point, objective) > leash * leash || !self BotSetGoal(point, 128))
		return false;

	self BotRespondTo(point, "fight near the objective", source, objective);
	return true;
}

// On the objective: an enemy around it contests it, maybe just around a corner. Move to the spot inside the area
// nearest the noise to get eyes on them and keep capturing. False if there's none.
function BotClearObjectiveFrom(guess, area)
{
	if (level.mv_cfg.hearDefend <= 0 ||
		Distance2DSquared(guess, area.origin) > level.mv_cfg.hearDefend * level.mv_cfg.hearDefend)
		return false;

	queryResult = PositionQuery_Source_Navigation(guess, 0, 300, 150, 48, self);
	if (!isDefined(queryResult) || !queryResult.data.size)
		return false;

	best = undefined;
	bestDistSq = undefined;
	foreach (spot in queryResult.data)
	{
		if (!BotPointInTrigger(spot.origin, area))
			continue;
		distSq = Distance2DSquared(spot.origin, guess);
		if (!isDefined(best) || distSq < bestDistSq)
		{
			best = spot.origin;
			bestDistSq = distSq;
		}
	}

	if (!isDefined(best) || Distance2DSquared(best, self.origin) < 48 * 48 || !self BotSetGoal(best, 24))
		return false;

	self BotRespondTo(best, "enemy on the objective");
	return true;
}

// Heading to the noise (goal already set to point). Whoever is there is fought as usual, if no one the bot looks around
// a moment, then gets new orders from the mode (back to the objective, or wherever it's needed now). While the source
// keeps showing on the minimap, follow it (no further than hearDefend from the objective). Sprint there facing the
// way it goes, slowing to face the noise for the last huntSlow units.
function BotRespondTo(point, reason, source, objective)
{
	self.mv_defending = true;
	self.mv_noisePos = point;
	facing = undefined;

	// On the way, until it gets there or sees someone (time to get there at a walk, and some), at most 45s
	thrown = false;
	lastPing = undefined;
	if (BotPinged(source))
		lastPing = source.mv_pingPos;
	endTime = GetTime() + int(Distance2D(self.origin, point) / 150 * 1000) + 3000;
	for (i = 0; i < 450 && GetTime() < endTime && !self bot_combat::has_threat() && !IS_TRUE(self.mv_hurt); i++)
	{
		// In throwing range of the noise: maybe a grenade first (one roll per noise)
		if (!thrown && i % 5 == 0)
		{
			spot = point;
			if (BotPinged(source))
				spot = source.mv_pingPos;
			distSq = Distance2DSquared(self.origin, spot);
			if (distSq > 450 * 450 && distSq < 1000 * 1000)
			{
				thrown = true;
				if (RandomInt(100) < level.mv_cfg.nades && self BotNadeReady())
				{
					targets = [];
					guess = GetClosestPointOnNavMesh(self BotGuessPosition(spot), 100, 64);
					if (isDefined(guess))
						targets[targets.size] = guess;
					targets[targets.size] = spot;
					if (self BotThrowNade(targets))
					{
						facing = undefined;
						endTime = GetTime() + int(Distance2D(self.origin, point) / 150 * 1000) + 3000;
					}
				}
			}
		}

		if (i % 5 == 0 && BotPinged(source) && isDefined(lastPing))
		{
			// Showing somewhere else now, or not where the bot thought: a closer look, closer guess
			reached = self BotGoalReached();
			if (reached || Distance2DSquared(source.mv_pingPos, lastPing) > 200 * 200)
			{
				lastPing = source.mv_pingPos;
				if (self BotFollowPing(source, objective))
				{
					point = self BotGetGoalPosition();
					self.mv_noisePos = point;
					endTime = GetTime() + int(Distance2D(self.origin, point) / 150 * 1000) + 3000;
					continue;
				}
			}
		}

		if (self BotGoalReached() && !BotPinged(source))
			break;

		// After a glance toward the noise (0.5s)
		close = Distance2DSquared(self.origin, point) < level.mv_cfg.huntSlow * level.mv_cfg.huntSlow;
		if (i >= 5 && (!isDefined(facing) || facing != close) && self BotCanLook())
		{
			facing = close;
			if (close)
			{
				self bot::end_sprint_to_goal();
				self BotLookAtPoint(point + (0, 0, 40));
			}
			else
			{
				self BotLookForward();
				self bot::sprint_to_goal();
			}
		}
		else if (IS_TRUE(facing) && self BotCanLook())
			self BotLookAtPoint(point + (0, 0, 40));
		wait 0.1;
	}
	self bot::end_sprint_to_goal();

	lookFrames = RandomIntRange(5, 16);
	for (i = 0; i < lookFrames && !self bot_combat::has_threat() && !IS_TRUE(self.mv_hurt); i++)
		wait 0.1;

	// Someone there: let the fight run its course (at most 30s, after that the usual idle check takes over)
	for (i = 0; i < 300 && self bot_combat::has_threat(); i++)
		wait 0.1;

	// Then back to the objective, never back after the noise
	self BotEndNoise(true);
}

// New goal near where the source shows on the minimap now (still within reach of the objective, if any)
function BotFollowPing(source, objective)
{
	guess = self BotGuessPosition(source.mv_pingPos);
	point = GetClosestPointOnNavMesh(guess, 200, 100);
	if (!isDefined(point))
		return false;
	if (isDefined(objective))
	{
		leash = level.mv_cfg.hearDefend + 200;
		if (Distance2DSquared(point, objective) > leash * leash)
			return false;
	}
	if (!self BotSetGoal(point, 128))
		return false;

	return true;
}

// Not wallrunning or mantling (steered by the look), sliding, jumping, in a killstreak, crate or grenade throw
function BotCanLook()
{
	return self BotFreeToLook() && !(isDefined(self.mv_throwUntil) && GetTime() < self.mv_throwUntil);
}

function BotFreeToLook()
{
	return !self BotUnderManualControl() && !IS_TRUE(self.mv_maneuvering) && !IS_TRUE(self.mv_usingStreak) &&
		!IS_TRUE(self.mv_capturingCrate) && !isDefined(self.mv_airTarget) && !self IsRemoteControlling() &&
		!self BotTraversing() && !self IsPlayerSwimming() && !IS_TRUE(self.isPlanting) && !IS_TRUE(self.isDefusing);
}

// Grenades. Stock bots throw one the moment they first see an enemy, which only stops them shooting and gives the
// enemy time to react (only the Combat Axe still does, see BotRunCombat). Instead a bot throws where an enemy just
// ducked out of sight, or toward a noise it's going after (at most one every 8s). Only along an arc that's clear all the
// way and at least 9m out, so nothing bounces back.
function BotThreatLost()
{
	// While a bot remembers an enemy it lost sight of, stock only takes a new one within threatRadiusMin (5-14m), so it
	// ignored others in plain view, even ones shooting it or a teammate next to it
	if (self bot_combat::get_new_threat())
	{
		self.mv_heldThreat = undefined;
		return;
	}

	threat = self.bot.threat;
	if (IS_TRUE(threat.wasVisible) && !IS_TRUE(threat.visible) && isDefined(threat.entity) &&
		isDefined(threat.lastPosition))
	{
		self thread BotNadeLostThreat(threat.entity, threat.lastPosition, threat.lastVelocity);

		// Stock clears the bot's aim here and sprints it to where the enemy was, looking where it runs, so an enemy
		// peeking straight back out got the first shot. Players hold that angle for a moment and walk up on it.
		if (level.mv_cfg.holdAngle > 0 && self BotFreeToLook())
		{
			self.mv_heldThreat = threat.entity;
			self.mv_heldSince = GetTime();
			self bot::end_sprint_to_goal();
			self BotSetGoal(threat.lastPosition);
		}
	}

	if (self BotHoldingAngle())
	{
		if (self BotCanLook())
			self BotLookAtPoint(threat.lastPosition + threat.aimOffset);
		return;
	}

	if (isDefined(self.mv_heldThreat))
	{
		// Held long enough: go after them like stock does
		self.mv_heldThreat = undefined;
		self bot_combat::clear_threat_aim();
		self bot::sprint_to_goal();
	}

	self [[level.mv_botThreatLost]]();
}

function BotHoldingAngle()
{
	return isDefined(self.mv_heldThreat) && self bot_combat::has_threat() && self.mv_heldThreat == self.bot.threat.entity &&
		GetTime() - self.mv_heldSince < level.mv_cfg.holdAngle && self BotFreeToLook();
}

// An enemy that shows up again while the bot holds the angle they left at: it's already looking there, so it reacts in
// repeekReaction % of its first-sight aim delay instead of the full delay and aim error, and doesn't fire before
function BotEngageThreat()
{
	threat = self.bot.threat;
	if (IS_TRUE(threat.visible) && self BotHoldingAngle())
	{
		self.mv_heldThreat = undefined;
		aimDelay = 0;
		if (isDefined(level.botSettings) && isDefined(level.botSettings.aimDelay))
			aimDelay = level.botSettings.aimDelay;
		self.mv_repeekThreat = threat.entity;
		self.mv_repeekUntil = GetTime() + int(aimDelay * 1000 * level.mv_cfg.repeekReaction / 100);
		threat.aimStartTime = self.mv_repeekUntil;
		threat.aimTime = 0;
		threat.aimError = undefined;
	}
	else if (isDefined(self.mv_heldThreat) && !IS_TRUE(threat.wasVisible) && IS_TRUE(threat.visible))
	{
		// Seen again just after the hold ran out, before the lost check cleared the aim: a fresh first sight
		self.mv_heldThreat = undefined;
		self bot_combat::clear_threat_aim();
	}

	if (isDefined(self.mv_repeekUntil))
	{
		if (GetTime() < self.mv_repeekUntil && IS_TRUE(threat.visible) && threat.entity == self.mv_repeekThreat)
			return;
		self.mv_repeekUntil = undefined;
		self.mv_repeekThreat = undefined;
	}

	self [[level.mv_botThreatEngage]]();
}

function BotNadeLostThreat(enemy, lastPosition, lastVelocity)
{
	self endon("death");
	self endon("disconnect");
	level endon("game_ended");

	if (RandomInt(100) >= level.mv_cfg.nades || !self BotNadeReady())
		return;

	// A moment to decide; not if they show up again meanwhile
	wait RandomFloatRange(0.3, 0.8);
	if (!isAlive(enemy) || !self bot_combat::has_threat() || self.bot.threat.entity != enemy ||
		IS_TRUE(self.bot.threat.visible) || !self BotNadeReady())
		return;

	// Where they were headed, a bit past where they disappeared, else right there
	targets = [];
	if (isDefined(lastVelocity))
	{
		ahead = GetClosestPointOnNavMesh(lastPosition + (lastVelocity[0], lastVelocity[1], 0) * 0.4, 64, 48);
		if (isDefined(ahead))
			targets[targets.size] = ahead;
	}
	targets[targets.size] = lastPosition;
	self BotThrowNade(targets);
}

function BotNadeReady()
{
	// Reloading doesn't matter, a throw cuts a reload short like it does for players
	return level.mv_cfg.nades > 0 &&
		(!isDefined(self.mv_nadeNext) || GetTime() >= self.mv_nadeNext) && isDefined(self BotNadeWeapon()) &&
		!isDefined(self.carryObject) && self BotCanLook() && !self IsThrowingGrenade() && !self IsSwitchingWeapons() &&
		!self IsMeleeing() && self IsOnGround() && !self IsSliding() && !IS_TRUE(self.mv_jumping) && !IS_TRUE(self.mv_dropshot);
}

function BotNadeWeapon()
{
	if (self BotHasNade(self.grenadeTypePrimary, array("frag_grenade", "sticky_grenade", "incendiary_grenade")))
		return self.grenadeTypePrimary;
	if (self BotHasNade(self.grenadeTypeSecondary, array("flash_grenade", "concussion_grenade")))
		return self.grenadeTypeSecondary;

	return undefined;
}

function BotHasNade(weapon, names)
{
	if (!isDefined(weapon) || !isDefined(weapon.rootWeapon) || self GetWeaponAmmoStock(weapon) <= 0)
		return false;

	return IsInArray(names, weapon.rootWeapon.name);
}

// Throw at the first target with a clear arc (rechecked while turning, the bot may be moving). False if nothing was
// thrown (no arc, not turned in time, or saw someone: the fight comes first). If the bot was headed there (or hold), it
// waits for the grenade to go off before going on.
function BotThrowNade(targets, hold)
{
	weapon = self BotNadeWeapon();
	if (!isDefined(weapon))
		return false;

	target = undefined;
	launch = undefined;
	foreach (point in targets)
	{
		if (!isDefined(point) || Distance2DSquared(self.origin, point) < 350 * 350)
			continue;
		launch = self BotNadeArc(self GetEye(), point);
		if (isDefined(launch))
		{
			target = point;
			break;
		}
	}
	if (!isDefined(launch))
	{
		return false;
	}

	// A deadline rather than a flag, so nothing stays blocked if this gets cut off
	self.mv_throwUntil = GetTime() + 3000;
	self.mv_nadeNext = GetTime() + 8000;
	sprinting = IS_TRUE(self.bot.sprintToGoal);
	self bot::end_sprint_to_goal();

	aimed = false;
	thrown = false;
	for (i = 0; i < 24 && isDefined(launch) && !self BotNadeInterrupted(); i++)
	{
		self BotSetLookAnglesFromPoint(self GetEye() + launch * 500);
		wait 0.05;
		launch = self BotNadeArc(self GetEye(), target);
		if (isDefined(launch) && VectorDot(AnglesToForward(self GetPlayerAngles()), launch) > 0.998)
		{
			aimed = true;
			break;
		}
	}

	// Throw, holding the aim until it's out of the hand (0.6s, longer while the throw lasts, 1.5s at most)
	if (aimed)
	{
		button = 15;
		if (weapon == self.grenadeTypePrimary)
			button = 14;
		ammo = self GetWeaponAmmoStock(weapon);
		aim = self GetEye() + launch * 500;
		for (i = 0; i < 3; i++)
		{
			self BotPressButton(button);
			self BotSetLookAnglesFromPoint(aim);
			wait 0.05;
		}
		for (i = 0; i < 27 && (i < 9 || self IsThrowingGrenade()); i++)
		{
			self BotSetLookAnglesFromPoint(aim);
			wait 0.05;
		}

		thrown = self GetWeaponAmmoStock(weapon) < ammo;
	}

	self.mv_throwUntil = undefined;
	if (self BotNadeInterrupted())
		return thrown;

	self BotLookForward();
	if (thrown && self BotGoalSet() &&
		(IS_TRUE(hold) || Distance2DSquared(self BotGetGoalPosition(), target) < 400 * 400))
		self BotNadeHold(weapon, target);
	if (sprinting && self BotGoalSet())
		self bot::sprint_to_goal();

	return thrown;
}

// Wait only as long as the bot would otherwise run into it (sprint ~250 units/s, blast ~300 units), watching the spot
function BotNadeHold(weapon, target)
{
	danger = 3.5;
	if (weapon.rootWeapon.name == "sticky_grenade")
		danger = 2.5;
	else if (weapon.rootWeapon.name == "incendiary_grenade")
		danger = 5.5;
	else if (weapon == self.grenadeTypeSecondary)
		danger = 1.5;
	frames = int((danger - (Distance2D(self.origin, target) - 300) / 250) * 10);
	if (frames <= 0)
		return;

	goal = self BotGetGoalPosition();
	radius = self BotGetGoalRadius();
	spot = self.origin;
	if (!self BotSetGoal(spot))
		return;

	// A deadline, not a flag, so a hold that gets cut off can't keep the bot standing there
	self.mv_nadeHoldUntil = GetTime() + frames * 100 + 500;
	if (self BotCanLook())
		self BotLookAtPoint(target + (0, 0, 40));
	for (i = 0; i < frames && !self BotSeesThreat() && self BotGoalSet() &&
		DistanceSquared(self BotGetGoalPosition(), spot) < 1; i++)
		wait 0.1;
	self.mv_nadeHoldUntil = undefined;

	if (self BotSeesThreat())
		return;
	if (self BotCanLook())
		self BotLookForward();
	if (self BotGoalSet() && DistanceSquared(self BotGetGoalPosition(), spot) < 1)
		self BotSetGoal(goal, radius);
}

function BotNadeInterrupted()
{
	return self BotSeesThreat() || !self BotFreeToLook();
}

function BotSeesThreat()
{
	return self bot_combat::has_threat() && IS_TRUE(self.bot.threat.visible);
}

// Domination / Hardpoint: on the way to a flag or hardpoint the enemy holds or is on (the HUD shows it), now and then
// throw a grenade onto it from 15-25m first. Once per objective per life.
function BotObjectiveNadeWatch()
{
	self endon("death");
	self endon("disconnect");
	level endon("game_ended");

	if (level.gametype != "dom" && level.gametype != "koth")
		return;

	rolled = [];
	for (;;)
	{
		wait 0.5;
		if (level.mv_cfg.objectiveNades <= 0 || IS_TRUE(level.inPrematchPeriod) || self bot_combat::has_threat() ||
			IS_TRUE(self.mv_hearing))
			continue;

		object = self BotNadeObjective();
		if (!isDefined(object) || self IsTouching(object.trigger))
			continue;
		center = object.trigger.origin;
		distSq = Distance2DSquared(self.origin, center);
		if (distSq < 600 * 600 || distSq > 1000 * 1000 || !self BotObjectiveHostile(object))
			continue;

		// One roll per objective (a moved hardpoint is a new one)
		seen = false;
		foreach (point in rolled)
		{
			if (DistanceSquared(point, center) < 1)
				seen = true;
		}
		if (seen)
			continue;
		rolled[rolled.size] = center;
		if (RandomInt(100) >= level.mv_cfg.objectiveNades || !self BotNadeReady())
			continue;

		// A few random spots on it, then its middle
		targets = [];
		queryResult = PositionQuery_Source_Navigation(center, 0, 200, 100, 48, self);
		if (isDefined(queryResult))
		{
			foreach (spot in array::randomize(queryResult.data))
			{
				if (targets.size >= 3)
					break;
				if (BotPointInTrigger(spot.origin, object.trigger))
					targets[targets.size] = spot.origin;
			}
		}
		middle = GetClosestPointOnNavMesh(center, 200, 100);
		if (isDefined(middle))
			targets[targets.size] = middle;

		self BotThrowNade(targets, true);
	}
}

// The flag the bot is going to, or the hardpoint
function BotNadeObjective()
{
	if (level.gametype == "koth")
	{
		if (isDefined(level.zone) && isDefined(level.zone.gameobject) && isDefined(level.zone.gameobject.trigger))
			return level.zone.gameobject;
		return undefined;
	}

	if (isDefined(self.bot.goalFlag) && isDefined(self.bot.goalFlag.trigger))
		return self.bot.goalFlag;
	return undefined;
}

// Held by the other team, or someone from it is on it
function BotObjectiveHostile(object)
{
	owner = object gameobjects::get_owner_team();
	if (isDefined(owner) && owner != self.team && owner != "neutral" && owner != "none")
		return true;

	if (!isDefined(object.numTouching))
		return false;
	foreach (team in level.teams)
	{
		if (team != self.team && isDefined(object.numTouching[team]) && object.numTouching[team] > 0)
			return true;
	}
	return false;
}

// Unit direction to throw from start to land on target: straight, else a lob over cover. Undefined if out of range or
// neither path is clear. Uses the stock bot throw speed.
function BotNadeArc(start, target)
{
	speed = 928.0;
	gravity = GetDvarFloat("bg_gravity");
	if (gravity <= 0)
		gravity = 800.0;

	dist = Distance2D(start, target);
	if (dist < 1)
		return undefined;
	height = target[2] - start[2];
	disc = speed * speed * speed * speed - gravity * (gravity * dist * dist + 2 * height * speed * speed);
	if (disc < 0)
		return undefined;

	dir = VectorNormalize((target[0] - start[0], target[1] - start[1], 0));
	root = Sqrt(disc);
	foreach (slope in array((speed * speed - root) / (gravity * dist), (speed * speed + root) / (gravity * dist)))
	{
		flat = 1 / Sqrt(1 + slope * slope);
		launch = dir * flat + (0, 0, slope * flat);
		if (self BotNadePathClear(start, launch * speed, gravity, target))
			return launch;
	}

	return undefined;
}

// Trace the flight in 0.1s steps until it's down by the target
function BotNadePathClear(start, velocity, gravity, target)
{
	time = Distance2D(start, target) / Distance2D((0, 0, 0), velocity);
	steps = int(time / 0.1) + 1;
	prev = start;
	for (i = 1; i <= steps; i++)
	{
		t = time * i / steps;
		point = start + velocity * t - (0, 0, 0.5 * gravity * t * t);
		if (DistanceSquared(point, target) < 96 * 96)
			return true;
		if (!BulletTracePassed(prev, point, false, self))
			return false;
		prev = point;
	}

	return true;
}

// Modes where bots roam around looking for enemies (no objective to go to)
function BotFreeRoamMode()
{
	return level.gametype == "tdm" || level.gametype == "dm" || level.gametype == "conf" || level.gametype == "gun" ||
		level.gametype == "sas" || level.gametype == "shrp";
}

// Routes: the game paths far objective goals the shortest way, so bots from one side always took the same route. For
// some far goals, go by a spot off to one side first.
function WrapBotRoutes()
{
	if (isDefined(level.mv_botIdleRoutes) || !isDefined(level.botIdle))
		return;

	level.mv_botIdleRoutes = level.botIdle;
	level.botIdle = &BotIdleWithRoutes;
}

function BotIdleWithRoutes()
{
	oldGoal = undefined;
	if (self BotGoalSet())
		oldGoal = self BotGetGoalPosition();

	self [[level.mv_botIdleRoutes]]();
	self BotMaybeDetour(oldGoal);
}

// New far goal: maybe a detour. Not carrying, defending in CTF (its patrol picks its own spots) or busy.
function BotMaybeDetour(oldGoal)
{
	if (level.mv_cfg.routeVary <= 0 || !self BotGoalSet() || self bot_combat::has_threat() || isDefined(self.carryObject) ||
		IS_TRUE(self.mv_ctfDefender) || IS_TRUE(self.mv_maneuvering) || self BotTraversing())
		return;

	goal = self BotGetGoalPosition();
	if (isDefined(oldGoal) && DistanceSquared(goal, oldGoal) < 1)
		return;
	if (Distance2DSquared(self.origin, goal) < level.mv_cfg.routeMin * level.mv_cfg.routeMin)
		return;

	// Once per place: after a detour the mode often hands out the same goal again
	if (isDefined(self.mv_detourGoal) && GetTime() - self.mv_detourTime < 30000 &&
		Distance2DSquared(goal, self.mv_detourGoal) < 600 * 600)
		return;
	self.mv_detourGoal = goal;
	self.mv_detourTime = GetTime();

	if (RandomInt(100) < level.mv_cfg.routeVary)
		self BotTryDetour(goal, self BotGetGoalRadius());
}

// A spot in the first half of the way, a quarter to half the distance out to the side (at most 1200 units), and not a
// long way round (at most 1.5x the straight distance)
function BotTryDetour(goal, radius)
{
	distance = Distance2D(self.origin, goal);
	toGoal = FlatDirection(self.origin, goal);
	side = (0 - toGoal[1], toGoal[0], 0);
	if (RandomInt(2) == 0)
		side = (0, 0, 0) - side;

	out = distance * RandomFloatRange(0.25, 0.5);
	if (out > 1200)
		out = 1200;
	center = self.origin + toGoal * (distance * RandomFloatRange(0.3, 0.55)) + side * out;

	sprint = IS_TRUE(self.bot.sprintToGoal);
	queryResult = PositionQuery_Source_Navigation(center, 0, 400, 150, 96, self);
	if (isDefined(queryResult) && queryResult.data.size)
	{
		points = array::randomize(queryResult.data);
		for (i = 0; i < points.size && i < 6; i++)
		{
			point = points[i].origin;
			if (Distance2D(self.origin, point) + Distance2D(point, goal) > distance * 1.5)
				continue;

			if (self BotSetGoal(point, 64))
			{
				if (sprint)
					self bot::sprint_to_goal();

				// Where the bot is really headed, for as long as its goal is this detour spot
				self.mv_routePoint = point;
				self.mv_routeGoal = goal;
				self thread BotFollowDetour(point, goal, radius, sprint);
				return;
			}
		}
	}

	// No good spot: keep the goal the mode gave
	if (!self BotGoalSet() || Distance2DSquared(self BotGetGoalPosition(), goal) > 1)
		self BotSetGoal(goal, radius);
}

// Once at the detour spot (or after a while), on to the real goal. A fight or any other new goal ends the detour.
function BotFollowDetour(point, goal, radius, sprint)
{
	self notify("mv_detour");
	self endon("mv_detour");
	self endon("death");
	self endon("disconnect");
	level endon("game_ended");


	endTime = GetTime() + int(Distance2D(self.origin, point) / 150 * 1000) + 5000;
	while (GetTime() < endTime)
	{
		wait 0.25;
		if (self bot_combat::has_threat() || (self BotGoalSet() && Distance2DSquared(self BotGetGoalPosition(), point) > 1))
			return;
		if (!self BotGoalSet() || self BotGoalReached() || Distance2DSquared(self.origin, point) < 96 * 96)
			break;
	}

	if (self bot_combat::has_threat() || (self BotGoalSet() && Distance2DSquared(self BotGetGoalPosition(), point) > 1))
		return;

	if (self BotSetGoal(goal, radius) && sprint)
		self bot::sprint_to_goal();
}

// Stock bots only get new orders with no goal at all, so one that reached its goal stands there until an enemy shows
// up. After 2.5 s still with nothing going on, ask the mode for new orders.
function BotIdleWatchdog()
{
	if (!isDefined(level.botIdle) || IS_TRUE(level.inPrematchPeriod) || self bot_combat::has_threat() ||
		IS_TRUE(self.mv_maneuvering) || IS_TRUE(self.mv_usingStreak) || IS_TRUE(self.mv_capturingCrate) ||
		isDefined(self.mv_airTarget) || isDefined(self.mv_defNext) || IS_TRUE(self.mv_dropshot) ||
		(isDefined(self.mv_nadeHoldUntil) && GetTime() < self.mv_nadeHoldUntil) || self BotTraversing() ||
		self IsRemoteControlling() || self BotSpeed2D() > 20 ||
		IS_TRUE(self.isPlanting) || IS_TRUE(self.isDefusing) || self UseButtonPressed())
	{
		self.mv_idleSince = undefined;
		return;
	}

	if (!isDefined(self.mv_idleSince))
	{
		self.mv_idleSince = GetTime();
		return;
	}

	if (GetTime() - self.mv_idleSince < 2500)
		return;

	self.mv_idleSince = undefined;

	// Still on the spot where it got its last orders: they didn't get it moving
	if (isDefined(self.mv_idleSpot) && DistanceSquared(self.origin, self.mv_idleSpot) < 48 * 48)
		self.mv_idleTries++;
	else
		self.mv_idleTries = 0;
	self.mv_idleSpot = self.origin;

	// Holding the hardpoint: standing in it is fine, a new spot in it is all it needs
	inZone = level.gametype == "koth" && isDefined(level.zone) && isDefined(level.zone.gameobject) &&
		isDefined(level.zone.gameobject.trigger) && self IsTouching(level.zone.gameobject.trigger);
	onMesh = IsPointOnNavMesh(self.origin, self);

	// A bot can only path from the walkable area: a jump, wallrun or slide that leaves it off it makes every new goal
	// fail. Only counts for a bot that should be going somewhere.
	goingNowhere = self.mv_idleTries >= 2 && (!self BotGoalSet() || !self BotGoalReached());
	if (!inZone && !self IsPlayerSwimming() && ((!onMesh && self.mv_idleTries >= 1) || goingNowhere))
	{
		self thread BotUnstick(onMesh);
		return;
	}

	self [[level.botIdle]]();
}

// Back onto the walkable area if off it, else to a spot nearby, then new orders
function BotUnstick(onMesh)
{
	self endon("death");
	self endon("disconnect");
	level endon("game_ended");

	self.mv_maneuvering = true;
	self.mv_idleTries = 0;

	target = undefined;
	if (!onMesh)
		target = GetClosestPointOnNavMesh(self.origin, 400);

	if (onMesh)
	{
		queryResult = PositionQuery_Source_Navigation(self.origin, 150, 500, 150, 64, self);
		points = array::randomize(queryResult.data);
		for (i = 0; i < points.size && i < 6; i++)
		{
			if (self BotSetGoal(points[i].origin, 32))
			{
				self bot::sprint_to_goal();
				self.mv_maneuvering = undefined;
				return;
			}
		}
	}

	// Walk there by hand (or anywhere if there's nothing), with a jump to get over whatever is in the way
	dir = AnglesToForward((0, RandomInt(360), 0));
	if (isDefined(target) && Distance2DSquared(self.origin, target) > 1)
		dir = FlatDirection(self.origin, target);

	self BotTakeManualControl();
	for (i = 0; i < 12 && !self bot_combat::has_threat(); i++)
	{
		self BotSetMoveAngle(dir);
		self BotSetMoveMagnitude(1);
		if (i == 4)
			self bot::tap_jump_button();
		wait 0.1;
		if (isDefined(target) && IsPointOnNavMesh(self.origin, self))
			break;
	}
	self BotReleaseManualControl();
	self.mv_maneuvering = undefined;

	if (!self bot_combat::has_threat())
		self [[level.botIdle]]();
}

// Enemy air scorestreaks: stock bots only target some, with a launcher they never switch to. Now and then a bot shoots
// one with its launcher (locking on if needed) or its gun. Players come first.
function BotConsiderAirStreaks()
{
	if (isDefined(self.mv_airTarget) || RandomInt(100) >= level.mv_cfg.shootStreaks)
		return;

	if (isDefined(self.mv_airNext) && GetTime() < self.mv_airNext)
		return;

	if (IS_TRUE(self.mv_maneuvering) || IS_TRUE(self.mv_usingStreak) || IS_TRUE(self.mv_capturingCrate) ||
		isDefined(self.carryObject) || self IsRemoteControlling() || self BotTraversing() || self BotFightingPlayer())
		return;

	target = self BotFindAirStreak();
	if (isDefined(target))
		self thread BotShootStreak(target);
}

function BotFightingPlayer()
{
	return self bot_combat::threat_visible() && IsPlayer(self.bot.threat.entity);
}

// Killstreak type without the "inventory_" prefix (from care packages), or "" if it isn't one flying around
function BotAirStreakType(type)
{
	if (isSubstr(type, "inventory_"))
		type = GetSubStr(type, 10);

	switch (type)
	{
	case "uav":
	case "counteruav":
	case "satellite":
	case "helicopter_comlink":		// Wraith
	case "helicopter_guard":
	case "helicopter_gunner":
	case "sentinel":
	case "raps":
		return type;
	}
	return "";
}

function BotGetLauncher()
{
	foreach (weapon in self GetWeaponsList())
	{
		if (weapon.isRocketLauncher && (self GetWeaponAmmoClip(weapon) || self GetWeaponAmmoStock(weapon)))
			return weapon;
	}
	return undefined;
}

// Nearest enemy air scorestreak with a clear shot (from each player's pers["activeKillstreaks"])
function BotFindAirStreak()
{
	hasLauncher = isDefined(self BotGetLauncher());
	eye = self GetEye();
	best = undefined;
	bestDistSq = 9000 * 9000;

	foreach (player in level.players)
	{
		if (player == self || !isDefined(player.pers["activeKillstreaks"]))
			continue;
		if (level.teamBased && player.team == self.team)
			continue;

		foreach (streak in player.pers["activeKillstreaks"])
		{
			if (!isDefined(streak) || !isDefined(streak.killstreakType))
				continue;

			type = BotAirStreakType(streak.killstreakType);
			if (type == "")
				continue;

			// Bullets don't hurt the Satellite
			if (type == "satellite" && !hasLauncher)
				continue;

			// Hacked ones switch teams
			if (level.teamBased && isDefined(streak.team) && streak.team == self.team)
				continue;

			distSq = DistanceSquared(self.origin, streak.origin);
			if (distSq >= bestDistSq || !BulletTracePassed(eye, streak.origin, false, streak))
				continue;

			best = streak;
			bestDistSq = distSq;
		}
	}
	return best;
}

function BotShootStreak(target)
{
	self endon("death");
	self endon("disconnect");
	level endon("game_ended");

	self.mv_airTarget = target;
	self.mv_maneuvering = true;

	launcher = self BotGetLauncher();
	if (isDefined(launcher) && self GetCurrentWeapon() != launcher)
	{
		self BotSwitchToWeapon(launcher);
		for (i = 0; i < 30 && isDefined(target) && self GetCurrentWeapon() != launcher; i++)
		{
			self BotLookAtPoint(target.origin);
			wait 0.05;
		}
	}

	weapon = self GetCurrentWeapon();
	useLauncher = weapon.isRocketLauncher;
	fired = false;
	lostSight = 0;
	timeLimit = RandomFloatRange(3, 6);
	if (useLauncher)
		timeLimit = 5;

	self.mv_holdButtons = array(BOT_BUTTON_ADS);
	for (t = 0; t < timeLimit; t += 0.05)
	{
		if (!isDefined(target) || self BotFightingPlayer())
			break;

		self BotLookAtPoint(target.origin);
		self BotPressButton(BOT_BUTTON_ADS);

		if (BulletTracePassed(self GetEye(), target.origin, false, target))
			lostSight = 0;
		else
		{
			lostSight += 0.05;
			if (lostSight > 1)
				break;
		}

		// Give the aim a moment to settle on it
		if (t >= 0.5 && lostSight == 0)
		{
			if (useLauncher)
			{
				// Lock-on launchers lock by themselves while aiming at it (heatseeking sets stingerLockFinalized)
				if (!fired && (!weapon.requireLockOnToFire || IS_TRUE(self.stingerLockFinalized)) && self PlayerAds() == 1)
				{
					self bot::tap_attack_button();
					fired = true;
				}
			}
			else if (weapon.fireType == "Full Auto")
				self bot::press_attack_button();
			else if (int(t * 20) % 2 == 0)
				self bot::press_attack_button();
			else
				self bot::release_attack_button();
		}

		// One rocket per try, give it time to fly
		if (fired && t >= 1.5)
			break;

		wait 0.05;
	}

	self.mv_holdButtons = undefined;
	self bot::release_attack_button();
	self bot::release_ads_button();
	self BotLookForward();
	self.mv_maneuvering = undefined;
	self.mv_airTarget = undefined;

	// Don't keep staring at the sky
	self.mv_airNext = GetTime() + RandomIntRange(4000, 9000);
}
