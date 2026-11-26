// Independently authored Island task allocator and lifecycle adapter.
// Loaded only by the native Island-only early schema dependency, never custom_scripts.
#using scripts/shared/array_shared;
#using scripts/codescripts/struct;
#using scripts/shared/callbacks_shared;
#using scripts/shared/clientfield_shared;
#using scripts/shared/flag_shared;
#using scripts/shared/system_shared;
#using scripts/zm/_zm_unitrigger;
#using scripts/zm/zm_island_challenges;

#namespace eroded_coop_island_trials;

function schema_current()
{
    capacity = getdvarint("zm_coopSchemaSlots");
    return isint(capacity) && capacity >= 5 && capacity <= 8 &&
        getdvarstring("mapname") == "zm_island" &&
        getdvarint("zm_coopSchemaServerRevision") == 3;
}

function autoexec __init__system__()
{
    if (getdvarstring("mapname") != "zm_island") return;
    if (isdefined(level.system_funcs) && isdefined(level.system_funcs["eroded_coop_island_trials"])) return;
    // The stock dependency executor is not recursive. Keep both direct requirements ordered.
    system::register("eroded_coop_island_trials", &early_install, undefined,
        array("zm_laststand", "eroded_coop_fields"));
}

function valid_capacity(capacity)
{
    return isint(capacity) && capacity >= 1 && capacity <= 8;
}

function valid_slot(slot, capacity)
{
    return valid_capacity(capacity) && isint(slot) && slot >= 0 && slot < capacity;
}

// The caller has verified the stock registry and pristine stock descriptor pools.
// These are the five fields produced by stock init_challenges, not arbitrary mod records.
function valid_task(task, tier)
{
    if (!isdefined(task) || !isint(tier) || tier < 1 || tier > 3) return false;
    if (!isdefined(task.n_index) || !isint(task.n_index) || task.n_index != tier ||
        !isdefined(task.n_count) || !isint(task.n_count) || task.n_count < 1 || task.n_count > 30 ||
        !isdefined(task.str_info) || !isdefined(task.str_notify) || !isstring(task.str_notify)) return false;
    if (!isdefined(task.func_think)) return true;
    return (tier == 1 && task.func_think == &zm_island_challenges::function_2dbc7cd3) ||
        (tier == 2 && task.func_think == &zm_island_challenges::function_25c1bab7) ||
        (tier == 3 && (task.func_think == &zm_island_challenges::function_5a96677a ||
        task.func_think == &zm_island_challenges::function_26c58398));
}

function clone_task(task, tier)
{
    if (!valid_task(task, tier)) return undefined;
    copy = spawnstruct();
    copy.n_index = task.n_index;
    copy.str_info = task.str_info;
    copy.n_count = task.n_count;
    copy.str_notify = task.str_notify;
    copy.func_think = task.func_think;
    return copy;
}

function make_pool(templates, tier, capacity)
{
    if (!valid_capacity(capacity) || !isarray(templates)) return undefined;
    expected = 5;
    if (tier == 2) expected = 6;
    if (templates.size != expected) return undefined;
    keys = getarraykeys(templates);
    for (key_index = 0; key_index < keys.size; key_index++)
    {
        key = keys[key_index];
        if (!isint(key) || key < 0 || key >= expected) return undefined;
    }
    state = spawnstruct();
    state.catalog = array();
    state.available = array();
    state.assigned = array();
    state.owners = array();
    state.tier = tier;
    state.capacity = capacity;
    state.allocated = expected;
    for (index = 0; index < expected; index++)
    {
        if (!valid_task(templates[index], tier)) return undefined;
        // Catalog retains immutable stock template references; it allocates no extra task records.
        state.catalog[index] = templates[index];
        state.available[index] = templates[index];
    }
    return state;
}

function take_task(state, slot, owner, selection)
{
    if (!isdefined(state) || !valid_slot(slot, state.capacity) || !isdefined(owner) ||
        !isint(selection) || selection < 0 || selection > 7) return undefined;
    if (isdefined(state.assigned[slot]))
    {
        if (state.owners[slot] == owner) return state.assigned[slot];
        return undefined;
    }
    if (state.available.size > 0)
    {
        if (selection >= state.available.size) return undefined;
        keys = getarraykeys(state.available);
        task = state.available[keys[selection]];
        // Match stock array::random's key selection and original remove-by-value operation.
        // Every assignment record is independent, so remove-by-value cannot remove a sibling's task.
        arrayremovevalue(state.available, task);
    }
    else
    {
        if (state.allocated >= state.capacity || state.allocated >= 8 || selection >= state.catalog.size)
            return undefined;
        task = clone_task(state.catalog[selection], state.tier);
        if (!isdefined(task)) return undefined;
        state.allocated++;
    }
    state.assigned[slot] = task;
    state.owners[slot] = owner;
    return task;
}

function return_task(state, slot, owner)
{
    if (!isdefined(state) || !valid_slot(slot, state.capacity) || !isdefined(owner) ||
        !isdefined(state.assigned[slot]) || state.owners[slot] != owner) return false;
    if (state.available.size >= state.allocated || state.allocated > 8) return false;
    available_index = -1;
    for (index = 0; index < 8; index++)
    {
        if (!isdefined(state.available[index]))
        {
            available_index = index;
            break;
        }
    }
    if (available_index < 0) return false;
    task = state.assigned[slot];
    state.assigned[slot] = undefined;
    state.owners[slot] = undefined;
    // Preserve every survivor even if native remove-by-value leaves sparse keys.
    state.available[available_index] = task;
    return true;
}


function registry_count(entries, func)
{
    if (!isarray(entries)) return -1;
    count = 0;
    keys = getarraykeys(entries);
    for (key_index = 0; key_index < keys.size; key_index++)
    {
        entry = entries[keys[key_index]];
        // An undefined optional object may leave a one-element stock array.
        if (!isarray(entry) || entry.size < 1 || entry.size > 2 || !isdefined(entry[0])) return -1;
        if (entry[0] == func && !isdefined(entry[1])) count++;
    }
    return count;
}

function early_install()
{
    if (!schema_current()) return;
    if (isdefined(level.eroded_island_trials_revision))
    {
        if (!registry_owned()) decline("early_ownership_lost");
        return;
    }
    if (isdefined(level.eroded_island_trials_pending)) return;
    epoch = getdvarint("zm_coopStartupEpoch");
    capacity = getdvarint("zm_coopSchemaSlots");
    if (!isint(epoch) || epoch < 1 || !valid_capacity(capacity)) return;
    pending = spawnstruct();
    pending.epoch = epoch;
    pending.capacity = capacity;
    pending.map = "zm_island";
    level.eroded_island_trials_ready = false;
    level.eroded_island_trials_pool_api = undefined;
    level.eroded_island_trials_pending = pending;
    setdvar("zm_coopIslandTrialsStatus", "registry_wait");
    level thread deferred_install(pending);
}

function pending_context_current(pending)
{
    return schema_current() && isdefined(pending) &&
        isdefined(level.eroded_island_trials_pending) && level.eroded_island_trials_pending == pending &&
        isint(pending.epoch) && pending.epoch > 0 && pending.epoch == getdvarint("zm_coopStartupEpoch") &&
        isint(pending.capacity) && pending.capacity >= 5 && pending.capacity <= 8 &&
        pending.capacity == getdvarint("zm_coopSchemaSlots") && pending.map == getdvarstring("mapname");
}

function finish_pending(pending, reason)
{
    if (!isdefined(level.eroded_island_trials_pending) || level.eroded_island_trials_pending != pending) return;
    level.eroded_island_trials_pending = undefined;
    level.eroded_island_trials_ready = false;
    level.eroded_island_trials_pool_api = undefined;
    // An old worker cannot publish over a later map/session's diagnostics.
    if (!isdefined(pending) || pending.map != getdvarstring("mapname") ||
        pending.epoch != getdvarint("zm_coopStartupEpoch") ||
        pending.capacity != getdvarint("zm_coopSchemaSlots")) return;
    setdvar("zm_coopIslandTrialsStatus", reason);
}

function pending_registry_count(entries, func)
{
    if (!isdefined(entries)) return 0;
    count = registry_count(entries, func);
    if (count < 0) return -1;
    keys = getarraykeys(entries);
    for (index = 0; index < keys.size; index++)
    {
        entry = entries[keys[index]];
        if (entry[0] == func && isdefined(entry[1])) return -2;
    }
    return count;
}

function deferred_install(pending)
{
    level endon(#"end_game");
    level endon(#"intermission");
    // Return from the system preinit callback before inspecting map registrations.
    // No player/challenge flag is forced or awaited at this scheduling seam.
    wait 0.05;
    connects = 0;
    disconnects = 0;
    for (attempt = 0; attempt < 120; attempt++)
    {
        if (!pending_context_current(pending))
        {
            finish_pending(pending, "registry_context_lost");
            return;
        }
        players = getplayers();
        if (!isarray(players) || players.size != 0)
        {
            finish_pending(pending, "early_roster_not_pristine");
            return;
        }
        connects = 0;
        disconnects = 0;
        if (isdefined(level._callbacks))
        {
            if (!isarray(level._callbacks))
            {
                finish_pending(pending, "registry_invalid_c-1_d-1");
                return;
            }
            connects = pending_registry_count(level._callbacks[#"hash_eaffea17"], &zm_island_challenges::on_player_connect);
            disconnects = pending_registry_count(level._callbacks[#"hash_aebdd257"], &zm_island_challenges::on_player_disconnect);
            if (connects < 0 || disconnects < 0 || connects > 1 || disconnects > 1)
            {
                finish_pending(pending, "registry_invalid_c" + connects + "_d" + disconnects);
                return;
            }
            if (pending_registry_count(level._callbacks[#"hash_eaffea17"], &on_player_connect) != 0 ||
                pending_registry_count(level._callbacks[#"hash_aebdd257"], &on_player_disconnect) != 0)
            {
                finish_pending(pending, "registry_owned_callback_conflict");
                return;
            }
        }
        if (connects == 1 && disconnects == 1)
        {
            install_ready_registry(pending);
            return;
        }
        setdvar("zm_coopIslandTrialsStatus", "registry_wait_c" + connects + "_d" + disconnects);
        wait 0.05;
    }
    finish_pending(pending, "registry_timeout_c" + connects + "_d" + disconnects);
}

function install_ready_registry(pending)
{
    if (!pending_context_current(pending)) return;
    // The worker verified an empty roster and exact stock registrations without
    // yielding. Removing registry entries never attempts to cancel old threads.
    level.eroded_island_trials_epoch = pending.epoch;
    level.eroded_island_trials_capacity = pending.capacity;
    level.eroded_island_trials_revision = 3;
    level.eroded_island_trials_ready = false;
    level.eroded_island_trials_pending = undefined;
    callback::remove_on_connect(&zm_island_challenges::on_player_connect);
    callback::remove_on_disconnect(&zm_island_challenges::on_player_disconnect);
    callback::on_connect(&on_player_connect);
    callback::on_disconnect(&on_player_disconnect);
    level thread ownership_watcher();
    if (prepare_world_templates())
    {
        level.eroded_island_trials_ready = true;
        if (publish_pool_api()) setdvar("zm_coopIslandTrialsStatus", "early_ready");
        else decline("pool_api_context_lost");
    }
    else decline("geometry_pending");
    println("[Zombies coop] island_trials=early_registry installed=1");
}

function context_current()
{
    return schema_current() && isdefined(level.eroded_island_trials_revision) &&
        level.eroded_island_trials_revision == 3 && isdefined(level.eroded_island_trials_epoch) &&
        level.eroded_island_trials_epoch == getdvarint("zm_coopStartupEpoch") &&
        isdefined(level.eroded_island_trials_capacity) &&
        level.eroded_island_trials_capacity == getdvarint("zm_coopSchemaSlots");
}

function ownership_watcher()
{
    level endon(#"end_game");
    level endon(#"intermission");
    // Independent of pool initialization and the one-shot earned-unlock monitor.
    while (context_current())
    {
        if (!registry_owned())
        {
            decline("registry_ownership_lost");
            return;
        }
        wait 0.1;
    }
    decline("context_ownership_lost");
}

function prepare_world_templates()
{
    if (!registry_owned() || isdefined(level.eroded_island_trials_world)) return false;
    triggers = struct::get_array("s_challenge_trigger");
    altars = struct::get_array("s_challenge_altar");
    if (!isarray(triggers) || triggers.size != 4 || !isarray(altars) || altars.size != 1 ||
        !isdefined(altars[0]) || !bounded_world_point(altars[0].origin)) return false;
    frames = array();
    keys = getarraykeys(triggers);
    for (index = 0; index < keys.size; index++)
    {
        trigger = triggers[keys[index]];
        // ACTS reserves script_* as raw hashes. Use the independently verified
        // retail/Ezz canonical ID for script_special, never its compiled-zero spelling.
        if (!isdefined(trigger) || !isint(trigger.var_7cc697d0) || trigger.var_7cc697d0 < 0 ||
            trigger.var_7cc697d0 > 3 || !bounded_world_point(trigger.origin) ||
            !bounded_world_point(trigger.angles)) return false;
        slot = trigger.var_7cc697d0;
        if (isdefined(frames[slot]) || abs(trigger.angles[0]) > 1 || abs(trigger.angles[2]) > 1) return false;
        frame = spawnstruct();
        frame.origin = trigger.origin;
        frame.angles = trigger.angles;
        frame.lookats = array();
        frames[slot] = frame;
    }
    for (tier = 1; tier <= 3; tier++)
    {
        lookats = getentarray("t_lookat_challenge_" + tier, "targetname");
        if (!isarray(lookats) || lookats.size != 4) return false;
        lookat_keys = getarraykeys(lookats);
        for (index = 0; index < lookat_keys.size; index++)
        {
            lookat = lookats[lookat_keys[index]];
            if (!isdefined(lookat) || !isentity(lookat) || !isint(lookat.var_7cc697d0) ||
                lookat.var_7cc697d0 < 0 || lookat.var_7cc697d0 > 3 ||
                !bounded_world_point(lookat.origin)) return false;
            slot = lookat.var_7cc697d0;
            if (!isdefined(frames[slot]) || isdefined(frames[slot].lookats[tier]) ||
                distance(lookat.origin, frames[slot].origin) > 128) return false;
            // Preserve the exact original entities for the four retail stations.
            frames[slot].lookats[tier] = lookat;
        }
    }
    for (slot = 0; slot < 4; slot++)
    {
        if (!isdefined(frames[slot]) || frames[slot].lookats.size != 3 ||
            distance(frames[slot].origin, altars[0].origin) > 512) return false;
    }
    // Actual slot order is west-bottom, west-top, east-top, east-bottom.
    // Add one station at each measured wall midpoint, facing the existing altar.
    pairs = array(array(0, 1), array(2, 3), array(1, 2), array(0, 3));
    for (extra = 0; extra < 4; extra++)
    {
        pair = pairs[extra];
        midpoint = (frames[pair[0]].origin + frames[pair[1]].origin) * 0.5;
        frame = derive_world_frame(frames[0], midpoint, altars[0].origin);
        if (!isdefined(frame) || !placement_clear(frame.origin, altars[0].origin)) return false;
        for (other = 0; other < 4 + extra; other++)
            if (distance(frame.origin, frames[other].origin) < 128) return false;
        frames[extra + 4] = frame;
    }
    // Publish complete validated geometry only; no frames actor is created here.
    level.eroded_island_trials_world = frames;
    return true;
}

// IsVec is exposed in the pinned BG builtin table and used by stock struct code.
// Reject malformed mod metadata before indexing it; never coerce numeric strings.
function bounded_world_point(point)
{
    if (!isdefined(point) || !isvec(point)) return false;
    for (axis = 0; axis < 3; axis++)
        if (!isdefined(point[axis]) || point[axis] != point[axis] || abs(point[axis]) > 32768) return false;
    return true;
}

function rotate_world_offset(offset, yaw)
{
    return (offset[0] * cos(yaw) - offset[1] * sin(yaw),
        offset[0] * sin(yaw) + offset[1] * cos(yaw), offset[2]);
}

function derive_world_frame(source, origin, altar)
{
    if (!isdefined(source) || !bounded_world_point(source.origin) || !bounded_world_point(source.angles) ||
        !isarray(source.lookats) || source.lookats.size != 3 || !bounded_world_point(origin) ||
        !bounded_world_point(altar) || distance(origin, altar) < 64 || distance(origin, altar) > 512) return undefined;
    facing = vectortoangles(altar - origin);
    result = spawnstruct();
    result.origin = origin;
    result.angles = (0, facing[1], 0);
    result.lookats = array();
    for (tier = 1; tier <= 3; tier++)
    {
        if (!isdefined(source.lookats[tier]) || !bounded_world_point(source.lookats[tier].origin)) return undefined;
        lookat = spawnstruct();
        lookat.origin = origin + rotate_world_offset(source.lookats[tier].origin - source.origin,
            result.angles[1] - source.angles[1]);
        if (!bounded_world_point(lookat.origin)) return undefined;
        result.lookats[tier] = lookat;
    }
    return result;
}

function placement_clear(origin, altar)
{
    // Validate a nearby actual floor and a clear view into the known altar room.
    // Do not move points onto an unverified floor or bypass obstructed geometry.
    floor = groundtrace(origin + (0, 0, 32), origin - (0, 0, 96), 0, undefined);
    if (!isdefined(floor) || !isdefined(floor["fraction"]) || !(floor["fraction"] > 0 &&
        floor["fraction"] < 1) || !bounded_world_point(floor["position"]) ||
        !bounded_world_point(floor["normal"]) || floor["normal"][2] < 0.5 ||
        distance(origin, floor["position"]) > 64 || floor["position"][2] > origin[2] + 8) return false;
    sight = bullettrace(origin + (0, 0, 48), altar + (0, 0, 24), 0, undefined);
    return isdefined(sight) && isdefined(sight["fraction"]) && sight["fraction"] >= 0.95 && sight["fraction"] <= 1;
}

function publish_pool_api()
{
    if (!registry_owned() || !isdefined(level.eroded_island_trials_ready) ||
        level.eroded_island_trials_ready != true) return false;
    api = spawnstruct();
    api.revision = 3;
    api.map = "zm_island";
    api.epoch = level.eroded_island_trials_epoch;
    api.capacity = level.eroded_island_trials_capacity;
    api.make_pool = &make_pool;
    api.take_task = &take_task;
    api.return_task = &return_task;
    // Publish only a complete current helper table after the native-authorized early anchor.
    level.eroded_island_trials_pool_api = api;
    return true;
}

function registry_owned()
{
    if (!context_current() || !isdefined(level._callbacks)) return false;
    connects = level._callbacks[#"hash_eaffea17"];
    disconnects = level._callbacks[#"hash_aebdd257"];
    if (isdefined(level.eroded_island_trials_pools) &&
        registry_count(disconnects, &zm_island_challenges::function_b1cd865a) != 0) return false;
    return registry_count(connects, &on_player_connect) == 1 &&
        registry_count(connects, &zm_island_challenges::on_player_connect) == 0 &&
        registry_count(disconnects, &on_player_disconnect) == 1 &&
        registry_count(disconnects, &zm_island_challenges::on_player_disconnect) == 0;
}

function decline(reason)
{
    level.eroded_island_trials_ready = false;
    level.eroded_island_trials_pool_api = undefined;
    // A terminating old VM must not overwrite the next map's diagnostic dvar.
    if (getdvarstring("mapname") != "zm_island" ||
        (isdefined(level.eroded_island_trials_epoch) &&
        level.eroded_island_trials_epoch != getdvarint("zm_coopStartupEpoch"))) return;
    setdvar("zm_coopIslandTrialsStatus", reason);
}

function initialize_pools()
{
    if (!registry_owned()) return false;
    if (isdefined(level.eroded_island_trials_pools)) return true;
    if (!isdefined(level._challenges)) return false;
    capacity = level.eroded_island_trials_capacity;
    pool1 = make_pool(level._challenges.challenge_1, 1, capacity);
    pool2 = make_pool(level._challenges.challenge_2, 2, capacity);
    pool3 = make_pool(level._challenges.challenge_3, 3, capacity);
    if (!isdefined(pool1) || !isdefined(pool2) || !isdefined(pool3)) return false;
    // Stock main starts its completion listener before publishing the initialization flag.
    // Remove only its historical-counter disconnect check. The listener stays dormant:
    // our initializer never starts stock function_974d5f1d or emits its level notification.
    if (registry_count(level._callbacks[#"hash_aebdd257"], &zm_island_challenges::function_b1cd865a) != 1)
        return false;
    callback::remove_on_disconnect(&zm_island_challenges::function_b1cd865a);
    level.eroded_island_trials_pools = array(pool1, pool2, pool3);
    level.eroded_island_trials_records = array();
    sync_stock_pools();
    println("[Zombies coop] island_trials=pools initialized=1 capacity=" + capacity);
    level thread completion_monitor();
    return true;
}

function sync_stock_pools()
{
    level._challenges.challenge_1 = level.eroded_island_trials_pools[0].available;
    level._challenges.challenge_2 = level.eroded_island_trials_pools[1].available;
    level._challenges.challenge_3 = level.eroded_island_trials_pools[2].available;
}

function reserve_player(player, slot)
{
    if (!registry_owned() || !isdefined(level.eroded_island_trials_records) ||
        !isdefined(level.eroded_island_trials_pools) || !valid_slot(slot, level.eroded_island_trials_capacity) ||
        isdefined(level.eroded_island_trials_records[slot])) return undefined;
    selected = array();
    for (tier = 0; tier < 3; tier++)
    {
        pool = level.eroded_island_trials_pools[tier];
        allocated_before = pool.allocated;
        choices = pool.available.size;
        if (choices == 0) choices = pool.catalog.size;
        selected[tier] = take_task(pool, slot, player, randomint(choices));
        if (!isdefined(selected[tier]))
        {
            for (rollback = 0; rollback < tier; rollback++)
                return_task(level.eroded_island_trials_pools[rollback], slot, player);
            sync_stock_pools();
            return undefined;
        }
        // Production receipts belong only to the actual participant pools, never private self-tests.
        if (pool.allocated > allocated_before)
            println("[Zombies coop] island_trials=pool_grown tier=" + pool.tier + " allocated=" + pool.allocated);
    }
    record = spawnstruct();
    record.owner = player;
    record.epoch = level.eroded_island_trials_epoch;
    record.tasks = spawnstruct();
    record.tasks.challenge_1 = selected[0];
    record.tasks.challenge_2 = selected[1];
    record.tasks.challenge_3 = selected[2];
    record.selected = selected;
    record.states = array(1, 1, 1);
    record.trigger_active = false;
    level.eroded_island_trials_records[slot] = record;
    sync_stock_pools();
    // These existing world fields also carry the extra visual state. No mover bits.
    if (!reset_world_states(slot, record, 1)) return undefined;
    return record;
}

function release_player(player, slot)
{
    if (!context_current() || !isdefined(level.eroded_island_trials_records) ||
        !valid_slot(slot, level.eroded_island_trials_capacity)) return false;
    record = level.eroded_island_trials_records[slot];
    if (!isdefined(record) || record.owner != player || record.epoch != level.eroded_island_trials_epoch)
        return false;
    // Reset only this exact owned lease before retiring it. Retail stations stay
    // at their stock idle state; extra stations use zero as owned absence.
    state = 0;
    if (slot < 4) state = 1;
    if (!reset_world_states(slot, record, state)) return false;
    level.eroded_island_trials_records[slot] = undefined;
    for (tier = 0; tier < 3; tier++)
        return_task(level.eroded_island_trials_pools[tier], slot, player);
    sync_stock_pools();
    if (isdefined(record.trigger_active) && record.trigger_active == true && isdefined(record.stub))
    {
        record.trigger_active = false;
        zm_unitrigger::unregister_unitrigger(record.stub);
    }
    return true;
}

function roster_by_slot(players)
{
    if (!isarray(players) || players.size > 8) return undefined;
    result = array();
    keys = getarraykeys(players);
    for (key_index = 0; key_index < keys.size; key_index++)
    {
        key = keys[key_index];
        if (!isint(key) || key < 0 || key > 7) return undefined;
        player = players[key];
        if (!isdefined(player) || !isentity(player) || !isplayer(player)) return undefined;
        if (isdefined(player.player_disconnected) && player.player_disconnected == 1) continue;
        slot = player getentitynumber();
        if (!isint(slot) || slot < 0 || slot > 7 || isdefined(result[slot])) return undefined;
        result[slot] = player;
    }
    return result;
}

function participant_complete(player, record, epoch)
{
    if (!isdefined(record) || record.owner != player || record.epoch != epoch ||
        !isdefined(player._challenges) || player._challenges != record.tasks) return false;
    for (tier = 1; tier <= 3; tier++)
    {
        flag = "flag_player_completed_challenge_" + tier;
        if (!player flag::exists(flag) || !player flag::get(flag)) return false;
    }
    return true;
}

function player_challenge_action(player, record, epoch)
{
    if (!isdefined(player) || !isentity(player) || !isplayer(player)) return "invalid_player";
    if (!isint(epoch) || epoch < 1) return "stale_slot_record";
    if (isdefined(record))
    {
        if (record.owner == player && isdefined(record.epoch) && isint(record.epoch) && record.epoch == epoch &&
            isdefined(player._challenges) && player._challenges == record.tasks) return "owned";
        return "stale_slot_record";
    }
    if (isdefined(player._challenges)) return "foreign_challenges";
    if (isdefined(player.var_873a3e27)) return "foreign_trial_progress";
    return "new";
}

function completed_roster(actual, listed, records, epoch)
{
    actual_slots = roster_by_slot(actual);
    listed_slots = roster_by_slot(listed);
    if (!isdefined(actual_slots) || !isdefined(listed_slots) || !isarray(records)) return false;
    present = 0;
    for (slot = 0; slot < 8; slot++)
    {
        if (actual_slots[slot] != listed_slots[slot]) return false;
        if (!isdefined(actual_slots[slot])) continue;
        if (!participant_complete(actual_slots[slot], records[slot], epoch)) return false;
        present++;
    }
    return present > 0;
}

function completion_monitor()
{
    level endon(#"end_game");
    level endon(#"intermission");
    while (registry_owned())
    {
        if (!level flag::exists("all_challenges_completed"))
        {
            decline("completion_flag_unavailable");
            return;
        }
        if (level flag::get("all_challenges_completed"))
            return;
        if (completed_roster(getplayers(), level.players, level.eroded_island_trials_records,
            level.eroded_island_trials_epoch))
        {
            // This is the actual earned stock unlock, not fabricated individual task completion.
            level flag::set("all_challenges_completed");
            level thread zm_island_challenges::function_397b26ee();
            return;
        }
        wait 0.1;
    }
    decline("registry_ownership_lost");
}

function prepare_player_world(player, slot, record)
{
    if (!registry_owned() || !valid_slot(slot, level.eroded_island_trials_capacity) ||
        !isarray(level.eroded_island_trials_world) || level.eroded_island_trials_world.size != 8 ||
        !isdefined(record) || record.owner != player || record.epoch != level.eroded_island_trials_epoch ||
        isdefined(record.stub)) return false;
    frame = level.eroded_island_trials_world[slot];
    if (!isdefined(frame) || !bounded_world_point(frame.origin) || !bounded_world_point(frame.angles) ||
        !isarray(frame.lookats) || frame.lookats.size != 3) return false;
    stub = spawnstruct();
    stub.origin = frame.origin;
    stub.angles = frame.angles;
    // Canonical script_unitrigger_type=0x5FA98D68; literal script_* is reserved by ACTS.
    stub.var_5fa98d68 = "unitrigger_radius_use";
    stub.cursor_hint = "HINT_NOICON";
    stub.radius = 128;
    stub.require_look_at = 0;
    stub.inactive_reassess_time = 0.5;
    stub.var_a879fa43 = slot;
    stub.var_8e2d9e6f = array();
    stub.var_e01fcddc = frame.lookats;
    for (tier = 1; tier <= 3; tier++) stub.var_8e2d9e6f[tier] = tier;
    stub.prompt_and_visibility_func = &zm_island_challenges::function_3ae0d6d5;
    record.stub = stub;
    return true;
}

function activate_player_world(player, slot, record)
{
    if (!registry_owned() || !isdefined(record) || record.owner != player ||
        record.epoch != level.eroded_island_trials_epoch || player._challenges != record.tasks ||
        !isdefined(record.stub) || !isdefined(record.trigger_active) || record.trigger_active != false ||
        !isdefined(level._unitriggers) || !isarray(level._unitriggers.trigger_stubs) ||
        !isdefined(level.zones) || level.zones.size == 0) return false;
    for (tier = 1; tier <= 3; tier++)
        if (!player flag::exists("flag_player_completed_challenge_" + tier) ||
            !player flag::exists("flag_player_collected_reward_" + tier) ||
            !isdefined(player.var_873a3e27[tier])) return false;
    // No deferred registration: all zones exist before this synchronous call.
    zm_unitrigger::unitrigger_force_per_player_triggers(record.stub, 1);
    zm_unitrigger::register_static_unitrigger(record.stub, &zm_island_challenges::function_a00e23d0);
    if (!isdefined(record.stub.registered) || record.stub.registered != 1) return false;
    record.trigger_active = true;
    return true;
}

function on_player_connect()
{
    if (!registry_owned())
    {
        decline("registry_ownership_lost");
        return;
    }
    if (!isdefined(self) || !isentity(self) || !isplayer(self)) return;
    self endon(#"disconnect");
    slot = self getentitynumber();
    if (!valid_slot(slot, level.eroded_island_trials_capacity)) return;
    level flag::wait_till("flag_init_player_challenges");
    if (!registry_owned())
    {
        decline("registry_ownership_lost");
        return;
    }
    existing = undefined;
    if (isdefined(level.eroded_island_trials_records)) existing = level.eroded_island_trials_records[slot];
    action = player_challenge_action(self, existing, level.eroded_island_trials_epoch);
    if (action == "owned") return;
    if (action != "new")
    {
        decline(action);
        return;
    }
    if (!initialize_pools())
    {
        decline("stock_pool_contract_unavailable");
        return;
    }
    record = reserve_player(self, slot);
    if (!isdefined(record))
    {
        decline("task_reservation_failed");
        return;
    }
    if (!prepare_player_world(self, slot, record))
    {
        release_player(self, slot);
        decline("geometry_pending");
        return;
    }
    // A world setup implementation must not overwrite a task owner that appeared
    // while it prepared owned objects. Roll back our lease/objects, never theirs.
    if (isdefined(self._challenges))
    {
        release_player(self, slot);
        decline("foreign_challenges");
        return;
    }
    if (isdefined(self.var_873a3e27))
    {
        release_player(self, slot);
        decline("foreign_trial_progress");
        return;
    }
    self.var_8575e180 = 0;
    self.var_26f3bd30 = 0;
    self.var_301c71e9 = 0;
    self._challenges = record.tasks;
    self.var_873a3e27 = array();
    for (tier = 1; tier <= 3; tier++)
    {
        self.var_873a3e27[tier] = 0;
        self flag::init("flag_player_collected_reward_" + tier);
        self flag::init("flag_player_completed_challenge_" + tier);
    }
    for (tier = 1; tier <= 3; tier++)
    {
        task = record.selected[tier - 1];
        self thread zm_island_challenges::function_2ce855f3(task.n_index, task.func_think, task.n_count, task.str_notify);
        self thread zm_island_challenges::function_fbbc8608(task.n_index, "flag_player_completed_challenge_" + tier);
        self thread pillar_state_worker(slot, tier, record);
        if (slot < 4) self thread zm_island_challenges::function_7fc84e9c(slot, tier);
    }
    if (!activate_player_world(self, slot, record))
    {
        // Workers may now be waiting on this player's events. Keep their owned
        // lease bounded until real disconnect instead of deleting data under them.
        decline("world_activation_failed");
        return;
    }
}

function on_player_disconnect()
{
    if (!isdefined(self) || !isentity(self) || !isplayer(self)) return;
    release_player(self, self getentitynumber());
}

function pillar_state_worker(slot, tier, record)
{
    self endon(#"disconnect");
    level flag::wait_till("flag_init_challenge_pillars");
    if (!registry_owned() || level.eroded_island_trials_records[slot] != record) return;
    publish_pillar_state(slot, tier, 2, record);
    self flag::wait_till("flag_player_completed_challenge_" + tier);
    if (!registry_owned() || level.eroded_island_trials_records[slot] != record) return;
    publish_pillar_state(slot, tier, 3, record);
    self flag::wait_till("flag_player_collected_reward_" + tier);
    if (!registry_owned() || level.eroded_island_trials_records[slot] != record) return;
    publish_pillar_state(slot, tier, 4, record);
}

function reset_world_states(slot, record, state)
{
    if (!context_current() || !valid_slot(slot, level.eroded_island_trials_capacity) ||
        !isdefined(record) || !isdefined(record.owner) || record.epoch != level.eroded_island_trials_epoch ||
        !isdefined(level.eroded_island_trials_records) || level.eroded_island_trials_records[slot] != record ||
        !isint(state) || state < 0 || state > 1 || !isarray(record.states) || record.states.size != 3) return false;
    for (tier = 1; tier <= 3; tier++)
    {
        record.states[tier - 1] = state;
        level clientfield::set("pillar_challenge_" + slot + "_" + tier, state);
    }
    return true;
}

function publish_pillar_state(slot, tier, state, record)
{
    if (!registry_owned() || !valid_slot(slot, level.eroded_island_trials_capacity) ||
        !isdefined(record) || !isdefined(record.owner) || record.epoch != level.eroded_island_trials_epoch ||
        !isdefined(level.eroded_island_trials_records) || level.eroded_island_trials_records[slot] != record ||
        !isint(tier) || tier < 1 || tier > 3 || !isint(state) || state < 1 || state > 4 ||
        !isarray(record.states) || record.states.size != 3) return false;
    record.states[tier - 1] = state;
    level clientfield::set("pillar_challenge_" + slot + "_" + tier, state);
    return true;
}
