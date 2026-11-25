# Expanded Zombies HUD implementation

The candidate adds a bounded extension to the existing retail HUD for an active
Zombies GAME session whose advertised capacity is five through eight. The local
next-lobby preference is not the connected server's capacity. Multiplayer,
Campaign, frontend menus, stock capacities and unsupported capacities keep their
existing constructors and data.

## Lower-left scores

`data/ui_scripts/zombies_coop_hud/rows.lua` constructs up to four additional
`CoD.ZMScr_ListingSm` widgets, reusing the retail portrait, score, colour,
inventory and wearable bindings. It appends display positions four through seven
above the existing group, under `Listing4` so the original HUD start/hide alpha
continues to apply. It does not replace the original four rows or their clips.
The existing stock ZMScr module is required explicitly before first construction;
the module loader itself is never wrapped. Unrecognized replacement HUD shapes
are left intact.

The retail `PlayerListZM.prepare` iterates `Dvar.com_maxclients`, queries
`Engine.GetPlayerListData`, places the local player first, then compacts connected
players. Display position four can therefore represent native client three when
the local client is seven. The extension binds the retail numeric display model
and reads its actual `clientNum`; it does not equate display order with native
slot identity. There is a second guard against the real current connected count:
retail compaction can leave trailing numeric models containing stale connected
players after a disconnect. This guard hides those rows rather than duplicating
the remaining players. Connection notifications and model changes refresh the
bounded rows without a polling timer.

The additional display rows show aggregate changes in the engine's actual score,
using the stock positive/negative point-effect constructor and clips. Initial
values and identity changes establish a baseline; they do not animate another
player's old score. Each row permits at most eight simultaneous effects, closes
them on disconnect or identity change, and releases its subscriptions and children
when the HUD closes. These aggregate effects are distinct from the stock
per-damage/death counter effects retained for the original display positions.
The stock `LUI.UIElement.close` detaches an element before closing it in C; the
extension closes and detaches its effects and row children before their panes,
then calls the original HUD close. Existing fifth-row or extra-points widgets
cause this extension to decline rather than duplicate that recognizable layout.

## Scoreboard and replication

The existing bounded `ScoreboardWidgetCP` wrapper changes the first team's visible
row extent to the actual advertised capacity. The retail widget uses the common
`ScoreboardFactionScoresList`, whose first team binds `ScoreboardTeam1List`.
That datasource reads the engine's `scoreboard.team1.count` and indexed models;
it does not truncate the data at four. The extension does not fabricate players,
scores, names, lobby capacity or native replication pools.

The paired GSC/CSC field contract is separate from displaying rows. See
`zombies-coop.md` and the debugging guide for the early registration and
receipt checks. A dedicated-server schema receipt does not demonstrate that a
real client's HKS widgets, worldspace revive indicators or scoreboard rendered.

## Existing revive timer models

`revive.lua` wraps the existing `WorldSpaceIndicators.getModel` datasource and
creates only missing `bleedOutModel4` through `bleedOutModel7` countdown model
paths for the fixed expanded schema. Existing models and timer values are
preserved, including another mod's models. The paired CSC's existing
`zm_laststand::update_bleedout_timer` callback can then find its extra-slot
countdown destination safely.

This does not call `setupBleedOutWidget`, choose objective IDs or manufacture
entity/world-position bindings. The retail HUD's native continuation loop still
decides which worldspace indicators can be created. Its support for slots above
four remains a real-client runtime question. Model-path tests must not be
reported as proof of extra-slot revive arrows or reviver progress rendering.

## Source references and validation boundary

The implementation is independently authored against PC retail source snapshots
at `KingslayerKyle/T7LuaRepo` commit
`5f45af23887a1d850fba748c45a16ab746da3600`, dump `PC_Ship_2025-05-31`:

- [ZMScr constructor, numeric model bindings and point-effect APIs](https://github.com/KingslayerKyle/T7LuaRepo/blob/5f45af23887a1d850fba748c45a16ab746da3600/Dumps/PC_Ship_2025-05-31/ui/uieditor/widgets/HUD/ZM_Score/ZMScr.lua)
- [Stock small row](https://github.com/KingslayerKyle/T7LuaRepo/blob/5f45af23887a1d850fba748c45a16ab746da3600/Dumps/PC_Ship_2025-05-31/ui/uieditor/widgets/HUD/ZM_Score/ZMScr_ListingSm.lua)
- [PlayerListZM and ScoreboardTeam1List datasources](https://github.com/KingslayerKyle/T7LuaRepo/blob/5f45af23887a1d850fba748c45a16ab746da3600/Dumps/PC_Ship_2025-05-31/ui/uieditor/datasources.lua)
- [Zombies scoreboard constructor](https://github.com/KingslayerKyle/T7LuaRepo/blob/5f45af23887a1d850fba748c45a16ab746da3600/Dumps/PC_Ship_2025-05-31/ui/uieditor/widgets/Scoreboard/CP/ScoreboardWidgetCP.lua)

`tests/zombies_coop_hud.lua` checks the existing capacity, colour, ownership and
forwarding guards in actual Lua 5.1 with engine stand-ins.
`tests/zombies_coop_hud_rows.lua` checks the actual candidate rows, disconnect
holes, rejoin/slot reuse, score effects, limits and teardown. When provided an
external pinned retail `datasources.lua` as argument two, it executes that
file's real `PlayerListZM` implementation inside the model stand-ins. Retail
source is not copied into either repository.
`tests/zombies_coop_revive.lua` similarly executes the external pinned
`WorldSpaceIndicators` getter and checks timer-model ownership, missing models,
reload, forwarding and error behavior. Pass the external datasource path as
argument two when the local research reference folder is unavailable.

Actual matching CB/Ezz client HKS execution, occupied five-to-eight-player HUD
layout, split-screen, custom replacement HUDs, down/revive markers and rendering
remain unverified until tested in real clients. These source and Lua checks must
not be reported as occupied-client gameplay or rendering proof.
