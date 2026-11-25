// Independently implemented paired schema adapter against pinned stock APIs.
// Loaded only by the pinned early native schema bootstrap, outside custom_scripts.
// Native freezes read-only zm_coopSchemaSlots from the actual session before loading.
// Report dvars are execution diagnostics, never an authentication proof.
#using scripts/codescripts/struct;
#using scripts/shared/clientfield_shared;
#using scripts/shared/system_shared;
#using scripts/zm/_zm_laststand;
#using scripts/zm/_zm_utility;

#namespace eroded_coop_fields;

function schema_revision()
{
    return 1;
}

function schema_contract()
{
    return "0d3c7d274e5f3753abe90c252e1b003942c9e9596a995166cd3d3043d4784019";
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
    if (!isdefined(level.laststands) || !isarray(level.laststands)) return;
    for (slot = 4; slot < 8; slot++)
        if (isdefined(level.laststands[slot])) return;
    // All four extra slots are a single fixed schema; connected count is irrelevant.
    for (slot = 4; slot < 8; slot++)
    {
        level.laststands[slot] = spawnstruct();
        level.laststands[slot].bleedouttime = 0;
        level.laststands[slot].lastbleedouttime = 0;
        level.laststands[slot].laststand_update_clientfields = "laststand_update" + slot;
        clientfield::register("world", level.laststands[slot].laststand_update_clientfields, 1, 5, "counter", &zm_laststand::update_bleedout_timer, false, false);
    }
    // Matching score clientuimodels are registered by the early UI Lua hook.

    register_map_slot_fields(tolower(getdvarstring("mapname")));
    level.eroded_coop_fields_revision = schema_revision();
    setdvar("zm_coopSchemaClientRevision", schema_revision());
    setdvar("zm_coopSchemaClientMask", schema_family_mask(tolower(getdvarstring("mapname"))));
    setdvar("zm_coopSchemaClientContract", schema_contract());
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
                clientfield::register("world", "pillar_challenge_" + slot + "_" + challenge, 9000, 3, "int", undefined, false, false);
            clientfield::register("world", "portal_state_ending_" + slot, 9000, 1, "int", undefined, false, false);
        }
    }
}

function register_inventory_fields(suffix, version, bits)
{
    for (slot = 4; slot < 8; slot++)
        clientfield::register("world", "player" + slot + suffix, version, bits, "int", &zm_utility::setsharedinventoryuimodels, false, false);
}
