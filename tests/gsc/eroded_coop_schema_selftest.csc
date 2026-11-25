// Test-only early-system fixture. Never install as normal application data.
#using scripts/shared/system_shared;
#using scripts/zm/eroded_coop_fields;
#namespace eroded_coop_schema_selftest;

function autoexec main()
{
    capacity = getdvarint("zm_coopSchemaSlots");
    if (!isint(capacity) || capacity < 5 || capacity > 8) return;
    system::register("eroded_coop_schema_selftest", &run, undefined, "eroded_coop_fields");
}

function check(value)
{
    if (value) return 0;
    return 1;
}

function run()
{
    failures = 0;
    failures += check(eroded_coop_fields::schema_revision() == 1);
    failures += check(eroded_coop_fields::schema_contract() == "0d3c7d274e5f3753abe90c252e1b003942c9e9596a995166cd3d3043d4784019");
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
    failures += check(getdvarint("zm_coopSchemaClientRevision") == 1);
    failures += check(getdvarstring("zm_coopSchemaClientContract") == eroded_coop_fields::schema_contract());
    for (slot = 4; slot < 8; slot++)
    {
        failures += check(isdefined(level.laststands[slot]));
        if (isdefined(level.laststands[slot]))
        {
            failures += check(level.laststands[slot].bleedouttime == 0);
            failures += check(level.laststands[slot].lastbleedouttime == 0);
            failures += check(level.laststands[slot].laststand_update_clientfields == "laststand_update" + slot);
        }
    }
    setdvar("zm_coopSchemaClientTestFailures", failures);
    if (failures == 0)
        setdvar("zm_coopSchemaClientTestStatus", "passed");
    else
        setdvar("zm_coopSchemaClientTestStatus", "failed");
}
