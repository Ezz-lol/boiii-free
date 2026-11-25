# Experimental dynamic Zombies coop

This is a draft implementation, not a release-ready eight-player mode. The default
remains four players. Above-four sessions require `-experimental-zombies-coop` and the
corresponding binary and data for this build family.

Lobby routing and discovery are described in [Lobby paths and
validation](zombies-coop-lobbies.md). Those source changes do not establish occupied
gameplay, client script compatibility, or support for every map.

## Capacity and lobby paths

- `zm_coopPlayerLimit` selects 1-8 for the next eligible frontend host action. Without
  the experimental flag, its registered maximum is four.
- A frontend host settings menu stores the next lobby size, 1-8 with default four. It
  applies after closing/recreating the lobby, not by writing a live allocated pool.
  A native 5-8-slot frontend host keeps its allocated capacity even when the local saved
  preference or a freshly loaded UI helper differs.
- The bounded GAME/ZM policy covers online staging target 400, Public 401, Custom 402,
  and LAN 410. It checks target mode/type metadata and leaves unknown targets, theater,
  MP, campaign, and local-player limits alone.
- Solo uses playlist 150 on screen 400 in the reviewed PC source. A specific frontend
  local-host launch guard restores its stock launch ceiling; it does not classify all of
  screen 400 as Solo or change the saved next-lobby preference.
- With experimental opt-in, the already allocated native `com_maxclients` value is
  authoritative for 1-8. A saved preference of four must not shrink an eight-slot joined
  or inherited lobby. Native 1-4 still wins for stock Solo and ordinary sessions. Native
  capacities above eight are rejected by the expanded policy; no allocated pool is
  enlarged here.
- Without opt-in, foreign native capacities above four remain outside this feature and
  retain the existing archived 1-4 path. Invalid native values outside 1-18 and
  contradictory expanded preferences are rejected.
- Applied capacity stays latched across map rotations. Changing it requires returning to
  the frontend to rehost, or restarting the dedicated process. This does not resize live
  client arrays.
- Custom lobby matchmaking here means browsing and explicitly selecting an eligible
  host. It adds no automatic queue, random matchmaking pool, or automatic privacy
  change.

## Gameplay controller

The server addon uses the stock player-count and spawn-delay callback seams. It does not
replace the whole retail `_zm.gsc`. Normal-round accounting uses the stock count formula
and current connected participants, retains consumed spawns through population
reductions and increases, and leaves concurrent AI/actor caps unchanged. Downed players
remain participants.

Stock timeout recycling is tracked with map-cumulative `zombie_total_subtract` deltas,
separately from explicit `zombie_respawns`. Polling cannot determine every spawn/recycle
event's order, so a target reduction conservatively retains potentially queued
replacements. This can retain extra recycled quota after a shrink. Exact per-AI event
attribution is not established. Unexplained additions, counter resets, unknown
controllers, and outside callback ownership stop adaptation rather than overwrite those
budgets.

Threads end at round restart, round end, game end, or intermission as appropriate.
Shadows of Evil retains its stock opening-round pacing and identified parasite/raps
callback; its map-only imports stay under `custom_scripts/zm_zod`.

The current native and GSC allowlist is **`zm_zod` and `zm_tomb` only**. This is a
testing boundary, not a claim that occupied play is validated even on those maps. Other
maps reject an above-four startup. Workshop gameplay mods and loaded usermaps are
rejected; the engine's exact builtin `usermaps` bootstrap for a stock lobby is
permitted. Content-selection checks do not fingerprint modified builtin assets or
same-export stock scripts.

Native roster and 16-use finite-stock policy classes remain scaffolding. Actual per-map
machine adapters are not connected. No two-per-player purchase restriction is
implemented. A policy unit test is not evidence of working machine stock in a game.

## Readiness and shutdown

Expanded remote-game admission waits for the current map's native epoch, exact map name,
installed GSC callbacks, and valid heartbeat state. Native engine loopback can bootstrap
a listen host; a remote address in 127/8 does not receive that transport exemption.

VM shutdown revokes map readiness before script memory is released, clears active-map
state, and preserves the session's applied capacity for rotation. A stopped server after
its first running observation latches a failure until an explicit fresh startup arm.
Reusing old same-map readiness dvars cannot reopen admission. Cold asset loading before
the first running observation does not spend the script-startup timeout.

The native gate reports errors from the main frame after scheduler and feature locks are
released. Returning to the frontend resets the hosted session state. Paused GSC time is
allowed: a stationary heartbeat alone is not a failure, and is not proof that every
thread remains alive.

Expanded in-game host migration is not supported. The custom-lobby migration refusal is
retained and expanded in-game GAME/ZM migration is refused. Preserving frontend lobby
capacity when ownership changes is separate from transferring a running GSC controller,
network schema, or round accounting.

## Listing and discovery

`zm_coopAdvertise` defaults to false. An eligible online custom host may explicitly use
the listing button; native `game.cooplobbyinfo()` supplies eligibility and the action
rechecks it. The feature never opens a private lobby or changes its privacy to make it
listable. Eligibility is recomputed from native host/session/content state, selected
map, capacity, and adapter readiness when already in a match.

Public listing is limited to open, ready, opted-in stock custom GAME/ZM hosts on the
current Zod/Tomb allowlist with native capacity 5-8. LAN discovery is separate and uses
`zm_coopLanDiscoverable`. Its bounded browser handle 2 scan validates nonce, address
scope, timing, response rates, and request generations before normal `getinfo` queries
and native party joins. A browser row reports the selected frontend map and gametype
rather than presenting `core_frontend` as the playable map.

The `coopLobby` revision and network-mode descriptor identify a transport route. They
are **not CSC/schema attestation**, do not authenticate matching addon data, and do not
prove a peer can safely decode extra-slot snapshots.

Validated version-1 custom routes check capacity, network mode, and experimental opt-in
before native joining. Explicit native refusal stops their raw-match fallback in both
forks. Ezz additionally binds strict custom requests to `match1_<32hex>` nonce, endpoint,
deadline, request generation, and the caller's current-request predicate; legacy routes
retain their existing wire behavior.

Connection generations guard stale DNS replies, queued mode changes, waits, and
not-yet-started join callbacks. Once an engine join has begun, there is no verified abort
API: that join may still finish, stale completion is ignored, and a second native join
FSM is not started over it. Persisted Workshop reconnect state is not globally cancelled
by these guards; the supported custom route is stock-only.

## Client compatibility still required

Extra-slot revive and score-field contracts remain blockers. Server GSC, client CSC, and
UI field models need a matching schema installed before registration freezes. Wider
scoreboard rows and bounded HUD colours do not register revive or score fields.

The client LobbyVM bootstrap is now implemented in source. On the supported PC client
image, it hooks the source-backed `LobbyVM_Init` entrypoint, waits for the original
initializer, obtains the actual LobbyVM, and compiles/protected-calls the fixed
`lobby_scripts/zombies_coop/__init__.lua` companion in that separate state. It does not
swap primary UI pointers or reuse UI references. The reader uses a 65,537-byte heap
buffer to reject files above 64 KiB; it restores stack/sharing state after the call.

Readiness is false before each client initialization and becomes true only when the
module reports version 4 and all three installed host-start, launch, and migration
callbacks still have the expected function identities at installation. Client-image
expanded startup and host listing require that readiness value. Dedicated processes
are exempt from this client-only bootstrap requirement.

This is source-level implementation, not observed client execution. Runtime evidence
must include `[zombies_coop_lobby_vm] role=client event=installed`; `event=hook_armed`
alone is insufficient. Actual listen-host/joining-client bootstrap proof remains
pending, and successful LobbyVM installation still does not register the missing CSC
revive/score schema.

The [pinned script compatibility review](zombies-coop-more-players-review.md) records
relevant source versions, licensing, field contracts, and packaging constraints. It does
not extend this map allowlist or certify revive, quest ownership, scoring, spectators,
or mixed CB/Ezz clients.

## Ezz script packaging

Ezz compiles readable `data/custom_scripts/zm/eroded_coop.gsc` and the map-specific
source with its own loader. Do not substitute CB's compiled payload for Ezz source. Keep
each build family's executable and data together.

ACTS BO3 revision-1C syntax checks are supplementary; they do not replace Ezz runtime
compilation and linkage checks.

The earlier isolated read-only-assets investigation found optional `zone/scriptgdb.zip`
persistence throwing before startup and a crash handler repeatedly resuming a
noncontinuable exception. Optional archive failure is now logged without discarding
compiled scripts, and noncontinuable failures are returned to the OS after reporting.
`tests/exception_recovery_policy.cpp` checks that policy. Current gameplay startup still
needs fresh sound-on and `-nosnd` canary evidence.

## Validation

In an x64 Visual Studio Developer shell, run the relevant standalone tests and Lua
fixtures; see the [lobby test matrix](zombies-coop-lobbies.md#local-checks) for the new
suites. For example:

```powershell
cl /nologo /std:c++20 /EHsc /W4 /WX /Fe:coop-policy.exe tests/zombies_coop_policy.cpp
./coop-policy.exe
cl /nologo /std:c++20 /EHsc /W4 /WX /Fe:coop-startup.exe tests/zombies_coop_startup.cpp
./coop-startup.exe
lua5.1 tests/zombies_coop_ui.lua .
```

Native tests check policy state and bounds. Lua fixtures use candidate modules against a
mocked engine. Neither proves native allocation, real network joins, client VM
bootstrap, or snapshot compatibility. Existing 18-slot MP structures/wire layout and
campaign behavior are not widened by these changes.

`tests/gsc/eroded_coop_selftest.gsc` is an isolated actual-VM fixture; keep it out of
production data. It publishes `zm_coopSelfTestStatus` and `zm_coopSelfTestFailures` for
arithmetic and duplicate-startup checks.

No occupied five-to-eight-player clients, actual eight-player listen-host/LAN joins, or
internet master-list visibility have been validated for the new lobby changes. Fresh
dedicated sound-on and `-nosnd` canaries must be recorded against the exact new
source/binary/data before their results are claimed. Historical empty-server runs are
not substitutes.

Before marking a PR ready, complete real 5/6/7/8-client tests, party carry, map
selection, privacy changes, cancellation/rehost, disconnect/rejoin, down/revive, points
awards, spectators, character/quest ownership, special/boss rounds, per-map machines,
long games, and clean shutdown. Missing or legacy companion rejection needs explicit
proof. CB/Ezz interoperability requires its own tests; same-version or same-capacity
metadata is insufficient.
