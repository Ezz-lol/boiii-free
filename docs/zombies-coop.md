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

## Ezz script packaging

Ezz compiles `data/custom_scripts/zm/eroded_coop.gsc` with its own script
loader. This is readable source, not CB's precompiled payload. Keep each build
family's binary and data together; do not distribute CB's binary script as Ezz
source. The equivalent source has also passed the BO3 revision-1C ACTS compiler
syntax check, which does not replace validation with Ezz's own compiler at
runtime.

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
