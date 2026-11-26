# Official map profiles and inherited stock compatibility

These are source-level experimental adapters. A profile is not a certificate for
occupied eight-player gameplay, all quests, network schema, or arbitrary custom
map scripts. Native allocation and early paired GSC/CSC compatibility remain
separate prerequisites. Keep each build family's executable and data together.

## Pinned inventory

The static review uses `ate47/bo3-source` commit
`fe040b1187b4850fe8da3c78b703c986c87d1028`. Map-only imports stay under their
own `custom_scripts/<map>` directory. The common controller imports no stock
map module. Private read-only AU1 inventory found all fourteen root map FF files;
file presence does not establish dependency completeness or successful loading.

| Map               | Identifier      | Source-owned profile / special paths preserved                                                                                           |
| ----------------- | --------------- | ---------------------------------------------------------------------------------------------------------------------------------------- |
| Shadows of Evil   | `zm_zod`        | Map pacing `function_59804866`; mixed parasite/raps `function_33aa4940`; stock opening-round delays                                      |
| The Giant         | `zm_factory`    | Mixed dog callback `zm_factory::function_33aa4940`; normal and dog rounds remain separate                                                |
| Der Eisendrache   | `zm_castle`     | Character module `zm_castle_characters`; mixed dog callback `zm_castle_dogs::function_33aa4940`; dog round ownership retained            |
| Zetsubou No Shima | `zm_island`     | Mixed spider callback `zm_island_spiders::function_33aa4940`; special spawners and map challenge/portal fields remain separate contracts |
| Gorod Krovi       | `zm_stalingrad` | Mixed mangler/valkyrie callback `function_33aa4940`; custom round-wait function retained                                                 |
| Revelations       | `zm_genesis`    | Normal stock pacing; chaos-round spawn/wait replacements retain their own budgets                                                        |
| Nacht der Untoten | `zm_prototype`  | Stock normal-round seams and map character setup                                                                                         |
| Verruckt          | `zm_asylum`     | Stock normal-round seams; map's own start locations and character setup                                                                  |
| Shi No Numa       | `zm_sumpf`      | Stock normal-round seams; dog-round spawners retain ownership                                                                            |
| Kino der Toten    | `zm_theater`    | Stock normal-round seams and mixed actor setup; this map ID is distinct from Theater replay mode                                         |
| Ascension         | `zm_cosmodrome` | Stock normal-round seams; monkey/special round ownership retained                                                                        |
| Shangri-La        | `zm_temple`     | Known `zombie_total_set_func = function_f10deff8` initializes special thresholds; it is recognized, never rerun by population adaptation |
| Moon              | `zm_moon`       | Known `moon_round_think_func`; No Man's Land and the resumed saved round keep map-owned quotas; later fresh normal rounds may adapt      |
| Origins           | `zm_tomb`       | Stock normal-round seams; `give_personality_characters`; stock timed-out-zombie queue recycling                                          |

Base/bonus maps are Zod and Factory. Awakening, Eclipse, Descent and Salvation add
Castle, Island, Stalingrad and Genesis. The remaining eight are Chronicles.
`tests/zombies_coop_maps.json` records the exact expected map-only callback
identities used by both build families. Same export names do not fingerprint
modified stock function bodies.

## Live population and round ownership

The session limit is a ceiling, not the current gameplay population. Counts come
from real `getplayers()` and the stock `level.players` roster, including bots,
humans, and downed participants. There is no synthetic human count, fixed-eight
round population, or bot allowance outside the native total capacity. A dedicated
match with eight bots and a listen host with seven bots both total eight entities.

Normal-round quota keeps already consumed spawns and already spawned AI through
8 to 4 to 8 changes. Future stock targets and pacing use current stable membership.
Zero players do not advance or rewrite a round budget; a roster update in flight
uses a safe delay until the two authoritative player views agree. Explicit
respawns and cumulative timeout recycling are preserved separately. Conservative
polling cannot recover exact per-AI event order, so it may retain possibly queued
recycled quota after a shrink. Concurrent AI/actor limits are not raised.

A temporary nonstock `round_spawn_func` stops normal quota adaptation. Known
mixed-AI callbacks remain called by stock; the addon observes their quota
consumption rather than spawning replacements itself. Unknown round thinkers,
total-set or mixed-AI callbacks retain ownership. Special rounds keep their stock calculations
and schedules; this does not promise a new live-resize implementation for every
special controller. Temple's initializer resets monkey counters and derives
napalm/screecher/monkey thresholds, so calling it on a membership change would be
incorrect and is explicitly avoided.

Moon's thinker restores `prev_round_zombies` after starting the normal spawner,
and clears `teleporter_used` before that assignment. A restored decrease must not
be counted as actual spawning, and an equal restored value does not prove a fresh
quota. The exact Moon thinker is recognized only by its map profile. Adaptation
declines in No Man's Land, during a pending teleport, and for the saved resumed
round (`_from_nml` with the current round at or below `nml_last_round`). Missing
on-moon/teleport state or an invalid saved round during resume also declines.
This guard is checked before capture and again before any
worker accounts or rewrites counters. A later fresh normal round above the saved
round can adapt; the resumed round is never reconstructed from its pending quota.

## Repeated crew, respawns, and bots

The known map character callback is wrapped only if it still exactly owns the
stock seam. A newly unassigned player above four gets the least occupied of the
four loaded crew indices. Respawns keep an existing selection, and occupancy is
recomputed from actual connected entities after departures/rejoins. The map still
performs its own body, voice, hands, loadout and animation setup; no unloaded crew
assets are invented.

The stock callbacks also assign `level.rich_sq_player`, and on several maps
`level.sndradioa`. A duplicate character must not steal a connected owner's
singleton quest/radio role. The wrapper restores only those specifically reviewed
fields, only when the stock callback replaced them with this player, and only
while the previous owner remains connected. It does not infer arbitrary mod
singletons, distribute all character-specific quests to eight independent owners,
or certify every Easter Egg.

The spawn callback uses a bounded navigation query only for a real overlap above
four players. Candidate points require a player volume, nearby floor height,
player clearance, telefrag rejection and an unobstructed trace. The worker has a
spawn generation and map epoch and rechecks ownership before moving a player.
Where geometry cannot establish a safe nearby point, it leaves coordinates alone
and supplies a short 2.5-second intersection grace through a delegating wrapper.
This is not permanent invulnerability or a broad removal of map collision rules.
Actual geometry, simultaneous spawns and vehicle/quest transitions need runtime
tests on every map.

For actual entity slots 4–7 only, an engine-created empty or five-null GobbleGum
pack may receive five existing nonconsumable classic gums. Real packs, custom pack
shapes, slots 0–3, missing gums and maps without gums are left alone. No consumable
inventory, online loot, or real profile is fabricated. This script fallback does
not repair native out-of-bounds bot/profile code; native bot initialization needs
its own verified bounds.

## Machines preserve their actual stock rules

Ordinary multiplayer perk vending is not universally a four-purchase finite
stock. Solo Quick Revive has its own three-life/expiry logic. The addon does not
turn unlimited perk machines into sixteen-use machines, raise solo lives, impose
a two-per-player quota, or reset machine state after population changes.

GobbleGum's three per-player uses per round, its move-frequency table, a profile's
five-item pack and finite consumable inventory are different systems. Existing
map/mod movement overrides retain ownership. The existing bounded movement fallback
uses actual population and stock four-player ranges above four. Any real finite
machine adapter must identify its own purchase/rollback/reset callback and retain
consumed stock; policy scaffolding alone is not an installed machine adapter.

## Existing custom maps without author changes

A custom map inheriting the known stock player-check and spawn-delay seams can use
the common compatibility layer without requiring an official map name. Its custom
character, special-round, machine and quest functions retain ownership. Unknown
core player/spawn-delay callbacks are reported and preserved instead of guessed
or invoked with a temporarily forged four-player roster. Compatible early paired
fields, native capacity/admission, content policy and active module fingerprints
are still required before admitting expanded clients.

No generic layer can infer that `custom_quest_state[player_slot]` has four
entries, which literal four-button quest should count only a subset, or what an
unknown per-character singleton means. Registering matching fields for known
inherited modules does not discover every arbitrary custom field name/width.
Such a map can require an explicit adapter or decline expanded startup with the
logged ownership reason. This is a concrete compatibility boundary, not universal
certification of all existing Workshop maps.

## Diagnostics and validation

Rare state changes emit bounded `[Zombies coop] phase=...` lines, throttled to
at most one every two seconds. Diagnostics contain no names, tokens or positions.
The heartbeat publishes actual population once per second; normal-round numeric
snapshots are updated at most once per second plus population changes:

- `zm_coopMapAdapter`, `zm_coopOwnerStatus`, `zm_coopLastEvent`
- `zm_coopPlayers`, `zm_coopRoundTarget`, `zm_coopRoundConsumed`, `zm_coopRoundPending`
- `zm_coopSpawnStatus`, `zm_coopGumStatus`, existing readiness/epoch/heartbeat/status dvars

`tests/zombies_coop_maps.py` checks the fourteen source profiles, map import
isolation, ownership gates, real-entity population use, bounded spawn checks and
pack boundaries. It does not execute GSC or a client. The isolated actual-VM
fixture adds pure roster/slot/gum matrices, 0-to-8 and 8-to-4-to-8 occupancy,
initializer non-invocation and temporary special-spawner ownership cases; it is
test-only and must be compiled/run separately. These newly added scenarios have
not yet been demonstrated in occupied gameplay. Native eight-bot and mixed
human/bot joining, slot reuse/reconnect, loadouts, every map's quests, all machines,
paired replication, client HKS, long play and clean shutdown remain runtime gates.
