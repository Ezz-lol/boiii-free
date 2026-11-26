"""Round thinker/Moon restore ownership contracts; VM fixture tests actual predicates."""
from pathlib import Path
import sys

root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[1]
source_path = root / "scripts/zombies_coop/eroded_coop.gsc"
if not source_path.is_file():
    source_path = root / "data/custom_scripts/zm/eroded_coop.gsc"
source = source_path.read_text(encoding="utf-8")
owner = source[source.index("function round_thinker_owned("):source.index("function moon_round_budget_owned(")]
assert "!isdefined(owner) || owner == &zm::round_think" in owner
assert "isdefined(known) && owner == known" in owner
moon = source[source.index("function moon_round_budget_owned("):source.index("function round_controller_reason(")]
for clause in ("!isdefined(on_moon)", "!on_moon", "!isdefined(teleporter_used)", "teleporter_used ||",
               "!isint(round)", "!isint(saved_round)", "round <= saved_round"):
    assert clause in moon, clause
reason = source[source.index("function round_controller_reason("):source.index("function track_round(")]
assert "round_thinker_owned(level.round_think_func, level.eroded_coop_known_round_think)" in reason
assert '"custom_round_thinker_preserved"' in reason
assert 'level.eroded_coop_profile_map == "zm_moon"' in reason
assert 'flag::exists("teleporter_used")' in reason and 'level flag::get("teleporter_used")' in reason
assert "moon_round_budget_owned(level.on_the_moon, teleporter_used, level._from_nml," in reason
assert "level.round_number, level.nml_last_round)" in reason
assert '"moon_restored_budget_preserved"' in reason
assert "[[" not in reason  # Recognition must never replay a map thinker.
listener = source[source.index("function round_listener("):source.index("function next_round_generation(")]
assert listener.index("if (custom_round_controller())") < listener.index("target = normal_round_target(round, count)")
worker = source[source.index("function track_round("):]
assert worker.index("if (custom_round_controller())", worker.index("while (true)")) < worker.index("spawns = accounted_spawn_delta(")
assert worker.index("if (custom_round_controller())", worker.index("while (true)")) < worker.index("level.zombie_total = next_total")
target = source[source.index("function normal_round_target("):source.index("function custom_round_controller(")]
assert target.index("custom_round_controller()") < target.index("zm::get_zombie_count_for_round(")
profile_path = root / "scripts/zombies_coop/zm_moon/eroded_coop_moon.gsc"
if not profile_path.is_file():
    profile_path = root / "data/custom_scripts/zm_moon/eroded_coop_moon.gsc"
assert "level.eroded_coop_known_round_think = &zm_moon::moon_round_think_func;" in profile_path.read_text(encoding="utf-8")
assert "#using scripts/zm/zm_moon;" not in source
fixture = (root / "tests/gsc/eroded_coop_selftest.gsc").read_text(encoding="utf-8")
for case in ("test_round_thinker_ownership()", "round_thinker_owned(undefined, undefined)",
             "round_thinker_owned(&zm::round_think, undefined)",
             "!eroded_coop::round_thinker_owned(&custom_round_thinker, undefined)",
             "level.eroded_coop_test_think_calls == 0", "level.round_think_func = saved_think",
             "restored = array(40, 100, 140)", "moon_round_budget_owned(true, false, true, 3, 3)",
             "moon_round_budget_owned(false, true, true, 4, 4)", "moon_round_budget_owned(true, false, true, 5, 4)"):
    assert case in fixture, case
print("PASS default/known/unknown thinker ownership; Moon NML/resume guard before capture/accounting/writes, durable saved-round boundary, VM predicate cases present")
