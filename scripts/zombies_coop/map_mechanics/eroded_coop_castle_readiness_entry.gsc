// SPDX-License-Identifier: GPL-3.0-only
// Experimental map mechanics contributed by Eroded Networks.
// Loaded only through the native, opt-in 1-8 map adapter.
// Fallback is an external ORIGINAL-module import; never redirect its refs.
#using custom_scripts/zm/eroded_coop;
#using custom_scripts/zm/eroded_coop_map_readiness;
#using scripts/shared/flag_shared;
#using scripts/zm/zm_castle_ee_bossfight;
#namespace eroded_coop_castle_readiness_entry;

function identity_live(context)
{
    if (!isdefined(context) || !isdefined(context.map) || context.map != "zm_castle" ||
        !isdefined(context.epoch) || !isint(context.epoch) || context.epoch < 1 ||
        !isdefined(context.capacity) || !isint(context.capacity) || context.capacity < 5 || context.capacity > 8)
        return false;
    return eroded_coop::startup_current(context.epoch, context.map) &&
        getdvarint("zm_coopAppliedLimit") == context.capacity && getdvarint("zm_coopSchemaSlots") == context.capacity &&
        isdefined(level.eroded_coop_started_epoch) && level.eroded_coop_started_epoch == context.epoch &&
        isdefined(level.eroded_coop_started_map) && level.eroded_coop_started_map == context.map;
}

// Stock fallback requires a positively known, consistent nonexpanded setup.
// Unknown/zero/invalid or mixed expanded capacities must never reach stock.
function stock_capacity_current()
{
    applied = getdvarint("zm_coopAppliedLimit");
    schema = getdvarint("zm_coopSchemaSlots");
    return isint(applied) && applied >= 1 && applied <= 4 &&
        isint(schema) && schema == applied && getdvarstring("mapname") == "zm_castle";
}

function physical_preflight(areas)
{
    named = getentarray("boss_gravity_spike_start_area", "targetname");
    if (!eroded_coop_map_readiness::same_four(areas, named)) return false;
    for (i = 0; i < 4; i++)
        if (!isdefined(areas[i].targetname) || areas[i].targetname != "boss_gravity_spike_start_area") return false;
    return true;
}

function work_owned(context, areas)
{
    return identity_live(context) && eroded_coop_map_readiness::current(context, "zm_castle") &&
        eroded_coop_map_readiness::castle_areas_valid(areas);
}

function boss_fight_ready()
{
    /* OWNED_GUARD_BEGIN */
    if (stock_capacity_current())
    {
        self zm_castle_ee_bossfight::boss_fight_ready();
        return;
    }
    context = eroded_coop_map_readiness::capture("zm_castle");
    // Unknown expanded entry stops before claim reset or worker creation.
    preflight_areas = getentarray("boss_gravity_spike_start_area", "targetname");
    if (!identity_live(context) || !eroded_coop_map_readiness::current(context, "zm_castle") || !physical_preflight(preflight_areas))
        return;
    level endon(#"pre_end_game");
    level endon(#"end_game");
    level endon(#"intermission");
    /* OWNED_GUARD_END */

    level endon( #"boss_fight_begin" );
    level.var_b366f2dc = 0;
    var_23a96e18 = getentarray( "boss_gravity_spike_start_area", "targetname" );

    for ( eroded_area_index = 0; eroded_area_index < var_23a96e18.size; eroded_area_index++ )
    {
        var_ad2fc56d = var_23a96e18[eroded_area_index];
        var_ad2fc56d.b_claimed = 0;
        var_ad2fc56d thread zm_castle_ee_bossfight::function_1c249965();
    }

    while ( level.var_b366f2dc < eroded_coop_map_readiness::castle_requirement(level.players.size, var_23a96e18, context) )
    {
        wait 0.05;
        /* OWNED_GUARD_BEGIN */
        if (!work_owned(context, var_23a96e18)) return;
        /* OWNED_GUARD_END */

    }

    wait 0.75;
    /* OWNED_GUARD_BEGIN */
    if (!work_owned(context, var_23a96e18)) return;
    /* OWNED_GUARD_END */


    eroded_player_keys = getarraykeys(level.players);
    for ( eroded_player_index = 0; eroded_player_index < eroded_player_keys.size; eroded_player_index++ )
    {
        player = level.players[eroded_player_keys[eroded_player_index]];
        player.var_d725b0f2 = undefined;
    }

    level flag::set( "boss_fight_begin" );
}
