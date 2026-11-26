"""Check source-owned official profiles and generic compatibility boundaries.

These structural checks do not execute GSC, clients, navigation or real bots.
The separately compiled actual-VM fixture exercises the pure GSC helpers.
"""
from pathlib import Path
import argparse
import json
import re


parser = argparse.ArgumentParser()
parser.add_argument("root", type=Path, nargs="?", default=Path(__file__).resolve().parents[1])
args = parser.parse_args()
root = args.root.resolve()
manifest = json.loads((root / "tests/zombies_coop_maps.json").read_text())
maps = manifest["maps"]
assert len(maps) == 14 and len({entry["map"] for entry in maps}) == 14
source_root = root / "scripts/zombies_coop"
if (source_root / "eroded_coop.gsc").is_file():
    common = (source_root / "eroded_coop.gsc").read_text()
else:
    source_root = root / "data/custom_scripts"
    common = (source_root / "zm/eroded_coop.gsc").read_text()


def function(name):
    match = re.search(r"function " + re.escape(name) + r"\([^)]*\)\s*\{", common)
    assert match, name
    start, depth = match.end(), 1
    i = start
    while depth:
        depth += (common[i] == "{") - (common[i] == "}")
        i += 1
    return common[start:i - 1]


assert set(re.findall(r'"(zm_[a-z_]+)"', function("official_map"))) == {entry["map"] for entry in maps}
assert not any(line.startswith("#using scripts/zm/zm_") for line in common.splitlines())
for entry in maps:
    name = entry["map"]
    text = (source_root / name / ("eroded_coop_" + name[3:] + ".gsc")).read_text()
    expected = {"scripts/zm/_zm", "scripts/zm/" + entry["source"]}
    if entry.get("spawn"):
        expected.add("scripts/zm/" + entry["spawn"].split("::")[0])
    assert set(re.findall(r"^#using ([^;]+);", text, re.M)) == expected, name
    assert f'getdvarstring("mapname") != "{name}"' in text, name
    assert "limit < 5 || limit > 8 || epoch < 1" in text, name
    assert f"&{entry['source']}::{entry['chars']}" in text, name
    assert ("level.eroded_coop_profile_radio = true;" in text) == entry["radio"], name
    for field, key in (("known_custom_spawn", "spawn"), ("known_total_setup", "total"), ("known_round_think", "think")):
        if entry.get(key):
            assert f"level.eroded_coop_{field} = &{entry[key]};" in text, name
        else:
            assert f"level.eroded_coop_{field} =" not in text, name
    assert text.index("level.eroded_coop_profile_epoch = epoch") > text.index("level.eroded_coop_profile_characters"), name
    assert not re.search(r"level\.(?:round_spawn_func|fn_custom_round_ai_spawn|zombie_total_set_func)\s*=", text), name

round_owner = function("round_controller_reason")
assert "level.round_spawn_func != &zm::round_spawning" in round_owner
assert "level.zombie_total_set_func != level.eroded_coop_known_total_setup" in round_owner
assert "[[" not in round_owner  # Recognition never reruns a map initializer.
assert not re.search(r"level\.players\s*=", common)  # Real human/bot entities own population.
tracking = function("track_round")
assert "current_count = getplayers().size" in tracking
assert "listed_count = level.players.size" in tracking
membership_guard = "!stable_round_membership(current_count, listed_count, level.eroded_coop_limit)"
assert membership_guard in tracking
assert tracking.index(membership_guard) < tracking.index("target = normal_round_target(round, current_count)")
assert tracking.index(membership_guard) < tracking.index("level.zombie_total = next_total")
membership = function("stable_round_membership")
for requirement in ("isint(limit)", "limit >= 1 && limit <= 8", "isint(actual)",
                    "actual >= 1 && actual <= limit", "isint(listed)", "actual == listed"):
    assert requirement in membership, requirement
assert "count = level.players.size" in function("round_listener")
assert "seed = make_round_seed(round, count, total, respawns, recycle_total, target, generation)" in function("round_listener")
assert "total != target || respawns != 0" in function("make_round_seed")
assert "seed.count" in tracking and "seed.pending" in tracking
assert "level.givecustomcharacters != level.eroded_coop_profile_characters" in function("install_character_hook")
character = function("give_characters")
assert "players = getplayers()" in character
assert "!isdefined(self.characterindex)" in character
assert "level.rich_sq_player == self" in character and "connected_owner(rich)" in character
assert "level.sndradioa == self" in character and "connected_owner(radio)" in character
spawn = function("separate_spawn")
for guard in ("i < 24", "16384", "positionwouldtelefrag", "check_point_in_playable_area", "bullettracepassed",
              'getentarray("player_volume", "script_noteworthy")', "self.eroded_coop_spawn_generation == generation"):
    assert guard in spawn, guard
assert "self getentitynumber()" in function("repair_empty_gum_pack")
assert "gum_pack_needs_fallback" in function("repair_empty_gum_pack")
assert "slot < 4 || slot >= 8 || slot >= limit" in function("gum_pack_needs_fallback")
assert "gum != \"weapon_null\"" in function("gum_pack_needs_fallback")
assert "level.bgb[gum].consumable" in function("repair_empty_gum_pack")
fixture = (root / "tests/gsc/eroded_coop_selftest.gsc").read_text()
for scenario in ("array(2, 0, 2, 2)", "slot <= 8", "saved_round_spawn", "level.eroded_coop_test_setup_calls == 0"):
    assert scenario in fixture, scenario
print("PASS: 14 isolated official map profiles, stock ownership/Temple initializer, dynamic real-entity population, roster/spawn/gum boundaries (source checks only)")
