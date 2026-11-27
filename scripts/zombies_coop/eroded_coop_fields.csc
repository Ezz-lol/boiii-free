// Independently implemented paired schema adapter against pinned stock APIs.
// Loaded only by the pinned early native schema bootstrap, outside custom_scripts.
// Native freezes read-only zm_coopSchemaSlots from the actual session before loading.
// Report dvars are execution diagnostics, never an authentication proof.
#using scripts/codescripts/struct;
#using scripts/shared/clientfield_shared;
#using scripts/shared/system_shared;
#using scripts/shared/util_shared;
#using scripts/zm/_zm_laststand;
#using scripts/zm/_zm_utility;

#namespace eroded_coop_fields;

function schema_revision()
{
    return 4;
}

function schema_contract()
{
    return "c08cfc132da28936a532f91936afe6970730b8351fe3c64aff52bf48bce395f1";
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
    // Extra score displays use existing playerScore deltas; no extra UI bit allocation.

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
            clientfield::register("world", "portal_state_ending_" + slot, 9000, 1, "int", undefined, false, false);
        }
        clientfield::register("world", "pillar_challenge_4_1", 9000, 3, "int", &island_pillar_4_1, false, true);
        clientfield::register("world", "pillar_challenge_4_2", 9000, 3, "int", &island_pillar_4_2, false, true);
        clientfield::register("world", "pillar_challenge_4_3", 9000, 3, "int", &island_pillar_4_3, false, true);
        clientfield::register("world", "pillar_challenge_5_1", 9000, 3, "int", &island_pillar_5_1, false, true);
        clientfield::register("world", "pillar_challenge_5_2", 9000, 3, "int", &island_pillar_5_2, false, true);
        clientfield::register("world", "pillar_challenge_5_3", 9000, 3, "int", &island_pillar_5_3, false, true);
        clientfield::register("world", "pillar_challenge_6_1", 9000, 3, "int", &island_pillar_6_1, false, true);
        clientfield::register("world", "pillar_challenge_6_2", 9000, 3, "int", &island_pillar_6_2, false, true);
        clientfield::register("world", "pillar_challenge_6_3", 9000, 3, "int", &island_pillar_6_3, false, true);
        clientfield::register("world", "pillar_challenge_7_1", 9000, 3, "int", &island_pillar_7_1, false, true);
        clientfield::register("world", "pillar_challenge_7_2", 9000, 3, "int", &island_pillar_7_2, false, true);
        clientfield::register("world", "pillar_challenge_7_3", 9000, 3, "int", &island_pillar_7_3, false, true);
    }
}

function register_inventory_fields(suffix, version, bits)
{
    for (slot = 4; slot < 8; slot++)
        clientfield::register("world", "player" + slot + suffix, version, bits, "int", &zm_utility::setsharedinventoryuimodels, false, false);
}


// Preserve the actual template offset in its measured vertical interaction frame.
function island_pillar_valid_vector(value, bound)
{
    if (!isvec(value) || !isint(bound) || bound < 1 || bound > 1000000)
        return false;
    for (component = 0; component < 3; component++)
    {
        number = value[component];
        if ((!isint(number) && !isfloat(number)) || number != number ||
            number <= (0 - bound) || number >= bound)
            return false;
    }
    return true;
}

function island_pillar_clone_pose(template_origin, template_angles, base_origin, base_angles, locator_origin, locator_angles)
{
    if (!island_pillar_valid_vector(template_origin, 1000000) ||
        !island_pillar_valid_vector(base_origin, 1000000) ||
        !island_pillar_valid_vector(locator_origin, 1000000) ||
        !island_pillar_valid_vector(template_angles, 3600) ||
        !island_pillar_valid_vector(base_angles, 3600) ||
        !island_pillar_valid_vector(locator_angles, 3600))
        return undefined;
    // The observed stock interaction frames are vertical. Do not silently
    // approximate a custom pitched/rolled frame with a yaw-only rotation.
    if (base_angles[0] != 0 || base_angles[2] != 0 ||
        locator_angles[0] != 0 || locator_angles[2] != 0)
        return undefined;
    dx = template_origin[0] - base_origin[0];
    dy = template_origin[1] - base_origin[1];
    dz = template_origin[2] - base_origin[2];
    yaw = locator_angles[1] - base_angles[1];
    cosine = cos(yaw);
    sine = sin(yaw);
    x = locator_origin[0] + dx * cosine - dy * sine;
    y = locator_origin[1] + dx * sine + dy * cosine;
    z = locator_origin[2] + dz;
    result = spawnstruct();
    result.origin = (x, y, z);
    result.angles = (template_angles[0], template_angles[1] + yaw, template_angles[2]);
    if (!island_pillar_valid_vector(result.origin, 1000000) ||
        !island_pillar_valid_vector(result.angles, 3600))
        return undefined;
    return result;
}



function island_pillar_read_state(slot)
{
    if (!isint(slot) || slot < 4 || slot >= getdvarint("zm_coopSchemaSlots")) return undefined;
    result = spawnstruct();
    result.slot = slot;
    result.states = array();
    for (tier = 1; tier <= 3; tier++)
    {
        value = level clientfield::get("pillar_challenge_" + slot + "_" + tier);
        // Zero is the extra slot's absent/default state, never a visible pillar.
        if (!isint(value) || value < 1 || value > 4) return undefined;
        result.states[tier - 1] = value;
    }
    return result;
}

function island_pillar_render_current(localclientnum, slot, generation)
{
    key = localclientnum * 8 + slot;
    return schema_capacity_current() && getdvarstring("mapname") == "zm_island" &&
        slot < getdvarint("zm_coopSchemaSlots") &&
        isdefined(level.eroded_coop_fields_revision) && level.eroded_coop_fields_revision == schema_revision() &&
        isdefined(level.eroded_coop_pillar_render_generation) &&
        isarray(level.eroded_coop_pillar_render_generation) &&
        level.eroded_coop_pillar_render_generation[key] == generation;
}

function island_pillar_retire(localclientnum, slot)
{
    key = localclientnum * 8 + slot;
    if (!isdefined(level.eroded_coop_pillar_models) || !isarray(level.eroded_coop_pillar_models)) return;
    record = level.eroded_coop_pillar_models[key];
    if (!isdefined(record)) return;
    level.eroded_coop_pillar_models[key] = undefined;
    record.active = false;
    if (isdefined(record.model)) record.model delete();
}

function island_pillar_world_update(localclientnum, slot, tier, newval)
{
    if (!isint(localclientnum) || localclientnum < 0 || localclientnum > 3 ||
        !isint(slot) || slot < 4 || slot > 7 || !isint(tier) || tier < 1 || tier > 3 ||
        getdvarstring("mapname") != "zm_island") return;
    if (!isdefined(level.eroded_coop_pillar_render_generation))
    {
        level.eroded_coop_pillar_render_generation = array();
        for (index = 0; index < 32; index++) level.eroded_coop_pillar_render_generation[index] = 0;
    }
    if (!isdefined(level.eroded_coop_pillar_models)) level.eroded_coop_pillar_models = array();
    if (!isarray(level.eroded_coop_pillar_render_generation) || !isarray(level.eroded_coop_pillar_models)) return;
    key = localclientnum * 8 + slot;
    generation = level.eroded_coop_pillar_render_generation[key];
    if (!isint(generation) || generation < 0 || generation > 1000000) return;
    generation++;
    level.eroded_coop_pillar_render_generation[key] = generation;
    // Rendering reads the committed triplet after this callback returns. Do not
    // assume GetWorldClientField has already committed the callback's snapshot.
    level thread island_pillar_render(localclientnum, slot, generation);
}

function island_pillar_template(localclientnum, slot)
{
    models = getentarray(localclientnum, "challenge_pillar_0", "targetname");
    triggers = struct::get_array("s_challenge_trigger", "targetname");
    altars = struct::get_array("s_challenge_altar", "targetname");
    if (!isarray(models) || models.size != 1 || !isdefined(models[0]) || !isdefined(models[0].model) ||
        !isarray(triggers) || triggers.size != 4 || !isarray(altars) || altars.size != 1 ||
        !isdefined(altars[0]) || !island_pillar_valid_vector(altars[0].origin, 32769)) return undefined;
    frames = array();
    keys = getarraykeys(triggers);
    for (index = 0; index < keys.size; index++)
    {
        trigger = triggers[keys[index]];
        // Canonical retail script_special ID; ACTS literal script_* emits zero.
        if (!isdefined(trigger) || !isint(trigger.var_7cc697d0) || trigger.var_7cc697d0 < 0 ||
            trigger.var_7cc697d0 > 3 || isdefined(frames[trigger.var_7cc697d0]) ||
            !island_pillar_valid_vector(trigger.origin, 32769) || !island_pillar_valid_vector(trigger.angles, 3600) ||
            trigger.angles[0] != 0 || trigger.angles[2] != 0) return undefined;
        frames[trigger.var_7cc697d0] = trigger;
    }
    if (!isint(slot) || slot < 4 || slot > 7) return undefined;
    // Same measured stock-slot pairing and altar-facing yaw as the server.
    pairs = array(array(0, 1), array(2, 3), array(1, 2), array(0, 3));
    pair = pairs[slot - 4];
    origin = (frames[pair[0]].origin + frames[pair[1]].origin) * 0.5;
    if (distance(origin, altars[0].origin) < 64 || distance(origin, altars[0].origin) > 512) return undefined;
    facing = vectortoangles(altars[0].origin - origin);
    angles = (0, facing[1], 0);
    pose = island_pillar_clone_pose(models[0].origin, models[0].angles,
        frames[0].origin, frames[0].angles, origin, angles);
    if (!isdefined(pose)) return undefined;
    result = spawnstruct();
    result.model = models[0].model;
    result.pose = pose;
    return result;
}

function island_pillar_model_current(localclientnum, slot, record)
{
    key = localclientnum * 8 + slot;
    return schema_capacity_current() && getdvarstring("mapname") == "zm_island" &&
        isdefined(level.eroded_coop_fields_revision) && level.eroded_coop_fields_revision == schema_revision() &&
        isdefined(record) && record.active && isdefined(record.model) && record.slot == slot &&
        isdefined(island_pillar_read_state(slot)) &&
        isdefined(level.eroded_coop_pillar_models) && isarray(level.eroded_coop_pillar_models) &&
        level.eroded_coop_pillar_models[key] == record;
}

function island_pillar_model_lifetime(localclientnum, slot, record)
{
    self endon(#"entityshutdown");
    while (island_pillar_model_current(localclientnum, slot, record)) wait 0.1;
    key = localclientnum * 8 + slot;
    if (isdefined(level.eroded_coop_pillar_models) && isarray(level.eroded_coop_pillar_models) &&
        level.eroded_coop_pillar_models[key] == record) level.eroded_coop_pillar_models[key] = undefined;
    record.active = false;
    self delete();
}

function island_pillar_render(localclientnum, slot, generation)
{
    key = localclientnum * 8 + slot;
    for (attempt = 0; attempt < 120; attempt++)
    {
        // Yield before the first read as well as retries: callbacks may run
        // while their native snapshot is still being committed.
        wait 0.05;
        if (!island_pillar_render_current(localclientnum, slot, generation)) return;
        decoded = island_pillar_read_state(slot);
        if (!isdefined(decoded))
        {
            island_pillar_retire(localclientnum, slot);
            return;
        }
        player = getlocalplayer(localclientnum);
        record = level.eroded_coop_pillar_models[key];
        if (isdefined(record) && (!record.active || record.slot != slot || !isdefined(record.model)))
        {
            island_pillar_retire(localclientnum, slot);
            record = undefined;
        }
        if (isdefined(player) && !isdefined(record))
        {
            template = island_pillar_template(localclientnum, slot);
            if (isdefined(template))
            {
                // No wait in stock spawn_model: publish ownership before yielding.
                model = util::spawn_model(localclientnum, template.model, template.pose.origin, template.pose.angles);
                if (isdefined(model))
                {
                    model hide();
                    record = spawnstruct();
                    record.active = true;
                    record.model = model;
                    record.slot = slot;
                    record.symbol_parts = template.model == "p7_zm_isl_ritual_pillar_symbol";
                    level.eroded_coop_pillar_models[key] = record;
                    model thread island_pillar_model_lifetime(localclientnum, slot, record);
                }
            }
        }
        if (isdefined(player) && isdefined(record) && record.model hasdobj(localclientnum)) break;
    }
    if (!island_pillar_render_current(localclientnum, slot, generation)) return;
    decoded = island_pillar_read_state(slot);
    if (!isdefined(decoded) || !isdefined(player) || !island_pillar_model_current(localclientnum, slot, record) ||
        !record.model hasdobj(localclientnum))
    {
        island_pillar_retire(localclientnum, slot);
        return;
    }
    // Missing/composite custom attachments are not guessed from a model name.
    if (record.symbol_parts) record.model island_pillar_apply_parts(localclientnum, decoded, player);
    record.model show();
}

function island_pillar_apply_parts(localclientnum, decoded, player)
{
    own = player getentitynumber() == decoded.slot;
    for (challenge = 0; challenge < 3; challenge++)
    {
        suffix = "0" + (challenge + 1);
        self hidepart(localclientnum, "j_player_started_" + suffix, "p7_zm_isl_ritual_pillar_symbol");
        self hidepart(localclientnum, "j_player_completed_" + suffix, "p7_zm_isl_ritual_pillar_symbol");
        self hidepart(localclientnum, "j_player_claimed_" + suffix, "p7_zm_isl_ritual_pillar_symbol");
        self hidepart(localclientnum, "j_ally_started_" + suffix, "p7_zm_isl_ritual_pillar_symbol");
        self hidepart(localclientnum, "j_ally_completed_" + suffix, "p7_zm_isl_ritual_pillar_symbol");
        self hidepart(localclientnum, "j_ally_claimed_" + suffix, "p7_zm_isl_ritual_pillar_symbol");
        state = decoded.states[challenge];
        if (state == 1)
            continue;
        role = "ally";
        if (own)
            role = "player";
        phase = "started";
        if (state == 3)
            phase = "completed";
        else if (state == 4)
            phase = "claimed";
        self showpart(localclientnum, "j_" + role + "_" + phase + "_" + suffix,
            "p7_zm_isl_ritual_pillar_symbol");
    }
}

function island_pillar_4_1(localclientnum, oldval, newval, bnewent, binitialsnap, fieldname, bwastimejump)
{
    island_pillar_world_update(localclientnum, 4, 1, newval);
}

function island_pillar_4_2(localclientnum, oldval, newval, bnewent, binitialsnap, fieldname, bwastimejump)
{
    island_pillar_world_update(localclientnum, 4, 2, newval);
}

function island_pillar_4_3(localclientnum, oldval, newval, bnewent, binitialsnap, fieldname, bwastimejump)
{
    island_pillar_world_update(localclientnum, 4, 3, newval);
}

function island_pillar_5_1(localclientnum, oldval, newval, bnewent, binitialsnap, fieldname, bwastimejump)
{
    island_pillar_world_update(localclientnum, 5, 1, newval);
}

function island_pillar_5_2(localclientnum, oldval, newval, bnewent, binitialsnap, fieldname, bwastimejump)
{
    island_pillar_world_update(localclientnum, 5, 2, newval);
}

function island_pillar_5_3(localclientnum, oldval, newval, bnewent, binitialsnap, fieldname, bwastimejump)
{
    island_pillar_world_update(localclientnum, 5, 3, newval);
}

function island_pillar_6_1(localclientnum, oldval, newval, bnewent, binitialsnap, fieldname, bwastimejump)
{
    island_pillar_world_update(localclientnum, 6, 1, newval);
}

function island_pillar_6_2(localclientnum, oldval, newval, bnewent, binitialsnap, fieldname, bwastimejump)
{
    island_pillar_world_update(localclientnum, 6, 2, newval);
}

function island_pillar_6_3(localclientnum, oldval, newval, bnewent, binitialsnap, fieldname, bwastimejump)
{
    island_pillar_world_update(localclientnum, 6, 3, newval);
}

function island_pillar_7_1(localclientnum, oldval, newval, bnewent, binitialsnap, fieldname, bwastimejump)
{
    island_pillar_world_update(localclientnum, 7, 1, newval);
}

function island_pillar_7_2(localclientnum, oldval, newval, bnewent, binitialsnap, fieldname, bwastimejump)
{
    island_pillar_world_update(localclientnum, 7, 2, newval);
}

function island_pillar_7_3(localclientnum, oldval, newval, bnewent, binitialsnap, fieldname, bwastimejump)
{
    island_pillar_world_update(localclientnum, 7, 3, newval);
}
