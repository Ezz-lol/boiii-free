#using scripts\zm\_zm_weapons;

#namespace boiii_weapon_kits;

function supports_added_weapon( weapon )
{
    name = zm_weapons::get_base_weapon( weapon ).name;
    return name == "smg_ak74u" || name == "smg_mp40" || name == "smg_ppsh" || name == "ar_peacekeeper" || name == "ar_an94" || name == "ar_garand" || name == "ar_famas" || name == "ar_m16" || name == "ar_galil" || name == "ar_m14" || name == "lmg_rpk" || name == "sniper_chargeshot" || name == "shotgun_energy" || name == "pistol_energy" || name == "pistol_m1911" || name == "launcher_multi";
}

detour scripts\zm\_zm_weapons::give_build_kit_weapon( weapon )
{
    if ( weapon.name == "hero_annihilator" )
    {
        self giveweapon( weapon );
        self givemaxammo( weapon );
        self gadgetpowerset( 0, 100 );
        self setlowready( 0 );
        self shoulddoinitialweaponraise( weapon, 1 );
        return weapon;
    }

    if ( weapon.name == "bowie_knife" && self hasperk( "specialty_widowswine" ) )
        weapon = getweapon( "bowie_knife_widows_wine" );

    if ( self hasweapon( weapon, 1 ) )
    {
        weapons = self getweaponslist( 1 );

        foreach ( owned_weapon in weapons )
        {
            if ( owned_weapon === weapon || owned_weapon.rootweapon === weapon.rootweapon )
                return owned_weapon;
        }
    }

    base_weapon = zm_weapons::get_base_weapon( weapon );

    if ( supports_added_weapon( weapon ) && isdefined( level.zombie_weapons[base_weapon] ) && isdefined( level.zombie_weapons[base_weapon].upgrade ) )
        prepareweaponkitassets( base_weapon.rootweapon.name, level.zombie_weapons[base_weapon].upgrade.rootweapon.name );

    kit_weapon = self getbuildkitweapon( weapon, 0 );

    if ( supports_added_weapon( weapon ) && zm_weapons::is_weapon_upgraded( weapon ) )
    {
        normal_kit = self getbuildkitweapon( base_weapon, 0 );

        if ( normal_kit.attachments.size )
        {
            upgraded_kit = getweapon( weapon.rootweapon.name, normal_kit.attachments );

            if ( upgraded_kit != level.weaponnone )
                kit_weapon = upgraded_kit;
        }
    }

    if ( zm_weapons::is_weapon_upgraded( kit_weapon ) )
        weapon_options = self zm_weapons::get_pack_a_punch_weapon_options( kit_weapon );
    else
        weapon_options = self getbuildkitweaponoptions( base_weapon, undefined );

    attachment_variants = self getbuildkitattachmentcosmeticvariantindexes( base_weapon, 0 );

    if ( kit_weapon != level.weaponnone )
    {
        self giveweapon( kit_weapon, weapon_options, attachment_variants );

        if ( supports_added_weapon( kit_weapon ) && !zm_weapons::is_weapon_upgraded( kit_weapon ) )
            self updateweaponoptions( kit_weapon, weapon_options );
    }

    return kit_weapon;
}
