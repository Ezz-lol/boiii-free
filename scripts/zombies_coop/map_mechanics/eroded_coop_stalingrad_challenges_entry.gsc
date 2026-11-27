// SPDX-License-Identifier: GPL-3.0-only
// Experimental challenge catalogs contributed by Eroded Networks.
// Server-only catalog allocation; paired clientfield layouts are unchanged.
// Native source/VM authentication is separate from these lifecycle predicates.
#using scripts/codescripts/struct;
#using scripts/zm/zm_stalingrad_challenges;
#namespace eroded_coop_stalingrad_challenges;

function stock_capacity(capacity)
{
    return isint(capacity) && capacity >= 1 && capacity <= 4;
}

function tuple_current(epoch, capacity)
{
    return isint(epoch) && epoch > 0 && isint(capacity) && capacity >= 5 && capacity <= 8 &&
        getdvarstring("mapname") == "zm_stalingrad" &&
        getdvarint("zm_coopStartupEpoch") == epoch && getdvarint("zm_coopAppliedLimit") == capacity;
}

function lifecycle_current(state)
{
    return isdefined(state) && isdefined(level.eroded_stalingrad_pool_state) &&
        level.eroded_stalingrad_pool_state == state && isdefined(state.map) && isstring(state.map) &&
        state.map == "zm_stalingrad" && isdefined(state.epoch) && isint(state.epoch) && state.epoch > 0 &&
        isdefined(state.capacity) && isint(state.capacity) && state.capacity >= 5 && state.capacity <= 8 &&
        getdvarstring("mapname") == state.map && getdvarint("zm_coopStartupEpoch") == state.epoch;
}

function identity_current(state)
{
    return lifecycle_current(state) && getdvarint("zm_coopAppliedLimit") == state.capacity;
}

function schema_current(state)
{
    return identity_current(state) && getdvarint("zm_coopSchemaSlots") == state.capacity &&
        getdvarint("zm_coopSchemaServerRevision") == 4 &&
        getdvarstring("zm_coopSchemaServerContract") == "c08cfc132da28936a532f91936afe6970730b8351fe3c64aff52bf48bce395f1" &&
        isdefined(level.eroded_coop_fields_revision) && level.eroded_coop_fields_revision == 4;
}

function stock_system_owned()
{
    if (!isdefined(level.system_funcs) || !isarray(level.system_funcs) ||
        !isdefined(level.system_funcs["zm_stalingrad_challenges"])) return false;
    stock = level.system_funcs["zm_stalingrad_challenges"];
    return isdefined(stock.prefunc) && stock.prefunc == &zm_stalingrad_challenges::__init__ &&
        isdefined(stock.postfunc) && stock.postfunc == &post_init &&
        isdefined(stock.ignore) && stock.ignore == false &&
        isdefined(stock.reqs) && isarray(stock.reqs) && stock.reqs.size == 0 &&
        isdefined(stock.predone) && stock.predone == true;
}

function stock_post_complete()
{
    if (!stock_system_owned()) return false;
    stock = level.system_funcs["zm_stalingrad_challenges"];
    return isdefined(stock.predone) && stock.predone == true &&
        isdefined(stock.postdone) && stock.postdone == true;
}

function callbacks_owned()
{
    return callback_count(#"hash_eaffea17", &connect) == 1 &&
        callback_count(#"hash_aebdd257", &disconnect) == 1 &&
        callback_count(#"hash_bc12b61f", &zm_stalingrad_challenges::on_player_spawned) == 1 &&
        callback_count(#"hash_eaffea17", &zm_stalingrad_challenges::on_player_connect) == 0 &&
        callback_count(#"hash_aebdd257", &zm_stalingrad_challenges::on_player_disconnect) == 0;
}

// Records never point back to state. The private table and the entity's marker
// prove ownership; primitive tuple fields keep old-lifecycle claims separate.
function record_tuple(state, record, slot)
{
    return isdefined(state) && isdefined(record) && isdefined(state.map) && isstring(state.map) &&
        state.map == "zm_stalingrad" && isdefined(state.epoch) && isint(state.epoch) && state.epoch > 0 &&
        isdefined(state.capacity) && isint(state.capacity) && state.capacity >= 5 && state.capacity <= 8 &&
        isint(slot) && slot >= 0 && slot < state.capacity &&
        isdefined(record.map) && isstring(record.map) && record.map == state.map &&
        isdefined(record.epoch) && isint(record.epoch) && record.epoch == state.epoch &&
        isdefined(record.capacity) && isint(record.capacity) && record.capacity == state.capacity &&
        isdefined(record.slot) && isint(record.slot) && record.slot == slot &&
        isdefined(record.serial) && isint(record.serial) && record.serial > 0 &&
        isdefined(record.returned) && (record.returned == false || record.returned == true);
}

function connection_owned(state, record, slot)
{
    return record_tuple(state, record, slot) && isdefined(state.connections) && isarray(state.connections) &&
        isdefined(state.connections[slot]) && state.connections[slot] == record;
}

function receiver_rostered(state, player, slot)
{
    if (!isdefined(player) || !isentity(player) || !isplayer(player) || !isint(slot) || slot < 0 ||
        slot >= state.capacity || (isdefined(player.player_disconnected) && player.player_disconnected != 0)) return false;
    actual = getplayers();
    if (!isarray(actual) || actual.size > state.capacity) return false;
    keys = getarraykeys(actual);
    for (i = 0; i < keys.size; i++)
        if (isint(keys[i]) && keys[i] >= 0 && keys[i] < state.capacity && actual[keys[i]] == player) return true;
    return false;
}

// Detach only references proved to belong to this captured private table.
// This can run for an old guardian without touching LEVEL, player fields,
// successor receipts, task objects or any mutable stock free pool.
function detach_owned_players(state)
{
    if (!isdefined(state) || !isdefined(state.capacity) || !isint(state.capacity) || state.capacity < 5 ||
        state.capacity > 8 || !isdefined(state.connections) || !isarray(state.connections)) return;
    for (slot = 0; slot < state.capacity; slot++)
    {
        if (!isdefined(state.connections[slot])) continue;
        record = state.connections[slot];
        if (!connection_owned(state, record, slot) || !isdefined(record.player)) continue;
        player = record.player;
        if (!isentity(player) || !isplayer(player) || !isdefined(player.eroded_stalingrad_pool_connection) ||
            player.eroded_stalingrad_pool_connection != record) continue;
        record.player = undefined;
        record.detached = true;
    }
    state.guard_active = false;
}

function withdraw_readiness(state, reason)
{
    if (!lifecycle_current(state))
    {
        detach_owned_players(state);
        return;
    }
    if (isdefined(state.declined))
    {
        detach_owned_players(state);
        return;
    }
    level.eroded_stalingrad_pool_ready = 0;
    level.eroded_stalingrad_pool_revision = 0;
    state.declined = reason;
    state.guard_active = false;
    setdvar("zm_coopStalingradPoolRevision", 0);
    setdvar("zm_coopStalingradPoolStatus", reason);
    if (getdvarint("zm_coopReadyEpoch") == state.epoch && getdvarstring("zm_coopReadyMap") == state.map)
    {
        setdvar("zm_coopReadyEpoch", 0);
        setdvar("zm_coopReadyMap", "");
        setdvar("zm_coopHeartbeat", 0);
    }
    detach_owned_players(state);
}

function preassignment_players(capacity)
{
    actual = getplayers();
    if (!isarray(actual) || actual.size > capacity) return false;
    keys = getarraykeys(actual);
    seen = array();
    for (i = 0; i < keys.size; i++)
    {
        if (!isint(keys[i]) || keys[i] < 0 || keys[i] >= capacity) return false;
        player = actual[keys[i]];
        if (!isdefined(player) || !isentity(player) || !isplayer(player) || isdefined(player._challenges)) return false;
        if (isdefined(player.player_disconnected) && player.player_disconnected != 0) return false;
        slot = player getentitynumber();
        if (!isint(slot) || slot < 0 || slot >= capacity || isdefined(seen[slot])) return false;
        seen[slot] = true;
    }
    return true;
}

// Called only from the authenticated stock POST boundary or its redirected
// connect callback. The original constructor gives the catalog and task fields
// their struct types. This draft has no portable CB arbitrary-type predicate.
function ensure_seed(state)
{
    if (!identity_current(state) || isdefined(state.declined) || !isdefined(state.post_started) ||
        state.post_started != true || isdefined(state.seeding)) return false;
    if (!stock_system_owned())
    {
        withdraw_readiness(state, "stock_post_owner_unrecognized");
        return false;
    }
    if (isdefined(state.seeded))
    {
        if (state.seeded != true) return false;
        current = observation(state);
        if (current.valid) return true;
        withdraw_readiness(state, "seeded_catalog_drift");
        return false;
    }
    if (!stock_system_owned() || !preassignment_players(state.capacity) || !isdefined(level._challenges))
    {
        withdraw_readiness(state, "unassigned_stock_catalog_required");
        return false;
    }
    state.seeding = true;
    pools = array(level._challenges.challenge_1, level._challenges.challenge_2, level._challenges.challenge_3);
    replacement = array();
    origins = array();
    for (tier = 1; tier <= 3; tier++)
    {
        if (!valid_pool(pools[tier - 1], tier))
        {
            withdraw_readiness(state, "stock_catalog_unrecognized");
            return false;
        }
        replacement[tier] = array();
        origins[tier] = array();
        for (index = 0; index < 6; index++)
        {
            replacement[tier][index] = pools[tier - 1][index];
            origins[tier][index] = index;
        }
        for (extra = 0; extra < state.capacity - 6; extra++)
        {
            index = clone_index(tier, extra);
            copy = clone_task(pools[tier - 1][index], tier, index);
            if (!isdefined(copy))
            {
                withdraw_readiness(state, "private_copy_failed");
                return false;
            }
            replacement[tier][6 + extra] = copy;
            origins[tier][6 + extra] = index;
        }
    }
    // Revalidate ordered stock task identities against replacement's independent
    // six-entry snapshot. GSC arrays cannot be compared with ==
    // or !=; a captured pool container would also not pin later replacements.
    // There is no wait/thread/notification before the three writebacks.
    if (!identity_current(state) || !stock_system_owned() || !preassignment_players(state.capacity) ||
        !isdefined(level._challenges))
    {
        withdraw_readiness(state, "precommit_context_changed");
        return false;
    }
    current_pools = array(level._challenges.challenge_1, level._challenges.challenge_2, level._challenges.challenge_3);
    for (tier = 1; tier <= 3; tier++)
    {
        if (!valid_pool(current_pools[tier - 1], tier) || !valid_pool(pools[tier - 1], tier))
        {
            withdraw_readiness(state, "precommit_catalog_changed");
            return false;
        }
        for (index = 0; index < 6; index++)
        {
            if (current_pools[tier - 1][index] != replacement[tier][index])
            {
                withdraw_readiness(state, "precommit_catalog_changed");
                return false;
            }
        }
        for (index = 0; index < replacement[tier].size; index++)
        {
            if (!valid_task(replacement[tier][index], tier, origins[tier][index]))
            {
                withdraw_readiness(state, "precommit_copy_changed");
                return false;
            }
            for (previous = 0; previous < index; previous++)
                if (replacement[tier][previous] == replacement[tier][index])
                {
                    withdraw_readiness(state, "aliased_catalog_object");
                    return false;
                }
        }
    }
    // The stable identity catalog MUST use separate arrays from the mutable
    // free pools. Only their immutable task objects are intentionally shared.
    catalogs = array();
    for (tier = 1; tier <= 3; tier++)
    {
        catalogs[tier] = array();
        for (index = 0; index < replacement[tier].size; index++)
            catalogs[tier][index] = replacement[tier][index];
    }
    if (!identity_current(state) || !stock_system_owned() || !preassignment_players(state.capacity)) return false;
    level._challenges.challenge_1 = replacement[1];
    level._challenges.challenge_2 = replacement[2];
    level._challenges.challenge_3 = replacement[3];
    state.catalogs = catalogs;
    state.origins = origins;
    state.catalog_size = replacement[1].size;
    state.seeded = true;
    state.seed_count = 1;
    state.seeding = undefined;
    setdvar("zm_coopStalingradPoolMap", state.map);
    setdvar("zm_coopStalingradPoolEpoch", state.epoch);
    setdvar("zm_coopStalingradPoolCapacity", state.capacity);
    setdvar("zm_coopStalingradPoolStatus", "seeded_before_assignment");
    setdvar("zm_coopStalingradPoolRevision", 1);
    return true;
}

function post_init()
{
    capacity = getdvarint("zm_coopAppliedLimit");
    // Stock1..4 references are never redirected by the native adapter. These
    // exports are internal expanded-session boundaries, not fallback APIs.
    if (isdefined(level.eroded_stalingrad_pool_state))
    {
        state = level.eroded_stalingrad_pool_state;
        if (lifecycle_current(state) && capacity != state.capacity)
            withdraw_readiness(state, "applied_limit_drift");
        return;
    }
    epoch = getdvarint("zm_coopStartupEpoch");
    if (!tuple_current(epoch, capacity)) return;
    state = spawnstruct();
    state.map = "zm_stalingrad";
    state.epoch = epoch;
    state.capacity = capacity;
    state.post_started = true;
    state.guard_active = false;
    state.connections = array();
    state.next_connection = 0;
    state.observation_sequence = 0;
    level.eroded_stalingrad_pool_state = state;
    level.eroded_stalingrad_pool_ready = 0;
    level.eroded_stalingrad_pool_revision = 0;
    if (!stock_system_owned())
    {
        withdraw_readiness(state, "stock_post_owner_unrecognized");
        return;
    }
    setdvar("zm_coopStalingradPoolRevision", 0);
    setdvar("zm_coopStalingradPoolStatus", "stock_post_in_progress");
    // This external call resolves the unchanged stock export. Only the three
    // authenticated original function-reference sites are redirected natively.
    zm_stalingrad_challenges::__main__();
    if (!identity_current(state) || !ensure_seed(state)) return;
    state.post_returned = true;
    state.guard_started = true;
    level thread guardian(state);
}

function connect()
{
    capacity = getdvarint("zm_coopAppliedLimit");
    if (!isdefined(level.eroded_stalingrad_pool_state)) return;
    state = level.eroded_stalingrad_pool_state;
    if (!lifecycle_current(state) || !isdefined(self) || !isentity(self) || !isplayer(self)) return;
    slot = self getentitynumber();
    // Late callbacks on an old or disconnected entity must not revoke the
    // successor's readiness. A new claim must be in the actual current roster.
    if (!receiver_rostered(state, self, slot)) return;
    if (isdefined(self.eroded_stalingrad_pool_connection))
    {
        previous = self.eroded_stalingrad_pool_connection;
        if (!connection_owned(state, previous, slot)) return;
        if (capacity != state.capacity)
        {
            withdraw_readiness(state, "applied_limit_drift");
            return;
        }
        if (!previous.returned && isdefined(previous.player) && previous.player == self) return;
        // The downloaded callback does not prove flag reset or script-object
        // reuse. Only a fresh entity can claim a returned slot.
        withdraw_readiness(state, "returned_entity_reuse_unverified");
        return;
    }
    if (!isdefined(state.connections) || !isarray(state.connections) || isdefined(self._challenges) ||
        (isdefined(state.connections[slot]) && !state.connections[slot].returned)) return;
    if (lifecycle_current(state) && capacity != state.capacity)
    {
        withdraw_readiness(state, "applied_limit_drift");
        return;
    }
    if (!identity_current(state) || isdefined(state.declined)) return;
    if (isdefined(state.post_returned) && state.post_returned == true && (!stock_post_complete() || !callbacks_owned()))
    {
        withdraw_readiness(state, "connect_owner_lost");
        return;
    }
    if (!ensure_seed(state)) return;
    record = spawnstruct();
    record.map = state.map;
    record.epoch = state.epoch;
    record.capacity = state.capacity;
    record.slot = slot;
    record.player = self;
    record.returned = false;
    record.detached = false;
    if (!isint(state.next_connection) || state.next_connection < 0 || state.next_connection >= 2147483647)
    {
        withdraw_readiness(state, "connection_serial_exhausted");
        return;
    }
    state.next_connection++;
    record.serial = state.next_connection;
    state.connections[slot] = record;
    self.eroded_stalingrad_pool_connection = record;
    // The stock delegate starts assignment only after a complete catalog exists.
    self zm_stalingrad_challenges::on_player_connect();
}

function disconnect()
{
    capacity = getdvarint("zm_coopAppliedLimit");
    if (!isdefined(level.eroded_stalingrad_pool_state)) return;
    state = level.eroded_stalingrad_pool_state;
    if (!lifecycle_current(state) || !isdefined(self) || !isentity(self) || !isplayer(self) ||
        !isdefined(self.eroded_stalingrad_pool_connection)) return;
    record = self.eroded_stalingrad_pool_connection;
    slot = self getentitynumber();
    if (!connection_owned(state, record, slot) || record.returned ||
        !isdefined(self.player_disconnected) || self.player_disconnected != 1) return;
    if (isdefined(record.player))
    {
        if (record.player != self) return;
    }
    else if (!isdefined(record.detached) || record.detached != true) return;
    if (lifecycle_current(state) && capacity != state.capacity)
    {
        withdraw_readiness(state, "applied_limit_drift");
    }
    // Revocation closes admission, but a real current flag1 disconnect may
    // release its own stock descriptors after detachment. Unknown ownership or
    // catalog shape is not repaired, and an old map/epoch cannot return tasks.
    if (!cleanup_context_valid(state))
    {
        withdraw_readiness(state, "disconnect_record_unrecognized");
        return;
    }
    if (!isdefined(self._challenges))
    {
        // A real disconnect before assignment owns no descriptors to return.
        // Preserve the original callback's final notification exactly once.
        record.returned = true;
        level notify("player_disconnected_" + slot);
        record.player = undefined;
        record.detached = true;
        return;
    }
    tasks = array(self._challenges.challenge_1, self._challenges.challenge_2, self._challenges.challenge_3);
    pools = array(level._challenges.challenge_1, level._challenges.challenge_2, level._challenges.challenge_3);
    for (tier = 1; tier <= 3; tier++)
    {
        task = tasks[tier - 1];
        index = catalog_index(state, tier, task);
        if (!isdefined(index) || !valid_task(task, tier, state.origins[tier][index]) ||
            !dense_pool(pools[tier - 1], state.catalog_size) || pools[tier - 1].size >= state.catalog_size)
        {
            withdraw_readiness(state, "disconnect_record_unrecognized");
            return;
        }
        keys = getarraykeys(pools[tier - 1]);
        for (i = 0; i < keys.size; i++)
        {
            if (!isint(keys[i]) || keys[i] < 0 || keys[i] >= state.catalog_size || pools[tier - 1][keys[i]] == task)
            {
                withdraw_readiness(state, "disconnect_record_already_free");
                return;
            }
        }
    }
    // Mark before the original non-yielding return/notification body. Repeated
    // callbacks for this current player cannot deposit or notify a second time.
    record.returned = true;
    self zm_stalingrad_challenges::on_player_disconnect();
    record.player = undefined;
    record.detached = true;
}

// Validate only the immutable catalog's bounded content. No GSC array
// container equality is used or claimed. Task structs come from the pinned
// stock constructor or our five-field spawnstruct copies.
function valid_catalog(state, tier)
{
    if (!isint(tier) || tier < 1 || tier > 3 ||
        !isdefined(state.catalog_size) || !isint(state.catalog_size) || state.catalog_size < 6 || state.catalog_size > 8 ||
        !isdefined(state.catalogs) || !isarray(state.catalogs) || state.catalogs.size != 3 ||
        !isdefined(state.origins) || !isarray(state.origins) || state.origins.size != 3 ||
        !isdefined(state.catalogs[tier]) || !dense_pool(state.catalogs[tier], state.catalog_size) ||
        state.catalogs[tier].size != state.catalog_size || !isdefined(state.origins[tier]) ||
        !dense_pool(state.origins[tier], state.catalog_size) || state.origins[tier].size != state.catalog_size) return false;
    for (index = 0; index < state.catalog_size; index++)
    {
        origin = state.origins[tier][index];
        if (!isint(origin) || origin < 0 || origin > 5 || !valid_task(state.catalogs[tier][index], tier, origin)) return false;
        for (previous = 0; previous < index; previous++)
            if (state.catalogs[tier][previous] == state.catalogs[tier][index]) return false;
    }
    return true;
}

function cleanup_context_valid(state)
{
    if (!lifecycle_current(state) || !stock_post_complete() || !callbacks_owned() ||
        !isdefined(state.seeded) || state.seeded != true || !isdefined(level._challenges) ||
        !isdefined(state.catalog_size) || !isint(state.catalog_size) || state.catalog_size < 6 || state.catalog_size > 8 ||
        !isdefined(state.catalogs) || !isarray(state.catalogs) || state.catalogs.size != 3 ||
        !isdefined(state.origins) || !isarray(state.origins) || state.origins.size != 3) return false;
    expected_size = 6;
    if (state.capacity > 6) expected_size = state.capacity;
    if (state.catalog_size != expected_size) return false;
    pools = array(level._challenges.challenge_1, level._challenges.challenge_2, level._challenges.challenge_3);
    for (tier = 1; tier <= 3; tier++)
    {
        if (!valid_catalog(state, tier) || !dense_pool(pools[tier - 1], state.catalog_size)) return false;
        seen = array();
        for (index = 0; index < pools[tier - 1].size; index++)
        {
            known = catalog_index(state, tier, pools[tier - 1][index]);
            if (!isdefined(known) || isdefined(seen[known])) return false;
            seen[known] = true;
        }
    }
    return true;
}

function ready(epoch, map, capacity)
{
    if (map != "zm_stalingrad" || !isdefined(level.eroded_stalingrad_pool_state)) return false;
    state = level.eroded_stalingrad_pool_state;
    if (!(schema_current(state) && state.epoch == epoch && state.capacity == capacity &&
        !isdefined(state.declined) && isdefined(state.seeded) && state.seeded == true &&
        isdefined(state.post_returned) && state.post_returned == true &&
        isdefined(state.guard_active) && state.guard_active == true && isdefined(state.guard_tick) &&
        isint(state.guard_tick) && gettime() >= state.guard_tick && gettime() - state.guard_tick <= 1000 &&
        stock_post_complete() && callbacks_owned())) return false;
    current = observation(state);
    return current.valid;
}

function dense_pool(pool, maximum)
{
    if (!isarray(pool) || pool.size > maximum) return false;
    keys = getarraykeys(pool);
    if (keys.size != pool.size) return false;
    for (i = 0; i < keys.size; i++)
        if (!isint(keys[i]) || keys[i] < 0 || keys[i] >= pool.size || !isdefined(pool[i])) return false;
    return true;
}

function catalog_index(state, tier, task)
{
    if (!isdefined(task) || !isint(tier) || tier < 1 || tier > 3 ||
        !isdefined(state.catalog_size) || !isint(state.catalog_size) || state.catalog_size < 6 || state.catalog_size > 8 ||
        !isdefined(state.catalogs) || !isarray(state.catalogs) ||
        !isdefined(state.catalogs[tier]) || !dense_pool(state.catalogs[tier], state.catalog_size) ||
        state.catalogs[tier].size != state.catalog_size) return undefined;
    for (index = 0; index < state.catalog_size; index++)
        if (task == state.catalogs[tier][index]) return index;
    return undefined;
}

// Read-only, bounded observation. Pending players are counted separately;
// transient disconnect bookkeeping is reported rather than repaired here.
function observation(state)
{
    result = spawnstruct();
    result.valid = false;
    result.reason = "catalog_not_current";
    result.players = 0;
    result.pending = 0;
    result.tracked = 0;
    result.disconnecting = 0;
    result.free = array(-1, -1, -1);
    result.owned = array(-1, -1, -1);
    if (!identity_current(state) || !isdefined(state.seeded) || state.seeded != true ||
        isdefined(state.declined) || !isdefined(level._challenges) || !isarray(state.connections) ||
        !isdefined(state.catalog_size) || !isint(state.catalog_size) || state.catalog_size < 6 || state.catalog_size > 8 ||
        !isdefined(state.catalogs) || !isarray(state.catalogs) || state.catalogs.size != 3 ||
        !isdefined(state.origins) || !isarray(state.origins) || state.origins.size != 3) return result;
    expected_size = 6;
    if (state.capacity > 6) expected_size = state.capacity;
    if (state.catalog_size != expected_size) return result;
    for (tier = 1; tier <= 3; tier++)
    {
        // Constructor-built catalogs are independent from mutable free pools.
        // Their fixed shape, exact unique task identities and free+owned
        // membership prove conservation without unsupported array equality.
        if (!valid_catalog(state, tier)) return result;
    }
    actual = getplayers();
    if (!isarray(actual) || actual.size > state.capacity) return result;
    keys = getarraykeys(actual);
    slots = array();
    for (i = 0; i < keys.size; i++)
    {
        if (!isint(keys[i]) || keys[i] < 0 || keys[i] >= state.capacity) return result;
        player = actual[keys[i]];
        if (!isdefined(player) || !isentity(player) || !isplayer(player)) return result;
        if (isdefined(player.player_disconnected) && player.player_disconnected == 1) continue;
        slot = player getentitynumber();
        if (!isint(slot) || slot < 0 || slot >= state.capacity || isdefined(slots[slot])) return result;
        slots[slot] = true;
        result.players++;
        if (!isdefined(state.connections[slot]) || state.connections[slot].returned || state.connections[slot].player != player)
        {
            // A real roster member can precede its owned connect callback.
            if (isdefined(player._challenges))
            {
                result.reason = "unowned_assignment";
                return result;
            }
            result.pending++;
        }
    }
    // Retain a departing owner's task membership until its real disconnect
    // callback returns it. getplayers() may already have removed that player.
    owners = array();
    owner_keys = getarraykeys(state.connections);
    if (owner_keys.size > state.capacity) return result;
    for (i = 0; i < owner_keys.size; i++)
    {
        slot = owner_keys[i];
        if (!isint(slot) || slot < 0 || slot >= state.capacity) return result;
        record = state.connections[slot];
        if (record.returned) continue;
        player = record.player;
        if (!connection_owned(state, record, slot) || !isdefined(player) || !isentity(player) || !isplayer(player) ||
            !isdefined(player.eroded_stalingrad_pool_connection) || player.eroded_stalingrad_pool_connection != record)
        {
            result.reason = "owner_record_unrecognized";
            return result;
        }
        owners[owners.size] = player;
        if (!isdefined(player._challenges)) result.pending++;
        if (isdefined(player.player_disconnected) && player.player_disconnected == 1) result.disconnecting++;
    }
    result.tracked = owners.size;
    pools = array(level._challenges.challenge_1, level._challenges.challenge_2, level._challenges.challenge_3);
    for (tier = 1; tier <= 3; tier++)
    {
        if (!dense_pool(pools[tier - 1], state.catalog_size))
        {
            result.reason = "free_pool_not_dense";
            return result;
        }
        seen = array();
        free_keys = getarraykeys(pools[tier - 1]);
        for (i = 0; i < free_keys.size; i++)
        {
            task = pools[tier - 1][free_keys[i]];
            index = catalog_index(state, tier, task);
            if (!isdefined(index) || isdefined(seen[index]) || !valid_task(task, tier, state.origins[tier][index])) return result;
            seen[index] = true;
        }
        owned = 0;
        for (i = 0; i < owners.size; i++)
        {
            player = owners[i];
            if (!isdefined(player._challenges)) continue;
            if (tier == 1) task = player._challenges.challenge_1;
            if (tier == 2) task = player._challenges.challenge_2;
            if (tier == 3) task = player._challenges.challenge_3;
            index = catalog_index(state, tier, task);
            if (!isdefined(index) || isdefined(seen[index]) || !valid_task(task, tier, state.origins[tier][index])) return result;
            seen[index] = true;
            owned++;
        }
        result.free[tier - 1] = free_keys.size;
        result.owned[tier - 1] = owned;
        if (free_keys.size + owned != state.catalog_size)
        {
            result.reason = "unaccounted_catalog_objects";
            return result;
        }
    }
    result.reason = "catalog_conserved";
    result.valid = true;
    return result;
}

function snapshot()
{
    if (!isdefined(level.eroded_stalingrad_pool_state)) return "";
    state = level.eroded_stalingrad_pool_state;
    if (!identity_current(state)) return "";
    current = observation(state);
    seeded = 0;
    if (isdefined(state.seeded) && state.seeded == true) seeded = 1;
    count = 0;
    if (isdefined(state.seed_count)) count = state.seed_count;
    size = 0;
    if (isdefined(state.catalog_size)) size = state.catalog_size;
    decline = "";
    if (isdefined(state.declined)) decline = state.declined;
    sequence = 0;
    if (isdefined(state.observation_sequence)) sequence = state.observation_sequence;
    tick = 0;
    if (isdefined(state.guard_tick)) tick = state.guard_tick;
    valid = 0;
    if (current.valid) valid = 1;
    return "{\"version\":1,\"map\":\"zm_stalingrad\",\"sequence\":" + sequence + ",\"tick_ms\":" + tick + ",\"epoch\":" + state.epoch +
        ",\"capacity\":" + state.capacity + ",\"seeded\":" + seeded + ",\"seed_count\":" + count +
        ",\"catalog_size\":" + size + ",\"players\":" + current.players + ",\"pending\":" + current.pending +
        ",\"tracked\":" + current.tracked + ",\"disconnecting\":" + current.disconnecting +
        ",\"free\":[" + current.free[0] + "," + current.free[1] + "," + current.free[2] +
        "],\"owned\":[" + current.owned[0] + "," + current.owned[1] + "," + current.owned[2] +
        "],\"valid\":" + valid + ",\"reason\":\"" + current.reason + "\",\"decline\":\"" + decline + "\"}";
}

// Flat typed LEVEL receipt lets the common controller check this map's owned
// result without importing Stalingrad modules into unrelated maps. It commits
// ready LAST; native source/helper retention and global schema gates are still
// independently required. No network field or clientfield layout is added.
function publish_receipt(state)
{
    if (!identity_current(state) || isdefined(state.declined) || !stock_post_complete() || !callbacks_owned() ||
        !isdefined(state.guard_active) || state.guard_active != true || !isdefined(state.guard_tick) ||
        !isint(state.guard_tick) || gettime() < state.guard_tick || gettime() - state.guard_tick > 1000) return;
    current = observation(state);
    if (!current.valid) return;
    level.eroded_stalingrad_pool_ready = 0;
    level.eroded_stalingrad_pool_map = state.map;
    level.eroded_stalingrad_pool_epoch = state.epoch;
    level.eroded_stalingrad_pool_capacity = state.capacity;
    level.eroded_stalingrad_pool_catalog_size = state.catalog_size;
    level.eroded_stalingrad_pool_tick = state.guard_tick;
    level.eroded_stalingrad_pool_revision = 1;
    level.eroded_stalingrad_pool_ready = 1;
}

function guardian(state)
{
    // system::exec_post_system publishes postdone after this POST wrapper exits.
    wait 0.05;
    while (identity_current(state) && !isdefined(state.declined))
    {
        status = getdvarstring("zm_coopStatus");
        if (status == "ending")
        {
            withdraw_readiness(state, "ending");
            return;
        }
        if (!stock_post_complete() || !callbacks_owned())
        {
            withdraw_readiness(state, "stock_owner_lost");
            return;
        }
        current = observation(state);
        if (!current.valid)
        {
            withdraw_readiness(state, "live_catalog_drift");
            setdvar("zm_coopStalingradPoolSnapshot", snapshot());
            return;
        }
        state.guard_tick = gettime();
        state.observation_sequence++;
        state.guard_active = true;
        publish_receipt(state);
        // This receipt is read-only diagnostic evidence, not the admission gate.
        setdvar("zm_coopStalingradPoolSnapshot", snapshot());
        wait 0.25;
    }
    if (lifecycle_current(state) && getdvarint("zm_coopAppliedLimit") != state.capacity)
        withdraw_readiness(state, "applied_limit_drift");
    else detach_owned_players(state);
}

// Catalog metadata and safe clone helpers are authored against the pinned
// retail contract. Original stock module bodies remain unchanged.


function callback_count(event, expected)
{
    if (!isdefined(level._callbacks) || !isarray(level._callbacks) ||
        !isdefined(level._callbacks[event]) || !isarray(level._callbacks[event])) return -1;
    entries = level._callbacks[event];
    keys = getarraykeys(entries);
    if (!isarray(keys) || keys.size > 128) return -1;
    count = 0;
    for (index = 0; index < keys.size; index++)
    {
        entry = entries[keys[index]];
        if (!isdefined(entry) || !isarray(entry)) return -1;
        if (isdefined(entry[0]) && entry[0] == expected)
        {
            if (isdefined(entry[1])) return -1;
            count++;
        }
    }
    return count;
}

function expected_counts(tier)
{
    if (tier == 1) return array(10, 3, 5, 3, 5, 1);
    if (tier == 2) return array(10, 30, 40, 45, 10, 1);
    if (tier == 3) return array(1, 3, 100, 60, 6, 6);
    return undefined;
}

function expected_info(tier)
{
    if (tier == 1) return array(&"ZM_STALINGRAD_CHALLENGE_1_1", &"ZM_STALINGRAD_CHALLENGE_1_2", &"ZM_STALINGRAD_CHALLENGE_1_3", &"ZM_STALINGRAD_CHALLENGE_1_4", &"ZM_STALINGRAD_CHALLENGE_1_5", &"ZM_STALINGRAD_CHALLENGE_1_6");
    if (tier == 2) return array(&"ZM_STALINGRAD_CHALLENGE_2_1", &"ZM_STALINGRAD_CHALLENGE_2_2", &"ZM_STALINGRAD_CHALLENGE_2_3", &"ZM_STALINGRAD_CHALLENGE_2_4", &"ZM_STALINGRAD_CHALLENGE_2_5", &"ZM_STALINGRAD_CHALLENGE_2_6");
    if (tier == 3) return array(&"ZM_STALINGRAD_CHALLENGE_3_1", &"ZM_STALINGRAD_CHALLENGE_3_2", &"ZM_STALINGRAD_CHALLENGE_3_3", &"ZM_STALINGRAD_CHALLENGE_3_4", &"ZM_STALINGRAD_CHALLENGE_3_5", &"ZM_STALINGRAD_CHALLENGE_3_6");
    return undefined;
}

function expected_think(tier)
{
    if (tier == 1) return array(&zm_stalingrad_challenges::function_960bfb56, &zm_stalingrad_challenges::function_f1c59ae, &zm_stalingrad_challenges::function_76bcffc2, &zm_stalingrad_challenges::function_dec401c2, &zm_stalingrad_challenges::function_c169b7dd, &zm_stalingrad_challenges::function_5efd7abf);
    if (tier == 2) return array(&zm_stalingrad_challenges::function_4322fb5f, &zm_stalingrad_challenges::function_f427e9ad, &zm_stalingrad_challenges::function_4e107409, &zm_stalingrad_challenges::function_81adc498, &zm_stalingrad_challenges::function_64e8cc03, &zm_stalingrad_challenges::function_31d5f655);
    if (tier == 3) return array(&zm_stalingrad_challenges::function_75fdfc25, &zm_stalingrad_challenges::function_2bf9f9d4, &zm_stalingrad_challenges::function_dcbd7aec, &zm_stalingrad_challenges::function_3d0619b6, &zm_stalingrad_challenges::function_cdeaa5f, &zm_stalingrad_challenges::function_e480fc42);
    return undefined;
}

function valid_task(task, tier, index)
{
    if (!isdefined(task) || !isint(tier) || tier < 1 || tier > 3 || !isint(index) || index < 0 || index > 5) return false;
    counts = expected_counts(tier);
    info = expected_info(tier);
    think = expected_think(tier);
    // Localized info handles are compared directly, never coerced to strings.
    return isdefined(task.n_index) && isint(task.n_index) && task.n_index == tier &&
        isdefined(task.n_count) && isint(task.n_count) && task.n_count == counts[index] &&
        isdefined(task.str_info) && task.str_info == info[index] &&
        isdefined(task.str_notify) && isstring(task.str_notify) && task.str_notify == "update_challenge_" + tier + "_" + (index + 1) &&
        isdefined(task.func_think) && task.func_think == think[index];
}

function valid_pool(pool, tier)
{
    if (!isarray(pool) || pool.size != 6) return false;
    keys = getarraykeys(pool);
    if (keys.size != 6) return false;
    for (index = 0; index < 6; index++)
    {
        if (!isint(keys[index]) || keys[index] < 0 || keys[index] > 5 || !valid_task(pool[index], tier, index)) return false;
        for (previous = 0; previous < index; previous++)
            if (pool[previous] == pool[index]) return false;
    }
    return true;
}

function clone_task(task, tier, index)
{
    if (!valid_task(task, tier, index)) return undefined;
    copy = spawnstruct();
    copy.n_index = task.n_index;
    copy.str_info = task.str_info;
    copy.n_count = task.n_count;
    copy.str_notify = task.str_notify;
    copy.func_think = task.func_think;
    return copy;
}

function clone_index(tier, extra)
{
    if (!isint(tier) || tier < 1 || tier > 3 || !isint(extra) || extra < 0 || extra > 1) return undefined;
    // Duplicate only known per-player task workers. Other templates retain
    // global singleton owners/shared death callback cleanup and cannot alias.
    if (tier == 1) return 1 + extra;
    if (tier == 2) return 2 + extra;
    return extra;
}
