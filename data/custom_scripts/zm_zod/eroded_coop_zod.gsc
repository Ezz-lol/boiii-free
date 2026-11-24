// Shadows of Evil's own pacing callback, identified by its retail export hash.
// Load only with this map; importing it globally would break other maps.
#using scripts/zm/zm_zod;
#namespace eroded_coop_zod;

function autoexec main()
{
    if (getdvarint("zm_coopAppliedLimit") > 4 && getdvarint("zm_coopAppliedLimit") <= 8)
    {
        level.eroded_coop_zod_delay = &zm_zod::function_59804866;
        level.eroded_coop_known_custom_spawn = &zm_zod::function_33aa4940;
    }
}
