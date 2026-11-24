// Experimental round controller contributed by Eroded Networks.
// Compiled separately for the retail BO3 1C VM; never replaces a retail script.
#using scripts/zm/_zm;
#using scripts/zm/_zm_game_module;

#namespace eroded_coop;

function autoexec main()
{
    setdvar("zm_coopStatus", "disabled");
    setdvar("zm_coopHeartbeat", 0);
    limit = getdvarint("zm_coopPlayerLimit");
    if (limit <= 4 || limit > 8)
        return;
    setdvar("zm_coopStatus", "initializing");
    level thread install(limit);
}

function install(limit)
{
    level endon(#"intermission");
    // Initial adapters only. Other maps need their own gameplay/character audit.
    map = getdvarstring("mapname");
    if (map != "zm_zod" && map != "zm_tomb")
    {
        setdvar("zm_coopStatus", "unsupported_map");
        println("[Zombies coop] Expanded gameplay is not validated for this map.");
        return;
    }

    deadline = gettime() + 30000;
    while (!isdefined(level.player_too_many_players_check_func) || !isdefined(level.func_get_zombie_spawn_delay))
    {
        if (gettime() >= deadline)
        {
            setdvar("zm_coopStatus", "initialization_timeout");
            return;
        }
        wait 0.05;
    }
    if (level.player_too_many_players_check_func != &zm::player_too_many_players_check ||
        level.func_get_zombie_spawn_delay != &zm::get_zombie_spawn_delay ||
        level.scr_zm_ui_gametype != "zclassic")
    {
        setdvar("zm_coopStatus", "custom_callbacks_preserved");
        return;
    }
    level.eroded_coop_limit = limit;
    level.player_too_many_players_check_func = &player_count_check;
    level.func_get_zombie_spawn_delay = &spawn_delay;
    level thread round_listener();
    level thread diagnostic_heartbeat();
    setdvar("zm_coopStatus", "experimental_callbacks_installed");
    println("[Zombies coop] Experimental stock-round callbacks installed; limit " + limit);
}

function diagnostic_heartbeat()
{
    level endon(#"intermission");
    level endon(#"end_game");
    heartbeat = 0;
    while (true)
    {
        setdvar("zm_coopHeartbeat", heartbeat);
        wait 1;
        heartbeat++;
    }
}

function player_count_check()
{
    if (getplayers().size > level.eroded_coop_limit)
    {
        zm_game_module::freeze_players(1);
        level notify(#"end_game");
    }
}

function spawn_delay(round)
{
    count = level.players.size;
    if (count <= 4)
        return zm::get_zombie_spawn_delay(round);
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
    return level.player_too_many_players_check_func == &player_count_check &&
        level.func_get_zombie_spawn_delay == &spawn_delay;
}

function round_listener()
{
    level endon(#"intermission");
    level endon(#"end_game");
    while (true)
    {
        level waittill(#"zombie_total_set");
        level thread track_round();
    }
}

function valid_counters(total, respawns)
{
    return isdefined(total) && isdefined(respawns) && total >= 0 && respawns >= 0 && respawns <= total;
}

function track_round()
{
    level endon(#"intermission");
    level endon(#"end_game");
    level endon(#"end_of_round");
    level endon(#"restart_round");
    level endon(#"zombie_total_set");
    // Custom round controllers and special-round paths retain ownership.
    if (!owns_callbacks() || isdefined(level.zombie_total_set_func) ||
        isdefined(level.fn_custom_round_ai_spawn))
        return;
    count = getplayers().size;
    if (count < 1 || count > level.eroded_coop_limit || count != level.players.size)
        return;
    round = level.round_number;
    target = zm::get_zombie_count_for_round(round, count);
    if (!isdefined(target) || target < 0 || !valid_counters(level.zombie_total, level.zombie_respawns))
        return;
    pending = level.zombie_total - level.zombie_respawns;
    if (pending > target)
        return;
    consumed = target - pending;
    level.zombie_vars["zombie_spawn_delay"] = spawn_delay(round);
    while (true)
    {
        wait 0.1;
        if (!owns_callbacks() || level.round_number != round ||
            isdefined(level.zombie_total_set_func) || isdefined(level.fn_custom_round_ai_spawn) ||
            !valid_counters(level.zombie_total, level.zombie_respawns))
            return;
        current_pending = level.zombie_total - level.zombie_respawns;
        if (current_pending > pending)
        {
            setdvar("zm_coopStatus", "custom_round_budget_preserved");
            return;
        }
        consumed += pending - current_pending;
        pending = current_pending;
        current_count = getplayers().size;
        // No empty-server round advancement. Wait for stable authoritative membership.
        if (current_count < 1 || current_count > level.eroded_coop_limit ||
            current_count != level.players.size || current_count == count)
            continue;
        target = zm::get_zombie_count_for_round(round, current_count);
        if (!isdefined(target) || target < 0)
            return;
        pending = target - consumed;
        if (pending < 0)
            pending = 0;
        // Replacement spawns are not normal round quota and must survive scaling.
        level.zombie_total = pending + level.zombie_respawns;
        level.zombie_vars["zombie_spawn_delay"] = spawn_delay(round);
        count = current_count;
    }
}
