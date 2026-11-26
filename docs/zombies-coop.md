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

The early companion bundle contains three precompiled BO3 PC 1C payloads: the
paired `eroded_coop_fields.gsc` and `.csc`, and the Island-only server companion
`eroded_coop_island_trials.gsc`. They ship under `data/coop_scripts`, alongside
the separately pinned primary-UI score-model constructor. Each executable pins
the complete bundle and schema revision/contract, rejecting partial or stale
installation roots. The existing script asset lookup seam appends reviewed
companion dependencies to a fresh retained copy of known stock anchors and the
selected map root. The Island server companion loads only from its verified
Island anchor. Original bytecode and opaque source CRC remain intact; no new
client offset or checksum bypass is added.
Recursive native linking receives a stable object for each canonical name
within an observed VM generation. A later generation copies the immutable
pre-link template; prior backing stays retained under the same bounded budget.
Exhaustion latches expanded admission closed until process restart. Valid
reserved/original assets let the native resolver unwind without a null asset.
The server BeginLoad seam is observed in the existing hook. Client generations
follow verified request/session/map boundaries; same-map CSC reload without a
new boundary is not established by source or dedicated tests.

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

The Island trials adapter expands the stock five/six/five task pools on demand
with independent five-field assignment records, bounded to eight per tier. It
keeps original selections while records remain and tracks exact player/slot/map
ownership through disconnect and rejoin. It reuses the retail task workers,
prompts, rewards, and shared altar. Its global unlock checks the completed current
roster and preserves a legitimately earned latch. Unknown callback owners or
foreign challenge state close readiness instead of being overwritten. An
empty-server installer yields once and observes stock registration under a
finite map/epoch/capacity guard; it never replays connected events or waits for
the player challenge flag. An already present local host still declines this
early path until listen-host dispatch ordering is verified. Original
four-player world references remain unchanged; extra private interaction frames
and the paired client renderer reuse twelve existing world state fields without
adding scriptmover bits or server locator actors.
See [Island trials and validation boundaries](zombies-coop-island-trials.md).
These new bindings require fresh isolated canaries and actual client interaction
checks; pool expansion and local helper tests do not certify the full quest.

Two additional lifecycle guards require the exact stock zclassic endgame and
board-melee callback owners before expanded readiness. On an effectively empty
native and listed roster, the endgame guard prunes only undefined or explicitly
disconnected placement entries; dead, downed and spectating players remain
present. The melee guard similarly removes stale player targets from its cached
array before chaining stock behavior. Both validate sparse slot keys and every
entry before assignment, preserve order and duplicates, and keep the original
callback context and argument. Each guard retains independent ownership, so a
custom takeover of the other callback cannot disable still-owned stock behavior.
Unknown owners remain untouched and close expanded admission. These guards do
not validate or replace map-owned barricade nodes, weapons or challenge rules.
The callback tests and compiled fixture are offline checks; fresh disconnect
canaries and occupied client gameplay remain necessary runtime validation.

The owned `zombie_total_set_func` setup callback captures the pristine stock
population/target/counters before starting a worker. A notification waiter can
resume after the first spawn and cannot establish that initial budget. The hook
accepts an absent stock owner or the exact known Temple initializer, which it
chains once per stock setup and never replays for a membership change. Hook
identity, saved original and map epoch are part of readiness; an unknown initial
owner or later takeover closes expanded admission.

That immutable seed survives
transient native-versus-script roster differences; event generation revokes old
workers even on a same-round restart. A missed or partial seed cannot establish
consumed quota, so it logs an explicit status and waits for the next owned event.
Normal-round targets and spawn delay follow actual current participants. Consumed
spawns and living AI remain accounted for after a shrink; they are not deleted to
match a smaller target. Explicit respawns and timeout recycling are tracked
separately. Conservative polling can retain queued recycling credits when exact
per-AI event ordering is ambiguous. Map-owned special-round setup is preserved and
never replayed just because someone joined or left.

Unknown round thinkers retain quota ownership. Moon's known thinker can restore
a saved budget after No Man's Land; that entire resumed round remains map-owned,
including restored decreases or equal values. Ordinary adaptation may resume on
a later fresh normal round above the saved round. A worker also stops before
accounting when Moon enters a teleport/No Man's Land transition. A defined
`kill_counter_hud` also preserves the map's budget at capture and before every
worker accounting step, including when it appears during an existing round.

Special dog-round totals and pacing remain stock snapshots; live join/leave
resizing is currently implemented for captured normal rounds, not dog-round
quotas. Their existing current-player concurrent-AI checks remain in effect.

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
