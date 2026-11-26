// Independently implemented paired schema adapter against pinned stock APIs.
// Loaded only by the pinned early native schema bootstrap, outside custom_scripts.
// Native freezes read-only zm_coopSchemaSlots from the actual session before loading.
// Report dvars are execution diagnostics, never an authentication proof.
#using scripts/codescripts/struct;
#using scripts/shared/array_shared;
#using scripts/shared/callbacks_shared;
#using scripts/shared/clientfield_shared;
#using scripts/shared/system_shared;
#using scripts/zm/_zm_laststand;

#namespace eroded_coop_fields;

function schema_revision()
{
    return 3;
}

function schema_contract()
{
    return "e1a7df2021d9fea869c1871e836af092c6e9e8fb6828f77a3f7dbfc5cde1eb3d";
}

function schema_family_mask(map)
{
    // Common revive + score contracts are fixed, including on inherited custom maps.
    mask = 3;
    if (map == "zm_castle") return mask + 4;
    if (map == "zm_stalingrad") return mask + 8;
    if (map == "zm_genesis") return mask + 16;
    if (map == "zm_moon") return mask + 32;
    if (map == "zm_island") return mask + 64;
    return mask;
}

function schema_capacity_current()
{
    capacity = getdvarint("zm_coopSchemaSlots");
    if (!isint(capacity) || capacity < 5 || capacity > 8) return false;
    return true;
}

function autoexec __init__system__()
{
    if (!schema_capacity_current()) return;
    if (isdefined(level.system_funcs) && isdefined(level.system_funcs["eroded_coop_fields"])) return;
    system::register("eroded_coop_fields", &register_schema, undefined, "zm_laststand");
}

function register_schema()
{
    if (!schema_capacity_current()) return;
    if (isdefined(level.eroded_coop_fields_revision)) return;
    if (!isdefined(level.laststand_update_clientfields) || !isarray(level.laststand_update_clientfields)) return;
    if (!isdefined(level.player_name_directive) || !isarray(level.player_name_directive)) return;
    level.eroded_coop_name_values = array();
    for (slot = 4; slot < 8; slot++)
        if (isdefined(level.laststand_update_clientfields[slot])) return;
    // All four extra slots are a single fixed schema; connected count is irrelevant.
    for (slot = 4; slot < 8; slot++)
    {
        field = "laststand_update" + slot;
        level.laststand_update_clientfields[slot] = field;
        clientfield::register("world", field, 1, 5, "counter");
        // A bot or connecting player may not have a name yet. Never alias a
        // different player's directive or leave the stock revive lookup undefined.
        if (!isdefined(level.player_name_directive[slot]))
        {
            name = extra_player_name(slot, undefined);
            level.player_name_directive[slot] = name;
            level.eroded_coop_name_values[slot] = name;
        }
    }
    for (slot = 4; slot < 8; slot++)
    {
        prefix = "PlayerList.client" + slot + ".score_cf_";
        clientfield::register("clientuimodel", prefix + "damage", 1, 3, "counter");
        clientfield::register("clientuimodel", prefix + "death_normal", 1, 2, "counter");
        clientfield::register("clientuimodel", prefix + "death_torso", 1, 2, "counter");
        clientfield::register("clientuimodel", prefix + "death_neck", 1, 2, "counter");
        clientfield::register("clientuimodel", prefix + "death_head", 1, 2, "counter");
        clientfield::register("clientuimodel", prefix + "death_melee", 1, 2, "counter");
    }
    register_map_slot_fields(tolower(getdvarstring("mapname")));
    // Inert metadata only until a fingerprinted stock VO call invokes the helper.
    // Inventory/CSC field families continue to follow their existing contracts.
    if (!castle_selector_bind(getdvarint("zm_coopStartupEpoch"), getdvarstring("mapname"), getdvarint("zm_coopSchemaSlots")))
        return;
    setdvar("zm_coopCastleVOStatus", "armed_for_known_module");
    level.eroded_coop_fields_revision = schema_revision();
    callback::on_connect(&remember_extra_player_name);
    callback::on_spawned(&remember_extra_player_name);
    setdvar("zm_coopSchemaServerRevision", schema_revision());
    setdvar("zm_coopSchemaServerMask", schema_family_mask(tolower(getdvarstring("mapname"))));
    setdvar("zm_coopSchemaServerContract", schema_contract());
}

function register_map_slot_fields(map)
{
    if (map == "zm_castle")
        register_inventory_fields("hasItem", 1, 1);
    else if (map == "zm_stalingrad")
        register_inventory_fields("wearableItem", 12000, 2);
    else if (map == "zm_genesis")
        register_inventory_fields("wearableItem", 15000, 4);
    else if (map == "zm_moon")
        register_inventory_fields("wearableItem", 21000, 1);
    else if (map == "zm_island")
    {
        for (slot = 4; slot < 8; slot++)
        {
            for (challenge = 1; challenge <= 3; challenge++)
                clientfield::register("world", "pillar_challenge_" + slot + "_" + challenge, 9000, 3, "int");
            clientfield::register("world", "portal_state_ending_" + slot, 9000, 1, "int");
        }
    }
}

function register_inventory_fields(suffix, version, bits)
{
    for (slot = 4; slot < 8; slot++)
        clientfield::register("world", "player" + slot + suffix, version, bits, "int");
}

// Connect/spawn callbacks execute on the player entity. Keep custom directives
// owned by map/mod code, and refresh our own entry when a slot is reused.
function remember_extra_player_name()
{
    if (!schema_capacity_current() || !isdefined(self) || !isplayer(self)) return;
    if (!isdefined(level.eroded_coop_fields_revision) || !isdefined(level.eroded_coop_name_values)) return;
    slot = self getentitynumber();
    if (!isint(slot) || slot < 4 || slot > 7) return;
    if (!name_directive_owned(level.player_name_directive[slot], level.eroded_coop_name_values[slot])) return;
    name = extra_player_name(slot, self.name);
    level.player_name_directive[slot] = name;
    level.eroded_coop_name_values[slot] = name;
}

function name_directive_owned(current, previous)
{
    if (!isdefined(current)) return true;
    return isdefined(previous) && current == previous;
}

function extra_player_name(slot, name)
{
    if (!isint(slot) || slot < 4 || slot > 7) return undefined;
    if (isdefined(name) && isstring(name) && name != "") return name;
    return "Player " + (slot + 1);
}

// Native authorization also requires actual ZM mode/current frozen map and
// exact CastleVO raw/call-site identity. This helper only rechecks safe map text.
function castle_selector_map_valid(map)
{
    if (!isdefined(map) || !isstring(map)) return false;
    if (map.size < 1 || map.size > 64 || map == "core_frontend") return false;
    allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";
    for (index = 0; index < map.size; index++)
        if (!issubstr(allowed, getsubstr(map, index, index + 1))) return false;
    return true;
}

function castle_selector_bind(epoch, map, capacity)
{
    if (!isint(epoch) || epoch < 1 || !castle_selector_map_valid(map) || !isint(capacity) || capacity < 5 || capacity > 8)
        return false;
    if (getdvarint("zm_coopStartupEpoch") != epoch || getdvarstring("mapname") != map ||
        getdvarint("zm_coopSchemaSlots") != capacity || getdvarint("zm_coopAppliedLimit") != capacity)
        return false;
    // Intended call site: the current pinned fields registration, before its
    // final native-ready receipt. Never rebind another epoch or revive a decline.
    if (isdefined(level.eroded_coop_castle_selector_epoch))
        return castle_selector_bound_current() && !isdefined(level.eroded_coop_castle_selector_declined);
    level.eroded_coop_castle_selector_epoch = epoch;
    level.eroded_coop_castle_selector_map = map;
    level.eroded_coop_castle_selector_capacity = capacity;
    return true;
}

function castle_selector_bound_current()
{
    return isdefined(level.eroded_coop_castle_selector_epoch) &&
        isint(level.eroded_coop_castle_selector_epoch) && level.eroded_coop_castle_selector_epoch > 0 &&
        isdefined(level.eroded_coop_castle_selector_map) &&
        castle_selector_map_valid(level.eroded_coop_castle_selector_map) &&
        isdefined(level.eroded_coop_castle_selector_capacity) &&
        isint(level.eroded_coop_castle_selector_capacity) &&
        level.eroded_coop_castle_selector_capacity >= 5 && level.eroded_coop_castle_selector_capacity <= 8 &&
        getdvarint("zm_coopStartupEpoch") == level.eroded_coop_castle_selector_epoch &&
        getdvarstring("mapname") == level.eroded_coop_castle_selector_map &&
        getdvarint("zm_coopSchemaSlots") == level.eroded_coop_castle_selector_capacity &&
        getdvarint("zm_coopAppliedLimit") == level.eroded_coop_castle_selector_capacity;
}

function castle_selector_decline(reason)
{
    // Old VM/map work may never clear a fresh server's schema receipts.
    if (!castle_selector_bound_current()) return;
    if (isdefined(level.eroded_coop_castle_selector_declined)) return;
    level.eroded_coop_castle_selector_declined = true;
    setdvar("zm_coopCastleVOStatus", reason);
    setdvar("zm_coopStatus", "declined_castle_dialogue_owner");
    setdvar("zm_coopSchemaServerRevision", 0);
    println("[Zombies coop] castle_selector=declined");
    // Native startup polling performs the deferred error outside VM/cache locks.
}

function castle_selector_table_valid(table)
{
    if (!isarray(table)) return false;
    keys = getarraykeys(table);
    if (keys.size != 3) return false;
    for (i = 0; i < keys.size; i++)
        if (!isint(keys[i]) || (keys[i] != 0 && keys[i] != 1 && keys[i] != 3)) return false;
    rows = array(0, 1, 3);
    for (row_index = 0; row_index < 3; row_index++)
    {
        row = table[rows[row_index]];
        if (!isarray(row)) return false;
        row_keys = getarraykeys(row);
        if (row_keys.size != 3) return false;
        for (i = 0; i < row_keys.size; i++)
            if (!isint(row_keys[i]) || row_keys[i] < 0 || row_keys[i] > 2) return false;
        for (dialogue_index = 0; dialogue_index < 3; dialogue_index++)
        {
            sequence = row[dialogue_index];
            if (!isarray(sequence)) return false;
            sequence_keys = getarraykeys(sequence);
            if (sequence_keys.size != 2) return false;
            for (i = 0; i < sequence_keys.size; i++)
                if (!isint(sequence_keys[i]) || sequence_keys[i] < 0 || sequence_keys[i] > 1) return false;
            if (!isstring(sequence[0]) || sequence[0] == "" || !isstring(sequence[1]) || sequence[1] == "")
                return false;
        }
    }
    return true;
}

function castle_selector_rows_current(table)
{
    if (!castle_selector_table_valid(table)) return false;
    // Freeze only observed values, not shipped retail alias literals. This is
    // an execution ownership guard, not proof that an arbitrary first table is stock.
    if (!isdefined(level.eroded_coop_castle_selector_rows))
    {
        snapshot = array();
        rows = array(0, 1, 3);
        for (r = 0; r < 3; r++)
        {
            row_id = rows[r];
            snapshot[row_id] = array();
            for (e = 0; e < 3; e++)
                snapshot[row_id][e] = array(table[row_id][e][0], table[row_id][e][1]);
        }
        level.eroded_coop_castle_selector_rows = snapshot;
        return true;
    }
    snapshot = level.eroded_coop_castle_selector_rows;
    if (!castle_selector_table_valid(snapshot)) return false;
    rows = array(0, 1, 3);
    for (r = 0; r < 3; r++)
        for (e = 0; e < 3; e++)
            for (a = 0; a < 2; a++)
                if (table[rows[r]][e][a] != snapshot[rows[r]][e][a]) return false;
    return true;
}

function castle_dialogue_get_all_closest(origin, candidates, exclusions, limit, radius)
{
    result = array();
    // A stale callback is inert and cannot poison a new map/epoch.
    if (!castle_selector_bound_current()) return result;
    if (isdefined(level.eroded_coop_castle_selector_declined)) return result;
    if (!isdefined(level.eroded_coop_fields_revision) || !isint(level.eroded_coop_fields_revision) ||
        level.eroded_coop_fields_revision != 3 || getdvarint("zm_coopSchemaServerRevision") != 3)
    {
        castle_selector_decline("schema_context_unavailable");
        return result;
    }
    if (!isvec(origin) || !isarray(candidates) || !isarray(exclusions) ||
        !isint(limit) || limit != 4 || !isint(radius) || radius != 900)
    {
        castle_selector_decline("selector_arguments_unrecognized");
        return result;
    }
    if (!isdefined(level.var_4ea3bfd0) || !castle_selector_rows_current(level.var_4ea3bfd0))
    {
        castle_selector_decline("dialogue_table_unrecognized");
        return result;
    }
    keys = getarraykeys(candidates);
    if (keys.size > level.eroded_coop_castle_selector_capacity)
    {
        castle_selector_decline("candidate_count_unrecognized");
        return result;
    }
    filtered = array();
    for (i = 0; i < keys.size; i++)
    {
        if (!isint(keys[i]) || keys[i] < 0 || keys[i] > 7)
        {
            castle_selector_decline("candidate_keys_unrecognized");
            return result;
        }
        candidate = candidates[keys[i]];
        if (!isdefined(candidate) || !isentity(candidate) || !isplayer(candidate)) continue;
        if (!isdefined(candidate.characterindex) || !isint(candidate.characterindex) ||
            candidate.characterindex < 0 || candidate.characterindex > 3)
        {
            castle_selector_decline("candidate_character_unrecognized");
            return result;
        }
        if (candidate.characterindex == 2) continue;
        filtered[filtered.size] = candidate;
    }
    if (!isdefined(level.eroded_coop_castle_selector_seen))
    {
        level.eroded_coop_castle_selector_seen = true;
        setdvar("zm_coopCastleVOStatus", "owned_selector_active");
        println("[Zombies coop] castle_selector=active");
    }
    // Keep stock distance ordering, exclusions, nearest-four bound and radius.
    // The original caller still owns its post-selection is_player_valid check.
    return array::get_all_closest(origin, filtered, exclusions, limit, radius);
}
