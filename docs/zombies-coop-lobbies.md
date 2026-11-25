# Zombies lobby paths, discovery, and validation

## Status and scope

This document describes the source policy for experimental 1-8-player Zombies lobby
routing. It is not an end-to-end compatibility certification. The only expanded gameplay
adapters currently allowed are stock `zm_zod` and `zm_tomb`.

The implemented routing changes can be reviewed and unit-tested separately from
unresolved client CSC/schema work. Actual eight-player listen hosting, LAN joins,
occupied clients, internet master-list visibility, and real joining-client bootstrap
remain unverified. Exact dedicated canary results belong to the PR description and
the matching private build report; no result is implied by this document.

Use [the main coop notes](zombies-coop.md) for gameplay accounting, packaging, and
readiness. Use [the pinned compatibility review](zombies-coop-more-players-review.md)
for the missing paired client/server field contracts.

## Entry points

| Reviewed PC target                     | Meaning                               | Capacity policy                                                                    | Boundary                                                                                                    |
| -------------------------------------- | ------------------------------------- | ---------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------- |
| 400 / `UI_ZMLOBBYONLINE`               | Online Zombies staging                | Bounded GAME/ZM host preference before allocation; preserve an active native lobby | Screen 400 is shared with Solo; do not classify it by screen ID alone                                       |
| 401 / `UI_ZMLOBBYONLINEPUBLICGAME`     | Stock Public Zombies lobby path       | Bounded GAME/ZM Public target; active native capacity wins                         | This does not establish a new public matchmaking service or expand a stock playlist's backend allocation    |
| 402 / `UI_ZMLOBBYONLINECUSTOMGAME`     | Online custom game                    | Bounded GAME/ZM Custom target; optional explicit listing when eligible             | Browsing and selecting a host is user-driven; no automatic queue                                            |
| 410 / `UI_ZMLOBBYLANGAME`              | LAN/system-link custom game           | Bounded GAME/ZM Custom target with native LAN mode                                 | Local discovery does not bypass native party admission                                                      |
| Playlist 150 on screen 400             | Solo launch in the reviewed PC source | Preserve stock Solo launch, then use its actual native allocation                  | Source-specific launch guards check playlist, screen, mode, local GAME hosting, and frontend state together |
| Theater, unknown targets, MP, campaign | Existing game paths                   | Retain the existing path and capacity                                              | No expansion or native 18-slot wire/layout changes                                                          |

Target matching checks the named target's GAME lobby type, ZM main mode, and expected
Custom/Public mode. It does not reinterpret every numeric target or rewrite another
mod's unrelated capacities. Existing local-player limits remain unchanged.

Both UI `CoD.LobbyBase.LaunchGame` and LobbyVM launch entrypoints have a narrowly scoped
Solo guard. A frontend local GAME/ZM host on screen 400 with playlist 150 and an actual
5-8 capacity restores its stock launch ceiling of four; existing 1-4 values remain
unchanged. This leaves `zm_coopPlayerLimit` as the next non-Solo preference. The
engine's eventual native Solo allocation is authoritative.

## Session capacity and lifecycle

Three inputs are distinct: the saved next-lobby preference, the already allocated native
capacity, and `-experimental-zombies-coop`.

| Inputs                                                               | Resolver result                                                                              |
| -------------------------------------------------------------------- | -------------------------------------------------------------------------------------------- |
| Invalid saved preference outside 1-8 or native capacity outside 1-18 | Reject                                                                                       |
| Native 1-4, with or without opt-in                                   | Keep the native value, including Solo                                                        |
| Native 5-8 with experimental opt-in                                  | Adopt native capacity, even when this client saved four or a different next-lobby preference |
| Native 9-18 with experimental opt-in                                 | Reject; this feature supports at most eight                                                  |
| Native above four without opt-in, saved value 1-4                    | Retain the old archived path; foreign allocation remains outside this feature                |
| Native above four without opt-in, saved value 5-8                    | Reject the contradictory expanded configuration                                              |

No already allocated pool is enlarged. Loading, in-game, remote-host, recovery, and
observed frontend host-transition paths preserve authoritative native capacity. A native
5-8-slot frontend host stays frozen even after a fresh UI-helper load or when its saved
preference differs. The frontend host settings menu selects the next lobby size, 1-8
with default four. Apply it by closing/recreating the lobby; it does not write a live
allocated pool. Capacity applied to a hosted
game remains latched over map rotations; an intentional resize requires returning to the
frontend/rehosting or restarting a dedicated process.

Expanded admission remains closed until the current map confirms the fresh native
epoch/map and installed GSC callbacks. Engine loopback is allowed to bootstrap a listen
host. VM shutdown closes admission before script allocations are cleared. An unexpected
server stop after the first running observation latches failure until an explicit fresh
arm; stale readiness cannot revive the old map. Before the first running observation,
cold asset loading can continue without consuming the script-startup timeout.

Expanded in-game host migration is refused. The stock custom-game refusal remains and
the expanded in-game GAME/ZM path also refuses migration. Keeping a frontend lobby's
observed capacity during ownership changes is not a running-game migration
implementation. Controller thread state, round budget, field schema, and objective
ownership have no validated migration-transfer contract.

## Explicit online listing

`zm_coopAdvertise` is an archived boolean with default **false**. A host must explicitly
enable listing. The frontend listing button appears only when native
`game.cooplobbyinfo()` reports `enabled` and `canAdvertise`; it uses the reported
`advertise` value for List/Unlist and checks eligibility again when activated.

The native publication policy samples authoritative host state. An online advertisement
requires all of the following:

- The experimental flag, a native ready/active local GAME/ZM host, Custom mode, and a
  valid local game-lobby identity.
- Successful client LobbyVM addon installation, recorded by its strict version-3
  function-ownership readiness check.
- Existing privacy is Open. The feature never changes privacy, creates an open session,
  or automatically enables listing.
- Native capacity is 5-8 and the client count is valid for that capacity.
- Selected content is stock, the selected map is `zm_zod` or `zm_tomb`, and no
  unsupported gameplay mod/usermap is active.
- The native network mode is online/live. If already in a map, the local server and
  current expanded adapter must also be ready.
- The host has explicitly enabled `zm_coopAdvertise`.

Eligibility is rechecked during publication. When it is lost, new listing heartbeats
stop. A cached browser/master row may outlive the host state; clicking an old row must
still pass fresh native join, privacy, content, and readiness checks. No claim is made
that the external internet master accepts or displays the new heartbeat until tested.

The listing describes `ui_mapname` in the frontend and the actual map once launched. It
reports the selected gametype, with a Zombies-specific fallback when that value is
absent. It must not label every frontend lobby `core_frontend` as if that were its
selected playable map.

Public target 401 and custom-listing eligibility are separate policies. Extending a
Public target's local capacity handling does not automatically list it as a custom host
or place the user into an automatic matchmaking queue.

## Metadata and normal join flow

The existing information response adds a bounded descriptor for an eligible custom host:

| Field                            | Meaning                                                                 |
| -------------------------------- | ----------------------------------------------------------------------- |
| `coopLobby=1`                    | Version 1 of this custom-lobby transport descriptor                     |
| `lobbyNetworkMode=live` or `lan` | Requested native party route                                            |
| `sv_maxclients` and `clients`    | Observed native capacity and population                                 |
| `mapname` and `gametype`         | Selected frontend game or current in-map game                           |
| `coopListed`                     | Current online-advertisement state when experimental support is enabled |

The descriptor is **not client capability negotiation, CSC/schema attestation, or
addon-data authentication**. A peer's claim to support capacity eight does not prove
that its field registration, snapshot decoding, revive state, or points model matches
the host. Do not relax normal signatures, privacy, content checks, reservations, or
native admission on this metadata's authority.

Descriptor routing requires the known revision, exact native network mode, bounded 5-8
capacity, experimental opt-in, and an appropriate address scope for LAN. Absent metadata
retains the legacy route; malformed/unsupported metadata is rejected rather than
guessed. A legacy client or missing addon may still be incompatible with expanded
gameplay: safe rejection before an incompatible snapshot remains a separate requirement.

After the user chooses a row, normal `getinfo` and native party joining own the
connection. Rechecks at this stage handle closed privacy, changed content/capacity,
failed readiness, and stale host state. A validated version-1 custom route checks
capacity, network mode, and experimental opt-in before starting the native join. Explicit
native refusal ends that custom attempt in both forks; it cannot retry as a raw match
connection after a privacy, reservation, compatibility, or other native denial. Legacy
fallback behavior is outside this scoped rule.

For an armed expanded listen host, authenticated direct connections must already be
active members of the native GAME lobby before profile distribution and game admission.
Only the engine's actual loopback address bypasses this membership check; a remote
address cannot obtain that exemption by claiming a local identity.

The additional native close check is limited to an actual 5-8-slot online Custom
listen game with experimental support enabled; Public, LAN, default-four, and
dedicated paths retain their existing rules. Earlier native refusals are preserved.

- CB: every new native reservation callback is refused while closed. Its opaque
  callback does not expose a verified member batch or transport address, so neither
  a known sender nor a claimed local XUID can exempt carried members. Already active
  GAME clients remain active, but reopen the lobby before any new reservation,
  including party carry, reconnect reservations, or a local bootstrap that uses this
  callback. No unverified request offsets are read.
- Ezz: a remote sender and every requested member must already be an active hosted
  GAME/PRIVATE member or an actual local XUID. Invalid/empty member counts and unknown
  carried users are refused before reservation. Only the native LOOPBACK transport
  bypasses the complete batch check; claiming a local sender XUID does not bypass it.

These membership checks do not prove client script-schema compatibility.

Ezz strict custom joins use `match1_<32hex>` requests tied to an exact endpoint, fresh
nonce, deadline, generation, and caller current-request predicate. This is restricted to
known descriptor hosts; legacy request/response wire behavior remains unchanged. These
bindings prevent an old same-address response from being consumed by a replacement
strict attempt. They do not attest the client script schema.

Connection-request generations begin before DNS and guard the query response, queued
main-thread work, mode changes, waits, and not-yet-started callbacks. A recognized Ezz
custom descriptor pending in the frontend can also be cancelled by the disconnect
command; legacy mode-transition commands are not intercepted by this scoped cancellation.

Once a native engine join has begun, no verified abort API is available. The old join
can still finish after cancellation; stale completion is ignored and no second join FSM
is started over it. Do not claim atomic cancellation of an already running engine join.
Persisted Workshop mod/download reconnect state is not globally cancelled. Supported
custom descriptor hosts are stock-only, avoiding the normal need for that download path.

## Bounded LAN discovery

Browser handle **2** owns the LAN list separately from the internet list. Discovery
sends a nonce query and accepts only the corresponding timely response from allowed
local IPv4 scope. A scan is bounded to two seconds and 128 discovered addresses. Host
replies have a fixed-size peer table, a global response budget, and a per-address rate
limit; public sources do not receive discovery reflection.

Each browser discovery/query batch has a generation. Refresh, cancellation, release, or
a replacement scan invalidates prior callbacks; a batch has one completion. Valid
discovery replies only provide candidate addresses. Normal information queries populate
rows, and the ordinary native party path performs joins.

`zm_coopLanDiscoverable` is separate from online listing and defaults to true. It only
permits replies while the host satisfies the open, ready, stock, expanded LAN
eligibility policy. Turning it off must suppress new discovery responses without
changing lobby privacy.

Limited broadcast and loopback scans are not proof of routed-VPN discovery. Broadcast
forwarding, adapter selection, NAT, and VPN address scope require actual tests. A
source-routed/manual-address join and automatic discovery are different outcomes. Do not
report VPN support merely because one policy helper accepts an address range.

## Bootstrap and gameplay blockers

Frontend UI VM, client LobbyVM, dedicated LobbyVM, server GSC, and client CSC are
distinct bootstrap paths. The client LobbyVM bootstrap is implemented in
`src/client/component/zombies_lobby_vm.cpp`, restricted to the supported PC client
image and experimental opt-in. It calls the original `LobbyVM_Init` before getting the
actual LobbyVM and uses source-backed client bindings rather than directing the UI VM
at another state's objects.

The fixed companion is `lobby_scripts/zombies_coop/__init__.lua`. Its reader allocates
65,537 bytes on the heap, reads at most that amount, accepts at most 65,536 bytes
(64 KiB), and rejects an empty, oversized, or failed first-opened file. It does not
allocate from the full untrusted file length or fall through from a corrupt first file
to a different installation root.

Compilation and protected execution use the actual client LobbyVM. The loader restores
the API stack relative to the current stack base and restores bytecode-sharing mode.
It does not swap primary UI state pointers or move UI references into LobbyVM.

Before client initialization, readiness is false. It is set only when the addon returns
the strict version-3/ownership check: `Lobby.Actions.LobbyHostStart`,
`LobbyVM.LaunchGame`, and `LobbyVM.CanLobbyCanMigrate` equal their installed coop
wrapper functions. Client-image expanded startup and listing both require this boolean.
A dedicated process has no client LobbyVM and is exempt from that requirement.

Source, syntax, structural, and mocked Lua checks do not prove actual execution. A real
client run must record `[zombies_coop_lobby_vm] role=client event=installed` after each
relevant initialization/reload; `event=hook_armed` is only hook setup. Missing, invalid,
incomplete, compile-failed, or execution-failed addons must leave readiness false.
Actual client bootstrap, listen-host, and joining-client runtime proof remains pending.

The common GSC round addon waits for stock callbacks before installing. That is not
proof of a safe early clientfield-registration stage. Extra slots still require matching
server/client revive fields, client CSC state, and score pop-up field models or verified
sender handling. Native transport revision 1 does not resolve these contracts.

Do not widen the map allowlist because another host appears in the browser. Other maps
can have slot-specific inventory fields, quests, bosses, spawn rules, finite machines,
HUD contracts, or reserved objectives. Same-export modified stock assets are not
fingerprinted by current preflight. Existing MP/campaign behavior and native 18-slot
ABI/wire remain outside this feature.

## Local checks

Run each checkout's corresponding standalone C++20 tests with `/W4 /WX` and the
candidate Lua fixtures. These suites cover policy and mocked entrypoints; record
compiler/runtime versions and results with the actual revision:

- `tests/zombies_coop_startup.cpp`: all valid preference/native-capacity/opt-in
  combinations, invalid values, cold loads, epochs, stopped-map rearm, and stock reset.
- `tests/zombies_coop_lobby_policy.cpp`: listing eligibility,
  map/content/privacy/capacity bounds, nonce lifetime, local discovery scope, and rate
  limits.
- `tests/zombies_coop_lobby_join.cpp`: descriptor revision, strict capacity parsing,
  opt-in, legacy routing, and local-scope rules.
- `tests/lan_browser_request.cpp`: cancellation, refresh generations, duplicate
  discovery, and single completion.
- `tests/zombies_coop_ui.lua` and `tests/zombies_coop_lobby_paths.lua`: actual candidate
  Lua against mocked engine entrypoints, target/mode guards, authoritative capacity,
  Solo and listing actions.
- `tests/zombies_coop_lobby_loader.py`: source/structure and bounded client LobbyVM
  loader checks; not an actual client execution test.
- Ezz `tests/connection_request.cpp`: stale actions, replacement/cancellation,
  cross-thread DNS tokens, and reconnect; `tests/nat_join_policy.cpp`: strict join
  request/endpoint/nonce policy.
- Ezz additionally uses `tests/zombies_coop_join_network.cpp` for native network-mode
  isolation and LAN request kinds.

Keep `tests/gsc/eroded_coop_selftest.gsc` isolated from production data. A fixture
passing in the game VM proves only the exercised helper behavior and startup assertions;
it does not substitute for real remote clients.

Use `-noupdate` when testing an exact experimental executable/data pair so the normal
updater cannot replace it with a different published revision during the test.

## Required real-client matrix

Run the applicable rows at **five, six, seven, and eight actual clients**, separately
for CB and Ezz, on **both `zm_zod` and `zm_tomb`**. Use the exact candidate
executable/data and record each client's build/data revision. Cross-family CB/Ezz tests
are an additional matrix, not inferred from separate-family success. Mark every unrun
cell Unverified.

| Scenario                                                                    | Required observations                                                                                                                                             |
| --------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Online custom 402, unlisted                                                 | Owner hosts; existing friends/invites join through normal native policy; no automatic public listing                                                              |
| Explicitly listed custom 402                                                | User enables listing; independent internet client sees selected map/count/capacity, selects it, joins correct host                                                |
| Staging 400 to Public 401                                                   | Preserve native allocation and party carry; do not invent backend matchmaking or silently lower an existing capacity                                              |
| Staging 400, Solo playlist 150                                              | Saved preference eight still launches stock Solo; existing 1-4 and non-Solo staging paths remain correct                                                          |
| Party carry into a selected custom game                                     | All intended party members arrive once, capacity is not reduced by a member's saved four, and overflow fails cleanly                                              |
| Late joins in slots 5-8                                                     | Names, profiles, loadouts, HUD, score rows, points events, spectating, and snapshots remain correct                                                               |
| Down/revive/bleedout in every extra slot                                    | Matching fields and timers, accurate player names/markers, cleanup and slot reuse; no unregistered-field errors                                                   |
| Map selection before launch                                                 | Change selected map/gametype, refresh row, join; reported frontend metadata and launched game agree                                                               |
| Unsupported map or active gameplay mod                                      | Host does not advertise as a supported expanded custom game; startup/join fails with a recoverable explanation                                                    |
| Privacy Open to Closed/Friends-only                                         | New listing/discovery eligibility is removed; stale entries cannot bypass the current privacy decision                                                            |
| Online unlist and LAN discovery off                                         | New relevant publication/replies stop; no privacy mutation; stale rows are revalidated                                                                            |
| Next-size setting or content changes while a row is selected                | Native 5-8 capacity stays frozen even with a fresh UI helper or different saved preference; a new size applies after close/recreate; stale content is revalidated |
| Cancel/refresh/release while discovery or a not-yet-started join is pending | Old generations cannot populate the replacement list, complete twice, flip its network mode, or start the superseded join                                         |
| Cancel after the engine join FSM has already begun                          | No second FSM is started and stale completion is ignored; record that the first native join may still finish because no abort API is verified                     |
| Same physical LAN                                                           | Discover multiple hosts, select the correct native LAN game, join 5-8, and verify live online/MP joins are unaffected                                             |
| Same-PC loopback and multiple game ports                                    | No duplicate completion, false remote exemptions, wrong host, or stuck browser request                                                                            |
| VPN/local routed address                                                    | Record actual network/address scope, whether discovery works, and whether explicit/native joining works; do not conflate them                                     |
| Rehost or restart on the same address/port                                  | Old entries/callbacks cannot enter the new session with stale identity, capacity, readiness epoch, or map                                                         |
| Disconnect/rejoin and 8 to 4 to 8                                           | Correct slot reuse, live round quota, explicit respawns/recycling, rewards, and no stale ownership callbacks                                                      |
| Host leaves during an expanded match                                        | Migration is refused; clients exit/recover cleanly rather than continue with stale controller state                                                               |
| Normal rotation and interrupted shutdown                                    | Admission closes before VM teardown; stale success never reopens; fresh epoch/arm succeeds                                                                        |
| Missing, legacy, wrong-version, or mismatched client data                   | Reject before incompatible schema/snapshot use, or record the unresolved failure; capacity metadata alone is not a pass                                           |
| Default four, stock Solo, MP/campaign, theater, foreign 18-slot mods        | Baseline behavior, local-player limits, and existing wire/layout remain unchanged                                                                                 |
| Special rounds, boss/quest encounters, machine purchases, long play         | Per-map rules and cleanup work beyond an empty startup; no two-per-player machine rule is introduced                                                              |

Dedicated sound-on and `-nosnd` canaries remain a separate prerequisite: fresh isolated
prefix/runtime, exact artifact hashes, both allowed maps and return rotation,
readiness/GSC heartbeat checks, complete clean shutdown, and no live-instance
deployment. They cannot prove the client-facing rows above.

For each run record source commit, executable SHA-256, data hashes, role/network mode,
map/gametype, native capacity/population, entrypoint, privacy/listing state, expected
result, actual result, and relevant error/epoch logs. Redact private identifiers before
attaching public PR evidence. An unrun scenario is a limitation, not a successful test.
