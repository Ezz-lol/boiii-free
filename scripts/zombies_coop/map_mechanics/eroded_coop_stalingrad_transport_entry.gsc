// SPDX-License-Identifier: GPL-3.0-only
// Experimental map mechanics contributed by Eroded Networks.
// Loaded only through the native, opt-in 1-8 map adapter.
// Original dragon exports remain intact for pre-mutation fallback.
#using custom_scripts/zm/eroded_coop;
#using custom_scripts/zm/eroded_coop_bgb_transport_entry;
#using scripts/codescripts/struct;
#using scripts/shared/array_shared;
#using scripts/shared/clientfield_shared;
#using scripts/shared/flag_shared;
#using scripts/shared/scene_shared;
#using scripts/shared/util_shared;
#using scripts/zm/_zm_bgb;
#using scripts/zm/_zm_powerups;
#using scripts/zm/_zm_utility;
#using scripts/zm/_zm_weapons;
#using scripts/zm/_zm_zonemgr;
#using scripts/zm/zm_stalingrad_dragon;
#using scripts/zm/zm_stalingrad;
#using scripts/zm/zm_stalingrad_wearables;
#using scripts/zm/zm_stalingrad_vo;
#namespace eroded_coop_stalingrad_transport;

function stock_capacity(capacity)
{
    return isint(capacity) && capacity >= 1 && capacity <= 4;
}

function context_current(epoch, capacity)
{
    return isint(epoch) && epoch > 0 && isint(capacity) && capacity >= 5 && capacity <= 8 &&
        getdvarstring("mapname") == "zm_stalingrad" &&
        getdvarint("zm_coopStartupEpoch") == epoch && getdvarint("zm_coopAppliedLimit") == capacity &&
        isdefined(level.eroded_coop_limit) && isint(level.eroded_coop_limit) &&
        level.eroded_coop_limit == capacity &&
        eroded_coop::startup_current(epoch, "zm_stalingrad") && eroded_coop::owns_callbacks();
}

function separation_current(epoch, capacity)
{
    if (!context_current(epoch, capacity)) return false;
    actual = actual_players(capacity);
    return isdefined(actual) && actual.size > 4 &&
        level.player_intersection_tracker_override == &eroded_coop::intersection_override &&
        eroded_coop::bgb_context_current();
}

function arm_grace(player, epoch, capacity)
{
    if (!separation_current(epoch, capacity) || !contains_entity(actual_players(capacity), player) || !isalive(player)) return false;
    until = gettime() + 2500;
    if (isdefined(player.eroded_coop_spawn_grace))
    {
        if (!isint(player.eroded_coop_spawn_grace)) return false;
        if (player.eroded_coop_spawn_grace > until) until = player.eroded_coop_spawn_grace;
    }
    player.eroded_coop_spawn_grace = until;
    return true;
}

function post_arrival(player, epoch, capacity)
{
    if (separation_current(epoch, capacity) && contains_entity(actual_players(capacity), player) && isalive(player))
        player eroded_coop::player_spawned();
}

function exposure_ready(point, player, epoch, capacity)
{
    if (!context_current(epoch, capacity) || !isvec(point)) return false;
    actual = actual_players(capacity);
    if (!isdefined(actual) || !contains_entity(actual, player)) return false;
    if (actual.size > 4) return arm_grace(player, epoch, capacity);
    return player position_clear(point, player, capacity);
}

function pick_landing(points, preferred, player, epoch, capacity, require_angles)
{
    if (!context_current(epoch, capacity) || !points_valid(points) || !isint(preferred) || preferred < 0 || preferred >= points.size)
        return undefined;
    actual = actual_players(capacity);
    if (!isdefined(actual) || !contains_entity(actual, player) || (actual.size > 4 && !separation_current(epoch, capacity))) return undefined;
    for (offset = 0; offset < points.size; offset++)
    {
        index = (preferred + offset) % points.size;
        if (require_angles && (!isdefined(points[index].angles) || !isvec(points[index].angles))) continue;
        if (player position_clear(points[index].origin, player, capacity)) return index;
    }
    if (actual.size > 4)
    {
        for (offset = 0; offset < points.size; offset++)
        {
            index = (preferred + offset) % points.size;
            if (require_angles && (!isdefined(points[index].angles) || !isvec(points[index].angles))) continue;
            if (zm_utility::check_point_in_playable_area(points[index].origin)) return index;
        }
    }
    return undefined;
}

function bounded_landing(record)
{
    deadline = gettime() + 2000;
    for (attempt = 0; attempt < 41 && rider_current(record); attempt++)
    {
        points = struct::get_array("s_pavlov_player", "targetname");
        if (!points_valid(points)) return false;
        index = pick_landing(points, record.landing, record.player, record.flight.epoch, record.flight.capacity, false);
        if (isdefined(index))
        {
            record.landing = index;
            record.destination = points[index];
            return true;
        }
        if (gettime() >= deadline) break;
        wait 0.05;
    }
    return false;
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

function known_active(capacity)
{
    actual = actual_players(capacity);
    if (!isdefined(actual) || !isarray(level.activeplayers) || level.activeplayers.size > capacity)
        return undefined;
    active = array();
    for (array_index_3 = 0; array_index_3 < level.activeplayers.size; array_index_3++)
    {
        player = level.activeplayers[array_index_3];

        if (!contains_entity(actual, player)) continue;
        if (contains_entity(active, player)) return undefined;
        if (isalive(player)) active[active.size] = player;

    }
    return active;
}

function points_valid(points)
{
    if (!isarray(points) || points.size < 1 || points.size > 4) return false;
    for (array_index_4 = 0; array_index_4 < points.size; array_index_4++)
    {
        point = points[array_index_4];
        if (!isdefined(point) || !isdefined(point.origin) || !isvec(point.origin)) return false;
    }
    return true;
}

function physical_seat(ordinal, active_count, solo_seat)
{
    if (!isint(ordinal) || ordinal < 1 || ordinal > 8 ||
        !isint(active_count) || active_count < 1 || active_count > 8) return undefined;
    if (active_count == 1)
    {
        if (!isint(solo_seat) || solo_seat < 1 || solo_seat > 4) return undefined;
        return solo_seat;
    }
    return ((ordinal - 1) % 4) + 1;
}

function landing_current(record)
{
    if (!isdefined(record) || !isint(record.landing) || record.landing < 0) return false;
    points = struct::get_array("s_pavlov_player", "targetname");
    return points_valid(points) && record.landing < points.size && points[record.landing] == record.destination;
}

function flight_identity(flight)
{
    return isdefined(flight) && isdefined(level.eroded_dragon_flight) &&
        level.eroded_dragon_flight == flight && !flight.closed &&
        getdvarstring("mapname") == "zm_stalingrad" &&
        getdvarint("zm_coopStartupEpoch") == flight.epoch;
}

function flight_current(flight)
{
    return flight_identity(flight) && context_current(flight.epoch, flight.capacity);
}

function record_owned(record)
{
    if (!isdefined(record) || !record.active || !flight_identity(record.flight)) return false;
    actual = actual_players(record.flight.capacity);
    if (!isdefined(actual) || !contains_entity(actual, record.player)) return false;
    return record.flight_token == record.flight.token &&
        isdefined(record.player.eroded_dragon_record) && record.player.eroded_dragon_record == record;
}

function rider_current(record)
{
    if (!record_owned(record) || record.cancel_requested || record.flight.cancelling ||
        !flight_current(record.flight) || !isalive(record.player)) return false;
    actual = actual_players(record.flight.capacity);
    if (!isdefined(actual) || (actual.size > 4 && !separation_current(record.flight.epoch, record.flight.capacity))) return false;
    active = known_active(record.flight.capacity);
    return isdefined(active) && contains_entity(active, record.player);
}

function position_clear(point, owner, capacity)
{
    if (!isvec(point) || !zm_utility::check_point_in_playable_area(point) || positionwouldtelefrag(point))
        return false;
    actual = actual_players(capacity);
    if (!isdefined(actual)) return false;
    for (array_index_5 = 0; array_index_5 < actual.size; array_index_5++)
    {
        player = actual[array_index_5];

        if (player == owner || !isalive(player) || !isdefined(player.sessionstate) || player.sessionstate != "playing")
            continue;
        if (abs(point[2] - player.origin[2]) < 60 && distance2dsquared(point, player.origin) < 44 * 44)
            return false;

    }
    return true;
}

function new_flight(epoch, capacity, points)
{
    if (!context_current(epoch, capacity) || !points_valid(points) ||
        !isarray(level.var_163a43e4) || level.var_163a43e4.size != 0 ||
        !isint(level.var_428b5b88) || level.var_428b5b88 != 0 ||
        !isdefined(level.var_e29dd7ca) || !isentity(level.var_e29dd7ca)) return undefined;
    if (!isdefined(level.eroded_dragon_token)) level.eroded_dragon_token = 0;
    if (!isint(level.eroded_dragon_token) || level.eroded_dragon_token < 0 || level.eroded_dragon_token >= 2147483647)
        return undefined;
    level.eroded_dragon_token++;
    flight = spawnstruct();
    flight.epoch = epoch;
    flight.capacity = capacity;
    flight.token = level.eroded_dragon_token;
    flight.records = array();
    flight.points = points;
    flight.dragon = level.var_e29dd7ca;
    flight.closed = false;
    flight.ejecting = false;
    flight.cancelling = false;
    flight.return_finished = false;
    flight.stock_reset = false;
    flight.expires = gettime() + 120000;
    level.eroded_dragon_flight = flight;
    level thread observe_flight(flight);
    return flight;
}

function find_record(flight, player)
{
    if (!isdefined(flight) || !isarray(flight.records)) return undefined;
    for (array_index_6 = 0; array_index_6 < flight.records.size; array_index_6++)
    {
        record = flight.records[array_index_6];
        if (record.player == player && record.active) return record;
    }
    return undefined;
}

// Cleanup has no death/disconnect endon. The level observer owns every acquired
// resource and exact record identity; a terminated player worker cannot leak it.
// A cancellation retries its saved departure for at most2s. Its final bounded
// abort uses that captured position; a <=4 all-blocked departure is not claimed
// telefrag-safe. No guessed coordinate or indefinite resource lease.
function release_record(record, successful)
{
    if (!isdefined(record) || !record.active || !flight_identity(record.flight) || record.flight_token != record.flight.token) return false;
    // Cleanup must not mistake a capacity-revoked but still connected entity
    // for a disconnect. Its identity comes from the actual engine array.
    actual = getplayers();
    connected = isarray(actual) && contains_entity(actual, record.player);
    owned = connected && isdefined(record.player.eroded_dragon_record) && record.player.eroded_dragon_record == record;
    if (connected && !owned)
    {
        if (isdefined(record.model)) { record.model delete(); record.model = undefined; }
        record.active = false;
        return true;
    }
    if (!successful) record.cancel_requested = true;
    if (owned && isalive(record.player) && !successful)
    {
        if (!isdefined(record.abort_deadline)) record.abort_deadline = gettime() + 2000;
        clear = record.player position_clear(record.board_origin, record.player, record.flight.capacity);
        grace = arm_grace(record.player, record.flight.epoch, record.flight.capacity);
        if (!clear && !grace && gettime() < record.abort_deadline) return false;
        if (record.scene) { record.player scene::stop(); record.scene = false; }
        if (record.linked) { record.player unlink(); record.linked = false; }
        record.player setorigin(record.board_origin);
        record.player setplayerangles(record.board_angles);
        post_arrival(record.player, record.flight.epoch, record.flight.capacity);
    }
    // Delete only this record's temporary launch entity. Never target a model
    // discovered by classname or a successor player's model.
    if (isdefined(record.model))
    {
        record.model delete();
        record.model = undefined;
    }
    record.active = false;
    if (owned)
    {
        if (record.scene) { record.player scene::stop(); record.scene = false; }
        if (record.linked) { record.player unlink(); record.linked = false; }
        if (record.controls)
        {
            record.player freezecontrols(0);
            record.player enableweapons();
            record.player enableoffhandweapons();
        }
        if (record.nonsolid) record.player solid();
        if (record.ejection) record.player clientfield::set("dragon_transport_eject", 0);
        if (record.exploder) record.player clientfield::set_to_player("dragon_transportation_exploders", 0);
        if (record.looping) record.player stoploopsound(0.5);
        if (record.invulnerability && !record.was_invulnerable) record.player disableinvulnerability();
        if (record.ignoreme) record.player zm_utility::decrement_ignoreme();
        record.player.bgb_disabled = record.prior_bgb_disabled;
        if (record.weapon_cycling) record.player bgb::resume_weapon_cycling();
        record.player.var_fa6d2a24 = record.prior_transport_flag;
        record.player.eroded_dragon_record = undefined;
    }
    arrayremovevalue(level.var_163a43e4, record.player);
    return true;
}

function reconcile_full(flight)
{
    if (!flight_current(flight) || flight.ejecting) return;
    active = known_active(flight.capacity);
    if (!isdefined(active) || active.size == 0) return;
    for (array_index_7 = 0; array_index_7 < active.size; array_index_7++)
    {
        player = active[array_index_7];

        record = find_record(flight, player);
        if (!isdefined(record) || !rider_current(record)) return;

    }
    if (!level flag::get("dragon_full"))
    {
        level flag::set("dragon_full");
        level thread dragon::function_825bb926();
    }
}

function observe_flight(flight)
{
    while (flight_identity(flight))
    {
        if (gettime() >= flight.expires) flight.cancelling = true;
        alive_records = 0;
        for (array_index_8 = 0; array_index_8 < flight.records.size; array_index_8++)
    {
        record = flight.records[array_index_8];

            if (!record.active) continue;
            if (!rider_current(record)) release_record(record, false);
            if (record.active) alive_records++;

    }
        if (!flight_current(flight) && alive_records == 0) flight.return_finished = true;
        if (flight.records.size > 0 && alive_records == 0) flight.return_finished = true;
        if (flight.return_finished && alive_records == 0)
        {
            if (!flight.stock_reset && flight_current(flight))
            {
                level.var_163a43e4 = array();
                level.var_428b5b88 = 0;
                level notify(#"hash_9a634383");
                flight.stock_reset = true;
            }
            flight.closed = true;
            return;
        }
        reconcile_full(flight);
        wait 0.05;
    }
}

// Original caller: dragon::function_31efd8da, player thread, argc0.
function dragon_entry()
{
    epoch = getdvarint("zm_coopStartupEpoch");
    capacity = getdvarint("zm_coopAppliedLimit");
    if (!context_current(epoch, capacity))
    {
        // Never run stock admission on a record already mutated by this owner.
        if (isdefined(self.eroded_dragon_record)) return;
        if (stock_capacity(capacity)) self dragon::function_280af5e8();
        return;
    }
    active = known_active(capacity);
    actual = actual_players(capacity);
    if (!isdefined(actual) || (actual.size > 4 && !separation_current(epoch, capacity))) return;
    points = struct::get_array("s_pavlov_player", "targetname");
    if (!isdefined(active) || active.size < 1 || !contains_entity(active, self) || !points_valid(points) ||
        !isarray(level.var_163a43e4) || !isint(level.var_428b5b88) || level.var_428b5b88 < 0 ||
        level.var_428b5b88 >= capacity || !isdefined(level.var_e29dd7ca) || !isentity(level.var_e29dd7ca)) return;
    if (contains_entity(level.var_163a43e4, self)) return;
    flight = undefined;
    if (isdefined(level.eroded_dragon_flight) && flight_identity(level.eroded_dragon_flight))
        flight = level.eroded_dragon_flight;
    else flight = new_flight(epoch, capacity, points);
    if (!flight_current(flight) || flight.ejecting || flight.cancelling || flight.dragon != level.var_e29dd7ca ||
        flight.records.size >= capacity || flight.records.size != level.var_428b5b88) return;
    ordinal = flight.records.size + 1;
    seat = physical_seat(ordinal, active.size, level.var_3f756142);
    if (!isdefined(seat)) return;
    landing = (ordinal - 1) % points.size;
    if (active.size == 1) landing = seat - 1;
    if (landing < 0 || landing >= points.size || !isvec(self.origin) || !isvec(self.angles) ||
        !zm_utility::check_point_in_playable_area(self.origin) || !self flag::exists("bgb_weapon_cycling")) return;
    // Preserve an already suspended unrelated cycling owner rather than resume
    // it on transport completion. No body changes have happened yet.
    if (!self flag::get("bgb_weapon_cycling")) return;
    if ((isdefined(self.bgb_disabled) && self.bgb_disabled != 0) ||
        (isdefined(self.var_fa6d2a24) && self.var_fa6d2a24 != 0)) return;
    tag = "tag_passenger" + seat;
    v_angles = flight.dragon gettagangles(tag);
    if (!isvec(v_angles)) return;
    record = spawnstruct();
    record.flight = flight;
    record.flight_token = flight.token;
    record.player = self;
    record.ordinal = ordinal;
    record.seat = seat;
    record.tag = tag;
    record.landing = landing;
    record.destination = points[landing];
    record.board_origin = self.origin;
    record.board_angles = self.angles;
    record.prior_bgb_disabled = 0;
    record.prior_transport_flag = 0;
    record.active = true;
    record.cancel_requested = false;
    record.controls = false;
    record.scene = false;
    record.linked = false;
    record.nonsolid = false;
    record.ejection = false;
    record.exploder = false;
    record.looping = false;
    record.invulnerability = false;
    record.ignoreme = false;
    record.weapon_cycling = false;
    self.eroded_dragon_record = record;
    flight.records[flight.records.size] = record;
    level.var_428b5b88++;
    array::add(level.var_163a43e4, self);
    // No yield between acquisition and recording its owner.
    self bgb::suspend_weapon_cycling(); record.weapon_cycling = true;
    prior_invulnerability = self enableinvulnerability();
    record.was_invulnerable = isdefined(prior_invulnerability) && prior_invulnerability;
    record.invulnerability = true;
    self.var_fa6d2a24 = 1;
    self zm_utility::increment_ignoreme(); record.ignoreme = true;
    self.bgb_disabled = 1;
    self thread zm_stalingrad_vo::function_cf8fccfe(1);
    self setplayerangles((0, v_angles[1], 0));
    self playerlinktodelta(flight.dragon, tag, 1, 45, 45, 80, 10, 1, 1);
    record.linked = true;
    if (level.var_163a43e4.size == 1)
    {
        level flag::set("dragon_platform_one_rider");
        level thread rider_timeout(flight);
        level thread dragon::function_4e945eab();
    }
    reconcile_full(flight);
}

function rider_timeout(flight)
{
    wait 10;
    if (!flight_current(flight) || flight.ejecting || level flag::get("dragon_full")) return;
    if (isarray(level.var_163a43e4) && level.var_163a43e4.size > 0) level flag::set("dragon_rider_timeout");
    level thread dragon::function_825bb926();
}

// Original caller: dragon::function_d2059ba7, level thread, argc0.
function dragon_return()
{
    if (!isdefined(level.eroded_dragon_flight) || level.eroded_dragon_flight.closed)
    {
        if (stock_capacity(getdvarint("zm_coopAppliedLimit"))) level dragon::function_3f0f5f61();
        return;
    }
    flight = level.eroded_dragon_flight;
    if (!flight_current(flight) || flight.ejecting) return;
    points = struct::get_array("s_pavlov_player", "targetname");
    if (!points_valid(points)) { flight.cancelling = true; return; }
    flight.ejecting = true;
    for (array_index_9 = 0; array_index_9 < flight.records.size; array_index_9++)
    {
        record = flight.records[array_index_9];

        if (!rider_current(record)) continue;
        player = record.player;
        player scene::stop();
        record.scene = false;
        player unlink();
        record.linked = false;
        player freezecontrols(0);
        player enableweapons();
        player enableoffhandweapons();
        record.controls = false;
        player zm_weapons::switch_back_primary_weapon(player.var_de933247, 0);
        player playerlinktoabsolute(flight.dragon, record.tag);
        record.linked = true;
        wait 0.05;
        if (rider_current(record))
        {
            player playerlinktodelta(flight.dragon, record.tag, 1, 45, 45, 80, 10, 1, 1);
            level thread clear_transport_exploder(record);
        }

    }
    if (!flight_current(flight)) return;
    level.var_fc730f22 = array();
    wait 2;
    if (!flight_current(flight)) return;
    for (array_index_10 = 0; array_index_10 < flight.records.size; array_index_10++)
    {
        record = flight.records[array_index_10];

        if (!rider_current(record)) continue;
        if (!bounded_landing(record)) { release_record(record, false); continue; }
        if (!rider_current(record)) continue;
        points = struct::get_array("s_pavlov_player", "targetname");
        if (!points_valid(points) || record.landing >= points.size || points[record.landing] != record.destination)
        {
            release_record(record, false);
            continue;
        }
        angles = flight.dragon gettagangles(record.tag);
        if (!isvec(angles)) { release_record(record, false); continue; }
        record.player unlink();
        record.linked = false;
        level thread eject_record(record, angles);
        wait 0.4;
        if (!flight_current(flight)) return;

    }
    if (!flight_current(flight)) return;
    level.var_163a43e4 = array();
    level.var_428b5b88 = 0;
    level notify(#"hash_9a634383");
    flight.stock_reset = true;
    flight.return_finished = true;
}

function clear_transport_exploder(record)
{
    deadline = gettime() + 35000;
    while (rider_current(record) && gettime() < deadline && !record.player zm_zonemgr::entity_in_zone("pavlovs_B_zone")) wait 0.5;
    if (rider_current(record)) record.player clientfield::set_to_player("dragon_transportation_exploders", 0);
}

function eject_record(record, angles)
{
    if (!rider_current(record)) return;
    if (!landing_current(record) || !isvec(angles) || distance(record.player.origin, record.destination.origin) < 1)
    {
        release_record(record, false);
        return;
    }
    player = record.player;
    record.model = spawn("script_model", player.origin);
    if (!isdefined(record.model)) { release_record(record, false); return; }
    record.model setmodel("tag_origin");
    record.model.angles = angles;
    player playerlinktodelta(record.model, "tag_origin", 1, 45, 45, 45, 45, 0, 0);
    record.linked = true;
    player notsolid(); record.nonsolid = true;
    record.model notsolid();
    player clientfield::set("dragon_transport_eject", 1); record.ejection = true;
    player playsound("zmb_dragon_eject");
    player playloopsound("zmb_dragon_fly_lp"); record.looping = true;
    player thread zm_stalingrad_vo::function_cf8fccfe(0);
    n_time = record.model zm_utility::fake_physicslaunch(record.destination.origin, 400);
    if (!isdefined(n_time) || n_time <= 0 || n_time > 30) { release_record(record, false); return; }
    wait n_time;
    if (!rider_current(record)) return;
    if (!bounded_landing(record) || !rider_current(record) ||
        !exposure_ready(record.destination.origin, player, record.flight.epoch, record.flight.capacity))
    { release_record(record, false); return; }
    player unlink(); record.linked = false;
    record.model delete(); record.model = undefined;
    player setorigin(record.destination.origin);
    post_arrival(player, record.flight.epoch, record.flight.capacity);
    player clientfield::set("dragon_transport_eject", 0); record.ejection = false;
    player stoploopsound(0.5); record.looping = false;
    player solid(); record.nonsolid = false;
    player notify(#"hash_2e47bc4a");
    if (record.invulnerability && !record.was_invulnerable) player disableinvulnerability();
    record.invulnerability = false;
    player zm_utility::decrement_ignoreme(); record.ignoreme = false;
    player.bgb_disabled = record.prior_bgb_disabled;
    player bgb::resume_weapon_cycling(); record.weapon_cycling = false;
    if (!level flag::get("dragon_pavlov_first_time")) level flag::set("dragon_pavlov_first_time");
    wait 4;
    if (record_owned(record)) release_record(record, true);
}

// Original caller: dragon::function_70d68a42, level call, argc0. This can begin
// before the first rider creates a flight. Preserve the stock scene/flag flow;
// acquire the current flight only after a first rider/timeout/full transition.
function dragon_idle()
{
    epoch = getdvarint("zm_coopStartupEpoch");
    capacity = getdvarint("zm_coopAppliedLimit");
    if (!context_current(epoch, capacity))
    {
        if (isdefined(level.eroded_dragon_flight) && flight_identity(level.eroded_dragon_flight)) return;
        if (stock_capacity(capacity)) level dragon::function_83dd194d();
        return;
    }
    while (!level flag::get_any(array("dragon_rider_timeout", "dragon_full")))
    {
        if (!context_current(epoch, capacity) || !isdefined(known_active(capacity)) ||
            !isdefined(level.var_6e68a823) || !isdefined(level.var_6e68a823[level.var_9d19c7e])) return;
        var_76862d83 = level.var_6e68a823[level.var_9d19c7e];
        level thread scene::play(var_76862d83);
        level waittill(#"hash_e276220e");
        if (!context_current(epoch, capacity)) return;
        if (level flag::get("dragon_reroute")) level flag::clear("dragon_reroute");
        level flag::set("dragon_platform_arrive");
        dragon::function_29710212();
        if (!context_current(epoch, capacity)) return;
        if (level flag::get("dragon_reroute"))
        {
            level flag::clear("dragon_platform_arrive");
            level scene::play(var_76862d83 + "_timeout");
            if (!context_current(epoch, capacity)) return;
        }
    }
    if (level flag::get_any(array("dragon_rider_timeout", "dragon_full")))
    {
        if (!isdefined(level.eroded_dragon_flight)) return;
        flight = level.eroded_dragon_flight;
        if (!flight_current(flight)) return;
        for (array_index_11 = 0; array_index_11 < flight.records.size; array_index_11++)
    {
        record = flight.records[array_index_11];

            if (!rider_current(record)) continue;
            player = record.player;
            if (player hasweapon(getweapon("minigun")))
            {
                zm_powerups::weapon_powerup_remove(player, "minigun_time_over", "minigun", 1);
                wait 0.3;
                if (!rider_current(record)) continue;
            }
            var_d2f4cbdf = player getweaponslistprimaries();
            w_current = player getcurrentweapon();
            if (!isarray(var_d2f4cbdf) || (zm_utility::is_offhand_weapon(w_current) && var_d2f4cbdf.size < 1))
            {
                release_record(record, false);
                continue;
            }
            if (zm_utility::is_offhand_weapon(w_current)) player.var_de933247 = var_d2f4cbdf[0];
            else player.var_de933247 = w_current;
            for (array_index_12 = 0; array_index_12 < var_d2f4cbdf.size; array_index_12++)
    {
        w_primary = var_d2f4cbdf[array_index_12];

                if (isdefined(level.limited_weapons[w_primary]))
                {
                    if (!isdefined(level.var_fc730f22)) level.var_fc730f22 = array();
                    else if (!isarray(level.var_fc730f22)) level.var_fc730f22 = array(level.var_fc730f22);
                    level.var_fc730f22[level.var_fc730f22.size] = w_primary;
                }

    }
            player freezecontrols(1);
            player disableweapons();
            player disableoffhandweapons();
            record.controls = true;

    }
        wait 2;
        if (!flight_current(flight)) return;
        level thread scene::play(var_76862d83 + "_idle_2_pavlovs");
        for (array_index_13 = 0; array_index_13 < flight.records.size; array_index_13++)
    {
        record = flight.records[array_index_13];

            if (!rider_current(record) || !record.controls) continue;
            player = record.player;
            if (isdefined(level.musicsystem.currentplaytype) && level.musicsystem.currentplaytype < 4 &&
                !(isdefined(level.musicsystemoverride) && level.musicsystemoverride))
                player playsoundtoplayer("mus_stalingrad_dragonride", player);
            player clientfield::set_to_player("dragon_transportation_exploders", 1);
            record.exploder = true;

    }
        for (array_index_14 = 0; array_index_14 < flight.records.size; array_index_14++)
    {
        record = flight.records[array_index_14];

            if (!rider_current(record) || !record.controls) continue;
            // SP keeps its admission-time chosen seat; MP uses precisely the
            // same seat for entry, rider scene and return, even after churn.
            active = known_active(capacity);
            if (!isdefined(active)) return;
            if (active.size != 1) { record.player unlink(); record.linked = false; }
            level thread scene::play(var_76862d83 + "_idle_2_pavlovs_p" + record.seat, record.player);
            record.scene = true;

    }
        level.var_f73b438a++;
    }
    level waittill(#"hash_803aa6bf");
    if (!context_current(epoch, capacity)) return;
    level flag::clear("dragon_console_triggered");
    level flag::clear("dragon_full");
    level flag::clear("dragon_rider_timeout");
    level flag::clear("dragon_platform_arrive");
    level flag::clear("dragon_platform_one_rider");
}

function wings_owned(record)
{
    return isdefined(record) && record.active &&
        getdvarstring("mapname") == "zm_stalingrad" && getdvarint("zm_coopStartupEpoch") == record.epoch &&
        contains_entity(getplayers(), record.player) && isdefined(record.player.eroded_pavlov_record) &&
        record.player.eroded_pavlov_record == record;
}

function release_wings(record)
{
    if (!isdefined(record) || !record.active) return;
    owned = wings_owned(record);
    record.active = false;
    if (!owned) return;
    if (record.ignoreme) record.player zm_utility::decrement_ignoreme();
    if (record.transport_flag) record.player.var_fa6d2a24 = 0;
    record.player.eroded_pavlov_record = undefined;
}

function observe_wings(record)
{
    while (wings_owned(record))
    {
        if (!context_current(record.epoch, record.capacity) || !isalive(record.player) ||
            gettime() >= record.expires ||
            (record.started && gettime() >= record.release_time))
        {
            release_wings(record);
            return;
        }
        wait 0.05;
    }
    release_wings(record);
}

// Exact original owner: zm_stalingrad_wearables::function_cc32e7df, self=player,
// argc0. Called synchronously by function_7e873fe6 on its triggered e_player.
function pavlov_wings()
{
    epoch = getdvarint("zm_coopStartupEpoch");
    capacity = getdvarint("zm_coopAppliedLimit");
    slot = self getentitynumber();
    if (!context_current(epoch, capacity))
    {
        if (isdefined(self.eroded_pavlov_record)) return;
        if (stock_capacity(capacity)) self zm_stalingrad_wearables::function_cc32e7df();
        return;
    }
    actual = actual_players(capacity);
    if (!isdefined(actual) || (actual.size > 4 && !separation_current(epoch, capacity))) return;
    points = struct::get_array("s_pavlov_player", "targetname");
    if (!isdefined(actual) || !contains_entity(actual, self) || !isint(slot) || slot >= capacity ||
        !points_valid(points) || !isalive(self) || isdefined(self.eroded_pavlov_record) ||
        (isdefined(self.var_fa6d2a24) && self.var_fa6d2a24 != 0)) return;
    point_index = slot;
    if (slot >= 4) point_index = slot % points.size;
    if (point_index >= points.size) return;
    destination = points[point_index];
    if (!isdefined(destination.angles) || !isvec(destination.angles)) return;
    record = spawnstruct();
    record.player = self;
    record.epoch = epoch;
    record.capacity = capacity;
    record.active = true;
    record.started = false;
    record.ignoreme = false;
    record.transport_flag = false;
    record.expires = gettime() + 6000;
    self.eroded_pavlov_record = record;
    level thread observe_wings(record);
    deadline = gettime() + 2000;
    chosen = undefined;
    for (attempt = 0; attempt < 41 && wings_owned(record) && context_current(epoch, capacity) && isalive(self); attempt++)
    {
        points = struct::get_array("s_pavlov_player", "targetname");
        if (!points_valid(points) || point_index >= points.size || points[point_index] != destination ||
            !isdefined(destination.angles) || !isvec(destination.angles))
        {
            release_wings(record);
            return;
        }
        chosen = pick_landing(points, point_index, self, epoch, capacity, true);
        if (isdefined(chosen)) { destination = points[chosen]; break; }
        if (gettime() >= deadline) break;
        wait 0.05;
    }
    if (!isdefined(chosen) || !wings_owned(record) || !context_current(epoch, capacity) || !isalive(self) ||
        !exposure_ready(destination.origin, self, epoch, capacity))
    {
        release_wings(record);
        return;
    }
    self zm_utility::increment_ignoreme(); record.ignoreme = true;
    self.var_fa6d2a24 = 1; record.transport_flag = true;
    if (!exposure_ready(destination.origin, self, epoch, capacity)) { release_wings(record); return; }
    self dontinterpolate();
    self setorigin(destination.origin);
    self setplayerangles(destination.angles + (0, 180, 0));
    post_arrival(self, epoch, capacity);
    self notify(#"hash_2e47bc4a");
    level notify(#"hash_9a634383");
    record.started = true;
    record.release_time = gettime() + 3000;
    // Preserve the original synchronous three-second caller flow. The level
    // observer still owns cleanup if this player worker terminates early.
    wait 3;
    release_wings(record);
}

function gauntlet_points_valid(points)
{
    if (!isarray(points) || points.size < 1) return false;
    for (point_index = 0; point_index < points.size; point_index++)
        if (!isdefined(points[point_index]) || !isdefined(points[point_index].origin) || !isvec(points[point_index].origin))
            return false;
    return true;
}

// Exact B/C labels are read through canonical raw field8202763a. The captured
// primary function_277575cc operand and whole-source ACTS/Ezz check agree.
// The array must be dense, contain the four unique stock labels, and contain
// only optional unique explicit extra labels5..8. Never rewrite quest IDs.
function gauntlet_bc_destination(points, target, slot, capacity)
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

function gauntlet_checked_destination()
{
    capacity = getdvarint("zm_coopAppliedLimit");
    actual = actual_players(capacity);
    slot = self getentitynumber();
    if (!isdefined(actual) || !contains_entity(actual, self) || !isint(slot) || slot < 0 || slot >= capacity)
        return undefined;
    if (!level flag::get("lockdown_active"))
    {
        if (!level flag::get("players_in_arena")) return undefined;
        points = struct::get_array("player_respawn_point_arena", "targetname");
        if (!gauntlet_points_valid(points)) return undefined;
        return arraygetfarthest(self.origin, points);
    }
    zone = zm_zonemgr::get_zone_from_position(self.origin + (0, 0, 32), 0);
    if (zone == "pavlovs_A_zone")
    {
        requested = struct::get_array("gauntlet_bgb_teleport_" + slot, "targetname");
        if (isarray(requested) && requested.size > 0)
        {
            if (!gauntlet_points_valid(requested)) return undefined;
            return arraygetfarthest(self.origin, requested);
        }
        if ((isdefined(requested) && !isarray(requested)) || slot < 4) return undefined;
        points = struct::get_array("gauntlet_bgb_teleport_" + (slot % 4), "targetname");
        if (!gauntlet_points_valid(points)) return undefined;
        return arraygetfarthest(self.origin, points);
    }
    if (zone == "pavlovs_B_zone" || zone == "pavlovs_C_zone")
    {
        master = struct::get("pavlovs_master_respawn", "script_label");
        if (!isdefined(master) || !isdefined(master.target) || !isstring(master.target) || master.target == "")
            return undefined;
        points = struct::get_array(master.target, "targetname");
        return gauntlet_bc_destination(points, master.target, slot, capacity);
    }
    return undefined;
}

// Generic stock selection probes enabled zones and chooses one random room.
// Keep that flow activation-only: validation must not consume RNG or spawn the
// stock temporary zone probe. Reuse the same canonical labels only inside the
// one selected existing room; never retry into a different room on failure.


function gauntlet_callback_owned()
{
    return isdefined(level.var_2c12d9a6) &&
        (level.var_2c12d9a6 == &gauntlet_respawn || level.var_2c12d9a6 == &zm_stalingrad::function_277575cc);
}

// Pinned BGB activation dereferences callback.origin without an undefined
// check. Pair the map callback redirect with this exact BGB register reference
// redirect; malformed layouts must be denied before activation/increment.


// Exact original owner: zm_stalingrad::function_277575cc, self=player, argc0;
// assigned to level.var_2c12d9a6 in the map entry body.
function gauntlet_respawn()
{
    epoch = getdvarint("zm_coopStartupEpoch");
    capacity = getdvarint("zm_coopAppliedLimit");
    slot = self getentitynumber();
    if (stock_capacity(capacity))
        return self zm_stalingrad::function_277575cc();
    if (!context_current(epoch, capacity) || !gauntlet_callback_owned()) return undefined;
    if (!level flag::get("lockdown_active"))
    {
        if (level flag::get("players_in_arena")) return self gauntlet_checked_destination();
        return self eroded_coop_bgb_transport::generic_respawn();
    }
    zone = zm_zonemgr::get_zone_from_position(self.origin + (0, 0, 32), 0);
    if (zone != "pavlovs_A_zone" && zone != "pavlovs_B_zone" && zone != "pavlovs_C_zone")
        return undefined;
    return self gauntlet_checked_destination();
}









// Pair with gauntlet_validation at the pinned BGB registration. Selection is
// repeated before this wrapper's first mutation, rather than trusting an older
// validation result. The upstream caller already consumed a one-shot use;
// declining here intentionally performs no invented charge/refund behavior.
