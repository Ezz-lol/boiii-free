# Scoped actor callback lifecycle guards

Expanded stock-compatible sessions install two narrow guards in addition to the placement and window-melee guards. These preserve the retail functions rather than replacing their implementations. Their runtime effectiveness requires fresh exact-build canaries; local stand-ins do not establish it.

`level.botthreatengage` is wrapped only when it is the exact stock `bot_combat::engage_threat` owner. The wrapper checks its current player, the stock-created bot/threat containers and current threat entity before a synchronous zero-argument stock call. It follows stock `bot_think` by declining an already dead bot. Death, disconnect and game-end cancellation apply to that callback thread, without binding cancellation to an initial target that stock may replace. The defined-container checks assume the stock `spawnstruct` contract; they do not validate arbitrary custom field types.

`level.custom_player_fake_death` is wrapped only when absent, which selects the stock default. The hook accepts the two arguments supplied by its retail caller and synchronously calls the original zero-argument `zm::player_fake_death`. Only disconnect cancellation is added. Stock notifications, weapon/control changes and its one-second wait remain owned by that function. This guard deliberately does not require a living player or cancel on death, end-game or intermission events.

Bot callback defaults come from the stock bot pre-system, independently of the first native bot. Missing optional callbacks do not block human or no-bot startup: a bounded observer and on-connect retry can install a late exact stock owner. Unknown initial owners are preserved and latched. Once a guard is installed, losing its callback or map/epoch context closes expanded readiness without overwriting the replacement owner.

Successful installation emits at most one fixed receipt per guard per map:

```text
[Zombies coop] actor_guard=bot_threat installed=1
[Zombies coop] actor_guard=fake_death installed=1
```

`zm_coopBotThreatStatus` and `zm_coopFakeDeathStatus` are diagnostic state, not schema or security attestation. Receipt presence establishes installation, not that a particular error was fixed. Source lookup positions and native error markers do not alone prove which cached object became invalid.

A later private dedicated test still reached errors through the installed bot guard. The actual captured retail 1C script places `bot_combat::engage_threat` at `0xAF8`; lookup positions `0xC67` and `0xD87` are inside its two one-argument `GetWeaponAmmoStock` calls at `0xC56` and `0xD7A`. Their receiver is `self`, with arguments `self.grenadetypeprimary` and `self.grenadetypesecondary`. These positions are VM source-lookup addresses, not native exception addresses. The capture identifies the operations; it does not record the argument or receiver values at the failure.

Stock Zombies maintains `current_lethal_grenade` and `current_tactical_grenade`, whereas the shared bot controller expects the MP/CP grenade alias fields above. The owned wrapper bridges missing aliases from those actual Zombies values, using the stock `level.weaponnone` fallback and native `IsWeapon` validation. It refreshes only aliases it introduced. Existing custom aliases and later takeovers are preserved and latched; invalid custom values are left untouched and can resume normal delegation after their owner repairs them. The bridge does not give, equip, remove or replenish weapons, change bot settings, or replace the retail AI. Fresh runtime tests are required to establish whether this compatibility repair prevents the observed errors.

The first successful adoption on each player entity emits `[Zombies coop] bot_grenade_alias=zm_loadout ready=1` once, without player identifiers or weapon names. Ordinary refreshes do not repeat it. Entirely custom-owned aliases do not claim adoption. This receipt proves that the bridge path ran, not that grenade use, combat or disconnect behavior passed.

The reviewed source interfaces are pinned to [stock shared bots](https://github.com/ate47/bo3-source/blob/fe040b1187b4850fe8da3c78b703c986c87d1028/scripts/shared/bots/_bot.gsc), [stock bot combat](https://github.com/ate47/bo3-source/blob/fe040b1187b4850fe8da3c78b703c986c87d1028/scripts/shared/bots/_bot_combat.gsc), [stock system registration](https://github.com/ate47/bo3-source/blob/fe040b1187b4850fe8da3c78b703c986c87d1028/scripts/shared/system_shared.gsc) and [stock Zombies](https://github.com/ate47/bo3-source/blob/fe040b1187b4850fe8da3c78b703c986c87d1028/scripts/zm/_zm.gsc).
