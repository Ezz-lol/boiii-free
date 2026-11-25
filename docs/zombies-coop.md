# Experimental dynamic Zombies coop

This draft adds opt-in capacity from one to eight. Default sessions remain four;
expansion requires `-experimental-zombies-coop` and the matching executable/data.
Eight configured slots do not mean eight active participants. Gameplay reads the
current roster, including native test clients and downed players. A session keeps
its allocated network fields across eight -> four -> eight; it never resizes the
replicated schema halfway through a match.

## Lobby and schema lifecycle

The host's next-lobby selector covers the reviewed online staging/Public/Custom
and native LAN/system-link targets. Native already allocated capacity wins over
saved preferences. Solo retains its stock ceiling. Rehost to change allocation.
Browsing means explicitly selecting a custom host, not an automatic queue.

The early companion bundle consists of paired precompiled BO3 PC 1C GSC/CSC
fields and a primary-UI score-model constructor. Each executable pins their hashes
and common revision/contract. It rejects partial or stale installation roots.
The existing script asset lookup seam appends one include to a fresh retained
copy of known stock anchors and the selected map root; it preserves the original
bytecode and opaque source CRC. No new client offset or checksum bypass is added.
Fresh copies isolate repeated lookup/link mutation and remain under a memory cap.

The native schema allocation is read-only to scripts/console. A verified join
descriptor can prepare it before a frontend mode change. Native actual GAME
state prepares eligible local hosting before CSC/UI loading. Pending requests
expire or cancel; active matches retain allocation through travel. Solo staging
does not prefreeze an expanded schema.

The UI wrapper runs the stock clientfield-model chunk first and extends only its
six exact score-event counters for slots 4-7, after checking versions, widths and
types. Paired scripts add stock-compatible revive state, score counters and the
selected official map's extra-player field families. Execution receipts are
diagnostics, not authentication. Expanded remote direct-connect also needs a
bounded signed capability tail bound to the challenge, native connect command
and schema token. Existing identity/lobby/privacy checks still apply. The engine's
native clientfield checksum remains enforced; signed metadata is not attestation
of arbitrary custom scripts or proof the client executed CSC/UI.

## Gameplay and maps

All fourteen official map roots have separate stock callback profiles and scoped
dependencies; see [map adapter contracts](zombies-coop-map-adapters.md). Compatible
custom maps can inherit the shared stock APIs without rebuilding the map. Unknown
custom callback owners/gametypes decline adaptation rather than being overwritten.
Hard-coded four-player quest arrays, custom HUDs and arbitrary map-owned scripting
cannot be made universally safe by changing the native capacity alone.

Normal-round targets and spawn delay follow actual current participants. Consumed
spawns and living AI remain accounted for after a shrink; they are not deleted to
match a smaller target. Explicit respawns and timeout recycling are tracked
separately. Conservative polling can retain queued recycling credits when exact
per-AI event ordering is ambiguous. Map-owned special-round setup is preserved and
never replayed just because someone joined or left.

The controller balances the stock four character archetypes and preserves known
connected singleton quest/radio owners. Extra-slot empty stock GobbleGum packs
get a fallback without replacing valid/custom packs. Spawn separation uses bounded
stock volume/navigation/trace checks with a short owned grace period. Stock
multiplayer perk machines usually have unlimited stock; this does not impose a
new universal sixteen-buy limit or alter solo Quick Revive rules.

## Bots and diagnostics

`spawnBot` counts occupied native slots, including loading humans, and never
exceeds allocation. Dedicated eight bots and listen host plus seven share the same
eight-slot ceiling. Queued batches recheck map generation, mode, capacity and
schema/admission readiness before every addition. Native bot entities participate
in ordinary GSC population counts; this does not add a combat-AI mod.

See [debugging and private crash reports](zombies-coop-debugging.md). Last-phase
events narrow an investigation; they do not by themselves establish crash cause.
Test-only fixtures and bot probes belong outside production data.

## Validation boundaries

Native policy/unit tests, Lua 5.1 stand-ins, ACTS compilation and dedicated canaries
exercise different contracts. Report each against its exact source/binary/data
pin, including failed attempts. An empty dedicated pass does not prove client CSC,
HKS, HUD, actual remote clients, Public backend allocation or LAN gameplay.

No supported BO3 PC client installation is available for this work. Client loader
code uses existing source-backed seams, but actual client execution, mixed CB/Ezz
joins, occupied network reconnects, all-map quests/bosses and arbitrary Workshop
maps remain unverified. Keep both PRs experimental until those checks exist.
No live deployment is part of this change.
