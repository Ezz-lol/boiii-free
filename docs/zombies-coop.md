# Experimental dynamic Zombies coop

This is the first implementation stage, not a release-ready eight-player mod.
The default remains four players. Above-four lobby settings require
`-experimental-zombies-coop` and matching client/server code and data.

## Implemented

- `zm_coopPlayerLimit` selects 1–8 before hosting; without the experimental
  launch flag its registered maximum is four.
- Zombies lobby targets and native host actions use this setting. Solo, MP and
  campaign targets retain their normal limits. A joining client follows the
  server's advertised capacity instead of restoring its local four-player
  setting.
- The initial gameplay controller uses the stock player-check and spawn-delay
  callbacks. It does not replace the whole retail `_zm.gsc` or bypass the check.
  It enforces the configured limit and defines pacing for five through eight.
- Normal-round accounting uses the stock count formula and current connected
  participants. It remembers consumed spawns across population reductions and
  increases, preserves replacement spawns and leaves concurrent AI/actor caps
  alone. Existing zombies finish naturally. Downed players remain participants.
- Custom spawn controllers, custom callbacks and unexpected budget additions
  retain ownership; the controller stops adapting them.
- Threads end on round restart, round end, game end or intermission. An
  observable `zm_coopStatus` and `zm_coopHeartbeat` describe script startup.
- Independent native policy tests cover slot generations, stale reconnect/map
  callbacks and 16-use shared machine stock. **The native roster and
  finite-stock classes are scaffolding; actual map-machine adapters are not
  connected yet.**

Initial adapters are restricted to `zm_zod` and `zm_tomb`. Other maps reject an
above-four setting during script startup with a recoverable game error. This
allowlist is a testing boundary, not a claim that occupied gameplay is
validated.

The native preflight permits the engine's exact `usermaps` bootstrap used by
stock dedicated lobbies. Other selected or active Workshop mods and loaded
usermaps are rejected. This checks native content selection, not file integrity:
modified fastfiles or files placed under the builtin `usermaps` directory are
not fingerprinted. Matching callbacks and unmodified stock assets remain
required.

## Ezz script packaging

Ezz compiles `data/custom_scripts/zm/eroded_coop.gsc` with its own script
loader. This is readable source, not CB's precompiled payload. Keep each build
family's binary and data together; do not distribute CB's binary script as Ezz
source. The equivalent source has also passed the BO3 revision-1C ACTS compiler
syntax check, which does not replace validation with Ezz's own compiler at
runtime.

Shadows of Evil keeps its stock opening-round pacing and known parasite/raps
callback. Its map-specific imports load only from `custom_scripts/zm_zod`.

## Local checks

In an x64 Visual Studio Developer shell:

```powershell
cl /nologo /std:c++20 /EHsc /W4 /WX /Fe:coop-policy.exe tests/zombies_coop_policy.cpp
.\coop-policy.exe
lua5.1 tests/zombies_coop_ui.lua .
```

The Lua test exercises actual candidate Lua modules against a mocked engine; it
is not an in-game lobby test. The C++ tests check policy behavior, not the
engine's native client arrays or wire protocol. Existing 18-slot MP structures
are deliberately unchanged.

## Required before marking the PR ready

- Real joins with five, six, seven and eight matching Ezz clients; mixed-build
  compatibility requires its own proof.
- Disconnect/rejoin and eight → four → eight occupied-round tests, including
  special rounds, boss encounters, reconnect rewards and empty-server behavior.
- Eight-row HUD, lobby, scoreboard and spectator verification; independent
  character/quest ownership when visual crew members repeat.
- Confirmed per-map finite-machine stock adapters (16 shared purchases, no
  two-per-player restriction). Preserve unlimited machines and solo rules.
- Remaining map adapters and custom-map compatibility rules.
- Sound-on and `-nosnd` isolated map rotations with complete clean shutdown.
- Source/image compatibility guards where new native hooks become necessary.

Do not promote this work based on a successful compile, policy tests or an empty
eight-slot server alone. Bug/security backports are a separate change set.

## Crash and lifecycle review follow-up

The startup gate now rejects remote expanded-game admission until the current
map acknowledges the native epoch and installed callbacks. Failed/missing
adapters and lost ownership stop expansion from the main frame, after scheduler
and feature-state locks have been released. Native loopback bootstraps listen
hosts. The native 18-slot ABI and normal four-player protocol are unchanged.
Capacity is applied per hosted session; changing it requires a frontend rehost
or dedicated process restart. Expanded startup rejects active Workshop sessions
and mismatched host capacity instead of enlarging a potentially allocated pool.

The gate allows normal paused GSC time: a stationary heartbeat alone is not a
failure. This also means it does not prove every script thread remains alive.
Same-export modified stock scripts still need image/fingerprint coverage.
Matching-client capability negotiation, trusted-admin bot insertion, occupied
five-to-eight-player joins, HUD, spectators and character/quest behavior remain
unverified. These are draft changes, not a completed compatibility
certification.

`tests/zombies_coop_startup.cpp` checks epochs, cold loads, timeouts, decline,
pause and readiness revocation. `tests/gsc/eroded_coop_selftest.gsc` is an
isolated actual-VM fixture; keep it out of production data. It exercises the
runtime arithmetic and duplicate-startup behavior, publishing
`zm_coopSelfTestStatus` and `zm_coopSelfTestFailures`.

The selected native `com_maxclients` determines stock Solo/Public capacity even
when the saved next-custom-lobby preference is eight. Above-four sessions must
match that preference; no already allocated capacity is enlarged. The builtin
`usermaps` bootstrap is permitted, while active Workshop maps/gameplay mods are
rejected. This does not fingerprint modified builtin assets or stock scripts.

Stock timeout recycling is tracked using cumulative `zombie_total_subtract`
deltas, separately from `zombie_respawns`. A polling sample cannot order every
spawn/recycle event, so the controller conservatively retains the maximum
possibly queued replacements when membership shrinks. It can retain extra
recycled quota after a shrink; exact per-AI event attribution remains
unverified. Unexplained additions and counter resets still relinquish ownership.

`tests/zombies_coop_stock_session.cpp` covers stock bootstrap and Workshop
classification. Startup tests include all configured 1-8/native 1-18 pairs,
Solo/Public selection, and mismatched expanded capacity.

The isolated read-only-assets test also exposed an existing Ezz startup failure:
persisting the optional `zone/scriptgdb.zip` threw before script startup, and
the crash handler repeatedly resumed a noncontinuable exception. Archive output
failure is now logged once without discarding compiled scripts, and the handler
reports an unhandled noncontinuable exception once before returning it to the
OS. `tests/exception_recovery_policy.cpp` checks the exception flags and codes.
Gameplay startup still has to pass the real read-only-assets canary.

## More Players reference

The [pinned More Players review](zombies-coop-more-players-review.md) records
ShawnDuPreez's GPLv3 source, the bounded HUD and GobbleGum movement additions,
and the paired GSC/CSC registration and joining-client work still required.
These additions do not fix the extra-slot revive or score-field contracts and do
not make this a release-ready eight-player implementation.
