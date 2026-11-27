// SPDX-License-Identifier: GPL-3.0-only
// Experimental map mechanics contributed by Eroded Networks.
// Loaded only through the native, opt-in 1-8 map adapter.
// Only exact caller-owned redirects may invoke these wrappers. The direct
// original imports below must remain external and excluded from redirection.
#using custom_scripts/zm/eroded_coop;
#using scripts/shared/util_shared;
#using scripts/shared/visionset_mgr_shared;
#using scripts/shared/exploder_shared;
#using scripts/shared/array_shared;
#using scripts/shared/clientfield_shared;
#using scripts/shared/scene_shared;
#using scripts/codescripts/struct;
#using scripts/zm/zm_theater_teleporter;
#namespace eroded_coop_theater;

function stock_path_allowed(map)
{
    if (getdvarstring("mapname") != map) return false;
    applied = getdvarint("zm_coopAppliedLimit");
    schema = getdvarint("zm_coopSchemaSlots");
    if (applied < 1 || applied > 4 || schema < 0 || schema > 4) return false;
    if (isdefined(level.eroded_coop_limit) &&
        (!isint(level.eroded_coop_limit) || level.eroded_coop_limit < 1 || level.eroded_coop_limit > 4))
        return false;
    return true;
}

function capture(map)
{
    if (map != "zm_factory" && map != "zm_theater") return undefined;
    if (!isdefined(level.eroded_coop_started_epoch) ||
        !isint(level.eroded_coop_started_epoch) || level.eroded_coop_started_epoch < 1 ||
        !isdefined(level.eroded_coop_limit) || !isint(level.eroded_coop_limit) ||
        level.eroded_coop_limit < 5 || level.eroded_coop_limit > 8)
        return undefined;
    ctx = spawnstruct();
    ctx.map = map;
    ctx.epoch = level.eroded_coop_started_epoch;
    ctx.limit = level.eroded_coop_limit;
    if (!current(ctx)) return undefined;
    return ctx;
}

function cleanup_identity(ctx)
{
    return isdefined(ctx) && isdefined(ctx.limit) && isint(ctx.limit) && ctx.limit >= 5 && ctx.limit <= 8 &&
        isdefined(ctx.epoch) && isint(ctx.epoch) && ctx.epoch > 0 &&
        isdefined(ctx.map) && isstring(ctx.map) && (ctx.map == "zm_factory" || ctx.map == "zm_theater") &&
        isdefined(level.eroded_coop_limit) && isint(level.eroded_coop_limit) && level.eroded_coop_limit == ctx.limit &&
        isdefined(level.eroded_coop_started_map) && isstring(level.eroded_coop_started_map) && level.eroded_coop_started_map == ctx.map &&
        isdefined(level.eroded_coop_started_epoch) && isint(level.eroded_coop_started_epoch) && level.eroded_coop_started_epoch == ctx.epoch &&
        getdvarint("zm_coopAppliedLimit") == ctx.limit && getdvarint("zm_coopStartupEpoch") == ctx.epoch &&
        getdvarstring("mapname") == ctx.map && eroded_coop::startup_identity_current(ctx.epoch, ctx.map);
}

function current(ctx)
{
    return cleanup_identity(ctx) && getdvarint("zm_coopSchemaSlots") == ctx.limit &&
        eroded_coop::startup_current(ctx.epoch, ctx.map) &&
        eroded_coop::owns_callbacks() && level.player_intersection_tracker_override == &eroded_coop::intersection_override;
}

function roster_shape_valid(players, ctx)
{
    if (!current(ctx) || !isarray(players) || players.size > ctx.limit)
        return false;
    keys = getarraykeys(players);
    for (i = 0; i < keys.size; i++)
        if (!isint(keys[i]) || keys[i] < 0 || keys[i] >= players.size) return false;
    for (i = 0; i < players.size; i++)
    {
        if (!isdefined(players[i]) || !isentity(players[i]) || !isplayer(players[i])) return false;
        if (isdefined(players[i].player_disconnected) && players[i].player_disconnected == 1) return false;
        entity_slot = players[i] getentitynumber();
        if (!isint(entity_slot) || entity_slot < 0 || entity_slot >= ctx.limit) return false;
        for (j = 0; j < i; j++)
            if (players[i] == players[j] || entity_slot == players[j] getentitynumber()) return false;
    }
    return true;
}

function roster_valid(players, ctx)
{
    if (!roster_shape_valid(players, ctx)) return false;
    actual = getplayers();
    if (!roster_shape_valid(actual, ctx)) return false;
    for (i = 0; i < players.size; i++)
    {
        member = false;
        for (j = 0; j < actual.size; j++)
            if (players[i] == actual[j]) member = true;
        if (!member) return false;
    }
    return true;
}

function four_entities(stem)
{
    result = array();
    for (i = 0; i < 4; i++)
    {
        ent = getent(stem + i, "targetname");
        if (!isdefined(ent) || !isdefined(ent.origin) || !isdefined(ent.angles)) return undefined;
        for (j = 0; j < i; j++)
            if (ent == result[j]) return undefined;
        result[i] = ent;
    }
    return result;
}

function slot(index)
{
    if (!isint(index) || index < 0 || index > 7) return undefined;
    return index % 4;
}

function occupancy(physical, players)
{
    result = array();
    for (j = 0; j < 4; j++)
    {
        result[j] = false;
        for (i = 0; i < players.size; i++)
            if (isdefined(players[i]) && distance2d(physical[j].origin, players[i].origin) < 16)
                result[j] = true;
    }
    return result;
}

function choose(occupied, first, allow_reuse)
{
    start = slot(first);
    if (!isdefined(start)) return undefined;
    for (attempt = 0; attempt < 4; attempt++)
    {
        candidate = (start + attempt) % 4;
        if (!occupied[candidate])
        {
            occupied[candidate] = true;
            return candidate;
        }
    }
    // A disconnect can leave <=4 real players, where the existing intersection
    // callback no longer grants grace. Refuse reuse in that reduced roster.
    if (!allow_reuse) return undefined;
    // Four physical destinations remain four. Reuse requires owned grace.
    occupied[start] = true;
    return start;
}

function arm_grace(ctx)
{
    if (!current(ctx)) return false;
    expiry = gettime() + 2500;
    if (!isdefined(self.eroded_coop_spawn_grace) || self.eroded_coop_spawn_grace < expiry)
        self.eroded_coop_spawn_grace = expiry;
    return true;
}

function theater_spots(sname, spots, ctx)
{
    if (!current(ctx) || ctx.map != "zm_theater" || !isarray(spots) || spots.size != 0)
        return undefined;
    if (sname != "teleport_room_" && sname != "projroom_teleport_player" &&
        sname != "ee_teleport_player" && sname != "theater_teleport_player")
        return undefined;
    physical = four_entities(sname);
    if (!isdefined(physical)) return undefined;
    result = array();
    for (i = 0; i < ctx.limit; i++) result[i] = physical[i % 4];
    return result;
}

function theater_fx_room(index, ctx)
{
    if (!current(ctx) || ctx.map != "zm_theater") return undefined;
    physical_slot = slot(index);
    if (!isdefined(physical_slot)) return undefined;
    room = struct::get("teleport_room_fx_" + (physical_slot + 1), "targetname");
    if (!isdefined(room) || !isdefined(room.origin) || !isdefined(room.angles)) return undefined;
    return room;
}

function teleport_journal(ctx)
{
    if (!current(ctx)) return undefined;
    if (isdefined(level.eroded_coop_theater_sequence) &&
        (!isint(level.eroded_coop_theater_sequence) || level.eroded_coop_theater_sequence >= 2147483647))
        return undefined;
    op = spawnstruct();
    op.ctx = ctx;
    op.entries = array();
    op.aborted = false;
    op.done = false;
    if (!isdefined(level.eroded_coop_theater_sequence))
        level.eroded_coop_theater_sequence = 0;
    level.eroded_coop_theater_sequence++;
    op.done_event = "eroded_coop_theater_done_" + level.eroded_coop_theater_sequence;
    // Register terminal observers before mutating any player or yielding.
    level thread teleport_terminal_watch(op, #"pre_end_game");
    level thread teleport_terminal_watch(op, #"end_game");
    level thread teleport_terminal_watch(op, #"intermission");
    level thread teleport_context_watch(op);
    return op;
}

function teleport_record(op, player)
{
    if (op.done || op.aborted || !current(op.ctx) ||
        !isdefined(player) || !roster_valid(array(player), op.ctx) || isdefined(player.eroded_coop_theater_entry) ||
        isdefined(player.teleport_origin) || (isdefined(player.is_teleporting) && player.is_teleporting))
        return undefined;
    entry = spawnstruct();
    entry.player = player;
    entry.ctx = op.ctx;
    entry.source_origin = player.origin;
    entry.source_angles = player.angles;
    entry.source_inteleportation = player.inteleportation;
    entry.source_weapons_held = isdefined(player.eroded_coop_theater_weapons_held) && player.eroded_coop_theater_weapons_held;
    entry.keep_weapons_disabled = false;
    entry.fx = undefined;
    entry.origin = undefined;
    entry.overlay = false;
    entry.weapons = false;
    entry.controls = false;
    entry.black_box = false;
    entry.cleaned = false;
    player.eroded_coop_theater_entry = entry;
    op.entries[op.entries.size] = entry;
    // The disconnect event is observed, never an endon that skips cleanup.
    player thread teleport_disconnect_watch(op, entry);
    return entry;
}

function teleport_disconnect_watch(op, entry)
{
    level endon(op.done_event);
    self waittill(#"disconnect");
    teleport_cleanup_entry(entry, true);
}

function teleport_terminal_watch(op, terminal)
{
    level endon(op.done_event);
    level waittill(terminal);
    teleport_abort(op);
}

function teleport_context_watch(op)
{
    level endon(op.done_event);
    while (!op.done)
    {
        if (!current(op.ctx))
        {
            teleport_abort(op);
            return;
        }
        wait 0.05;
    }
}

function teleport_release_link(entry)
{
    player = entry.player;
    if (!isdefined(player) || !cleanup_identity(entry.ctx) || player.eroded_coop_theater_entry != entry) return false;
    if (isdefined(entry.origin) && player.teleport_origin == entry.origin)
    {
        player unlink();
        player.teleport_origin = undefined;
    }
    if (isdefined(entry.origin)) entry.origin delete();
    entry.origin = undefined;
    return true;
}

function teleport_cleanup_entry(entry, restore_departure)
{
    if (entry.cleaned) return;
    entry.cleaned = true;
    player = entry.player;
    if (isdefined(player) && cleanup_identity(entry.ctx) && player.eroded_coop_theater_entry == entry)
    {
        teleport_release_link(entry);
        // A shrink-to-four abort returns to a known departure point instead of
        // leaving a playable, unfrozen player inside an image room. A map/epoch
        // replacement must never receive coordinates captured by the old map.
        if (restore_departure)
        {
            player setorigin(entry.source_origin);
            player setplayerangles(entry.source_angles);
        }
        if (entry.black_box) util::setclientsysstate("levelNotify", "black_box_end", player);
        if (entry.overlay) visionset_mgr::deactivate("overlay", "zm_theater_teleport", player);
        if (entry.weapons)
        {
            if (restore_departure && entry.source_weapons_held)
            {
                player disableweapons();
                player disableoffhandweapons();
            }
            else if (restore_departure || !entry.keep_weapons_disabled)
            {
                player enableweapons();
                player enableoffhandweapons();
            }
        }
        if (entry.controls) player freezecontrols(false);
        if (restore_departure)
        {
            player.inteleportation = entry.source_inteleportation;
            player.eroded_coop_theater_weapons_held = entry.source_weapons_held;
        }
        else player.eroded_coop_theater_weapons_held = entry.keep_weapons_disabled;
        player.is_teleporting = 0;
        player.eroded_coop_theater_entry = undefined;
    }
    if (isdefined(entry.fx)) entry.fx struct::delete();
    entry.fx = undefined;
    // Delete the saved owned entity even when its player disconnected.
    if (isdefined(entry.origin)) entry.origin delete();
    entry.origin = undefined;
}

function teleport_abort(op)
{
    if (op.done) return;
    op.aborted = true;
    for (i = 0; i < op.entries.size; i++) teleport_cleanup_entry(op.entries[i], true);
    teleport_finish(op);
}

function teleport_finish(op)
{
    if (op.done) return;
    op.done = true;
    level notify(op.done_event);
}


function get_array_spots(sname, spots)
{
    if (stock_path_allowed("zm_theater")) return zm_theater_teleporter::get_array_spots(sname, spots);
    ctx = capture("zm_theater");
    if (!isdefined(ctx)) return undefined;
    return theater_spots(sname, spots, ctx);
}

function function_7e0ed731(index, offset)
{
    if (stock_path_allowed("zm_theater")) return self zm_theater_teleporter::function_7e0ed731(index, offset);
    ctx = capture("zm_theater");
    if (!isdefined(ctx)) return;
    room = theater_fx_room(index, ctx);
    if (!isdefined(room)) return;
    wormhole = struct::spawn(room.origin - offset, room.angles);
    if (!isdefined(wormhole)) return;
    entry = self.eroded_coop_theater_entry;
    if (isdefined(entry) && !entry.cleaned) entry.fx = wormhole;
    waittillframeend();
    if (!isdefined(wormhole)) return;
    if (!current(ctx) || !isdefined(self))
    {
        wormhole struct::delete();
        if (isdefined(entry) && entry.fx == wormhole) entry.fx = undefined;
        return;
    }
    wormhole scene::play("p7_fxanim_zm_kino_wormhole_bundle");
    if (isdefined(wormhole)) wormhole struct::delete();
    if (isdefined(entry) && entry.fx == wormhole) entry.fx = undefined;
}

function teleport_players(requested, loc)
{
    if (stock_path_allowed("zm_theater")) return self zm_theater_teleporter::teleport_players(requested, loc);
    ctx = capture("zm_theater");
    if (!isdefined(ctx)) return array();
    all_players = getplayers();
    if (!roster_valid(all_players, ctx) || !roster_valid(requested, ctx)) return array();
    if (all_players.size == 0 || (loc != "projroom" && requested.size == 0)) return requested;
    if (loc != "projroom" && loc != "eerooms" && loc != "theater") return array();
    image_room = four_entities("teleport_room_");
    stem = "projroom_teleport_player";
    if (loc == "eerooms") stem = "ee_teleport_player";
    else if (loc == "theater") stem = "theater_teleport_player";
    destination = four_entities(stem);
    if (!isdefined(image_room) || !isdefined(destination)) return array();
    for (i = 0; i < 4; i++)
        if (!isdefined(theater_fx_room(i, ctx))) return array();
    players = requested;
    if (loc == "projroom") players = all_players;
    touching = array();
    for (i = 0; i < players.size; i++)
    {
        player = players[i];
        if (loc == "projroom" && !self zm_theater_teleporter::player_is_near_pad(player)) continue;
        if (isdefined(player.teleport_origin) || isdefined(player.eroded_coop_theater_entry) ||
            (isdefined(player.is_teleporting) && player.is_teleporting)) return array();
        touching[touching.size] = player;
    }
    if (!touching.size) return requested;
    op = teleport_journal(ctx);
    if (!isdefined(op)) return array();
    occupied = occupancy(image_room, all_players);
    for (i = 0; i < touching.size; i++)
    {
        if (op.done || op.aborted || !current(ctx)) { teleport_abort(op); return array(); }
        player = touching[i];
        if (!isdefined(player)) continue;
        entry = teleport_record(op, player);
        if (!isdefined(entry)) { teleport_abort(op); return array(); }
        if (loc == "projroom") array::add(requested, player, 0);
        player.is_teleporting = 1;
        player clientfield::increment("player_teleport_fx");
        player clientfield::set_to_player("player_dust_mote", 0);
        entry.controls = true;
        player freezecontrols(true);
        entry.weapons = true;
        player disableweapons();
        player disableoffhandweapons();
        util::wait_network_frame();
        if (op.done || op.aborted || !current(ctx)) { teleport_abort(op); return array(); }
        if (entry.cleaned || !isdefined(player)) continue;
        live = getplayers();
        if (!roster_valid(live, ctx)) { teleport_abort(op); return array(); }
        occupied = occupancy(image_room, live);
        selected = choose(occupied, i % 4, live.size > 4);
        if (!isdefined(selected) || !isdefined(image_room[selected])) { teleport_abort(op); return array(); }
        room = image_room[selected];
        player.inteleportation = 1;
        entry.overlay = true;
        visionset_mgr::activate("overlay", "zm_theater_teleport", player);
        offset = (0, 0, 0);
        if (player getstance() == "prone") offset = (0, 0, 49);
        else if (player getstance() == "crouch") offset = (0, 0, 20);
        entry.black_box = true;
        util::setclientsysstate("levelNotify", "black_box_start", player);
        if (!(player arm_grace(ctx))) { teleport_abort(op); return array(); }
        player setorigin(room.origin);
        player setplayerangles(room.angles);
        entry.origin = spawn("script_origin", player.origin);
        if (!isdefined(entry.origin)) { teleport_abort(op); return array(); }
        player.teleport_origin = entry.origin;
        entry.origin.angles = player.angles;
        player linkto(entry.origin);
        player thread function_7e0ed731(selected, offset);
        player playrumbleonentity("zm_castle_moon_explosion_rumble");
    }
    wait 2;
    if (op.done || op.aborted || !current(ctx)) { teleport_abort(op); return array(); }
    live = getplayers();
    if (!roster_valid(live, ctx)) { teleport_abort(op); return array(); }
    for (j = 0; j < 4; j++)
        if (!isdefined(destination[j])) { teleport_abort(op); return array(); }
    occupied = occupancy(destination, live);
    if (loc == "eerooms") level.eeroomsinuse = 1;
    if (loc == "theater" && isdefined(self.target))
    {
        ent = getent(self.target, "targetname");
        self thread zm_theater_teleporter::teleport_nuke(undefined, 20);
    }
    retained = array();
    for (i = 0; i < op.entries.size; i++)
    {
        entry = op.entries[i];
        player = entry.player;
        if (entry.cleaned || !isdefined(player)) { teleport_cleanup_entry(entry, true); continue; }
        if (!current(ctx) || player.eroded_coop_theater_entry != entry) { teleport_abort(op); return array(); }
        if (!zm_theater_teleporter::function_1488cf91(player))
        {
            teleport_cleanup_entry(entry, true);
            continue;
        }
        live = getplayers();
        if (!roster_valid(live, ctx)) { teleport_abort(op); return array(); }
        occupied = occupancy(destination, live);
        selected = choose(occupied, randomintrange(0, 4), live.size > 4);
        if (!isdefined(selected) || !isdefined(destination[selected])) { teleport_abort(op); return array(); }
        util::setclientsysstate("levelNotify", "black_box_end", player);
        entry.black_box = false;
        player notify(#"stop_teleport_fx");
        if (!(player arm_grace(ctx)) || !teleport_release_link(entry)) { teleport_abort(op); return array(); }
        player setorigin(destination[selected].origin);
        player setplayerangles(destination[selected].angles);
        player clientfield::increment("player_teleport_fx");
        entry.keep_weapons_disabled = loc == "eerooms";
        // Original aftereffects and room-specific side effects are preserved.
        util::setclientsysstate("levelNotify", "t2bfx", player);
        visionset_mgr::deactivate("overlay", "zm_theater_teleport", player);
        entry.overlay = false;
        // Stock aftereffects can yield. Restore controls before entering them,
        // while the journal/observers continue owning disconnected/abort cleanup.
        if (!entry.keep_weapons_disabled) { player enableweapons(); player enableoffhandweapons(); }
        player freezecontrols(false);
        player zm_theater_teleporter::teleport_aftereffects();
        if (op.done || op.aborted || !current(ctx)) { teleport_abort(op); return array(); }
        if (entry.cleaned || !isdefined(player)) continue;
        if (loc == "projroom")
        {
            level.second_hand thread zm_theater_teleporter::start_wall_clock();
            thread zm_theater_teleporter::extra_cam_startup();
            player clientfield::set_to_player("player_dust_mote", 1);
        }
        else if (loc == "theater")
        {
            player.inteleportation = 0;
            player.var_35c3d096 = undefined;
            player clientfield::set_to_player("player_dust_mote", 1);
        }
        else player notify(#"player_teleported", selected);
        retained[retained.size] = player;
        teleport_cleanup_entry(entry, false);
    }
    teleport_finish(op);
    if (loc == "projroom") return retained;
    if (loc == "theater")
    {
        level.eeroomsinuse = undefined;
        exploder::exploder(302);
    }
}
