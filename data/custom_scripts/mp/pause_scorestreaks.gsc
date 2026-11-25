#using scripts\shared\callbacks_shared;
#using scripts\shared\killstreaks_shared;
#using scripts\shared\system_shared;
#using scripts\mp\gametypes\_loadout;
#using scripts\mp\killstreaks\_killstreaks;
#insert scripts\shared\shared.gsh;

#namespace pause_scorestreaks;

REGISTER_SYSTEM("pause_scorestreaks", &init, undefined)

function init()
{
	if (!IsDedicated())
		return;
	callback::on_start_gametype(&begin_match);
	callback::on_connect(&connected);
}

// game[] survives round restarts. Process dvars survive ExitLevel(false),
// including a vote that loads the same map again. Only carry occupied rows.
function begin_match()
{
	if (isDefined(game["boiii_streaks_active"]))
		return;
	game["boiii_streaks_active"] = [];
	game["boiii_streaks_pending"] = [];
	game["boiii_streaks_rows"] = [];
	for (i = 0; i < 64; i++)
	{
		name = "boiii_nextStreaks" + i;
		raw = GetDvarString(name);
		if (raw == "")
			continue;
		SetDvar(name, "");
		row = StrTok(raw, ":");
		if (row.size == 4)
		{
			slots = [];
			for (j = 0; j < 3; j++)
				slots[j] = int(row[j + 1]);
			game["boiii_streaks_active"][row[0]] = slots;
		}
	}
}

function allow_selection()
{
	return GetGametypeSetting("loadoutKillstreaksEnabled") && GetDvarInt("custom_killstreak_mode") != 2;
}

function in_opening_countdown()
{
	return isDefined(level.inPrematchPeriod) && level.inPrematchPeriod
		&& game["state"] == "playing" && game["roundsplayed"] == 0
		&& !(isDefined(level.prematch_over) && level.prematch_over);
}

function refresh_countdown_streaks()
{
	// A player still choosing a class will use the new indices on first spawn.
	if (!isAlive(self) || self.sessionstate != "playing" || !isDefined(self.class_num_for_global_weapons)
		|| !isDefined(self.killstreak) || isDefined(level.giveCustomLoadout))
		return;

	previous = self.killstreak;
	if (isDefined(level.usingMomentum) && level.usingMomentum)
	{
		// giveKillstreaks only writes occupied slots. Clear the old bindings
		// first so selecting None cannot leave an obsolete HUD action behind.
		self SetActionSlot(4, "");
		self SetActionSlot(2, "");
		self SetActionSlot(1, "");
	}
	self loadout::giveKillstreaks();
	if (isDefined(level.usingMomentum) && level.usingMomentum)
	{
		foreach (ref in previous)
		{
			if (IsInArray(self.killstreak, ref))
				continue;
			weapon = killstreaks::get_killstreak_weapon(killstreaks::get_by_menu_name(ref));
			if (weapon != level.weaponNone && self HasWeapon(weapon))
				self TakeWeapon(weapon);
		}
	}
}

function valid_selection(slots)
{
	if (!allow_selection())
		return false;
	seen = [];
	foreach (item in slots)
	{
		if (item == 0)
			continue;
		if (item < 0 || item >= 256 || isDefined(seen[item]) || !isDefined(level.tbl_KillStreakData[item]))
			return false;
		if (IsItemRestricted(level.tbl_KillStreakData[item]))
			return false;
		if (!SessionModeIsPrivate() && (self IsItemLocked(item) || !self IsItemPurchased(item)))
			return false;
		seen[item] = true;
	}
	return true;
}

function stash(key, slots)
{
	index = self GetEntityNumber();
	rows = game["boiii_streaks_rows"];
	// A reconnect can give the same player a different network slot.
	if (isDefined(rows[key]) && rows[key] != index)
	{
		oldName = "boiii_nextStreaks" + rows[key];
		oldRow = StrTok(GetDvarString(oldName), ":");
		if (oldRow.size && oldRow[0] == key)
			SetDvar(oldName, "");
	}
	game["boiii_streaks_rows"][key] = index;
	SetDvar("boiii_nextStreaks" + index, key + ":" + slots[0] + ":" + slots[1] + ":" + slots[2]);
}

function connected()
{
	if (self IsTestClient())
		return;
	begin_match();
	key = "u" + self GetXuid();
	self.boiii_streaks_key = key;
	self thread listen(key);
	// Stats and the item table are available by the first loadout. Carry a
	// returning player's selection even on a map that temporarily bans it.
	if (isDefined(game["boiii_streaks_pending"][key]))
		self stash(key, game["boiii_streaks_pending"][key]);
	else if (isDefined(game["boiii_streaks_active"][key]))
		self stash(key, game["boiii_streaks_active"][key]);
}

function listen(key)
{
	self endon("disconnect");
	for (;;)
	{
		self waittill("menuresponse", menu, response);
		if (menu != "ingame_options" || !isString(response) || response.size > 42 || getSubStr(response, 0, 4) != "ks2:")
			continue;
		req = StrTok(response, ":");
		if (req.size != 3 && req.size != 6)
			continue;
		controller = int(req[1]);
		serial = int(req[2]);
		if (controller < 0 || controller > 7 || serial < 1 || serial > 999999 || req[1] != "" + controller || req[2] != "" + serial)
			continue;
		now = GetTime();
		if (isDefined(self.boiii_streaks_last_request) && now - self.boiii_streaks_last_request < 100)
			continue;
		self.boiii_streaks_last_request = now;
		// A lost reply may be retried after the timer reaches zero.
		if (isDefined(self.boiii_streaks_request) && self.boiii_streaks_request == response)
		{
			self setclientdvar(self.boiii_streaks_reply);
			continue;
		}
		state = "active";
		if (req.size == 6)
		{
			submitted = [];
			for (i = 3; i < 6; i++)
			{
				item = int(req[i]);
				if (req[i] != "" + item)
					break;
				submitted[submitted.size] = item;
			}
			if (submitted.size == 3 && self valid_selection(submitted))
			{
				if (in_opening_countdown())
				{
					game["boiii_streaks_active"][key] = submitted;
					game["boiii_streaks_pending"][key] = undefined;
					self.boiii_streaks_logged = undefined;
					self refresh_countdown_streaks();
				}
				else
				{
					game["boiii_streaks_pending"][key] = submitted;
					state = "queued";
				}
				self stash(key, submitted);
				// println("[scorestreaks] " + state + " player=" + self GetEntityNumber() + " items=" + submitted[0] + "," + submitted[1] + "," + submitted[2]);
			}
			else
				state = "rejected";
		}
		if (isDefined(game["boiii_streaks_pending"][key]))
		{
			slots = game["boiii_streaks_pending"][key];
			if (state != "rejected")
				state = "queued";
		}
		else
		{
			classNum = self.class_num_for_global_weapons;
			if (!isDefined(classNum))
				classNum = 0;
			slots = [];
			for (i = 0; i < 3; i++)
			{
				item = self streak_index(classNum, i, false);
				slots[i] = 0;
				if (isDefined(item) && item > 0)
					slots[i] = item;
			}
		}
		reply = "boiii_streakReply" + controller + " " + serial + ":" + state + ":" + slots[0] + ":" + slots[1] + ":" + slots[2];
		self.boiii_streaks_request = undefined;
		if (req.size == 6)
		{
			self.boiii_streaks_request = response;
			self.boiii_streaks_reply = reply;
		}
		self setclientdvar(reply);
	}
}

// This is the lookup giveKillstreaks uses BEFORE assigning HUD/action slots,
// streak costs and weapons on every spawn. Preserve the stock fourth slot.
detour loadout<scripts\mp\gametypes\_loadout.gsc>::getKillStreakIndex(weaponclass, killstreakNum)
{
	return self streak_index(weaponclass, killstreakNum, true);
}

function streak_index(weaponclass, index, equipping)
{
	slot = "killstreak" + (index + 1);
	if (GetDvarInt("custom_killstreak_mode") == 2)
		return GetDvarInt("custom_" + slot);
	if (index >= 0 && index < 3 && isDefined(self.boiii_streaks_key))
	{
		slots = game["boiii_streaks_active"][self.boiii_streaks_key];
		if (isDefined(slots) && self valid_selection(slots))
		{
			if (equipping && index == 0 && !isDefined(self.boiii_streaks_logged))
			{
				println("[scorestreaks] loadout player=" + self GetEntityNumber() + " items=" + slots[0] + "," + slots[1] + "," + slots[2]);
				self.boiii_streaks_logged = true;
			}
			return slots[index];
		}
	}
	return self GetLoadoutItem(weaponclass, slot);
}
