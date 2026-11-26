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
    return 2;
}

function schema_contract()
{
    return "5e18c32294a479c8188f5625dfc9204300f228332e2e925d326dfb53c12b460b";
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
        clientfield::register("scriptmover", "eroded_island_pillar", 9000, 11, "int", &island_pillar_clientfield, false, false);
    }
}

function register_inventory_fields(suffix, version, bits)
{
    for (slot = 4; slot < 8; slot++)
        clientfield::register("world", "player" + slot + suffix, version, bits, "int", &zm_utility::setsharedinventoryuimodels, false, false);
}


function island_pillar_decode(value, capacity)
{
    if (!isint(value) || value <= 0 || value > 2047 ||
        !isint(capacity) || capacity < 5 || capacity > 8)
        return undefined;
    slot = (value & 3) + 4;
    if (slot >= capacity)
        return undefined;
    result = spawnstruct();
    result.slot = slot;
    result.states = array(0, 0, 0);
    for (challenge = 0; challenge < 3; challenge++)
    {
        state = (value >> (2 + challenge * 3)) & 7;
        if (state < 1 || state > 4)
            return undefined;
        result.states[challenge] = state;
    }
    return result;
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


function island_pillar_render_current(localclientnum, generation)
{
    return isdefined(self) && schema_capacity_current() && getdvarstring("mapname") == "zm_island" &&
        isdefined(self.eroded_coop_pillar_render_generation) &&
        isarray(self.eroded_coop_pillar_render_generation) &&
        self.eroded_coop_pillar_render_generation[localclientnum] == generation;
}

function island_pillar_retire(localclientnum)
{
    if (!isdefined(self.eroded_coop_pillar_models) || !isarray(self.eroded_coop_pillar_models)) return;
    record = self.eroded_coop_pillar_models[localclientnum];
    if (!isdefined(record)) return;
    self.eroded_coop_pillar_models[localclientnum] = undefined;
    record.active = false;
    if (isdefined(record.model)) record.model delete();
}

function island_pillar_clientfield(localclientnum, oldval, newval, bnewent, binitialsnap, fieldname, bwastimejump)
{
    if (!isdefined(self) || !isint(localclientnum) || localclientnum < 0 || localclientnum > 3 ||
        getdvarstring("mapname") != "zm_island") return;
    if (!isdefined(self.eroded_coop_pillar_render_generation))
        self.eroded_coop_pillar_render_generation = array(0, 0, 0, 0);
    if (!isdefined(self.eroded_coop_pillar_models))
        self.eroded_coop_pillar_models = array();
    if (!isarray(self.eroded_coop_pillar_render_generation) || !isarray(self.eroded_coop_pillar_models)) return;
    generation = self.eroded_coop_pillar_render_generation[localclientnum];
    if (!isdefined(generation)) generation = 0;
    if (!isint(generation) || generation < 0 || generation > 1000000) return;
    generation++;
    self.eroded_coop_pillar_render_generation[localclientnum] = generation;
    decoded = island_pillar_decode(newval, getdvarint("zm_coopSchemaSlots"));
    if (!isdefined(decoded))
    {
        island_pillar_retire(localclientnum);
        return;
    }
    self thread island_pillar_render(localclientnum, decoded, generation);
}

function island_pillar_template(localclientnum)
{
    // Use the actual local retail model and its interaction frame. Server-side
    // tag_origin is only a replicated locator, never a DObj or a pillar model.
    models = getentarray(localclientnum, "challenge_pillar_0", "targetname");
    triggers = struct::get_array("s_challenge_trigger", "targetname");
    if (!isarray(models) || models.size != 1 || !isdefined(models[0]) ||
        !isdefined(models[0].model) || !isarray(triggers) || triggers.size != 4) return undefined;
    base = undefined;
    seen = array();
    keys = getarraykeys(triggers);
    for (index = 0; index < keys.size; index++)
    {
        trigger = triggers[keys[index]];
        // ACTS script_* spelling compiles to field ID zero. This canonical ID
        // was independently checked against the actual retail field operand.
        if (!isdefined(trigger) || !isint(trigger.var_7cc697d0) || trigger.var_7cc697d0 < 0 ||
            trigger.var_7cc697d0 > 3 || isdefined(seen[trigger.var_7cc697d0])) return undefined;
        seen[trigger.var_7cc697d0] = true;
        if (trigger.var_7cc697d0 != 0) continue;
        if (isdefined(base)) return undefined;
        base = trigger;
    }
    if (!isdefined(base)) return undefined;
    pose = island_pillar_clone_pose(models[0].origin, models[0].angles,
        base.origin, base.angles, self.origin, self.angles);
    if (!isdefined(pose)) return undefined;
    result = spawnstruct();
    result.model = models[0].model;
    result.pose = pose;
    return result;
}

function island_pillar_model_current(localclientnum, locator, record)
{
    return isdefined(locator) && schema_capacity_current() && getdvarstring("mapname") == "zm_island" &&
        isdefined(record) && record.active && isdefined(record.model) &&
        record.slot < getdvarint("zm_coopSchemaSlots") &&
        isdefined(locator.eroded_coop_pillar_models) && isarray(locator.eroded_coop_pillar_models) &&
        locator.eroded_coop_pillar_models[localclientnum] == record;
}

function island_pillar_model_lifetime(localclientnum, locator, record)
{
    // Run on the owned clone: ending on locator shutdown would skip deletion.
    self endon(#"entityshutdown");
    while (island_pillar_model_current(localclientnum, locator, record)) wait 0.1;
    if (isdefined(locator) && isdefined(locator.eroded_coop_pillar_models) &&
        isarray(locator.eroded_coop_pillar_models) && locator.eroded_coop_pillar_models[localclientnum] == record)
        locator.eroded_coop_pillar_models[localclientnum] = undefined;
    record.active = false;
    self delete();
}

function island_pillar_render(localclientnum, decoded, generation)
{
    for (attempt = 0; attempt < 120; attempt++)
    {
        if (!island_pillar_render_current(localclientnum, generation) ||
            decoded.slot >= getdvarint("zm_coopSchemaSlots")) return;
        player = getlocalplayer(localclientnum);
        record = self.eroded_coop_pillar_models[localclientnum];
        if (isdefined(record) && (!record.active || record.slot != decoded.slot || !isdefined(record.model)))
        {
            island_pillar_retire(localclientnum);
            record = undefined;
        }
        if (isdefined(player) && !isdefined(record))
        {
            template = island_pillar_template(localclientnum);
            if (isdefined(template))
            {
                // spawn_model has no wait; publish the owned record before any
                // yield so concurrent field updates cannot allocate duplicates.
                model = util::spawn_model(localclientnum, template.model, template.pose.origin, template.pose.angles);
                if (isdefined(model))
                {
                    model hide();
                    record = spawnstruct();
                    record.active = true;
                    record.model = model;
                    record.slot = decoded.slot;
                    record.symbol_parts = template.model == "p7_zm_isl_ritual_pillar_symbol";
                    self.eroded_coop_pillar_models[localclientnum] = record;
                    model thread island_pillar_model_lifetime(localclientnum, self, record);
                }
            }
        }
        if (isdefined(player) && isdefined(record) && record.model hasdobj(localclientnum)) break;
        wait 0.05;
    }
    if (!island_pillar_render_current(localclientnum, generation)) return;
    if (!isdefined(player) || !island_pillar_model_current(localclientnum, self, record) ||
        !record.model hasdobj(localclientnum))
    {
        island_pillar_retire(localclientnum);
        return;
    }
    // Do not address absent joints on a different/composite custom model.
    // Client-image verification is still required for the retail attachment layout.
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
