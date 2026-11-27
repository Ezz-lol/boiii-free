// SPDX-License-Identifier: GPL-3.0-only
// Experimental map mechanics contributed by Eroded Networks.
// Loaded only through the native, opt-in 1-8 map adapter.
// Only exact caller-owned redirects may invoke these wrappers. The direct
// original imports below must remain external and excluded from redirection.
#using custom_scripts/zm/eroded_coop;
#using scripts/shared/util_shared;
#using scripts/shared/visionset_mgr_shared;
#using scripts/shared/exploder_shared;
#using scripts/zm/zm_factory_teleporter;
#namespace eroded_coop_factory;

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

function teleport_journal(ctx)
{
    if (!current(ctx)) return undefined;
    if (isdefined(level.eroded_coop_factory_sequence) &&
        (!isint(level.eroded_coop_factory_sequence) || level.eroded_coop_factory_sequence >= 2147483647))
        return undefined;
    op = spawnstruct();
    op.ctx = ctx;
    op.entries = array();
    op.aborted = false;
    op.done = false;
    if (!isdefined(level.eroded_coop_factory_sequence))
        level.eroded_coop_factory_sequence = 0;
    level.eroded_coop_factory_sequence++;
    op.done_event = "eroded_coop_factory_done_" + level.eroded_coop_factory_sequence;
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
        !isdefined(player) || !roster_valid(array(player), op.ctx) || isdefined(player.eroded_coop_factory_entry) ||
        isdefined(player.teleport_origin) || (isdefined(player.b_teleporting) && player.b_teleporting))
        return undefined;
    entry = spawnstruct();
    entry.player = player;
    entry.ctx = op.ctx;
    entry.source_origin = player.origin;
    entry.source_angles = player.angles;
    entry.origin = undefined;
    entry.overlay = false;
    entry.weapons = false;
    entry.controls = false;
    entry.black_box = false;
    entry.cleaned = false;
    player.eroded_coop_factory_entry = entry;
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
    if (!isdefined(player) || !cleanup_identity(entry.ctx) || player.eroded_coop_factory_entry != entry) return false;
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
    if (isdefined(player) && cleanup_identity(entry.ctx) && player.eroded_coop_factory_entry == entry)
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
        if (entry.overlay) visionset_mgr::deactivate("overlay", "zm_factory_teleport", player);
        if (entry.weapons)
        {
            player enableweapons();
            player enableoffhandweapons();
        }
        if (entry.controls) player freezecontrols(false);
        player.b_teleporting = 0;
        player.eroded_coop_factory_entry = undefined;
    }
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


function teleport_players()
{
    if (stock_path_allowed("zm_factory")) return self zm_factory_teleporter::teleport_players();
    ctx = capture("zm_factory");
    if (!isdefined(ctx)) return;
    players = getplayers();
    if (!roster_valid(players, ctx) || players.size == 0) return;
    core_pos = four_entities("origin_teleport_player_");
    image_room = four_entities("teleport_room_");
    core = getent("trigger_teleport_core", "targetname");
    if (!isdefined(core_pos) || !isdefined(image_room) || !isdefined(core)) return;
    touching = array();
    for (i = 0; i < players.size; i++)
    {
        player = players[i];
        if (!self zm_factory_teleporter::player_is_near_pad(player)) continue;
        if (isdefined(player.teleport_origin) || isdefined(player.eroded_coop_factory_entry) ||
            (isdefined(player.b_teleporting) && player.b_teleporting)) return;
        touching[touching.size] = player;
    }
    if (!touching.size) return;
    op = teleport_journal(ctx);
    if (!isdefined(op)) return;
    for (i = 0; i < players.size; i++)
    {
        player = players[i];
        if (!isdefined(player)) continue;
        if (!self zm_factory_teleporter::player_is_near_pad(player)) continue;
        if (op.done || op.aborted || !current(ctx)) { teleport_abort(op); return; }
        entry = teleport_record(op, player);
        if (!isdefined(entry)) { teleport_abort(op); return; }
        live = getplayers();
        if (!roster_valid(live, ctx)) { teleport_abort(op); return; }
        image_occupied = occupancy(image_room, live);
        image_slot = choose(image_occupied, i % 4, live.size > 4);
        if (!isdefined(image_slot)) { teleport_abort(op); return; }
        room = image_room[image_slot];
        if (!isdefined(room) || !isdefined(core)) { teleport_abort(op); return; }
        player.b_teleporting = 1;
        visionset_mgr::deactivate("overlay", "zm_trap_electric", player);
        entry.overlay = true;
        visionset_mgr::activate("overlay", "zm_factory_teleport", player);
        entry.weapons = true;
        player disableoffhandweapons();
        player disableweapons();
        desired_origin = room.origin;
        if (player getstance() == "prone") desired_origin += (0, 0, 49);
        else if (player getstance() == "crouch") desired_origin += (0, 0, 20);
        entry.origin = spawn("script_origin", player.origin);
        if (!isdefined(entry.origin)) { teleport_abort(op); return; }
        player.teleport_origin = entry.origin;
        entry.origin.angles = player.angles;
        if (!(player arm_grace(ctx))) { teleport_abort(op); return; }
        player linkto(entry.origin);
        entry.origin.origin = desired_origin;
        entry.controls = true;
        player freezecontrols(true);
        util::wait_network_frame();
        if (op.done || op.aborted || !current(ctx)) { teleport_abort(op); return; }
        if (entry.cleaned || !isdefined(player)) continue;
        if (!isdefined(entry.origin) || !isdefined(room)) { teleport_abort(op); return; }
        entry.black_box = true;
        util::setclientsysstate("levelNotify", "black_box_start", player);
        entry.origin.angles = room.angles;
    }
    wait 2;
    if (op.done || op.aborted || !current(ctx) || !isdefined(core)) { teleport_abort(op); return; }
    current_players = getplayers();
    if (!roster_valid(current_players, ctx)) { teleport_abort(op); return; }
    if (!current_players.size) { teleport_abort(op); return; }
    core thread zm_factory_teleporter::teleport_nuke(undefined, 300);
    for (j = 0; j < 4; j++)
        if (!isdefined(core_pos[j])) { teleport_abort(op); return; }
    occupied = occupancy(core_pos, current_players);
    for (i = 0; i < current_players.size; i++)
        util::setclientsysstate("levelNotify", "black_box_end", current_players[i]);
    util::wait_network_frame();
    if (op.done || op.aborted || !current(ctx)) { teleport_abort(op); return; }
    // Recompute after the last yield; a disconnect can change overlap policy.
    current_players = getplayers();
    if (!roster_valid(current_players, ctx)) { teleport_abort(op); return; }
    occupied = occupancy(core_pos, current_players);
    for (i = 0; i < op.entries.size; i++)
    {
        entry = op.entries[i];
        player = entry.player;
        if (entry.cleaned || !isdefined(player)) { teleport_cleanup_entry(entry, true); continue; }
        if (!current(ctx) || player.eroded_coop_factory_entry != entry) { teleport_abort(op); return; }
        selected = choose(occupied, i % 4, current_players.size > 4);
        if (!isdefined(selected) || !isdefined(core_pos[selected])) { teleport_abort(op); return; }
        if (!(player arm_grace(ctx)) || !teleport_release_link(entry)) { teleport_abort(op); return; }
        player setorigin(core_pos[selected].origin);
        player setplayerangles(core_pos[selected].angles);
        teleport_cleanup_entry(entry, false);
        player thread zm_factory_teleporter::teleport_aftereffects();
        vox_rand = randomintrange(1, 100);
    }
    teleport_finish(op);
    exploder::exploder_duration("mainframe_arrival", 1.7);
    exploder::exploder_duration("mainframe_steam", 14.6);
}
