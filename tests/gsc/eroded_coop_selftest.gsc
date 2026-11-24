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
    failures += check(eroded_coop::extended_spawn_delay(1, 8) == 0.335);
    failures += check(eroded_coop::extended_spawn_delay(1, 0) == 0.67);
    failures += check(eroded_coop::extended_spawn_delay(1, 9) == 0.67);
    failures += check(eroded_coop::extended_spawn_delay(60, 8) >= 0.1);
    failures += check(eroded_coop::extended_spawn_delay(60, 8) == eroded_coop::extended_spawn_delay(10000, 8));
    failures += check(eroded_coop::startup_current(epoch, map));
    failures += check(!eroded_coop::startup_current(epoch + 1, map));
    failures += check(!eroded_coop::startup_current(epoch, "invalid_map"));

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
