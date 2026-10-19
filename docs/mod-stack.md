# Mod stacking

Mod stacking loads more than one Workshop mod at a time. One mod is the main
mod (`fs_game`), and the extra mods load on top of it, in priority order.

## Dvars

| Dvar | Meaning |
|---|---|
| `fs_game` | The main mod, as before. It always has the highest priority. |
| `fs_extramods` | Extra mods, as a comma separated list of Workshop publisher ids (or mod folder names), highest priority first. Empty by default. |

Extra mods only load when a main mod is set. With no `fs_game`, or with
`usermaps`, `fs_extramods` is ignored.

## Players

Open **Mods** from the main menu:

- Select a mod to add it to the stack, or select it again to remove it. The
  first mod in the stack is the main mod.
- **Q** / **E** (LB / RB) moves the selected mod to a higher or lower
  priority.
- **F** (right stick) loads the stack. **U** (Y) clears it.

When you join a server or a friend's lobby, the client loads the same mods in
the same order as the host. If you are missing any, it offers to download
them, and you can join once they are installed.

## Servers

Set both dvars on the command line, so they are known before any mod loads:

```
boiii.exe -dedicated +set fs_game 2631943123 +set fs_extramods "1733719429,3812744240" +exec server_zm.cfg +map_rotate
```

- Every mod in `fs_game` and `fs_extramods` must be installed in the server's
  `mods` folder, the same way a single mod is. The server warns at startup
  about any mod it can't find.
- The server advertises its stack in `getinfo` as `modStack`, so the server
  browser and joining players know what to load. Players with the mods from
  the Workshop get the download prompt automatically; mods that aren't on the
  Workshop have to be shared with players another way.
- Changing the order changes which mod wins conflicts (see below), so keep it
  the same as what you tested.

## Which mod wins

When two mods ship the same file, the higher one in the stack wins and the
other mod's copy is ignored. The main mod always wins. A few kinds of files
are handled specially:

- **Scripts.** A script's server half (`.gsc`) and client half (`.csc`) always
  come from the same mod. Once a higher mod ships either half of a script, a
  lower mod's copy of that script is dropped completely. Mixing halves from two
  mods causes clientfield mismatches.
- **Entry scripts.** Mods usually start their code from
  `scripts/<mode>/gametypes/_clientids.gsc`, and only one file can have that
  name. Every mod's copy runs under its own name instead
  (`_clientids_<publisher id>.gsc`), next to the game's own copy, so mod menus
  and similar mods work together.
- **UI.** Any mod can replace a game UI file. If two mods replace the same
  one, the higher mod's copy is used.
- **Precache lists.** The lists of assets each mod precaches are merged.

## Limits

- Mods that change the same game systems are often not compatible, whatever
  the order. If something breaks, try the mods on their own first.
- The script pairing above only knows about files with the same name. A mod
  that registers a clientfield in one script and its client side in a script
  with a different name can still end up mismatched.
