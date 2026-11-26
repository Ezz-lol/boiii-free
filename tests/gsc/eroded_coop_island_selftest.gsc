// PRIVATE TEST ONLY: exercise authenticated Island helper references on private arrays.
// No static Island module import: custom scripts can load before its native anchor.
#namespace eroded_coop_island_selftest;

function autoexec main()
{
    if (getdvarstring("mapname") != "zm_island") return;
    setdvar("zm_coopIslandPoolTestStatus", "waiting");
    setdvar("zm_coopIslandPoolTestFailures", -1);
    setdvar("zm_coopIslandPoolTestEpoch", 0);
    level thread run();
}

function api_current(api, epoch, capacity)
{
    return isdefined(api) && isdefined(level.eroded_island_trials_pool_api) &&
        level.eroded_island_trials_pool_api == api &&
        isdefined(api.revision) && isint(api.revision) && api.revision == 3 &&
        isdefined(api.map) && api.map == "zm_island" &&
        isdefined(api.epoch) && isint(api.epoch) && api.epoch == epoch &&
        isdefined(api.capacity) && isint(api.capacity) && api.capacity == capacity &&
        isdefined(api.make_pool) && isdefined(api.take_task) && isdefined(api.return_task) &&
        isdefined(level.eroded_island_trials_revision) && level.eroded_island_trials_revision == 3 &&
        isdefined(level.eroded_island_trials_epoch) && level.eroded_island_trials_epoch == epoch &&
        isdefined(level.eroded_island_trials_capacity) && level.eroded_island_trials_capacity == capacity &&
        isdefined(level.eroded_island_trials_ready) && level.eroded_island_trials_ready == true &&
        isint(epoch) && epoch > 0 && isint(capacity) && capacity >= 5 && capacity <= 8 &&
        getdvarint("zm_coopStartupEpoch") == epoch && getdvarint("zm_coopSchemaSlots") == capacity &&
        getdvarint("zm_coopSchemaServerRevision") == 3 && getdvarstring("mapname") == "zm_island";
}

function check(value)
{
    if (value) return 0;
    return 1;
}

function templates(tier)
{
    counts = array(1, 1, 5, 5, 5);
    if (tier == 2) counts = array(1, 1, 15, 10, 20, 20);
    else if (tier == 3) counts = array(8, 3, 1, 30, 5);
    result = array();
    for (index = 0; index < counts.size; index++)
    {
        task = spawnstruct();
        task.n_index = tier;
        task.str_info = "private_pool_fixture";
        task.n_count = counts[index];
        task.str_notify = "private_pool_fixture_" + tier + "_" + index;
        task.func_think = undefined;
        result[index] = task;
    }
    return result;
}

function unique_records(records)
{
    if (!isarray(records)) return false;
    keys = getarraykeys(records);
    for (left = 0; left < keys.size; left++)
    {
        if (!isdefined(records[keys[left]])) return false;
        for (right = left + 1; right < keys.size; right++)
            if (records[keys[left]] == records[keys[right]]) return false;
    }
    return true;
}

function template_member(records, task)
{
    keys = getarraykeys(records);
    for (index = 0; index < keys.size; index++)
        if (records[keys[index]] == task) return true;
    return false;
}

function exercise_pool(api, tier, capacity)
{
    failures = 0;
    catalog = templates(tier);
    pool = [[api.make_pool]](catalog, tier, capacity);
    if (!isdefined(pool)) return 1;
    owners = array();
    assigned = array();
    foreign = spawnstruct();
    for (slot = 0; slot < capacity; slot++)
    {
        owners[slot] = spawnstruct();
        task = [[api.take_task]](pool, slot, owners[slot], 0);
        failures += check(isdefined(task));
        assigned[slot] = task;
        if (slot < 4) failures += check(template_member(catalog, task));
        failures += check([[api.take_task]](pool, slot, owners[slot], 0) == task);
        failures += check(!isdefined([[api.take_task]](pool, slot, foreign, 0)));
        failures += check(pool.allocated <= 8);
    }
    failures += check(unique_records(assigned));
    failures += check(assigned.size == capacity && getarraykeys(assigned).size == capacity);
    failures += check(pool.available.size + capacity == pool.allocated);
    allocated = pool.allocated;
    for (cycle = 0; cycle < 3; cycle++)
    {
        for (slot = 0; slot < capacity; slot++)
        {
            failures += check(![[api.return_task]](pool, slot, foreign));
            failures += check([[api.return_task]](pool, slot, owners[slot]));
            failures += check(![[api.return_task]](pool, slot, owners[slot]));
        }
        failures += check(pool.available.size == allocated && getarraykeys(pool.available).size == allocated);
        failures += check(unique_records(pool.available));
        for (slot = 0; slot < capacity; slot++)
        {
            owners[slot] = spawnstruct();
            assigned[slot] = [[api.take_task]](pool, slot, owners[slot], 0);
            failures += check(isdefined(assigned[slot]));
        }
        failures += check(unique_records(assigned) && pool.allocated == allocated);
    }
    return failures;
}

function exercise_transition(api, tier)
{
    failures = 0;
    pool = [[api.make_pool]](templates(tier), tier, 8);
    if (!isdefined(pool)) return 1;
    owners = array();
    records = array();
    for (slot = 0; slot < 8; slot++)
    {
        owners[slot] = spawnstruct();
        records[slot] = [[api.take_task]](pool, slot, owners[slot], 0);
        failures += check(isdefined(records[slot]));
    }
    failures += check(unique_records(records) && pool.allocated == 8);
    for (slot = 4; slot < 8; slot++)
        failures += check([[api.return_task]](pool, slot, owners[slot]));
    failures += check(pool.available.size == 4 && getarraykeys(pool.available).size == 4);
    for (slot = 0; slot < 4; slot++)
        failures += check(pool.assigned[slot] == records[slot]);
    for (slot = 4; slot < 8; slot++)
    {
        old_owner = owners[slot];
        owners[slot] = spawnstruct();
        records[slot] = [[api.take_task]](pool, slot, owners[slot], 0);
        failures += check(isdefined(records[slot]));
        failures += check(![[api.return_task]](pool, slot, old_owner));
        failures += check(pool.assigned[slot] == records[slot]);
    }
    failures += check(unique_records(records) && pool.allocated == 8 && pool.available.size == 0);
    return failures;
}

function run()
{
    level endon(#"end_game");
    level endon(#"intermission");
    api = undefined;
    epoch = getdvarint("zm_coopStartupEpoch");
    capacity = getdvarint("zm_coopSchemaSlots");
    for (attempt = 0; attempt < 600; attempt++)
    {
        if (getdvarstring("mapname") != "zm_island" || getdvarint("zm_coopStartupEpoch") != epoch) return;
        api = level.eroded_island_trials_pool_api;
        if (api_current(api, epoch, capacity)) break;
        wait 0.05;
    }
    if (!api_current(api, epoch, capacity))
    {
        setdvar("zm_coopIslandPoolTestStatus", "timeout");
        setdvar("zm_coopIslandPoolTestFailures", 1);
        return;
    }
    setdvar("zm_coopIslandPoolTestStatus", "running");
    // No yield after snapshot validation: all owners/arrays below are private.
    failures = 0;
    for (test_capacity = 1; test_capacity <= 8; test_capacity++)
        for (tier = 1; tier <= 3; tier++)
            failures += exercise_pool(api, tier, test_capacity);
    for (tier = 1; tier <= 3; tier++) failures += exercise_transition(api, tier);
    if (!api_current(api, epoch, capacity)) return;
    setdvar("zm_coopIslandPoolTestFailures", failures);
    setdvar("zm_coopIslandPoolTestEpoch", epoch);
    if (failures == 0) setdvar("zm_coopIslandPoolTestStatus", "passed");
    else setdvar("zm_coopIslandPoolTestStatus", "failed");
}
