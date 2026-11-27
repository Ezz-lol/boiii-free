// Eroded Networks official-map profile. Independently written against pinned
// ate47/bo3-source fe040b1187b4850fe8da3c78b703c986c87d1028.
// Map-only imports must never be moved into the common controller.
#using scripts/zm/_zm;
#using scripts/zm/zm_genesis;
#namespace eroded_coop_genesis;

function autoexec main()
{
    limit = getdvarint("zm_coopAppliedLimit");
    epoch = getdvarint("zm_coopStartupEpoch");
    if (limit < 5 || limit > 8 || epoch < 1 || getdvarstring("mapname") != "zm_genesis")
        return;
    level.eroded_coop_profile_delay = &zm::get_zombie_spawn_delay;
    level.eroded_coop_profile_characters = &zm_genesis::givecustomcharacters;
    level.eroded_coop_profile_radio = false;
    level.eroded_coop_profile_map = "zm_genesis";
    level.eroded_coop_profile_epoch = epoch;
}

