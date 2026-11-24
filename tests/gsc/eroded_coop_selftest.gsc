// Isolated canary fixture only. Do not install in production custom_scripts.
#using custom_scripts/zm/eroded_coop;
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

function custom_bgb_callback()
{
}
