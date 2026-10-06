# Black Market Drops

Earn Black Market items by playing instead of unlocking everything at once.
Works without Demonware: items are stored locally per player.
I have replicated the Black Market in a more custom way, but feel free to change something
if you feel it's for the better!

## How it works

- Players earn **Cryptokeys** by playing (any mode, not in menus) and from
  server rewards if enabled
- Keys open **supply drops** in the Black Market menu (replaces the original,
  non-functional Black Market button):
  - **Common** (10 keys): 3 items.
  - **Rare** (30 keys): 3 items, first guaranteed Rare or better, better odds,
    plus 2-5 bonus keys.
- Drops only roll items the player doesn't own yet (no duplicates).
- Item data (2818 items, names, icons) is in
  `src/client/component/loot_pool.hpp`, generated from `mplootitems.csv`
  (I used ate47/bo3-source) and the game's own localized strings and images.

Local files (in `boiii_players/user/`): `loot_owned.txt`, `loot_recent.txt`,
`loot_keys.txt`.

## Settings menu

Press **S** (or the Settings prompt / Y / Triangle) in the Black Market to open
**Black Market Settings**: server rewards on/off, server keys and items per
match, and replacing the original Black Market button.

## Economy (fixed in the build, feel free to adjust this)

Key rate (1 per minute), drop prices (10 / 30), items per drop (3 / 3) and the
Rare drop bonus (2-5 keys) are constants at the top of
`src/client/component/loot.cpp`

## Client DVARS

| Dvar                                          | Default | Meaning                                          |
| --------------------------------------------- | ------- | ------------------------------------------------ |
| `cg_loot_progression`                         | 1       | Turn the feature on/off                          |
| `cg_loot_server_rewards`                      | 1       | Accept rewards from servers (0 = ignore)         |
| `cg_loot_server_max_keys`                     | 100     | Most keys a server can give per match            |
| `cg_loot_server_max_items`                    | 10      | Most items a server can give per match           |
| `cg_loot_hide_blackmarket`                    | 1       | Replace the original Black Market button         |
| `cg_loot_debug`                               | 0       | Log server reward events to the console          |

## Console commands

`lootstatus`, `lootlist`, `lootkeys [amount]`, `lootopen common|rare`,
`lootreset`.

## Server rewards (GSC)

BO3 only delivers UI events whose name the game already knows, and GSC's
`setclientdvar` doesn't change client dvars, so rewards ride on a stock event
that the Campaign  uses, marked with the code `7357`:

```gsc
player luinotifyevent(&"close_side_mission_countdown", 3, 7357, 1, amount); // Cryptokeys
player luinotifyevent(&"close_side_mission_countdown", 3, 7357, 2, amount); // items
```

This is stock GSC, so it works on any server (BOIII, T7x). See
`docs/black-market-drops/loot_rewards.gsc` for an end-of-match reward script
(rewards once per match when you get the match bonus exp) and
`docs/black-market-drops/loot_test.gsc` for a quick test. T7x only loads
compiled scripts, so if using on T7X, it must be precompiled
