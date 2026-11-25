// Experimental round controller contributed by Eroded Networks.
// Compiled separately for the retail BO3 1C VM; never replaces a retail script.
#using scripts/zm/_zm;
#using scripts/zm/_zm_game_module;
#using scripts/zm/_zm_utility;
#using scripts/shared/callbacks_shared;

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

function official_map(map)
{
    return map == "zm_zod" || map == "zm_factory" || map == "zm_castle" ||
        map == "zm_island" || map == "zm_stalingrad" || map == "zm_genesis" ||
        map == "zm_prototype" || map == "zm_asylum" || map == "zm_sumpf" ||
        map == "zm_theater" || map == "zm_cosmodrome" || map == "zm_temple" ||
        map == "zm_moon" || map == "zm_tomb";
}

function map_profile_current(epoch, map)
{
    return isdefined(level.eroded_coop_profile_map) && level.eroded_coop_profile_map == map &&
        isdefined(level.eroded_coop_profile_epoch) && level.eroded_coop_profile_epoch == epoch;
}

function diagnostic_event(event)
{
    if (isdefined(level.eroded_coop_last_event) && level.eroded_coop_last_event == event)
        return;
    level.eroded_coop_last_event = event;
    setdvar("zm_coopLastEvent", event);
    now = gettime();
    if (!isdefined(level.eroded_coop_last_print) || now - level.eroded_coop_last_print >= 2000)
    {
        println("[Zombies coop] phase=" + event + " players=" + getplayers().size);
        level.eroded_coop_last_print = now;
    }
}

function diagnostic_round_values(players, target, consumed, pending)
{
    setdvar("zm_coopPlayers", players);
    setdvar("zm_coopRoundTarget", target);
    setdvar("zm_coopRoundConsumed", consumed);
    setdvar("zm_coopRoundPending", pending);
}

function install(limit, epoch, map)
{
    level endon(#"intermission");
    level endon(#"end_game");
    started = gettime();
    while (!isdefined(level.player_too_many_players_check_func) || !isdefined(level.func_get_zombie_spawn_delay) ||
        (official_map(map) && !map_profile_current(epoch, map)))
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
    if (map_profile_current(epoch, map) && isdefined(level.eroded_coop_profile_delay))
        expected_delay = level.eroded_coop_profile_delay;
    if (level.player_too_many_players_check_func != &zm::player_too_many_players_check)
    {
        setdvar("zm_coopStatus", "custom_player_check_preserved");
        setdvar("zm_coopOwnerStatus", "custom_player_check_preserved");
        diagnostic_event("custom_player_check_preserved");
        return;
    }
    if (level.func_get_zombie_spawn_delay != expected_delay)
    {
        setdvar("zm_coopStatus", "custom_spawn_delay_preserved");
        setdvar("zm_coopOwnerStatus", "custom_spawn_delay_preserved");
        diagnostic_event("custom_spawn_delay_preserved");
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
    diagnostic_round_values(getplayers().size, 0, 0, 0);
    setdvar("zm_coopOwnerStatus", "known_stock_callbacks");
    install_bgb_movement_hook();
    install_character_hook();
    install_spawn_hook();
    if (official_map(map))
        setdvar("zm_coopMapAdapter", "official_profile");
    else
        setdvar("zm_coopMapAdapter", "inherited_stock_callbacks_custom_logic_preserved");
    diagnostic_event("callbacks_installed");
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
        setdvar("zm_coopPlayers", getplayers().size);
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
    count = getplayers().size;
    if (count != level.players.size)
        return 0.67; // Do not run a stock four-case callback on a transient roster.
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

// The four loaded crew archetypes are reused, not replaced with unloaded models.
// Character setup remains the map's own function; only its known stock slot
// selection and documented singleton ownership are adapted.
function least_used_character(counts)
{
    if (!isarray(counts) || counts.size != 4)
        return undefined;
    selected = 0;
    for (i = 0; i < 4; i++)
    {
        if (!isint(counts[i]) || counts[i] < 0 || counts[i] > 8)
            return undefined;
        if (counts[i] < counts[selected])
            selected = i;
    }
    return selected;
}

function connected_owner(owner)
{
    if (!isdefined(owner))
        return false;
    owners = getplayers();
    for (owner_index = 0; owner_index < owners.size; owner_index++)
    {
        player = owners[owner_index];
        if (player == owner)
            return true;
    }
    return false;
}

function install_character_hook()
{
    if (!isdefined(level.eroded_coop_profile_characters) || !isdefined(level.givecustomcharacters))
        return false;
    if (level.givecustomcharacters == &give_characters)
        return true;
    if (level.givecustomcharacters != level.eroded_coop_profile_characters)
    {
        setdvar("zm_coopOwnerStatus", "custom_characters_preserved");
        diagnostic_event("custom_characters_preserved");
        return false; // Unknown custom character/quest setup keeps ownership.
    }
    level.eroded_coop_character_setup = level.givecustomcharacters;
    level.givecustomcharacters = &give_characters;
    return true;
}

function give_characters()
{
    if (!isdefined(level.eroded_coop_character_setup))
        return;
    if (!bgb_context_current() || level.givecustomcharacters != &give_characters)
    {
        self [[level.eroded_coop_character_setup]]();
        return;
    }
    players = getplayers();
    if (players.size > 4 && !isdefined(self.characterindex))
    {
        counts = array(0, 0, 0, 0);
        for (roster_index = 0; roster_index < players.size; roster_index++)
        {
            player = players[roster_index];
            if (player != self && isdefined(player.characterindex) && isint(player.characterindex) &&
                player.characterindex >= 0 && player.characterindex < 4)
                counts[player.characterindex]++;
        }
        choice = least_used_character(counts);
        if (isdefined(choice))
            self.characterindex = choice;
    }
    // Stock character functions assign these known singleton quest/radio
    // owners. A duplicate crew member must not steal a connected owner's role.
    rich = undefined;
    radio = undefined;
    if (isdefined(level.rich_sq_player) && connected_owner(level.rich_sq_player))
        rich = level.rich_sq_player;
    if (isdefined(level.eroded_coop_profile_radio) && level.eroded_coop_profile_radio &&
        isdefined(level.sndradioa) && connected_owner(level.sndradioa))
        radio = level.sndradioa;
    self [[level.eroded_coop_character_setup]]();
    if (isdefined(rich) && connected_owner(rich) && isdefined(level.rich_sq_player) && level.rich_sq_player == self)
        level.rich_sq_player = rich;
    if (isdefined(radio) && connected_owner(radio) && isdefined(level.sndradioa) && level.sndradioa == self)
        level.sndradioa = radio;
}

function install_spawn_hook()
{
    if (isdefined(level.eroded_coop_spawn_hook_installed) && level.eroded_coop_spawn_hook_installed)
        return;
    level.eroded_coop_spawn_hook_installed = true;
    if (isdefined(level.player_intersection_tracker_override))
        level.eroded_coop_intersection_original = level.player_intersection_tracker_override;
    level.player_intersection_tracker_override = &intersection_override;
    callback::on_spawned(&player_spawned);
    callback::on_connect(&player_connected);
    connected = getplayers();
    for (connected_index = 0; connected_index < connected.size; connected_index++)
        connected[connected_index] thread repair_empty_gum_pack(level.eroded_coop_started_epoch, level.eroded_coop_started_map);
}

function player_connected()
{
    if (bgb_context_current())
        self thread repair_empty_gum_pack(level.eroded_coop_started_epoch, level.eroded_coop_started_map);
}

function gum_pack_needs_fallback(pack, slot, limit)
{
    if (!isint(slot) || !isint(limit) || limit < 5 || limit > 8 ||
        slot < 4 || slot >= 8 || slot >= limit || !isarray(pack))
        return false;
    if (pack.size != 0 && pack.size != 5)
        return false; // A map's deliberately different loadout is not a stock empty pack.
    for (gum_index = 0; gum_index < pack.size; gum_index++)
    {
        gum = pack[gum_index];
        if (isdefined(gum) && gum != "weapon_null")
            return false;
    }
    return true;
}

function repair_empty_gum_pack(epoch, map)
{
    self endon(#"disconnect");
    level endon(#"intermission");
    level endon(#"end_game");
    slot = self getentitynumber();
    if (slot < 4 || slot >= 8 || slot >= level.eroded_coop_limit)
        return;
    if (isdefined(self.eroded_coop_gum_checked_epoch) && self.eroded_coop_gum_checked_epoch == epoch)
        return;
    self.eroded_coop_gum_checked_epoch = epoch;
    for (attempt = 0; attempt < 200; attempt++)
    {
        if (!startup_current(epoch, map) || !owns_callbacks())
            return;
        if (isdefined(self.bgb_pack) && isdefined(level.bgb))
            break;
        wait 0.1;
    }
    if (!isdefined(self.bgb_pack) || !isarray(self.bgb_pack) || !isdefined(level.bgb))
        return; // Maps without GobbleGums retain their own rules.
    if (!gum_pack_needs_fallback(self.bgb_pack, slot, level.eroded_coop_limit))
        return; // Never replace a real or custom pack, human or bot.
    starter = array("zm_bgb_always_done_swiftly", "zm_bgb_arms_grace", "zm_bgb_coagulant",
        "zm_bgb_in_plain_sight", "zm_bgb_stock_option");
    for (starter_index = 0; starter_index < starter.size; starter_index++)
    {
        gum = starter[starter_index];
        if (!isdefined(level.bgb[gum]) || !isdefined(level.bgb[gum].consumable) || level.bgb[gum].consumable)
            return; // No synthetic consumable inventory or partial five-slot pack.
    }
    if (!startup_current(epoch, map) || !owns_callbacks())
        return;
    self.bgb_pack = starter;
    self.bgb_pack_randomized = array();
    setdvar("zm_coopGumStatus", "empty_extra_slot_classic_pack_repaired");
    diagnostic_event("classic_gum_pack_repaired");
}

function intersection_override(other)
{
    if (bgb_context_current() && getplayers().size > 4)
    {
        now = gettime();
        if ((isdefined(self.eroded_coop_spawn_grace) && now < self.eroded_coop_spawn_grace) ||
            (isdefined(other.eroded_coop_spawn_grace) && now < other.eroded_coop_spawn_grace))
            return true;
    }
    if (isdefined(level.eroded_coop_intersection_original))
        return self [[level.eroded_coop_intersection_original]](other);
    return false;
}

function position_clear(point, players, clearance)
{
    for (clear_index = 0; clear_index < players.size; clear_index++)
    {
        player = players[clear_index];
        if (player == self || !isdefined(player.sessionstate) || player.sessionstate != "playing")
            continue;
        if (abs(point[2] - player.origin[2]) < 60 && distance2dsquared(point, player.origin) < clearance * clearance)
            return false;
    }
    return true;
}

function player_spawned()
{
    if (!bgb_context_current() || getplayers().size <= 4 ||
        level.player_intersection_tracker_override != &intersection_override)
        return;
    if (!isdefined(self.eroded_coop_spawn_generation) || !isint(self.eroded_coop_spawn_generation) ||
        self.eroded_coop_spawn_generation >= 2147483647)
        self.eroded_coop_spawn_generation = 0;
    self.eroded_coop_spawn_generation++;
    self.eroded_coop_spawn_grace = gettime() + 2500;
    self thread separate_spawn(self.eroded_coop_spawn_generation,
        level.eroded_coop_started_epoch, level.eroded_coop_started_map);
}

function separate_spawn(generation, epoch, map)
{
    self endon(#"disconnect");
    self endon(#"death");
    level endon(#"intermission");
    level endon(#"end_game");
    wait 0.05;
    if (!startup_current(epoch, map) || !owns_callbacks() || !isalive(self) ||
        level.player_intersection_tracker_override != &intersection_override ||
        self.eroded_coop_spawn_generation != generation || self.sessionstate != "playing")
        return;
    players = getplayers();
    if (self position_clear(self.origin, players, 20))
    {
        self.eroded_coop_spawn_grace = 0;
        return;
    }
    // Navigation alone includes zombie-only/outside geometry. Require an actual
    // player volume plus floor/visibility/telefrag tests, or keep the short grace
    // and let players separate. Never invent coordinates for an unknown layout.
    if (!getentarray("player_volume", "script_noteworthy").size)
    {
        setdvar("zm_coopSpawnStatus", "no_player_volumes_preserved");
        return;
    }
    source = self.origin;
    query = positionquery_source_navigation(source, 44, 128, 32, 24);
    if (!isdefined(query) || !isdefined(query.data) || !isarray(query.data))
        return;
    best = undefined;
    best_distance = 16385;
    for (i = 0; i < query.data.size && i < 24; i++)
    {
        if (!isdefined(query.data[i].origin))
            continue;
        point = query.data[i].origin;
        distance = distance2dsquared(source, point);
        if (distance >= best_distance || distance > 16384 || abs(point[2] - source[2]) > 32)
            continue;
        if (!self position_clear(point, players, 44) || positionwouldtelefrag(point) ||
            !zm_utility::check_point_in_playable_area(point))
            continue;
        if (!bullettracepassed(source + (0, 0, 40), point + (0, 0, 40), false, self))
            continue;
        best = point;
        best_distance = distance;
    }
    if (isdefined(best) && startup_current(epoch, map) && owns_callbacks() &&
        level.player_intersection_tracker_override == &intersection_override &&
        self.eroded_coop_spawn_generation == generation && self position_clear(best, getplayers(), 44))
    {
        self setorigin(best);
        self.eroded_coop_spawn_grace = 0;
        setdvar("zm_coopSpawnStatus", "bounded_spawn_separated");
        diagnostic_event("bounded_spawn_separated");
    }
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
    return round_controller_reason() != "normal";
}

function round_controller_reason()
{
    // Dog/spider/chaos rounds temporarily replace the normal spawner. Their
    // counters and special-actor budgets are not normal zombie quotas.
    if (isdefined(level.round_spawn_func) && level.round_spawn_func != &zm::round_spawning)
        return "special_round_spawner_preserved";
    if (isdefined(level.zombie_total_set_func) &&
        (!isdefined(level.eroded_coop_known_total_setup) ||
        level.zombie_total_set_func != level.eroded_coop_known_total_setup))
        return "custom_total_setup_preserved";
    if (!isdefined(level.fn_custom_round_ai_spawn))
        return "normal";
    // Known stock mixed-AI callbacks keep running unchanged. Their quota
    // decrements are observed along with ordinary spawns, never replaced.
    if (!isdefined(level.eroded_coop_known_custom_spawn) ||
        level.fn_custom_round_ai_spawn != level.eroded_coop_known_custom_spawn)
        return "custom_mixed_ai_preserved";
    return "normal";
}

function track_round(epoch, map)
{
    level endon(#"intermission");
    level endon(#"end_game");
    level endon(#"end_of_round");
    level endon(#"restart_round");
    level endon(#"zombie_total_set");
    // Custom round controllers and special-round paths retain ownership.
    if (!startup_current(epoch, map) || !owns_callbacks())
        return;
    if (custom_round_controller())
    {
        reason = round_controller_reason();
        setdvar("zm_coopOwnerStatus", reason);
        setdvar("zm_coopRoundStatus", reason);
        diagnostic_event(reason);
        return;
    }
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
    setdvar("zm_coopOwnerStatus", "known_stock_callbacks");
    diagnostic_round_values(count, target, consumed, pending);
    diagnostic_event("normal_round_tracking");
    last_diagnostic = gettime();
    level.zombie_vars["zombie_spawn_delay"] = spawn_delay(round);
    while (true)
    {
        wait 0.1;
        if (!startup_current(epoch, map) || !owns_callbacks() || level.round_number != round ||
            !isdefined(level.players) || !isarray(level.players) ||
            !valid_counters(level.zombie_total, level.zombie_respawns))
            return;
        if (custom_round_controller())
        {
            reason = round_controller_reason();
            setdvar("zm_coopOwnerStatus", reason);
            setdvar("zm_coopRoundStatus", reason);
            diagnostic_event(reason);
            return;
        }
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
        if (gettime() - last_diagnostic >= 1000)
        {
            diagnostic_round_values(current_count, target, consumed, pending);
            last_diagnostic = gettime();
        }
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
        diagnostic_round_values(count, target, consumed, pending);
        diagnostic_event("normal_round_population_changed");
    }
}
