"""Check the captured-event boundary; actual arithmetic cases ship in the VM fixture."""
from pathlib import Path
import sys

root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[1]
path = root / "scripts/zombies_coop/eroded_coop.gsc"
if not path.is_file():
    path = root / "data/custom_scripts/zm/eroded_coop.gsc"
source = path.read_text(encoding="utf-8")
listener = source[source.index("function round_listener("):source.index("function next_round_generation(")]
capture = listener[listener.index('level waittill(#"zombie_total_set")'):]
assert capture.count("waittill(") == 1 and "wait " not in capture
steps = [
    "generation = next_round_generation(level.eroded_coop_round_generation)",
    "level.eroded_coop_round_generation = generation",
    "count = level.players.size",
    "round = level.round_number",
    "total = level.zombie_total",
    "respawns = level.zombie_respawns",
    "recycle_total = level.zombie_total_subtract",
    "target = normal_round_target(round, count)",
    "seed = make_round_seed(round, count, total, respawns, recycle_total, target, generation)",
    "level thread track_round(epoch, map, seed)",
]
positions = [capture.index(step) for step in steps]
assert positions == sorted(positions)
assert "getplayers()" not in "\n".join(line.split("//")[0] for line in capture.splitlines())
assert '"late_round_seed_unavailable"' in listener and '"round_seed_unavailable"' in listener
seed = source[source.index("function make_round_seed("):source.index("function valid_counters(")]
assert "total != target || respawns != 0" in seed
assert "seed.pending = total - respawns" in seed and "seed.consumed = target - seed.pending" in seed
worker = source[source.index("function track_round("):]
for field in ("count", "round", "target", "pending", "consumed", "recycle_total"):
    assert f"{field} = seed.{field}" in worker
assert worker.count("round_generation_current(seed.generation, level.eroded_coop_round_generation") == 2
assert "target = normal_round_target(round, count)" not in worker
assert worker.index("spawns = accounted_spawn_delta(") < worker.index("!stable_round_membership(")
assert worker.index("round_generation_current(", worker.index("while (true)")) < worker.index("level.zombie_total = next_total")
assert "current_count != level.players.size" not in worker
fixture = (root / "tests/gsc/eroded_coop_selftest.gsc").read_text(encoding="utf-8")
for case in (
    "test_captured_round_event()",
    "next_round_generation(2147483647)",
    "!eroded_coop::round_generation_current(7, 8, 3, 3)",
    "make_round_seed(3, 8, 99, 0, 17, 100, 8)",
    "!eroded_coop::stable_round_membership(4, 8, 8)",
    "!eroded_coop::stable_round_membership(8, 4, 8)",
    "spawns == 10 && credits == 2 && consumed == 10",
    "consumed == 16 && credits == 2",
    "remaining_budget(100, consumed, credits) == 86",
):
    assert case in fixture, case
print("PASS captured event freezes stock seed before threading; late seeds decline, generations revoke stale workers, transient-roster deltas precede resize; VM cases present")
