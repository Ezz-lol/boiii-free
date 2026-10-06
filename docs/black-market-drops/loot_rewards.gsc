// Black Market Drops: end-of-match Cryptokeys. Players need the client build.
// T7x: compile first (e.g. `acts gscc loot_rewards.gsc -g t7`) and place in t7x\custom_scripts.

function keys_per_match()
{
    return 10; // Cryptokeys per player at the end of a match
}

function give_loot_keys(amount)
{
    self luinotifyevent(&"close_side_mission_countdown", 3, 7357, 1, amount);
}

function give_loot_items(amount)
{
    self luinotifyevent(&"close_side_mission_countdown", 3, 7357, 2, amount);
}

function autoexec loot_rewards_init()
{
    level thread award_multiplayer();
    level thread award_zombies();
}

function award_multiplayer()
{
    level waittill("game_ended");

    for (;;)
    {
        if (isdefined(level.finalgameend) && level.finalgameend)
        {
            break;
        }
        if (isdefined(level.intermission) && level.intermission == 1)
        {
            break;
        }
        wait 0.1;
    }

    award_once();
}

function award_zombies()
{
    level waittill("end_game");
    award_once();
}

function award_once()
{
    if (isdefined(level.loot_rewards_given))
    {
        return;
    }
    level.loot_rewards_given = true;

    foreach (player in level.players)
    {
        if (player istestclient())
        {
            continue;
        }
        player give_loot_keys(keys_per_match());
        // player give_loot_items(1); // uncomment to also give a random item
    }
}
