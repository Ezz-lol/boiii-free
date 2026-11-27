// SPDX-License-Identifier: GPL-3.0-only
// Experimental map mechanics contributed by Eroded Networks.
// Loaded only through the native, opt-in 1-8 map adapter.
#using custom_scripts/zm/eroded_coop;
#using scripts/codescripts/struct;
#using scripts/shared/array_shared;
#using scripts/shared/flag_shared;
#using scripts/zm/_zm_stats;
#using scripts/zm/_zm_utility;
#using scripts/zm/_zm_zonemgr;
#using scripts/zm/bgbs/_zm_bgb_anywhere_but_here;
#namespace eroded_coop_bgb_transport;

function context_current(epoch, map, capacity)
{
    if (!isint(epoch) || epoch < 1 || !isstring(map) || map == "" ||
        !isint(capacity) || capacity < 5 || capacity > 8 || getdvarstring("mapname") != map ||
        getdvarint("zm_coopStartupEpoch") != epoch || getdvarint("zm_coopAppliedLimit") != capacity ||
        !isdefined(level.eroded_coop_limit) || !isint(level.eroded_coop_limit) || level.eroded_coop_limit != capacity ||
        !eroded_coop::startup_current(epoch, map) || !eroded_coop::owns_callbacks()) return false;
    if (!isdefined(level.bgb) || !isdefined(level.bgb["zm_bgb_anywhere_but_here"])) return false;
    entry = level.bgb["zm_bgb_anywhere_but_here"];
    return isdefined(entry.validation_func) && entry.validation_func == &validation &&
        isdefined(entry.activation_func) && entry.activation_func == &activation;
}

function separation_current(epoch, map, capacity)
{
    if (!context_current(epoch, map, capacity)) return false;
    actual = actual_players(capacity);
    return isdefined(actual) && actual.size > 4 &&
        level.player_intersection_tracker_override == &eroded_coop::intersection_override &&
        eroded_coop::bgb_context_current();
}

function exposure_ready(point, player, epoch, map, capacity)
{
    if (!context_current(epoch, map, capacity) || !isvec(point)) return false;
    actual = actual_players(capacity);
    if (!isdefined(actual) || !contains_entity(actual, player)) return false;
    if (actual.size > 4)
    {
        if (!separation_current(epoch, map, capacity)) return false;
        until = gettime() + 2500;
        if (isdefined(player.eroded_coop_spawn_grace))
        {
            if (!isint(player.eroded_coop_spawn_grace)) return false;
            if (player.eroded_coop_spawn_grace > until) until = player.eroded_coop_spawn_grace;
        }
        player.eroded_coop_spawn_grace = until;
        return true;
    }
    for (player_index = 0; player_index < actual.size; player_index++)
    {
        other = actual[player_index];
        if (other == player || !isalive(other) || !isdefined(other.sessionstate) || other.sessionstate != "playing") continue;
        if (abs(point[2] - other.origin[2]) < 60 && distance2dsquared(point, other.origin) < 44 * 44) return false;
    }
    return true;
}

function post_arrival(player, epoch, map, capacity)
{
    if (separation_current(epoch, map, capacity) && contains_entity(actual_players(capacity), player) && isalive(player))
        player eroded_coop::player_spawned();
}



function stock_capacity(capacity)
{
    return isint(capacity) && capacity >= 1 && capacity <= 4;
}

function actual_players(capacity)
{
    players = getplayers();
    if (!isint(capacity) || capacity < 5 || capacity > 8 || !isarray(players) || players.size > capacity)
        return undefined;
    seen = array();
    for (array_index_1 = 0; array_index_1 < players.size; array_index_1++)
    {
        player = players[array_index_1];

        if (!isdefined(player) || !isentity(player) || !isplayer(player)) return undefined;
        slot = player getentitynumber();
        if (!isint(slot) || slot < 0 || slot >= capacity || isdefined(seen[slot])) return undefined;
        seen[slot] = true;

    }
    return players;
}

function contains_entity(players, player)
{
    if (!isarray(players) || !isdefined(player)) return false;
    for (array_index_2 = 0; array_index_2 < players.size; array_index_2++)
    {
        candidate = players[array_index_2];
        if (candidate == player) return true;
    }
    return false;
}

function slot_destination(points, target, slot, capacity)
{
    if (!isint(capacity) || capacity < 5 || capacity > 8 || !isint(slot) || slot < 0 || slot >= capacity ||
        !isstring(target) || target == "" || !isarray(points) || points.size < 4 || points.size > 8)
        return undefined;
    labels = array();
    for (point_index = 0; point_index < points.size; point_index++)
    {
        point = points[point_index];
        if (!isdefined(point) || !isdefined(point.origin) || !isvec(point.origin) ||
            !isdefined(point.targetname) || !isstring(point.targetname) || point.targetname != target ||
            !isdefined(point.var_8202763a) || !isint(point.var_8202763a) ||
            point.var_8202763a < 1 || point.var_8202763a > 8)
            return undefined;
        label = point.var_8202763a;
        if (isdefined(labels[label - 1])) return undefined;
        for (prior_index = 0; prior_index < point_index; prior_index++)
        {
            prior = points[prior_index];
            if (prior == point || (distance2dsquared(prior.origin, point.origin) == 0 && prior.origin[2] == point.origin[2]))
                return undefined;
        }
        labels[label - 1] = point;
    }
    for (stock_label = 0; stock_label < 4; stock_label++)
        if (!isdefined(labels[stock_label])) return undefined;
    // Preserve a real custom record for this exact extra slot. Only absence
    // falls back to its corresponding existing stock label1..4.
    if (isdefined(labels[slot])) return labels[slot];
    return labels[slot % 4];
}

function generic_respawn()
{
    epoch = getdvarint("zm_coopStartupEpoch");
    map = getdvarstring("mapname");
    capacity = getdvarint("zm_coopAppliedLimit");
    if (stock_capacity(capacity)) return self zm_bgb_anywhere_but_here::function_728dfe3();
    if (!context_current(epoch, map, capacity)) return undefined;
    actual = actual_players(capacity);
    slot = self getentitynumber();
    if (!isdefined(actual) || !contains_entity(actual, self) || !isint(slot) || slot < 0 || slot >= capacity)
        return undefined;
    if (!isdefined(level.zones) || !isarray(level.zones)) return undefined;
    zone = zm_zonemgr::get_zone_from_position(self.origin + (0, 0, 32), 0);
    if (!isdefined(zone)) zone = self.zone_name;
    ignore_zone = undefined;
    if (isdefined(zone) && isdefined(level.zones)) ignore_zone = level.zones[zone];
    masters = struct::get_array("player_respawn_point", "targetname");
    if (!isarray(masters) || masters.size < 1) return undefined;
    candidates = array();
    known_candidates = array();
    for (master_index = 0; master_index < masters.size; master_index++)
    {
        master = masters[master_index];
        if (!isdefined(master) || !isdefined(master.origin) || !isvec(master.origin) ||
            !isdefined(master.targetname) || master.targetname != "player_respawn_point" ||
            !isdefined(master.target) || !isstring(master.target) || master.target == "") return undefined;
        if (zm_utility::is_point_inside_enabled_zone(master.origin, ignore_zone))
        {
            candidates[candidates.size] = master;
            known_candidates[known_candidates.size] = master;
        }
    }
    valid = candidates;
    if (isdefined(level.var_2d4e3645))
    {
        if (!isfunctionptr(level.var_2d4e3645)) return undefined;
        valid = [[level.var_2d4e3645]](candidates);
    }
    if (!context_current(epoch, map, capacity) || !isarray(valid) || valid.size < 1) return undefined;
    for (master_index = 0; master_index < valid.size; master_index++)
        if (!isdefined(valid[master_index]) || !contains_entity(known_candidates, valid[master_index])) return undefined;
    master = array::random(valid);
    if (!isdefined(master) || !isdefined(master.target) || !isstring(master.target) || master.target == "") return undefined;
    points = struct::get_array(master.target, "targetname");
    return slot_destination(points, master.target, slot, capacity);
}

function activation_owned(record)
{
    return isdefined(record) && record.active && getdvarstring("mapname") == record.map &&
        getdvarint("zm_coopStartupEpoch") == record.epoch && contains_entity(getplayers(), record.player) &&
        isdefined(record.player.eroded_bgb_transport_record) && record.player.eroded_bgb_transport_record == record;
}

function release_activation(record)
{
    if (!isdefined(record) || !record.active) return;
    owned = activation_owned(record);
    record.active = false;
    if (!owned) return;
    if (record.controls) record.player freezecontrols(0);
    if (record.ignoreme) record.player zm_utility::decrement_ignoreme();
    record.player.eroded_bgb_transport_record = undefined;
}

function activation_current(record)
{
    return activation_owned(record) && context_current(record.epoch, record.map, record.capacity) && isalive(record.player);
}

function observe_activation(record)
{
    while (activation_owned(record))
    {
        if (!activation_current(record))
        {
            release_activation(record);
            return;
        }
        wait 0.05;
    }
    release_activation(record);
}

function validation()
{
    epoch = getdvarint("zm_coopStartupEpoch");
    map = getdvarstring("mapname");
    capacity = getdvarint("zm_coopAppliedLimit");
    if (stock_capacity(capacity)) return self zm_bgb_anywhere_but_here::validation();
    if (!context_current(epoch, map, capacity)) return 0;
    actual = actual_players(capacity);
    if (!isdefined(actual) || !contains_entity(actual, self) || !isalive(self) ||
        (actual.size > 4 && !separation_current(epoch, map, capacity))) return 0;
    return self zm_bgb_anywhere_but_here::validation();
}

function activation()
{
    epoch = getdvarint("zm_coopStartupEpoch");
    map = getdvarstring("mapname");
    capacity = getdvarint("zm_coopAppliedLimit");
    if (stock_capacity(capacity))
    {
        if (isdefined(self.eroded_bgb_transport_record)) return;
        self zm_bgb_anywhere_but_here::activation();
        return;
    }
    if (!context_current(epoch, map, capacity) || isdefined(self.eroded_bgb_transport_record)) return;
    actual = actual_players(capacity);
    if (!isdefined(actual) || !contains_entity(actual, self) || !isalive(self)) return;
    if (actual.size > 4 && !separation_current(epoch, map, capacity)) return;
    // Exactly one current map callback, as stock. A missing callback selects
    // one existing generic room. Never run RNG/zone probes from validation.
    if (isdefined(level.var_2c12d9a6))
    {
        if (!isfunctionptr(level.var_2c12d9a6)) return;
        s_respawn_point = self [[level.var_2c12d9a6]]();
    }
    else s_respawn_point = self generic_respawn();
    if (!context_current(epoch, map, capacity) || !contains_entity(actual_players(capacity), self) ||
        !isalive(self) || !isdefined(s_respawn_point) || !isdefined(s_respawn_point.origin) ||
        !isvec(s_respawn_point.origin) || isdefined(self.eroded_bgb_transport_record)) return;
    if (isdefined(level.var_2300a8ad) && !isfunctionptr(level.var_2300a8ad)) return;
    destination_origin = s_respawn_point.origin;
    if (!exposure_ready(destination_origin, self, epoch, map, capacity)) return;
    record = spawnstruct();
    record.player = self;
    record.epoch = epoch;
    record.map = map;
    record.capacity = capacity;
    record.active = true;
    record.ignoreme = false;
    record.controls = false;
    self.eroded_bgb_transport_record = record;
    level thread observe_activation(record);
    self zm_utility::increment_ignoreme(); record.ignoreme = true;
    playsoundatposition("zmb_bgb_abh_teleport_out", self.origin);
    self cleargroundent();
    self setvelocity((0, 0, 0));
    if (!exposure_ready(destination_origin, self, epoch, map, capacity)) { release_activation(record); return; }
    self setorigin(destination_origin);
    self freezecontrols(1); record.controls = true;
    post_arrival(self, epoch, map, capacity);
    v_return_pos = self.origin + (0, 0, 60);
    a_ai = getaiteamarray(level.zombie_team);
    a_closest = array();
    ai_closest = undefined;
    if (isarray(a_ai) && a_ai.size > 0)
    {
        a_closest = arraysortclosest(a_ai, self.origin);
        if (!isarray(a_closest)) a_closest = array();
        for (ai_index = 0; ai_index < a_closest.size; ai_index++)
        {
            ai = a_closest[ai_index];
            if (!isdefined(ai) || !isentity(ai)) continue;
            n_trace_val = ai sightconetrace(v_return_pos, self);
            if (n_trace_val > 0.2)
            {
                ai_closest = ai;
                break;
            }
        }
        if (isdefined(ai_closest)) self setplayerangles(vectortoangles(ai_closest getcentroid() - v_return_pos));
    }
    self playsound("zmb_bgb_abh_teleport_in");
    if (isdefined(level.var_2300a8ad)) self [[level.var_2300a8ad]]();
    wait 0.5;
    if (!activation_current(record)) return;
    self show();
    playfx(level._effect["teleport_splash"], self.origin);
    playfx(level._effect["teleport_aoe"], self.origin);
    a_ai = getaiarray();
    a_aoe_ai = array();
    if (isarray(a_ai)) a_aoe_ai = arraysortclosest(a_ai, self.origin, a_ai.size, 0, 200);
    if (!isarray(a_aoe_ai)) a_aoe_ai = array();
    for (ai_index = 0; ai_index < a_aoe_ai.size; ai_index++)
    {
        ai = a_aoe_ai[ai_index];
        if (isdefined(ai) && isactor(ai))
        {
            if (ai.archetype === "zombie") playfx(level._effect["teleport_aoe_kill"], ai gettagorigin("j_spineupper"));
            else playfx(level._effect["teleport_aoe_kill"], ai.origin);
            ai.marked_for_recycle = 1;
            ai.has_been_damaged_by_player = 0;
            ai dodamage(ai.health + 1000, self.origin, self);
        }
    }
    wait 0.2;
    if (!activation_current(record)) return;
    self freezecontrols(0); record.controls = false;
    self zm_stats::increment_challenge_stat("GUM_GOBBLER_ANYWHERE_BUT_HERE");
    wait 3;
    if (activation_owned(record)) release_activation(record);

}
