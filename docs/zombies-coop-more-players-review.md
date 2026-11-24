# More Players reference review

## Source and attribution

Reviewed on 2026-11-24: ShawnDuPreez's **More Players (Black Ops 3 zombies)**,
submitted through
[CBServers/mods-index issue 34](https://github.com/CBServers/mods-index/issues/34).
The reviewed source is pinned to
[`0b2eafd75aacc451d458e781caf3dec18b228b22`](https://github.com/ShawnDuPreez/cb-mods/tree/0b2eafd75aacc451d458e781caf3dec18b228b22/BO3/more-players).
The source repository and both BOIII forks declare GPLv3; retain the upstream
license, creator attribution and pinned provenance for adapted additions. See
the
[upstream license](https://github.com/ShawnDuPreez/cb-mods/blob/0b2eafd75aacc451d458e781caf3dec18b228b22/LICENSE)
and
[upstream README and stock-source credits](https://github.com/ShawnDuPreez/cb-mods/blob/0b2eafd75aacc451d458e781caf3dec18b228b22/BO3/more-players/README.md).

This review does not install the external mod or enable its complete script set.
The changes adopted here are a bounded HUD colour fallback, scoreboard row-count
adaptation, and a stock GobbleGum movement-frequency fallback above four
players. They do **not** fix the missing revive/score replication contracts or
complete an eight-player gameplay implementation. The PR remains experimental
and draft.

## Bounded additions in this PR

- `data/ui_scripts/zombies_coop_hud/__init__.lua` provides colour fallbacks for
  additional advertised slots and adjusts the scoreboard data-source row extent
  for an active GAME lobby with five through eight slots. It does not set lobby
  capacity, create extra lower-left score-list widgets, or implement revive
  icons. `tests/zombies_coop_hud.lua` checks the candidate Lua against engine
  stand-ins; it does not establish that every retail HUD layout works with real
  clients.
- The common controller's `bgb_movement_limits(count)` and
  `bgb_movement_limits_for_players(players)` validate the player-count input.
  `install_bgb_movement_hook()` preserves an existing custom override; its own
  `bgb_machine_movement_frequency()` uses the stock four-player movement range
  for counts above four while preserving the stock one-through-four table. This
  is machine _movement_ pacing, not a player loadout, GobbleGum inventory,
  finite-machine stock or purchase-limit implementation.
- `tests/gsc/eroded_coop_selftest.gsc` covers the bounded GSC helpers and custom
  callback preservation. It remains test-only and must not ship as normal data.

The movement callback also checks the current map/epoch, callback ownership, and
configured capacity before writing its two movement-range values. The table was
checked against the
[pinned retail `_zm_bgb_machine.gsc`](https://github.com/ate47/bo3-source/blob/fe040b1187b4850fe8da3c78b703c986c87d1028/scripts/zm/_zm_bgb_machine.gsc#L875-L901);
the Mod Tools mirror exposes a protected-function stub for this function.

Ezz runtime source is `data/custom_scripts/zm/eroded_coop.gsc`, compiled by its
own loader. Keep that source format separate from CB's precompiled payload.

## Paired replication schema required before extending slots

Game player slots are zero-based: slots 4-7 correspond to players 5-8. The
reference registers these fields early on **both** the server GSC and client CSC
sides. Names, versions, types and widths must match the frozen network schema.
They cannot be selected from each joining client's saved capacity preference or
from a changing connected-player count.

| Scope   | Map             | Field for each extra slot `i`                | Version | Width                     | Type / client callback                           |
| ------- | --------------- | -------------------------------------------- | ------- | ------------------------- | ------------------------------------------------ |
| `world` | All             | `laststand_update<i>`                        | `1`     | `5`                       | `counter`; `zm_laststand::update_bleedout_timer` |
| `world` | `zm_castle`     | `player<i>hasItem`                           | `1`     | `1`                       | `int`; `zm_utility::setSharedInventoryUIModels`  |
| `world` | `zm_stalingrad` | `player<i>wearableItem`                      | `12000` | `2`                       | `int`; same inventory callback                   |
| `world` | `zm_genesis`    | `player<i>wearableItem`                      | `15000` | `4`                       | `int`; same inventory callback                   |
| `world` | `zm_moon`       | `player<i>wearableItem`                      | `21000` | `1`                       | `int`; same inventory callback                   |
| `world` | `zm_island`     | `pillar_challenge_<i>_<k>` for `k = 1, 2, 3` | `9000`  | `GetMinBitCountForNum(4)` | `int`; no callback in the reference              |
| `world` | `zm_island`     | `portal_state_ending_<i>`                    | `9000`  | `1`                       | `int`; no callback in the reference              |

For the revive fields, the CSC also initializes `level.laststands[i]` and its
bleedout timer members. Its system registration depends on `zm_laststand`, whose
initialization would otherwise replace that list. The reference uses autoexec
`system::register` initialization, reached through stock script imports, rather
than waiting for players to connect or for normal round callbacks to appear. Our
common controller's delayed `install()` is not a proven place to add fields
after registration has frozen.

The paired implementations are
[`zm_more_players.gsc`](https://github.com/ShawnDuPreez/cb-mods/blob/0b2eafd75aacc451d458e781caf3dec18b228b22/BO3/more-players/more_players/scripts/zm/zm_more_players.gsc)
and
[`zm_more_players.csc`](https://github.com/ShawnDuPreez/cb-mods/blob/0b2eafd75aacc451d458e781caf3dec18b228b22/BO3/more-players/more_players/scripts/zm/zm_more_players.csc).
The table records a compatibility contract for a future paired implementation;
these fields have **not** been integrated by the bounded additions above. It
also does not expand this PR's current Zod/Tomb map allowlist.

## Score pop-up crash path is separate from scoreboard rows

The reference's `_zm_score.gsc` still registers stock `clientuimodel` counters
for slots 0-3 only. The field family is `PlayerList.client<i>.score_cf_<event>`,
version `VERSION_SHIP` (`1`): `damage` uses `GetMinBitCountForNum(7)` and
`death_normal`, `death_torso`, `death_neck`, `death_head`, `death_melee` use
`GetMinBitCountForNum(3)`. Client-side registration belongs to
`ui/uieditor/clientfieldmodels.lua`.

A later `score_cf_increment_info` call for an extra slot can therefore name an
unregistered field even if its scoreboard row and colour look correct. The
reference avoids that path by guarding the sender for slots above the stock
range; it does not implement all of those extra pop-up fields. Its documented
tradeoff is missing normal per-event points pop-ups for players 5-8. Our new HUD
module does not patch that sender or the client field-model file. A future
change must either implement matching field models on both sides or use an
explicit, verified sender guard. Do not assume a wider scoreboard fixes this
server-side crash path or replace the entire stock score script without checking
its exact retail source and compiled dependencies.

## Client loading, ownership and compatibility gaps

Both forks currently load custom scripts through the server load callsite in
`src/client/component/script.cpp`, at client-image address `0x141AA255F` or
dedicated-image address `0x1402D81FF`. That is not a verified global client-VM
bootstrap. A successful dedicated startup cannot demonstrate that a joining
client has loaded a matching CSC before its clientfield schema freezes.

Ezz recognizes `.csc`, queues it for `SCRIPTINSTANCE_CLIENT` on a client image,
and compiles both extensions. However, the current population/execution entry
point is still the server load callsite; recognizing the suffix does not prove
that a joining client reaches it before CSC initialization. Its shared script
cache and execution queue also need a per-instance lifecycle review before a
global load hook is considered.

The external mod instead supplies `zm_mod.ff` assets. Its stock `_clientids.gsc`
import loads the server addon, and its stock `_zm_score.csc` import loads the
CSC addon through the game's own mod loading. Every participant must load its
game scripts and Lua. The reference's fixed extended schema also applies to
four-player sessions _inside that mod_; that is different from proving vanilla
four-player network compatibility when no companion is loaded.

Our native expanded-session preflight permits the builtin `usermaps` bootstrap
but rejects other selected/active gameplay mods and loaded usermaps. The common
round controller also owns callbacks that the external mod wraps. Loading both
systems together is unsupported: do not whitelist `more_players` merely by name
or bypass the preflight to stack them. An authenticated, matching companion
schema or an explicitly supported mod integration needs its own implementation.

## Revive-marker and character limitations

The reference reserves objective indices `116 + slot`, therefore `120-123` for
slots 4-7, and uses the game's revive widget on those objectives. Being near the
end of a 128-objective pool does not prove those IDs are unused by every map,
boss encounter or another mod. A port needs objective ownership/reservation and
lifecycle cleanup so an old disconnect cannot hide a newly reused objective. No
objective-based revive marker is adopted here.

The reference also aliases extra `level.player_name_directive` entries with
`slot % 4`. That avoids an undefined four-entry lookup, but can show another
slot's name in the revive message. Its repeated-character balancing does not
establish independent quest or character ownership. Its own revive-icon limits
include no off-screen direction arrow and the existing crew models/voices only.
These are compatibility limitations, not verified fixes in this PR.

## What evidence exists

The author's README reports a host plus seven bots across 14 maps. Seven maps
are listed on the final scripts, six on earlier scripts before the added revive
icon, and Moon only at startup without completing three rounds. The reported
15-round Giant run also predates the revive icon. These are the author's
reported results, not tests rerun by this PR. The author explicitly has not
tried real people in slots 5-8, late network joins without the mod, or all long
games, quests and bosses. Bots do not supply independent profiles, loadouts,
HUDs or network connections.

Our own checks are listed in the PR description and private build/test report.
The bounded Lua tests, GSC fixture and server-only canaries are separate from
the author's bot runs. They do not establish real-client revive, score-field,
character, spectating or join/reconnect compatibility.

## Concrete next steps

1. Define and negotiate the companion schema before admitting an expanded
   connection. Reject missing/incompatible client data before it can use the
   session; do not infer compatibility from lobby size or a local saved dvar.
2. Establish early server and joining-client bootstrap. For an engine-native
   companion mod, verify its assets and integration ownership; for a loose CSC
   loader, verify the client load seam, per-instance caches and allocator
   lifetime, shutdown/reload behavior, and dedicated exclusion before adding
   hooks.
3. Implement and compare the complete frozen GSC/CSC schema, including the
   client-side revive state and each supported map's field sizes and capacity.
   Preserve vanilla schema when the feature is inactive. Verify retail source
   fingerprints before any stock script replacement or function detour.
4. Resolve the score pop-up sender/client-model contract, objective collisions,
   accurate revive names, character/quest ownership and remaining HUD widgets.
5. Test real matching clients in slots 5-8, mixed/missing data rejection,
   down/revive, points awards, disconnect/rejoin, map rotations, special/boss
   rounds and long games before marking either PR ready. Keep the map allowlist
   until each map's schema and gameplay adapter have actually been validated.
