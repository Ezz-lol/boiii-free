// SPDX-License-Identifier: GPL-3.0-only
// Experimental map mechanics contributed by Eroded Networks.
// Loaded only through the native, opt-in 1-8 map adapter.
// Only exact known Temple/Castle source call sites may call these helpers.
// Common is imported by canonical logical name; its pinned source is unchanged.
#using custom_scripts/zm/eroded_coop;
#namespace eroded_coop_map_readiness;

// Capture once at the ORIGINAL stock function entry, before any wait. Temple
// starts before power_on and before common readiness, so capture is identity
// only. Current readiness and callback ownership are checked on every use.
function capture(map)
{
    if (map != "zm_temple" && map != "zm_castle") return undefined;
    epoch = getdvarint("zm_coopStartupEpoch");
    capacity = getdvarint("zm_coopAppliedLimit");
    if (!isint(epoch) || epoch < 1 || !isint(capacity) || capacity < 5 || capacity > 8 ||
        getdvarstring("mapname") != map) return undefined;
    context = spawnstruct();
    context.map = map;
    context.epoch = epoch;
    context.capacity = capacity;
    return context;
}

function current(context, map)
{
    if (!isdefined(context) || !isdefined(context.map) || context.map != map ||
        !isdefined(context.epoch) || !isint(context.epoch) || context.epoch < 1 ||
        !isdefined(context.capacity) || !isint(context.capacity) || context.capacity < 5 || context.capacity > 8)
        return false;
    if (!eroded_coop::startup_current(context.epoch, map) ||
        getdvarint("zm_coopAppliedLimit") != context.capacity ||
        getdvarint("zm_coopSchemaSlots") != context.capacity ||
        getdvarint("zm_coopReadyEpoch") != context.epoch || getdvarstring("zm_coopReadyMap") != map ||
        getdvarstring("zm_coopStatus") != "experimental_callbacks_installed") return false;
    if (!isdefined(level.eroded_coop_started_epoch) || level.eroded_coop_started_epoch != context.epoch ||
        !isdefined(level.eroded_coop_started_map) || level.eroded_coop_started_map != map ||
        !isdefined(level.eroded_coop_limit) || !isint(level.eroded_coop_limit) || level.eroded_coop_limit != context.capacity ||
        !eroded_coop::map_profile_current(context.epoch, map)) return false;
    // A retained positive receipt alone must not authorize a callback takeover.
    return eroded_coop::owns_callbacks();
}

function dense_physical_four(entries)
{
    if (!isarray(entries) || entries.size != 4) return false;
    keys = getarraykeys(entries);
    if (!isarray(keys) || keys.size != 4) return false;
    for (i = 0; i < 4; i++)
    {
        if (!isint(keys[i]) || keys[i] < 0 || keys[i] > 3 ||
            !isdefined(entries[i]) || !isentity(entries[i])) return false;
        for (j = 0; j < i; j++)
            if (entries[j] == entries[i]) return false;
    }
    return true;
}

// Compare physical identity sets rather than depending on getentarray order.
function same_four(entries, named)
{
    if (!dense_physical_four(entries) || !dense_physical_four(named)) return false;
    for (i = 0; i < 4; i++)
    {
        matches = 0;
        for (j = 0; j < 4; j++)
            if (entries[i] == named[j]) matches++;
        if (matches != 1) return false;
    }
    return true;
}

// Stock uses actual players in Temple and listed players in Castle. Require
// both live sets to agree before changing either requirement. Sparse live slot
// keys are allowed, but duplicate/out-of-capacity/disconnected players are not.
function roster_matches(stock_count, capacity)
{
    actual = getplayers();
    if (!isarray(actual) || !isdefined(level.players) || !isarray(level.players) ||
        actual.size != stock_count || level.players.size != stock_count || stock_count > capacity) return false;
    actual_keys = getarraykeys(actual);
    listed_keys = getarraykeys(level.players);
    if (!isarray(actual_keys) || !isarray(listed_keys) ||
        actual_keys.size != stock_count || listed_keys.size != stock_count) return false;
    seen_slots = array();
    for (i = 0; i < stock_count; i++)
    {
        key = actual_keys[i];
        if (!isint(key) || key < 0 || key >= capacity) return false;
        player = actual[key];
        if (!isdefined(player) || !isentity(player) || !isplayer(player)) return false;
        if (isdefined(player.player_disconnected) &&
            (!isint(player.player_disconnected) || player.player_disconnected != 0)) return false;
        slot = player getentitynumber();
        if (!isint(slot) || slot < 0 || slot >= capacity || isdefined(seen_slots[slot])) return false;
        seen_slots[slot] = true;
        matches = 0;
        for (j = 0; j < stock_count; j++)
        {
            listed_key = listed_keys[j];
            if (!isint(listed_key) || listed_key < 0 || listed_key >= capacity) return false;
            if (level.players[listed_key] == player) matches++;
        }
        if (matches != 1) return false;
    }
    return true;
}

function temple_plates_valid(triggers)
{
    if (!dense_physical_four(triggers)) return false;
    seen_requirements = array();
    for (i = 0; i < 4; i++)
    {
        trigger = triggers[i];
        name = "pap_blocker_trigger" + (i + 1);
        if (!isdefined(trigger.targetname) || trigger.targetname != name) return false;
        named = getentarray(name, "targetname");
        if (!isarray(named) || named.size != 1 || !isdefined(named[0]) || named[0] != trigger) return false;
        if (!isdefined(trigger.requiredplayers) || !isint(trigger.requiredplayers) ||
            trigger.requiredplayers < 1 || trigger.requiredplayers > 4 ||
            isdefined(seen_requirements[trigger.requiredplayers])) return false;
        seen_requirements[trigger.requiredplayers] = true;
        if (!isdefined(trigger.target) || !isstring(trigger.target) || trigger.target == "" ||
            !isdefined(trigger.plate) || !isentity(trigger.plate)) return false;
        plates = getentarray(trigger.target, "targetname");
        if (!isarray(plates) || plates.size != 1 || !isdefined(plates[0]) || plates[0] != trigger.plate) return false;
        for (j = 0; j < i; j++)
            if (triggers[j].plate == trigger.plate) return false;
    }
    return true;
}

function castle_areas_valid(areas)
{
    named = getentarray("boss_gravity_spike_start_area", "targetname");
    if (!same_four(areas, named) || !isdefined(level.var_b366f2dc) ||
        !isint(level.var_b366f2dc) || level.var_b366f2dc < 0 || level.var_b366f2dc > 4) return false;
    claimed = 0;
    for (i = 0; i < 4; i++)
    {
        area = areas[i];
        if (!isdefined(area.targetname) || area.targetname != "boss_gravity_spike_start_area" ||
            !isdefined(area.b_claimed) || !isint(area.b_claimed) || (area.b_claimed != 0 && area.b_claimed != 1))
            return false;
        claimed += area.b_claimed;
    }
    return claimed == level.var_b366f2dc;
}

// The original count is returned verbatim for stock1..4, invalid identity,
// transient rosters, unknown ownership, and unexpected physical layouts.
function temple_requirement(stock_count, triggers, context)
{
    if (!isint(stock_count) || stock_count < 5 || stock_count > 8) return stock_count;
    if (!current(context, "zm_temple") || !roster_matches(stock_count, context.capacity) ||
        !temple_plates_valid(triggers)) return stock_count;
    return 4;
}

function castle_requirement(stock_count, areas, context)
{
    if (!isint(stock_count) || stock_count < 5 || stock_count > 8) return stock_count;
    if (!current(context, "zm_castle") || !roster_matches(stock_count, context.capacity) ||
        !castle_areas_valid(areas)) return stock_count;
    return 4;
}
