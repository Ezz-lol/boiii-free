// SPDX-License-Identifier: GPL-3.0-only
// Experimental map mechanics contributed by Eroded Networks.
// Loaded only through the native, opt-in 1-8 map adapter.
// Fallback is an external ORIGINAL-module import; never redirect its refs.
#using custom_scripts/zm/eroded_coop;
#using custom_scripts/zm/eroded_coop_map_readiness;
#using scripts/shared/array_shared;
#using scripts/shared/flag_shared;
#using scripts/shared/util_shared;
#using scripts/zm/zm_temple_pack_a_punch;
#namespace eroded_coop_temple_readiness_entry;

function identity_live(context)
{
    if (!isdefined(context) || !isdefined(context.map) || context.map != "zm_temple" ||
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
        isint(schema) && schema == applied && getdvarstring("mapname") == "zm_temple";
}

// Temple enters during setup, before common readiness and before power_on.
// Only its own current initializing startup may enter this early phase.
function entry_owned(context)
{
    if (!identity_live(context)) return false;
    status = getdvarstring("zm_coopStatus");
    if (status == "initializing") return true;
    return eroded_coop_map_readiness::current(context, "zm_temple");
}

// Diagnostic/native admission receipt only: never change a successor's epoch.
function decline_current(context)
{
    if (!isdefined(context) || !isdefined(context.map) || context.map != "zm_temple" ||
        !isdefined(context.epoch) || !isint(context.epoch) || context.epoch < 1 ||
        !isdefined(context.capacity) || !isint(context.capacity) || context.capacity < 5 || context.capacity > 8)
        return;
    if (getdvarstring("mapname") == context.map && getdvarint("zm_coopStartupEpoch") == context.epoch &&
        getdvarint("zm_coopAppliedLimit") == context.capacity)
        setdvar("zm_coopTemplePapEpoch", 0);
}

// Preflight before any deletion or thread creation. Plate ownership fields do
// not exist until the STOCK pressure workers initialize them after power_on.
function physical_preflight()
{
    triggers = array();
    plates = array();
    for (i = 0; i < 4; i++)
    {
        name = "pap_blocker_trigger" + (i + 1);
        named = getentarray(name, "targetname");
        if (!isarray(named) || named.size != 1 || !isdefined(named[0]) || !isentity(named[0])) return false;
        trigger = named[0];
        if (!isdefined(trigger.targetname) || trigger.targetname != name ||
            !isdefined(trigger.target) || !isstring(trigger.target) || trigger.target == "") return false;
        target = getentarray(trigger.target, "targetname");
        if (!isarray(target) || target.size != 1 || !isdefined(target[0]) || !isentity(target[0])) return false;
        for (j = 0; j < i; j++)
            if (triggers[j] == trigger || plates[j] == target[0]) return false;
        triggers[i] = trigger;
        plates[i] = target[0];
    }
    return true;
}

function work_owned(context, triggers)
{
    return identity_live(context) && eroded_coop_map_readiness::current(context, "zm_temple") &&
        eroded_coop_map_readiness::temple_plates_valid(triggers);
}

function _setup_simultaneous_pap_triggers()
{
    /* OWNED_GUARD_BEGIN */
    if (stock_capacity_current())
    {
        self zm_temple_pack_a_punch::_setup_simultaneous_pap_triggers();
        return;
    }
    context = eroded_coop_map_readiness::capture("zm_temple");
    if (!entry_owned(context) || !physical_preflight())
    {
        decline_current(context);
        return;
    }
    // Native clears this before each server VM and requires the current epoch.
    // Publish only after known entry ownership and physical preflight succeed.
    setdvar("zm_coopTemplePapEpoch", context.epoch);
    level endon(#"pre_end_game");
    level endon(#"end_game");
    level endon(#"intermission");
    /* OWNED_GUARD_END */

    spots = getentarray( "hanging_base", "targetname" );

    for ( i = 0; i < spots.size ; i++ )
    {
        spots[ i ] delete();
    }

    level flag::wait_till( "power_on" );
    /* OWNED_GUARD_BEGIN */
    if (!identity_live(context) || !eroded_coop_map_readiness::current(context, "zm_temple") || !physical_preflight()) { decline_current(context); return; }
    /* OWNED_GUARD_END */

    triggers = array();

    for ( i = 0; i < 4 ; i++ )
    {
        triggers[ i ] = getent( "pap_blocker_trigger" + ( i + 1 ), "targetname" );
    }

    zm_temple_pack_a_punch::_randomize_pressure_plates( triggers );
    array::thread_all( triggers, &zm_temple_pack_a_punch::_pap_pressure_plate_move );
    wait 1;
    /* OWNED_GUARD_BEGIN */
    if (!work_owned(context, triggers)) { decline_current(context); return; }
    /* OWNED_GUARD_END */

    last_num_plates_active = -1;
    last_plate_state = -1;

    while ( true )
    {
        /* OWNED_GUARD_BEGIN */
        if (!work_owned(context, triggers)) { decline_current(context); return; }
        /* OWNED_GUARD_END */

        players = getplayers();
        num_plates_needed = eroded_coop_map_readiness::temple_requirement(players.size, triggers, context);

        /#
            if ( getdvarint( "<dev string:x40>" ) == 2 )
            {
                num_plates_needed = 1;
            }
        #/

        num_plates_active = 0;
        plate_state = 0;

        for ( i = 0; i < triggers.size ; i++ )
        {
            if ( triggers[ i ].plate.active )
            {
                num_plates_active++;
            }

            if ( triggers[ i ].plate.active || triggers[ i ].requiredplayers - 1 >= num_plates_needed )
            {
                plate_state += zm_temple_pack_a_punch::power( 2, triggers[ i ].requiredplayers - 1 );
            }
        }

        if ( last_num_plates_active != num_plates_active || plate_state != last_plate_state )
        {
            last_num_plates_active = num_plates_active;
            last_plate_state = plate_state;
            zm_temple_pack_a_punch::_set_num_plates_active( num_plates_active, plate_state );
        }

        zm_temple_pack_a_punch::_update_stairs( triggers );

        if ( num_plates_active >= num_plates_needed )
        {
            for ( i = 0; i < triggers.size ; i++ )
            {
                triggers[ i ] notify( #"pap_active" );
                triggers[ i ].plate zm_temple_pack_a_punch::_plate_move_down();
            }

            zm_temple_pack_a_punch::_pap_think();
            /* OWNED_GUARD_BEGIN */
            if (!work_owned(context, triggers)) { decline_current(context); return; }
            /* OWNED_GUARD_END */

            zm_temple_pack_a_punch::_randomize_pressure_plates( triggers );
            array::thread_all( triggers, &zm_temple_pack_a_punch::_pap_pressure_plate_move );
            zm_temple_pack_a_punch::_set_num_plates_active( 4, 15 );
            wait 1;
            /* OWNED_GUARD_BEGIN */
            if (!work_owned(context, triggers)) { decline_current(context); return; }
            /* OWNED_GUARD_END */

        }

        util::wait_network_frame();
        /* OWNED_GUARD_BEGIN */
        if (!work_owned(context, triggers)) { decline_current(context); return; }
        /* OWNED_GUARD_END */

    }
}
