#using scripts\zm\_zm_weapons;

#namespace ranked;

function supports_added_weapon( weapon )
{
    name = zm_weapons::get_base_weapon( weapon ).name;

    switch (name) {
      case "smg_ak74u":
      case "smg_mp40":
      case "smg_ppsh":
      case "ar_peacekeeper":
      case "ar_an94":
      case "ar_garand":
      case "ar_famas":
      case "ar_m16":
      case "ar_galil":
      case "ar_m14":
      case "lmg_rpk":
      case "sniper_chargeshot":
      case "shotgun_energy":
      case "pistol_energy":
      case "pistol_m1911":
      case "pistol_standard":
      case "launcher_multi":
        return true;
      default:
        return false;
    }

}

detour scripts\zm\_zm_weapons::give_build_kit_weapon( weapon )
{
    if ( weapon.name == "hero_annihilator" ) {
        self giveweapon( weapon );
        self givemaxammo( weapon );
        self gadgetpowerset( 0, 100 );
        self setlowready( 0 );
        self shoulddoinitialweaponraise( weapon, 1 );
        return weapon;
    }

    if ( weapon.name == "bowie_knife" && self hasperk( "specialty_widowswine" ) ) {
        weapon = getweapon( "bowie_knife_widows_wine" );
    }

    if ( self hasweapon( weapon, 1 ) ) {
        weapons = self getweaponslist( 1 );

        foreach ( owned_weapon in weapons ) {
            if ( owned_weapon === weapon || owned_weapon.rootweapon === weapon.rootweapon )
                return owned_weapon;
        }
    }

    base_weapon = zm_weapons::get_base_weapon( weapon );

    if ( supports_added_weapon( weapon ) && isdefined( level.zombie_weapons[base_weapon] ) && 
         isdefined( level.zombie_weapons[base_weapon].upgrade ) ) {
        prepareweaponkitassets( base_weapon.rootweapon.name, level.zombie_weapons[base_weapon].upgrade.rootweapon.name );
    }

    kit_weapon = self getbuildkitweapon( weapon, 0 );

    if ( supports_added_weapon( weapon ) && zm_weapons::is_weapon_upgraded( weapon ) ) {
        normal_kit = self getbuildkitweapon( base_weapon, 0 );

        if ( normal_kit.attachments.size )
        {
            upgraded_kit = getweapon( weapon.rootweapon.name, normal_kit.attachments );

            if ( upgraded_kit != level.weaponnone ) {
                kit_weapon = upgraded_kit;
            }
        }
    }

    if ( zm_weapons::is_weapon_upgraded( kit_weapon ) ) {
        weapon_options = self zm_weapons::get_pack_a_punch_weapon_options( kit_weapon );
    } else {
        weapon_options = self getbuildkitweaponoptions( base_weapon, undefined );

        if ( supports_added_weapon( weapon ) && weapon_options == self calcweaponoptions( 0, 0, 0 ) ) {
            weapon_options = self calcweaponoptions( 1, 0, 0 );
        }
    }

    attachment_variants = self getbuildkitattachmentcosmeticvariantindexes( base_weapon, 0 );

    if ( kit_weapon != level.weaponnone ) {
        self giveweapon( kit_weapon, weapon_options, attachment_variants );

        if ( supports_added_weapon( kit_weapon ) && !zm_weapons::is_weapon_upgraded( kit_weapon ) ) {
            self updateweaponoptions( kit_weapon, weapon_options );
        }
    }

    return kit_weapon;
}
