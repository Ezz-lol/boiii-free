// Test only: 5 Cryptokeys to each player 3 seconds after spawning. Remove after testing.

function autoexec loot_test_init()
{
    level thread loot_test_on_connect();
}

function loot_test_on_connect()
{
    for (;;)
    {
        level waittill("connected", player);
        player thread loot_test_on_spawn();
    }
}

function loot_test_on_spawn()
{
    self endon("disconnect");
    self waittill("spawned_player");
    wait 3;

    if (self istestclient())
    {
        return;
    }

    self luinotifyevent(&"close_side_mission_countdown", 3, 7357, 1, 5);
    self iprintln("Black Market test: server sent 5 Cryptokeys");
}
