// Experimental round controller contributed by Eroded Networks.
// Compiled separately for the retail BO3 1C VM; never replaces a retail script.
#using scripts/zm/_zm;
#using scripts/zm/_zm_game_module;

#namespace eroded_coop;

function autoexec main()
{
    limit = getdvarint("zm_coopAppliedLimit");
    epoch = getdvarint("zm_coopStartupEpoch");
    map = getdvarstring("mapname");
    // A duplicate autoexec must not reset another installer's readiness/heartbeat.
    if (isdefined(level.eroded_coop_started_epoch) && level.eroded_coop_started_epoch == epoch &&
        isdefined(level.eroded_coop_started_map) && level.eroded_coop_started_map == map)
        return;
    setdvar("zm_coopStatus", "disabled");
    clear_readiness();
    setdvar("zm_coopRoundStatus", "inactive");
    if (limit < 1 || limit > 8)
    {
        setdvar("zm_coopStatus", "invalid_applied_limit");
        return;
    }
    if (limit <= 4)
        return;
    if (epoch < 1)
    {
        setdvar("zm_coopStatus", "invalid_startup_epoch");
        return;
    }
    level.eroded_coop_started_epoch = epoch;
    level.eroded_coop_started_map = map;
    setdvar("zm_coopStatus", "initializing");
    level thread install(limit, epoch, map);
}

function clear_readiness()
{
    setdvar("zm_coopReadyEpoch", 0);
    setdvar("zm_coopReadyMap", "");
    setdvar("zm_coopHeartbeat", 0);
}

function startup_current(epoch, map)
{
    return getdvarint("zm_coopStartupEpoch") == epoch && getdvarstring("mapname") == map;
}

function install(limit, epoch, map)
{
    level endon(#"intermission");
    level endon(#"end_game");
    // Initial adapters only. Other maps need their own gameplay/character audit.
    if (map != "zm_zod" && map != "zm_tomb")
    {
        setdvar("zm_coopStatus", "unsupported_map");
        println("[Zombies coop] Expanded gameplay is not validated for this map.");
        return;
    }

    started = gettime();
    while (!isdefined(level.player_too_many_players_check_func) || !isdefined(level.func_get_zombie_spawn_delay) ||
        (map == "zm_zod" && !isdefined(level.eroded_coop_zod_delay)))
    {
        // An old VM/thread must not clear a newer map's readiness values.
        if (!startup_current(epoch, map))
            return;
        if (gettime() - started >= 30000)
        {
            setdvar("zm_coopStatus", "initialization_timeout");
            return;
        }
        wait 0.05;
    }
    if (!startup_current(epoch, map))
        return;
    expected_delay = &zm::get_zombie_spawn_delay;
    if (map == "zm_zod")
        expected_delay = level.eroded_coop_zod_delay;
    if (level.player_too_many_players_check_func != &zm::player_too_many_players_check)
    {
        setdvar("zm_coopStatus", "custom_player_check_preserved");
        return;
    }
    if (level.func_get_zombie_spawn_delay != expected_delay)
    {
        setdvar("zm_coopStatus", "custom_spawn_delay_preserved");
        return;
    }
    if (level.scr_zm_ui_gametype != "zclassic")
    {
        setdvar("zm_coopStatus", "custom_gametype_preserved");
        return;
    }
    level.eroded_coop_stock_spawn_delay = expected_delay;
    level.eroded_coop_limit = limit;
    level.player_too_many_players_check_func = &player_count_check;
    level.func_get_zombie_spawn_delay = &spawn_delay;
    install_bgb_movement_hook();
    level thread round_listener(epoch, map);
    level thread diagnostic_heartbeat(epoch, map);
    setdvar("zm_coopReadyMap", map);
    setdvar("zm_coopReadyEpoch", epoch);
    setdvar("zm_coopStatus", "experimental_callbacks_installed");
    println("[Zombies coop] Experimental stock-round callbacks installed; limit " + limit);
}

function diagnostic_heartbeat(epoch, map)
{
    level endon(#"intermission");
    level endon(#"end_game");
    heartbeat = 0;
    while (true)
    {
        if (!startup_current(epoch, map))
            return;
        if (!owns_callbacks())
        {
            clear_readiness();
            setdvar("zm_coopStatus", "callback_ownership_lost");
            return;
        }
        setdvar("zm_coopHeartbeat", heartbeat);
        wait 1;
        heartbeat++;
    }
}

function player_count_check()
{
    if (!isint(level.eroded_coop_limit) || level.eroded_coop_limit < 5 || level.eroded_coop_limit > 8)
    {
        clear_readiness();
        setdvar("zm_coopStatus", "callback_ownership_lost");
        return;
    }
    if (getplayers().size > level.eroded_coop_limit)
    {
        zm_game_module::freeze_players(1);
        level notify(#"end_game");
    }
}

function spawn_delay(round)
{
    // Round advancement can race the last disconnect; stock has no zero-player case.
    if (!isdefined(level.players) || !isarray(level.players))
        return 0.67;
    count = level.players.size;
    if (count < 1 || count > 8 || !isdefined(level.eroded_coop_stock_spawn_delay))
        return 0.67;
    if (!isint(round) || round < 1)
        round = 1;
    if (count <= 4)
        return [[ level.eroded_coop_stock_spawn_delay ]](round);
    // Zod deliberately paces round 1 at 3s and rounds 2-4 at 2s, for all counts.
    if (getdvarstring("mapname") == "zm_zod" && round < 5)
        return [[ level.eroded_coop_stock_spawn_delay ]](round);
    return extended_spawn_delay(round, count);
}

function extended_spawn_delay(round, count)
{
    if (!isint(count) || count < 5 || count > 8)
        return 0.67;
    if (!isint(round) || round < 1)
        round = 1;
    // Extend four-player pacing without raising the concurrent AI/actor limits.
    delay = 0.67 * 4 / count;
    if (round > 60)
        round = 60;
    for (i = 1; i < round; i++)
    {
        delay *= 0.95;
        if (delay <= 0.1)
            return 0.1;
    }
    return delay;
}

function owns_callbacks()
{
    return isint(level.eroded_coop_limit) && level.eroded_coop_limit >= 5 && level.eroded_coop_limit <= 8 &&
        isdefined(level.eroded_coop_stock_spawn_delay) &&
        level.player_too_many_players_check_func == &player_count_check &&
        level.func_get_zombie_spawn_delay == &spawn_delay;
}


// GobbleGum move-frequency fallback reviewed from ShawnDuPreez/cb-mods
// 0b2eafd75aacc451d458e781caf3dec18b228b22 (GPL-3.0). The stock 1-4 table
// was independently checked against ate47/bo3-source fe040b1187b4850fe8da3c78b703c986c87d1028,
// scripts/zm/_zm_bgb_machine.gsc. Only the current expanded stock adapter installs it;
// existing map/mod callbacks keep ownership. These are movement uses, not stock capacity.
function bgb_movement_limits(count)
{
    if (!isint(count) || count < 1 || count > 8)
        return undefined;
    if (count == 1)
        return array(1, 3);
    if (count == 2)
        return array(1, 4);
    if (count == 3)
        return array(3, 5);
    return array(3, 6);
}

function bgb_movement_limits_for_players(players)
{
    if (!isarray(players))
        return undefined;
    return bgb_movement_limits(players.size);
}

function bgb_context_current()
{
    if (!isdefined(level.eroded_coop_started_epoch) || !isint(level.eroded_coop_started_epoch) ||
        level.eroded_coop_started_epoch < 1 || !isdefined(level.eroded_coop_started_map))
        return false;
    return startup_current(level.eroded_coop_started_epoch, level.eroded_coop_started_map) && owns_callbacks();
}

function install_bgb_movement_hook()
{
    if (!bgb_context_current())
        return false;
    if (isdefined(level.bgb_machine_movement_frequency_override_func))
        return level.bgb_machine_movement_frequency_override_func == &bgb_machine_movement_frequency;
    level.bgb_machine_movement_frequency_override_func = &bgb_machine_movement_frequency;
    return true;
}

function bgb_machine_movement_frequency()
{
    if (!bgb_context_current() || !isdefined(level.bgb_machine_movement_frequency_override_func) ||
        level.bgb_machine_movement_frequency_override_func != &bgb_machine_movement_frequency)
        return;
    limits = bgb_movement_limits_for_players(level.players);
    if (!isdefined(limits))
        return;
    if (level.players.size > level.eroded_coop_limit)
        return;
    level.bgb_machine_min_uses_before_move = limits[0];
    level.bgb_machine_max_uses_before_move = limits[1];
}

function round_listener(epoch, map)
{
    level endon(#"intermission");
    level endon(#"end_game");
    while (true)
    {
        level waittill(#"zombie_total_set");
        if (!startup_current(epoch, map))
            return;
        level thread track_round(epoch, map);
    }
}

function valid_counters(total, respawns)
{
    // BO3 script integers are signed 32-bit; defer unexpected floats/objects.
    return isint(total) && isint(respawns) && total >= 0 && respawns >= 0 && respawns <= total;
}

// Pure arithmetic shared by the runtime controller and the actual-VM fixture.
function remaining_budget(target, consumed, respawns)
{
    if (!isint(target) || !isint(consumed) || !isint(respawns) ||
        target < 0 || consumed < 0 || respawns < 0)
        return undefined;
    remaining = target - consumed;
    if (remaining < 0)
        remaining = 0;
    if (remaining > 2147483647 - respawns)
        return undefined;
    return remaining + respawns;
}

function accounted_spawn_delta(previous_pending, current_pending, recycle_delta)
{
    if (!isint(previous_pending) || !isint(current_pending) || !isint(recycle_delta) ||
        previous_pending < 0 || current_pending < 0 || recycle_delta < 0 ||
        recycle_delta > 2147483647 - previous_pending)
        return undefined;
    available = previous_pending + recycle_delta;
    // Only stock-accounted additions are ours. Leave other map/mod budgets alone.
    if (current_pending > available)
        return undefined;
    return available - current_pending;
}

function recycle_credit_balance(previous_credits, recycle_delta, spawns, current_pending)
{
    if (!isint(previous_credits) || !isint(recycle_delta) || !isint(spawns) ||
        !isint(current_pending) || previous_credits < 0 || recycle_delta < 0 || spawns < 0 || current_pending < 0 ||
        recycle_delta > 2147483647 - previous_credits)
        return undefined;
    // A normal spawn followed by a recycle can occur within one polling sample.
    // Keep the maximum queued replacements consistent with the observed queue;
    // otherwise shrinking its target could erase a still-pending replacement.
    credits = previous_credits + recycle_delta;
    if (credits > current_pending)
        credits = current_pending;
    if (previous_credits + recycle_delta - credits > spawns)
        return undefined;
    return credits;
}

function custom_round_controller()
{
    if (isdefined(level.zombie_total_set_func))
        return true;
    if (!isdefined(level.fn_custom_round_ai_spawn))
        return false;
    // Known Zod parasite/raps spawning keeps running unchanged. Its quota
    // decrements are observed along with ordinary spawns, not replaced.
    return !isdefined(level.eroded_coop_known_custom_spawn) ||
        level.fn_custom_round_ai_spawn != level.eroded_coop_known_custom_spawn;
}

function track_round(epoch, map)
{
    level endon(#"intermission");
    level endon(#"end_game");
    level endon(#"end_of_round");
    level endon(#"restart_round");
    level endon(#"zombie_total_set");
    // Custom round controllers and special-round paths retain ownership.
    if (!startup_current(epoch, map) || !owns_callbacks() || custom_round_controller())
        return;
    if (!isdefined(level.players) || !isarray(level.players) || !isint(level.round_number) || level.round_number < 1)
        return;
    count = getplayers().size;
    if (count < 1 || count > level.eroded_coop_limit || count != level.players.size)
        return;
    round = level.round_number;
    target = zm::get_zombie_count_for_round(round, count);
    if (!isint(target) || target < 0 || !valid_counters(level.zombie_total, level.zombie_respawns) ||
        !isint(level.zombie_total_subtract) || level.zombie_total_subtract < 0)
        return;
    pending = level.zombie_total - level.zombie_respawns;
    if (pending > target)
        return;
    consumed = target - pending;
    // Stock timeout/recycle paths add total++ and total_subtract++, separately
    // from the map cleanup total++/respawns++ pair. The subtract counter is
    // cumulative for the entire map, not reset when a new round starts.
    recycle_total = level.zombie_total_subtract;
    recycle_credits = 0;
    setdvar("zm_coopRoundStatus", "tracking");
    level.zombie_vars["zombie_spawn_delay"] = spawn_delay(round);
    while (true)
    {
        wait 0.1;
        if (!startup_current(epoch, map) || !owns_callbacks() || level.round_number != round ||
            !isdefined(level.players) || !isarray(level.players) || custom_round_controller() ||
            !valid_counters(level.zombie_total, level.zombie_respawns))
            return;
        if (!isint(level.zombie_total_subtract) || level.zombie_total_subtract < recycle_total)
        {
            setdvar("zm_coopRoundStatus", "custom_round_budget_preserved");
            return;
        }
        recycle_delta = level.zombie_total_subtract - recycle_total;
        current_pending = level.zombie_total - level.zombie_respawns;
        spawns = accounted_spawn_delta(pending, current_pending, recycle_delta);
        next_credits = recycle_credit_balance(recycle_credits, recycle_delta, spawns, current_pending);
        if (!isdefined(spawns) || !isdefined(next_credits) || next_credits > current_pending)
        {
            setdvar("zm_coopRoundStatus", "custom_round_budget_preserved");
            return;
        }
        // Consume ordinary quota first and retain possibly queued recycling.
        // Stock does not expose the ordering of individual recycle/spawn events.
        delta = spawns - (recycle_credits + recycle_delta - next_credits);
        if (delta > 2147483647 - consumed)
        {
            setdvar("zm_coopRoundStatus", "counter_range_preserved");
            return;
        }
        consumed += delta;
        pending = current_pending;
        recycle_credits = next_credits;
        recycle_total = level.zombie_total_subtract;
        current_count = getplayers().size;
        // No empty-server round advancement. Wait for stable authoritative membership.
        if (current_count < 1 || current_count > level.eroded_coop_limit ||
            current_count != level.players.size || current_count == count)
            continue;
        target = zm::get_zombie_count_for_round(round, current_count);
        if (!isint(target) || target < 0)
            return;
        // Both replacement systems survive a reduced target, including when
        // ordinary quota has clamped to zero. Do not reset the stock counters.
        replacements = level.zombie_respawns;
        if (recycle_credits > 2147483647 - replacements)
        {
            setdvar("zm_coopRoundStatus", "counter_range_preserved");
            return;
        }
        next_total = remaining_budget(target, consumed, replacements + recycle_credits);
        if (!isdefined(next_total))
        {
            setdvar("zm_coopRoundStatus", "counter_range_preserved");
            return;
        }
        pending = next_total - level.zombie_respawns;
        level.zombie_total = next_total;
        level.zombie_vars["zombie_spawn_delay"] = spawn_delay(round);
        count = current_count;
    }
}
