#using scripts\shared\array_shared;
#using scripts\shared\flag_shared;
#using scripts\shared\util_shared;
#using scripts\zm\zm_temple_pack_a_punch;

#namespace more_players_temple;

detour zm_temple_pack_a_punch<scripts\zm\zm_temple_pack_a_punch.gsc>::_setup_simultaneous_pap_triggers()
{
	spots = GetEntArray("hanging_base", "targetname");
	foreach (spot in spots)
		spot Delete();
	level flag::wait_till("power_on");
	triggers = [];
	for (i = 0; i < 4; i++)
		triggers[i] = GetEnt("pap_blocker_trigger" + (i + 1), "targetname");
	zm_temple_pack_a_punch::_randomize_pressure_plates(triggers);
	array::thread_all(triggers, &zm_temple_pack_a_punch::_pap_pressure_plate_move);
	wait 1;
	last_num_plates_active = -1;
	last_plate_state = -1;
	for (;;)
	{
		num_plates_needed = min(GetPlayers().size, triggers.size);
		num_plates_active = 0;
		plate_state = 0;
		foreach (trigger in triggers)
		{
			if (trigger.plate.active)
				num_plates_active++;
			if (trigger.plate.active || (trigger.requiredplayers - 1) >= num_plates_needed)
				plate_state += pow(2, trigger.requiredplayers - 1);
		}
		if (last_num_plates_active != num_plates_active || plate_state != last_plate_state)
		{
			last_num_plates_active = num_plates_active;
			last_plate_state = plate_state;
			zm_temple_pack_a_punch::_set_num_plates_active(num_plates_active, plate_state);
		}
		zm_temple_pack_a_punch::_update_stairs(triggers);
		if (num_plates_active >= num_plates_needed)
		{
			foreach (trigger in triggers)
			{
				trigger notify("pap_active");
				trigger.plate zm_temple_pack_a_punch::_plate_move_down();
			}
			zm_temple_pack_a_punch::_pap_think();
			zm_temple_pack_a_punch::_randomize_pressure_plates(triggers);
			array::thread_all(triggers, &zm_temple_pack_a_punch::_pap_pressure_plate_move);
			zm_temple_pack_a_punch::_set_num_plates_active(4, 15);
			wait 1;
		}
		util::wait_network_frame();
	}
}
