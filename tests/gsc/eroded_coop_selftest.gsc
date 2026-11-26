// Isolated canary fixture only. Do not install in production custom_scripts.
#using custom_scripts/zm/eroded_coop;
#using scripts/shared/ai/zombie_utility;
#using scripts/zm/_zm;
#namespace eroded_coop_selftest;

function autoexec main()
{
    setdvar("zm_coopSelfTestStatus", "waiting");
    setdvar("zm_coopSelfTestFailures", 0);
    level thread run();
}

function check(condition)
{
    if (condition)
        return 0;
    return 1;
}

function run()
{
    level endon(#"intermission");
    level endon(#"end_game");
    epoch = getdvarint("zm_coopStartupEpoch");
    map = getdvarstring("mapname");
    failures = 0;
    // Map/roster helpers do not synthesize players: runtime uses real human/bot
    // entities. These pure cases cover occupancy and extra-slot pack boundaries.
    maps = array("zm_zod", "zm_factory", "zm_castle", "zm_island", "zm_stalingrad", "zm_genesis",
        "zm_prototype", "zm_asylum", "zm_sumpf", "zm_theater", "zm_cosmodrome", "zm_temple", "zm_moon", "zm_tomb");
    for (map_index = 0; map_index < maps.size; map_index++)
        failures += check(eroded_coop::official_map(maps[map_index]));
    failures += check(!eroded_coop::official_map("zm_custom"));
    failures += check(!eroded_coop::official_map("mp_nuketown"));
    failures += check(eroded_coop::least_used_character(array(0, 0, 0, 0)) == 0);
    failures += check(eroded_coop::least_used_character(array(1, 0, 0, 0)) == 1);
    failures += check(eroded_coop::least_used_character(array(1, 1, 0, 0)) == 2);
    failures += check(eroded_coop::least_used_character(array(1, 1, 1, 0)) == 3);
    failures += check(eroded_coop::least_used_character(array(1, 1, 1, 1)) == 0);
    failures += check(eroded_coop::least_used_character(array(2, 1, 1, 1)) == 1);
    failures += check(eroded_coop::least_used_character(array(2, 2, 1, 1)) == 2);
    failures += check(eroded_coop::least_used_character(array(2, 2, 2, 1)) == 3);
    failures += check(eroded_coop::least_used_character(array(2, 0, 2, 2)) == 1);
    occupancy = array(0, 0, 0, 0);
    for (member = 0; member < 8; member++)
    {
        choice = eroded_coop::least_used_character(occupancy);
        failures += check(choice >= 0 && choice < 4);
        occupancy[choice]++;
    }
    for (character = 0; character < 4; character++)
    {
        failures += check(occupancy[character] == 2);
        occupancy[character]--; // Four actual departures, not downed players.
    }
    for (member = 0; member < 4; member++)
    {
        choice = eroded_coop::least_used_character(occupancy);
        occupancy[choice]++;
    }
    for (character = 0; character < 4; character++)
        failures += check(occupancy[character] == 2);
    failures += check(!isdefined(eroded_coop::least_used_character(undefined)));
    failures += check(!isdefined(eroded_coop::least_used_character(array(1, 2, 3))));
    failures += check(!isdefined(eroded_coop::least_used_character(array(-1, 0, 0, 0))));
    failures += check(!isdefined(eroded_coop::least_used_character(array(0, 0.5, 0, 0))));
    null_pack = array("weapon_null", "weapon_null", "weapon_null", "weapon_null", "weapon_null");
    for (limit = 5; limit <= 8; limit++)
    {
        for (slot = 0; slot <= 8; slot++)
            failures += check(eroded_coop::gum_pack_needs_fallback(null_pack, slot, limit) == (slot >= 4 && slot < limit));
    }
    failures += check(eroded_coop::gum_pack_needs_fallback(array(), 7, 8));
    failures += check(!eroded_coop::gum_pack_needs_fallback(null_pack, 4, 4));
    failures += check(!eroded_coop::gum_pack_needs_fallback(undefined, 4, 8));
    failures += check(!eroded_coop::gum_pack_needs_fallback(array("weapon_null"), 4, 8));
    failures += check(!eroded_coop::gum_pack_needs_fallback(array("real", "weapon_null", "weapon_null", "weapon_null", "weapon_null"), 4, 8));
    saved_round_spawn = level.round_spawn_func;
    saved_round_think = level.round_think_func;
    level.round_think_func = &zm::round_think;
    failures += check(eroded_coop::count_callback_owned(undefined));
    failures += check(eroded_coop::count_callback_owned(&zombie_utility::default_max_zombie_func));
    failures += check(!eroded_coop::count_callback_owned(&custom_count_callback));
    saved_count_callback = level.max_zombie_func;
    saved_count_calls = level.eroded_coop_test_count_calls;
    level.eroded_coop_test_count_calls = 0;
    level.max_zombie_func = &custom_count_callback;
    failures += check(eroded_coop::round_controller_reason() == "custom_count_callback_preserved");
    failures += check(!isdefined(eroded_coop::normal_round_target(1, 8)));
    failures += check(level.eroded_coop_test_count_calls == 0);
    level.max_zombie_func = undefined;
    failures += check(eroded_coop::count_callback_owned(level.max_zombie_func));
    failures += check(!isdefined(eroded_coop::normal_round_target(1, 0)));
    failures += check(!isdefined(eroded_coop::normal_round_target(0, 8)));
    level.max_zombie_func = saved_count_callback;
    level.eroded_coop_test_count_calls = saved_count_calls;
    saved_setup = level.zombie_total_set_func;
    saved_known_setup = level.eroded_coop_known_total_setup;
    saved_custom_spawn = level.fn_custom_round_ai_spawn;
    saved_known_spawn = level.eroded_coop_known_custom_spawn;
    saved_setup_calls = level.eroded_coop_test_setup_calls;
    level.eroded_coop_test_setup_calls = 0;
    level.round_spawn_func = &zm::round_spawning;
    level.zombie_total_set_func = &custom_total_setup;
    level.eroded_coop_known_total_setup = &custom_total_setup;
    level.fn_custom_round_ai_spawn = undefined;
    failures += check(!eroded_coop::custom_round_controller());
    failures += check(level.eroded_coop_test_setup_calls == 0);
    level.zombie_total_set_func = &custom_bgb_callback;
    failures += check(eroded_coop::custom_round_controller());
    level.zombie_total_set_func = &custom_total_setup;
    level.fn_custom_round_ai_spawn = &custom_bgb_callback;
    level.eroded_coop_known_custom_spawn = undefined;
    failures += check(eroded_coop::custom_round_controller());
    level.eroded_coop_known_custom_spawn = &custom_bgb_callback;
    failures += check(!eroded_coop::custom_round_controller());
    level.round_spawn_func = &custom_bgb_callback;
    failures += check(eroded_coop::custom_round_controller());
    failures += check(level.eroded_coop_test_setup_calls == 0);
    // No yield or real game-thread callback invocation while test globals differ.
    level.round_spawn_func = saved_round_spawn;
    level.zombie_total_set_func = saved_setup;
    level.eroded_coop_known_total_setup = saved_known_setup;
    level.fn_custom_round_ai_spawn = saved_custom_spawn;
    level.eroded_coop_known_custom_spawn = saved_known_spawn;
    level.eroded_coop_test_setup_calls = saved_setup_calls;
    level.round_think_func = saved_round_think;
    failures += test_round_thinker_ownership();
    failures += test_captured_round_event();
    failures += test_seed_callback_owners();
    failures += check(eroded_coop::valid_counters(100, 5));
    failures += check(!eroded_coop::valid_counters(1, 2));
    failures += check(!eroded_coop::valid_counters(-1, 0));
    failures += check(!eroded_coop::valid_counters(1.5, 0));
    failures += check(!eroded_coop::valid_counters("100", 0));
    failures += check(!eroded_coop::valid_counters(undefined, 0));
    failures += check(!eroded_coop::valid_counters(level, 0));
    failures += check(eroded_coop::remaining_budget(100, 80, 0) == 20);
    failures += check(eroded_coop::remaining_budget(50, 80, 0) == 0);
    failures += check(eroded_coop::remaining_budget(100, 80, 5) == 25);
    failures += check(eroded_coop::remaining_budget(0, 0, 2147483647) == 2147483647);
    failures += check(!isdefined(eroded_coop::remaining_budget(2147483647, 0, 1)));
    failures += check(!isdefined(eroded_coop::remaining_budget(100, -1, 0)));
    failures += check(!isdefined(eroded_coop::remaining_budget(100, 0, -1)));
    failures += check(!isdefined(eroded_coop::remaining_budget(100.5, 0, 0)));
    // A stock recycle is accounted even if a spawn hides the addition between polls.
    failures += check(eroded_coop::accounted_spawn_delta(20, 25, 5) == 0);
    failures += check(eroded_coop::accounted_spawn_delta(20, 20, 5) == 5);
    failures += check(eroded_coop::accounted_spawn_delta(23, 18, 2) == 7);
    failures += check(!isdefined(eroded_coop::accounted_spawn_delta(20, 26, 5)));
    failures += check(!isdefined(eroded_coop::accounted_spawn_delta(2147483647, 1, 1)));
    failures += check(!isdefined(eroded_coop::accounted_spawn_delta(1, 1, -1)));
    failures += check(!isdefined(eroded_coop::accounted_spawn_delta(1.5, 1, 0)));
    failures += check(eroded_coop::recycle_credit_balance(0, 5, 0, 25) == 5);
    failures += check(eroded_coop::recycle_credit_balance(5, 0, 2, 3) == 3);
    failures += check(eroded_coop::recycle_credit_balance(3, 2, 7, 18) == 5);
    failures += check(eroded_coop::recycle_credit_balance(0, 5, 5, 0) == 0);
    failures += check(eroded_coop::recycle_credit_balance(0, 10, 10, 20) == 10);
    failures += check(!isdefined(eroded_coop::recycle_credit_balance(2147483647, 1, 1, 1)));
    failures += check(!isdefined(eroded_coop::recycle_credit_balance(0, 0, undefined, 0)));
    failures += check(!isdefined(eroded_coop::recycle_credit_balance(0, -1, 0, 0)));
    failures += check(!isdefined(eroded_coop::recycle_credit_balance(1, 0, 0, 0)));
    failures += check(eroded_coop::remaining_budget(50, 80, 5) == 5);
    failures += check(eroded_coop::remaining_budget(100, 80, 3) == 23);
    failures += check(eroded_coop::extended_spawn_delay(1, 8) == 0.335);
    failures += check(eroded_coop::extended_spawn_delay(1, 0) == 0.67);
    failures += check(eroded_coop::extended_spawn_delay(1, 9) == 0.67);
    failures += check(eroded_coop::extended_spawn_delay(60, 8) >= 0.1);
    failures += check(eroded_coop::extended_spawn_delay(60, 8) == eroded_coop::extended_spawn_delay(10000, 8));
    failures += check(eroded_coop::startup_current(epoch, map));
    failures += check(!eroded_coop::startup_current(epoch + 1, map));
    failures += check(!eroded_coop::startup_current(epoch, "invalid_map"));

    for (count = 1; count <= 8; count++)
    {
        limits = eroded_coop::bgb_movement_limits(count);
        expected_min = 3;
        expected_max = 6;
        if (count <= 2)
            expected_min = 1;
        if (count <= 3)
            expected_max = count + 2;
        failures += check(isarray(limits) && limits.size == 2);
        failures += check(limits[0] == expected_min && limits[1] == expected_max);
    }
    failures += check(!isdefined(eroded_coop::bgb_movement_limits(0)));
    failures += check(!isdefined(eroded_coop::bgb_movement_limits(9)));
    failures += check(!isdefined(eroded_coop::bgb_movement_limits(-1)));
    failures += check(!isdefined(eroded_coop::bgb_movement_limits(1.5)));
    failures += check(!isdefined(eroded_coop::bgb_movement_limits("4")));
    failures += check(!isdefined(eroded_coop::bgb_movement_limits(undefined)));
    failures += check(!isdefined(eroded_coop::bgb_movement_limits(level)));
    failures += check(!isdefined(eroded_coop::bgb_movement_limits_for_players(array())));
    failures += check(!isdefined(eroded_coop::bgb_movement_limits_for_players(array(0, 0, 0, 0, 0, 0, 0, 0, 0))));
    failures += check(!isdefined(eroded_coop::bgb_movement_limits_for_players(undefined)));
    failures += check(!isdefined(eroded_coop::bgb_movement_limits_for_players(1.5)));
    failures += check(!isdefined(eroded_coop::bgb_movement_limits_for_players(level)));
    failures += check(!isdefined(eroded_coop::bgb_movement_limits_for_players("players")));

    started = gettime();
    while (getdvarstring("zm_coopStatus") != "experimental_callbacks_installed" ||
        getdvarint("zm_coopReadyEpoch") != epoch)
    {
        if (gettime() - started >= 35000 || !eroded_coop::startup_current(epoch, map))
        {
            setdvar("zm_coopSelfTestStatus", "failed");
            setdvar("zm_coopSelfTestFailures", failures + 1);
            return;
        }
        wait 0.05;
    }
    // Test-only globals are restored before any yield. No production payload
    // supplies fake players; this fixture runs only on the isolated empty canary.
    saved_bgb = level.bgb_machine_movement_frequency_override_func;
    saved_min = level.bgb_machine_min_uses_before_move;
    saved_max = level.bgb_machine_max_uses_before_move;
    saved_players = level.players;
    saved_limit = level.eroded_coop_limit;
    saved_epoch = level.eroded_coop_started_epoch;
    saved_map = level.eroded_coop_started_map;
    level.players = array(0);
    level.bgb_machine_movement_frequency_override_func = &custom_bgb_callback;
    level.bgb_machine_min_uses_before_move = 73;
    level.bgb_machine_max_uses_before_move = 79;
    failures += check(!eroded_coop::install_bgb_movement_hook());
    failures += check(level.bgb_machine_movement_frequency_override_func == &custom_bgb_callback);
    eroded_coop::bgb_machine_movement_frequency();
    failures += check(level.bgb_machine_min_uses_before_move == 73 && level.bgb_machine_max_uses_before_move == 79);
    level.bgb_machine_movement_frequency_override_func = undefined;
    failures += check(eroded_coop::install_bgb_movement_hook());
    failures += check(eroded_coop::install_bgb_movement_hook());
    failures += check(level.bgb_machine_movement_frequency_override_func == &eroded_coop::bgb_machine_movement_frequency);
    fake_players = array();
    level.players = fake_players;
    eroded_coop::bgb_machine_movement_frequency();
    failures += check(level.bgb_machine_min_uses_before_move == 73 && level.bgb_machine_max_uses_before_move == 79);
    for (count = 1; count <= level.eroded_coop_limit; count++)
    {
        fake_players[fake_players.size] = 0;
        level.players = fake_players;
        eroded_coop::bgb_machine_movement_frequency();
        expected_min = 3;
        expected_max = 6;
        if (count <= 2)
            expected_min = 1;
        if (count <= 3)
            expected_max = count + 2;
        failures += check(level.bgb_machine_min_uses_before_move == expected_min);
        failures += check(level.bgb_machine_max_uses_before_move == expected_max);
    }
    fake_players[fake_players.size] = 0;
    level.players = fake_players;
    level.bgb_machine_min_uses_before_move = 73;
    level.bgb_machine_max_uses_before_move = 79;
    eroded_coop::bgb_machine_movement_frequency();
    failures += check(level.bgb_machine_min_uses_before_move == 73 && level.bgb_machine_max_uses_before_move == 79);
    level.players = array(0);
    level.eroded_coop_started_epoch = epoch + 1;
    level.bgb_machine_min_uses_before_move = 73;
    level.bgb_machine_max_uses_before_move = 79;
    failures += check(!eroded_coop::install_bgb_movement_hook());
    eroded_coop::bgb_machine_movement_frequency();
    failures += check(level.bgb_machine_min_uses_before_move == 73 && level.bgb_machine_max_uses_before_move == 79);
    level.eroded_coop_started_epoch = saved_epoch;
    level.eroded_coop_started_map = "invalid_map";
    failures += check(!eroded_coop::install_bgb_movement_hook());
    eroded_coop::bgb_machine_movement_frequency();
    failures += check(level.bgb_machine_min_uses_before_move == 73 && level.bgb_machine_max_uses_before_move == 79);
    level.eroded_coop_started_map = saved_map;
    level.eroded_coop_limit = 4;
    level.bgb_machine_movement_frequency_override_func = undefined;
    failures += check(!eroded_coop::install_bgb_movement_hook());
    failures += check(!isdefined(level.bgb_machine_movement_frequency_override_func));
    level.eroded_coop_limit = saved_limit;
    level.players = saved_players;
    level.bgb_machine_min_uses_before_move = saved_min;
    level.bgb_machine_max_uses_before_move = saved_max;
    level.bgb_machine_movement_frequency_override_func = saved_bgb;
    failures += check(eroded_coop::owns_callbacks());

    // Duplicate autoexec is intentionally exercised after readiness without waits
    // between the calls; it must not reset the heartbeat or start another worker.
    heartbeat = getdvarint("zm_coopHeartbeat");
    eroded_coop::main();
    eroded_coop::main();
    failures += check(getdvarint("zm_coopHeartbeat") == heartbeat);
    failures += check(getdvarint("zm_coopReadyEpoch") == epoch);
    failures += check(getdvarstring("zm_coopReadyMap") == map);
    failures += check(getdvarstring("zm_coopStatus") == "experimental_callbacks_installed");
    setdvar("zm_coopSelfTestFailures", failures);
    if (failures == 0)
        setdvar("zm_coopSelfTestStatus", "passed");
    else
        setdvar("zm_coopSelfTestStatus", "failed");
}

// No real roster/counter writes or yields. These use the same pure helpers as
// captured production events, including elapsed spawns/recycles while lists
// temporarily disagree, and a same-round replacement generation.
function test_captured_round_event()
{
    failures = 0;
    failures += check(eroded_coop::next_round_generation(undefined) == 1);
    failures += check(eroded_coop::next_round_generation(0) == 1);
    failures += check(eroded_coop::next_round_generation(7) == 8);
    failures += check(eroded_coop::next_round_generation(2147483646) == 2147483647);
    failures += check(!isdefined(eroded_coop::next_round_generation(2147483647)));
    failures += check(!isdefined(eroded_coop::next_round_generation(-1)));
    failures += check(!isdefined(eroded_coop::next_round_generation(1.5)));
    failures += check(!isdefined(eroded_coop::next_round_generation("8")));
    failures += check(eroded_coop::round_generation_current(8, 8, 3, 3));
    failures += check(!eroded_coop::round_generation_current(7, 8, 3, 3));
    failures += check(!eroded_coop::round_generation_current(8, 8, 3, 4));
    failures += check(!eroded_coop::round_generation_current(8, undefined, 3, 3));
    failures += check(!eroded_coop::round_generation_current(0, 0, 3, 3));
    failures += check(!eroded_coop::round_generation_current("8", 8, 3, 3));
    failures += check(eroded_coop::stable_round_membership(8, 8, 8));
    failures += check(eroded_coop::stable_round_membership(4, 4, 8));
    failures += check(!eroded_coop::stable_round_membership(4, 8, 8));
    failures += check(!eroded_coop::stable_round_membership(8, 4, 8));
    failures += check(!eroded_coop::stable_round_membership(0, 0, 8));
    failures += check(!eroded_coop::stable_round_membership(8, 8, 7));
    failures += check(!eroded_coop::stable_round_membership(4, undefined, 8));
    failures += check(!eroded_coop::stable_round_membership(4.5, 4.5, 8));
    seed = eroded_coop::make_round_seed(3, 8, 100, 0, 17, 100, 8);
    failures += check(isdefined(seed));
    if (!isdefined(seed))
        return failures + 1;
    failures += check(seed.round == 3 && seed.count == 8 && seed.target == 100);
    failures += check(seed.pending == 100 && seed.consumed == 0 && seed.recycle_total == 17 && seed.generation == 8);
    // A partial/late/unknown seed is never reconstructed using today's roster.
    failures += check(!isdefined(eroded_coop::make_round_seed(3, 8, 99, 0, 17, 100, 8)));
    failures += check(!isdefined(eroded_coop::make_round_seed(3, 8, 101, 1, 17, 100, 8)));
    failures += check(!isdefined(eroded_coop::make_round_seed(3, 8, 101, 0, 18, 100, 8)));
    failures += check(!isdefined(eroded_coop::make_round_seed(3, 0, 100, 0, 17, 100, 8)));
    failures += check(!isdefined(eroded_coop::make_round_seed(3, 9, 100, 0, 17, 100, 8)));
    failures += check(!isdefined(eroded_coop::make_round_seed(0, 8, 100, 0, 17, 100, 8)));
    failures += check(!isdefined(eroded_coop::make_round_seed(3, 8, 100, 0, -1, 100, 8)));
    failures += check(!isdefined(eroded_coop::make_round_seed(3, 8, 100, 0, 17, undefined, 8)));
    failures += check(!isdefined(eroded_coop::make_round_seed(3, 8, 100, 0, 17, 100, 0)));
    // Ten spawns and two stock recycles occur after the captured event while
    // actual=4/listed=8. Continue accounting without changing the frozen target.
    current_pending = 92;
    recycle_delta = 19 - seed.recycle_total;
    spawns = eroded_coop::accounted_spawn_delta(seed.pending, current_pending, recycle_delta);
    credits = eroded_coop::recycle_credit_balance(0, recycle_delta, spawns, current_pending);
    consumed = seed.consumed + spawns - (recycle_delta - credits);
    failures += check(spawns == 10 && credits == 2 && consumed == 10);
    failures += check(!eroded_coop::stable_round_membership(4, 8, 8));
    failures += check(seed.count == 8 && seed.target == 100 && seed.pending == 100);
    // Once both lists settle at four, retain spent quota and queued recycling.
    failures += check(eroded_coop::stable_round_membership(4, 4, 8));
    pending = eroded_coop::remaining_budget(50, consumed, credits);
    failures += check(pending == 42);
    spawns = eroded_coop::accounted_spawn_delta(pending, 36, 0);
    next_credits = eroded_coop::recycle_credit_balance(credits, 0, spawns, 36);
    consumed += spawns - (credits - next_credits);
    credits = next_credits;
    failures += check(consumed == 16 && credits == 2);
    failures += check(!eroded_coop::stable_round_membership(8, 4, 8));
    failures += check(eroded_coop::stable_round_membership(8, 8, 8));
    failures += check(eroded_coop::remaining_budget(100, consumed, credits) == 86);
    // A zero roster may pause resizing without losing the event or accounting.
    failures += check(!eroded_coop::stable_round_membership(0, 0, 8));
    failures += check(seed.target == 100 && seed.generation == 8);
    failures += check(!eroded_coop::round_generation_current(seed.generation, 9, seed.round, 3));
    replacement = eroded_coop::make_round_seed(3, 4, 50, 0, 19, 50, 9);
    failures += check(isdefined(replacement) && eroded_coop::round_generation_current(replacement.generation, 9, replacement.round, 3));
    return failures;
}

function custom_bgb_callback()
{
}

// Real production predicates, with all temporary owner fields restored before
// returning or yielding. No map-only import and no map-owned callback is run.
function test_round_thinker_ownership()
{
    failures = 0;
    failures += check(eroded_coop::round_thinker_owned(undefined, undefined));
    failures += check(eroded_coop::round_thinker_owned(&zm::round_think, undefined));
    failures += check(!eroded_coop::round_thinker_owned(&custom_round_thinker, undefined));
    failures += check(eroded_coop::round_thinker_owned(&custom_round_thinker, &custom_round_thinker));
    failures += check(!eroded_coop::round_thinker_owned(&custom_round_thinker, &custom_bgb_callback));
    saved_think = level.round_think_func;
    saved_known_think = level.eroded_coop_known_round_think;
    saved_count = level.max_zombie_func;
    saved_calls = level.eroded_coop_test_think_calls;
    level.eroded_coop_test_think_calls = 0;
    level.round_think_func = &custom_round_thinker;
    level.eroded_coop_known_round_think = undefined;
    level.max_zombie_func = &zombie_utility::default_max_zombie_func;
    failures += check(eroded_coop::round_controller_reason() == "custom_round_thinker_preserved");
    failures += check(!isdefined(eroded_coop::normal_round_target(3, 8)));
    failures += check(level.eroded_coop_test_think_calls == 0);
    level.round_think_func = saved_think;
    level.eroded_coop_known_round_think = saved_known_think;
    level.max_zombie_func = saved_count;
    level.eroded_coop_test_think_calls = saved_calls;

    failures += check(eroded_coop::moon_round_budget_owned(true, false, undefined, 1, undefined));
    failures += check(eroded_coop::moon_round_budget_owned(true, false, false, 3, undefined));
    failures += check(!eroded_coop::moon_round_budget_owned(undefined, false, false, 3, 2));
    failures += check(!eroded_coop::moon_round_budget_owned(false, false, false, 3, 2));
    failures += check(!eroded_coop::moon_round_budget_owned(true, undefined, false, 3, 2));
    failures += check(!eroded_coop::moon_round_budget_owned(true, true, false, 3, 2));
    failures += check(!eroded_coop::moon_round_budget_owned(true, false, true, 3, undefined));
    failures += check(!eroded_coop::moon_round_budget_owned(true, false, true, 3, 3));
    failures += check(!eroded_coop::moon_round_budget_owned(true, false, true, 2, 3));
    failures += check(!eroded_coop::moon_round_budget_owned(true, false, true, 0, 3));
    failures += check(!eroded_coop::moon_round_budget_owned(true, false, true, 3, 0));
    failures += check(eroded_coop::moon_round_budget_owned(true, false, true, 4, 3));
    restored = array(40, 100, 140);
    for (i = 0; i < restored.size; i++)
    {
        // Both restore directions, including a coincidentally equal target,
        // remain unchanged even after stock cleared teleporter_used.
        seed = eroded_coop::make_round_seed(3, 8, 100, 0, 17, 100, 9);
        pending = restored[i];
        consumed = seed.consumed;
        if (eroded_coop::moon_round_budget_owned(true, false, true, 3, 3))
            consumed += eroded_coop::accounted_spawn_delta(seed.pending, pending, 0);
        failures += check(pending == restored[i] && consumed == 0);
    }
    // A normal worker must stop when an in-flight Moon -> NML transition takes
    // ownership, before reinterpreting a map-cleared total as spawned zombies.
    consumed = 0;
    failures += check(eroded_coop::moon_round_budget_owned(true, false, true, 4, 3));
    if (eroded_coop::moon_round_budget_owned(false, true, true, 4, 4))
        consumed += eroded_coop::accounted_spawn_delta(100, 0, 0);
    failures += check(consumed == 0);
    // Fresh n+1 can resume ordinary 8 -> 4 -> 8 accounting; the separate
    // captured-event fixture exercises elapsed spawns/recycles and these quotas.
    failures += check(eroded_coop::moon_round_budget_owned(true, false, true, 5, 4));
    failures += check(eroded_coop::remaining_budget(50, 30, 2) == 22);
    failures += check(eroded_coop::remaining_budget(100, 30, 2) == 72);
    return failures;
}

function custom_round_thinker()
{
    level.eroded_coop_test_think_calls++;
}

function custom_total_setup()
{
    level.eroded_coop_test_setup_calls++;
}

function custom_count_callback(maximum, round)
{
    level.eroded_coop_test_count_calls++;
    return maximum + round;
}


// Actual production predicates: no map-only imports, no real owner changes.
function test_seed_callback_owners()
{
    failures = 0;
    known = &seed_callback_known;
    foreign = &seed_callback_foreign;
    wrapper = &eroded_coop::round_total_setup;
    failures += check(eroded_coop::total_setup_original_owned(undefined, undefined));
    failures += check(eroded_coop::total_setup_original_owned(undefined, known));
    failures += check(eroded_coop::total_setup_original_owned(known, known));
    failures += check(!eroded_coop::total_setup_original_owned(foreign, known));
    failures += check(!eroded_coop::total_setup_original_owned(foreign, undefined));
    failures += check(eroded_coop::total_setup_owner_owned(undefined, undefined, undefined, false, false));
    failures += check(eroded_coop::total_setup_owner_owned(known, known, undefined, false, false));
    failures += check(eroded_coop::total_setup_owner_owned(wrapper, undefined, undefined, true, true));
    failures += check(eroded_coop::total_setup_owner_owned(wrapper, known, known, true, true));
    failures += check(!eroded_coop::total_setup_owner_owned(wrapper, known, foreign, true, true));
    failures += check(!eroded_coop::total_setup_owner_owned(wrapper, known, known, true, false));
    failures += check(!eroded_coop::total_setup_owner_owned(known, known, known, true, true));
    failures += check(!eroded_coop::total_setup_owner_owned(foreign, known, known, true, true));
    saved = level.eroded_coop_seed_test_calls;
    level.eroded_coop_seed_test_calls = 0;
    eroded_coop::run_original_total_setup(undefined, known);
    eroded_coop::run_original_total_setup(foreign, known);
    failures += check(level.eroded_coop_seed_test_calls == 0);
    eroded_coop::run_original_total_setup(known, known);
    failures += check(level.eroded_coop_seed_test_calls == 1);
    level.eroded_coop_seed_test_calls = saved;
    return failures;
}

function seed_callback_known()
{
    level.eroded_coop_seed_test_calls++;
}

function seed_callback_foreign()
{
    level.eroded_coop_seed_test_calls += 100;
}
