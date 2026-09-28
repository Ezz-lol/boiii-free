# 4-player local splitscreen (PC)

Ezz BOIII now includes a client component that raises the PC local-player limit
from 2 to 4 for offline splitscreen (Zombies / offline Multiplayer).

## Origin

Adapted from the public-domain (Unlicense) project
[LocalPlayer123/BO3-4-Player-Local-Splitscreen-on-PC](https://github.com/LocalPlayer123/BO3-4-Player-Local-Splitscreen-on-PC).

That component targeted `BlackOps3.exe` PE checksum `0x06517980`. Ezz launches
checksum `0x06531394`. This port:

- Retargets code RVAs at/above `0x1DFF150` by `-0x6C0` (measured from Ezz's
  symbol table vs the donor component)
- Leaves data/global RVAs unchanged (they matched)
- Uses `game::get_engine_base()` and `game::com::Com_IsRunningUILevel()`
- Refuses to patch unless `game::header_checksum() == 0x06531394`
- Keeps the donor's verify-before-write policy (mismatched sites are skipped)

## Files

| Path                                          | Role                                  |
| --------------------------------------------- | ------------------------------------- |
| `src/client/component/splitscreen.cpp`        | Engine patches / seats / hooks        |
| `src/client/component/splitscreen_reloc.hpp`  | Per-local-client array reloc tables   |
| `src/client/component/splitscreen_signin.hpp` | Guest sign-in helpers                 |
| `data/ui_scripts/zz_splitscreen/`             | Console-style A join / B leave lobby  |
| `data/ui_scripts/zz_table_insert/`            | `table.insert` nil-safe (Zombies HUD) |
| `data/ui_scripts/zz_mplan/`                   | Offline Multiplayer menu enable       |

Premake already compiles every `src/client/**/*.cpp`, so no project-file change
is required. UI scripts ship through the normal `data/` updater path into
`%LOCALAPPDATA%\boiii\data\ui_scripts\`.

## How to play

1. Use a current Ezz client with the matching `BlackOps3.exe` (`0x06531394`).
2. Steam Input **on** for Black Ops III; one controller per player.
3. Play Offline → Zombies (or Multiplayer with `zz_mplan` present).
4. Extra players press **A** to join, **B** to leave (or use Activate
   Splitscreen).

Online stays at two local players per PC (engine / Demonware rule).

## Known limitations (donor beta)

See the upstream `docs/OPEN_PROBLEMS.md` for the full list. Short version:

- Sun shadows on panes 2–4 can shimmer
- Panes 3–4 can show blocky indoor lighting on some maps
- Rare offline MP memory-corruption report while experimental sun4 was on
- Occasional silent exit during boot (retry launch)

Environment switches from the donor still work when set before launch, e.g.
`BO3_CG_FRAME=on` (verified config), `BO3_SPLITSCREEN=off` (kill switch),
`BO3_SKIP_FIX=...` / `BO3_PANES=...` for bisection.

## Build / test notes for reviewers

- Release|x64 client build is enough; no new dependencies.
- Confirm checksum gate: wrong `BlackOps3.exe` → component no-ops, 2-player
  only.
- Smoke: 2-player still works; then 3 and 4 players through a Zombies round on
  Shadows of Evil / The Giant / Der Eisendrache.
