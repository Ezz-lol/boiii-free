# Island trials adapter

The retail Island allocator removes assignments from pools containing five, six,
and five task records. Eight concurrent players can exhaust those pools. The
adapter keeps the original selections without replacement while records remain,
then creates independent copies of the five stock descriptor fields. Each tier
owns at most eight assignment records. Disconnect returns an assignment once;
reusing a player slot cannot return a newer player's records or reset players
who remained connected.

The Island-only early asset is `scripts/zm/eroded_coop_island_trials.gsc`, shipped
as `data/coop_scripts/eroded_coop_island_trials.gsc`. Its system registration names
both `zm_laststand` and `eroded_coop_fields` as ordered direct dependencies because
the retail dependency executor is not recursive. Its preinit callback queues one
map/epoch-owned installer, which yields once before observing the registry. It
waits for absent stock registrations for at most 120 polls plus the first yield
(6.05 simulation seconds), without awaiting challenge initialization or forcing
flags. It replaces only exact single stock connect/disconnect registrations on
an empty roster. Malformed, duplicate, object-bound, conflicting, or missing
registrations remain closed with bounded count diagnostics; a player appearing
during the wait also declines installation. Existing listen-host dispatch ordering still requires client runtime
verification; this does not retrofit already dispatched stock callbacks.

All players use the existing task-progress, completion-toast, and reward
functions. The adapter omits the historical completion counter and checks the
actual and listed entity sets, their owned assignments, and all three completed
flags. A nonempty completed current roster earns the existing global unlock and
stock completion FX once. The earned unlock remains latched for later joins.
The shared reward altar and its original pickup/disconnect handling remain owned
by the retail reward functions.

Extra pillar state reuses the twelve already allocated world fields
`pillar_challenge_4_1` through `pillar_challenge_7_3`. No extra scriptmover field,
server locator actor, or native pool expansion is required. Each exact owned
reservation initializes all three fields to idle state one. Owned release resets
extra slots to zero, allowing the client to retire its own visual model; the
original four slots retain the retail disconnect reset of one. A stale owner or
old worker cannot reset a later occupant's fields.

The server preserves the original four trigger and look-at references. Four
extra private frames use the measured wall midpoints, face the existing altar,
and rotate the original look-at offsets into each new frame. Floor,
line-of-sight, spacing, vector types, and unique real-slot bindings must pass
before readiness opens. Failure retains `geometry_pending` and closes expanded
Island admission. The paired client derives the matching frames and uses the
current three world states to own one local visual pillar per occupied extra
slot. It cancels stale render work and retires an absent slot. These new world
bindings still require an isolated runtime check and actual client rendering and
interaction checks.
ACTS treats `script_` identifiers as raw hash literals. The adapter uses verified
canonical aliases for `script_special` and `script_unitrigger_type`, while keeping
the retail fields' actual identities. Compiler checks must inspect the emitted
field operands; compiling successfully alone cannot detect a zero identifier.

`tests/zombies_coop_island_trials.py` exercises mechanically translated actual
allocator, ownership, rollback, cleanup, and completed-roster helpers.
`tests/zombies_coop_island_gate.py` exercises the actual common readiness
predicate and checks its startup/heartbeat wiring. The actual-source world test
checks the measured frame transforms, bounded traces, map-field aliases, delayed
prompt activation, and occupied-slot-only locators. These local stand-ins do not
execute the retail GSC VM, collision system, or client renderer.

The map-only VM fixture waits for a current native-authorized early helper table
instead of importing the Island asset before its map anchor. It exercises only
private pools through the owned helper references and reports its own status,
failure count, and matching startup epoch. It does not complete real challenges,
move players, or change the global unlock to pass a test.

The reviewed interfaces are pinned to [retail Island trials](https://github.com/ate47/bo3-source/blob/fe040b1187b4850fe8da3c78b703c986c87d1028/scripts/zm/zm_island_challenges.gsc),
[callback registration](https://github.com/ate47/bo3-source/blob/fe040b1187b4850fe8da3c78b703c986c87d1028/scripts/shared/callbacks_shared.gsc),
and [system initialization](https://github.com/ate47/bo3-source/blob/fe040b1187b4850fe8da3c78b703c986c87d1028/scripts/shared/system_shared.gsc).
