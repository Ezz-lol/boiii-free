"""Ensure quota sampling never invokes an unknown custom count callback."""
from pathlib import Path
import sys
root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[1]
source_path = root / "scripts/zombies_coop/eroded_coop.gsc"
if not source_path.is_file(): source_path = root / "data/custom_scripts/zm/eroded_coop.gsc"
source = source_path.read_text(encoding="utf-8")
assert '#using scripts/shared/ai/zombie_utility;' in source
assert source.count('zm::get_zombie_count_for_round(') == 1
helper = source[source.index('function normal_round_target('):source.index('function custom_round_controller(')]
assert helper.index('!count_callback_owned(level.max_zombie_func)') < helper.index('return zm::get_zombie_count_for_round')
owner = source[source.index('function count_callback_owned('):source.index('function normal_round_target(')]
assert '!isdefined(owner) || owner == &zombie_utility::default_max_zombie_func' in owner
reason = source[source.index('function round_controller_reason('):source.index('function track_round(')]
assert '"custom_count_callback_preserved"' in reason and 'count_callback_owned(level.max_zombie_func)' in reason
assert 'target = normal_round_target(round, count)' in source
assert 'target = normal_round_target(round, current_count)' in source
fixture = (root / 'tests/gsc/eroded_coop_selftest.gsc').read_text(encoding='utf-8')
assert 'count_callback_owned(undefined)' in fixture
assert 'count_callback_owned(&zombie_utility::default_max_zombie_func)' in fixture
assert '!eroded_coop::count_callback_owned(&custom_count_callback)' in fixture
assert 'level.eroded_coop_test_count_calls == 0' in fixture
assert 'level.max_zombie_func = saved_count_callback' in fixture
assert 'level.eroded_coop_test_count_calls = saved_count_calls' in fixture
print('PASS all quota getter calls guarded; missing/default/custom ownership and no-call fixture restored')
