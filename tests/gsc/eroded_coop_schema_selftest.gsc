// Test-only early-system fixture. Never install as normal application data.
#using scripts/shared/system_shared;
#using scripts/zm/eroded_coop_fields;
#namespace eroded_coop_schema_selftest;

function autoexec main()
{
    capacity = getdvarint("zm_coopSchemaSlots");
    if (!isint(capacity) || capacity < 5 || capacity > 8) return;
    system::register("eroded_coop_schema_selftest", &run, undefined, array("zm_laststand", "eroded_coop_fields"));
}

function check(value)
{
    if (value) return 0;
    return 1;
}

function run()
{
    failures = 0;
    failures += check(eroded_coop_fields::schema_revision() == 2);
    failures += check(eroded_coop_fields::schema_contract() == "5e18c32294a479c8188f5625dfc9204300f228332e2e925d326dfb53c12b460b");
    failures += check(eroded_coop_fields::schema_family_mask("zm_zod") == 3);
    failures += check(eroded_coop_fields::schema_family_mask("zm_factory") == 3);
    failures += check(eroded_coop_fields::schema_family_mask("zm_castle") == 7);
    failures += check(eroded_coop_fields::schema_family_mask("zm_island") == 67);
    failures += check(eroded_coop_fields::schema_family_mask("zm_stalingrad") == 11);
    failures += check(eroded_coop_fields::schema_family_mask("zm_genesis") == 19);
    failures += check(eroded_coop_fields::schema_family_mask("zm_prototype") == 3);
    failures += check(eroded_coop_fields::schema_family_mask("zm_asylum") == 3);
    failures += check(eroded_coop_fields::schema_family_mask("zm_sumpf") == 3);
    failures += check(eroded_coop_fields::schema_family_mask("zm_theater") == 3);
    failures += check(eroded_coop_fields::schema_family_mask("zm_cosmodrome") == 3);
    failures += check(eroded_coop_fields::schema_family_mask("zm_temple") == 3);
    failures += check(eroded_coop_fields::schema_family_mask("zm_moon") == 35);
    failures += check(eroded_coop_fields::schema_family_mask("zm_tomb") == 3);
    failures += check(eroded_coop_fields::schema_family_mask("zm_user_inherited") == 3);
    failures += check(getdvarint("zm_coopSchemaServerRevision") == 2);
    failures += check(getdvarstring("zm_coopSchemaServerContract") == eroded_coop_fields::schema_contract());
    for (slot = 4; slot < 8; slot++)
    {
        failures += check(level.laststand_update_clientfields[slot] == "laststand_update" + slot);
        failures += check(isdefined(level.player_name_directive[slot]));
        failures += check(eroded_coop_fields::extra_player_name(slot, "actual bot name") == "actual bot name");
        failures += check(eroded_coop_fields::extra_player_name(slot, undefined) == "Player " + (slot + 1));
        failures += check(eroded_coop_fields::extra_player_name(slot, "") == "Player " + (slot + 1));
    }
    failures += check(eroded_coop_fields::name_directive_owned(undefined, undefined));
    failures += check(eroded_coop_fields::name_directive_owned("old bot", "old bot"));
    failures += check(!eroded_coop_fields::name_directive_owned("custom", "our bot"));
    failures += check(!eroded_coop_fields::name_directive_owned("custom", undefined));
    failures += check(!isdefined(eroded_coop_fields::extra_player_name(3, "stock")));
    failures += check(!isdefined(eroded_coop_fields::extra_player_name(8, "foreign")));
    setdvar("zm_coopSchemaServerTestFailures", failures);
    if (failures == 0)
        setdvar("zm_coopSchemaServerTestStatus", "passed");
    else
        setdvar("zm_coopSchemaServerTestStatus", "failed");
}
