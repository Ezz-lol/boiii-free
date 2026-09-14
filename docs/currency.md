# CoD Points & Liquid Divinium

Both premium currencies are normally served by Demonware's `bdMarketplace`. That
service is stubbed in this client — every task replies with an empty payload —
so the engine's balance cache is never populated, both currencies read as zero,
and every purchase path bails out before it does anything.

The `currency` component
([`src/client/component/currency.cpp`](../src/client/component/currency.cpp))
replaces that backend with a local wallet instead of emulating the marketplace
wire protocol. Balances and purchased items live in
`boiii_players/user/wallet.json` and survive restarts.

> **Engine naming:** Liquid Divinium is `ZMVials` in the engine, and gobblegums
> are `BGB` ("BubbleGum Buffs"). Search for those names, not the player-facing
> ones.

## Commands

| Command              | Effect                                           |
| -------------------- | ------------------------------------------------ |
| `codpoints`          | Show the current CoD Points balance              |
| `codpoints <amount>` | Set the CoD Points balance                       |
| `divinium`           | Show the current Liquid Divinium balance         |
| `divinium <amount>`  | Set the Liquid Divinium balance                  |
| `wallet`             | Show both balances                               |
| `wallet_reset`       | Clear both balances and every locally owned item |

Balances are clamped to `0 … 9999999`.

## Dvars

| Dvar                      | Default | Effect                                               |
| ------------------------- | ------- | ---------------------------------------------------- |
| `cg_local_currency`       | `1`     | Master switch. `0` restores stock (broken) behaviour |
| `cg_codpoints_per_minute` | `120`   | CoD Points awarded per minute of play at match end   |
| `cg_divinium_per_match`   | `3`     | Liquid Divinium awarded when a Zombies match ends    |
| `cg_currency_award_cap`   | `5000`  | Ceiling on CoD Points from any single match          |

All four are `archive`, so they persist to the user config.

## Earning

A match is detected by polling `Com_IsInGame` once a second rather than by
hooking `G_InitGame` / `G_ShutdownGame`, because those only run on the machine
hosting the match — polling also covers playing on someone else's dedicated
server. When a match ends, CoD Points are awarded per whole minute played
(capped by `cg_currency_award_cap`), plus a flat Divinium award in Zombies. A
toast reports the total.

Matches shorter than a minute award nothing.

## Purchasing

`Lua_CoD_LuaCall_PurchaseItem` is detoured to record ownership locally, and
`Lua_CoD_LuaCall_SpendZMVials` deducts from the local Divinium balance and
reports success. `loot.cpp` consults `currency::owned_quantity` in
`Loot_GetItemQuantity` before falling through to the engine, so a purchased item
stays owned across restarts.

## Relationship to `cg_unlockall_loot`

`cg_unlockall_loot` (and the `unlockall` command) grant _everything_ outright
and take precedence — they short-circuit before the ownership store is
consulted. Turn it off if you want purchases to actually mean something.

## Known gaps

- **Loot crates are not emulated.** `BuyLootCrate` is deliberately left alone.
  Opening a crate requires a server-side loot roll (`IsLootReady`,
  `GetLootResults`, `LootResultsReady`), and faking that convincingly means
  reproducing the rarity tables. Buying a crate will still fail rather than
  silently taking your money.
- **`GetPlayerBalanceByCurrencyId` is not hooked.** Any menu reading a balance
  through the generic currency-id accessor rather than `GetCoDPoints` /
  `GetZMVials` will still show zero. Wiring it up needs the currency-id mapping
  from `initial_cod_points_id` and the `zm_vials_*_id` dvars.
- **MP unlock-token purchases are a separate id space.** The ownership store
  feeds `Loot_GetItemQuantity` (Black Market loot: gobblegums, camos, calling
  cards). Multiplayer create-a-class items go through
  `BG_Unlockables_IsItemPurchased`, which indexes unlockables rather than loot
  items, so it is still governed by `cg_unlockall_purchases`. The two were not
  merged because the id spaces overlap numerically and a wrong mapping would
  silently unlock the wrong things.
- Awards are client-side and per-local-user; nothing is reported to a server.

## Dr. Monty's Factory

Powering up a vat drives the _loot_ pipeline, not the marketplace. The frontend
calls `SpendZMVials`, then polls `LootResultsReady` / `LootFailureReason` until
the machine has something to show, and reads the winnings from `GetLootResults`.
Without Demonware the engine answers `LOOT_RESULT_FAILURE` immediately, which is
what produced the "An error has occurred running Dr. Monty's Machine" dialog -
and returning `false` from `SpendZMVials` lands on the same dialog, so there is
no path through the menu that does not need emulating.

The `currency` component answers all three: one vat costs one vial and yields
one mega gobblegum, rolled from the `bubblegum_consumable` rows of
`gamedata/stats/zm/zm_statsTable.csv`. Those 44 rows are baked into
`currency.cpp` because the engine's own loot pool (`GetLootItems`) is served by
Demonware and reads as empty here. `GetLootResults` hands back a flat array of
item _references_ (`zm_bgb_perkaholic`); item indices leave the reveal unable to
resolve the gobblegum and it renders as a blank box.

Each stub falls through to the engine unless a run of ours is outstanding, so
the Black Market's decryption flow - which polls the same three functions - is
untouched.

| Dvar                 | Default | Effect                                      |
| -------------------- | ------- | ------------------------------------------- |
| `cg_megachew_roller` | `1`     | Emulate the factory. `0` restores the error |
| `cg_megachew_grants` | `0`     | Keep what is rolled (see below)             |

### Ownership

Gobblegums are otherwise unrestricted in game, and every one of them reads as
quantity zero in the frontend. `cg_megachew_grants` opts into counting what the
machine hands out: the grid shows a real number and using one spends it.

It is a partial version of the retail model. Gobblegums that have never been
rolled still read as zero and are still usable in game, so this adds stock
tracking for what has been won rather than restricting anything.

Rolled gobblegums are recorded to `wallet.json` and reported back through
`Lua_CoD_LuaCall_GetLootItemQuantity`, which is what the frontend's
`GetConsumableCountFromIndex` reads. Take care not to confuse it with the
engine's `Loot_GetItemQuantity` that `loot.cpp` hooks: the Lua entry point is
keyed by the gobblegum's **reference** (`zm_bgb_perkaholic`) while the engine
one is keyed by item index, and the frontend's count only ever goes through the
former. The hook falls through to the engine unless the reference is one of the
44 mega gobblegums and the wallet has stock for it.

Rolls stack, so the same gobblegum won twice is worth two.

### Known limitations

Liquid Divinium picked up during a match reports "error retrieving vials" and is
not credited - that pickup goes through Demonware, and only the end-of-match
award in this component works offline.
