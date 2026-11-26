// Independently implemented paired schema adapter against pinned stock APIs.
// Loaded only by the pinned early native schema bootstrap, outside custom_scripts.
// Native freezes read-only zm_coopSchemaSlots from the actual session before loading.
// Report dvars are execution diagnostics, never an authentication proof.
#using scripts/codescripts/struct;
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
