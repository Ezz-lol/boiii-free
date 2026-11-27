// Eroded Networks official-map profile. Independently written against pinned
// ate47/bo3-source fe040b1187b4850fe8da3c78b703c986c87d1028.
// Map-only imports must never be moved into the common controller.
#using scripts/zm/_zm;
#using scripts/zm/zm_zod;
#namespace eroded_coop_zod;

function autoexec main()
{
    limit = getdvarint("zm_coopAppliedLimit");
    epoch = getdvarint("zm_coopStartupEpoch");
    if (limit < 5 || limit > 8 || epoch < 1 || getdvarstring("mapname") != "zm_zod")
        return;
    level.eroded_coop_profile_delay = &zm_zod::function_59804866;
    level.eroded_coop_profile_characters = &zm_zod::givecustomcharacters;
    level.eroded_coop_profile_radio = false;
    level.eroded_coop_known_custom_spawn = &zm_zod::function_33aa4940;
    level.eroded_coop_zod_delay = level.eroded_coop_profile_delay;
    level.eroded_coop_profile_map = "zm_zod";
    level.eroded_coop_profile_epoch = epoch;
}
