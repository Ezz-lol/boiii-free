"""Owned pristine callback source contracts; runtime scheduling needs separate proof.

The compiled GSC fixture executes actual arithmetic/owner predicates. These
structural checks establish guard/order presence, not native VM thread ordering.
"""
from pathlib import Path
import sys
root=Path(sys.argv[1])if len(sys.argv)>1 else Path(__file__).resolve().parents[1]
path=root/'scripts/zombies_coop/eroded_coop.gsc'
if not path.is_file():path=root/'data/custom_scripts/zm/eroded_coop.gsc'
source=path.read_text()
callback=source[source.index('function round_total_setup()'):source.index('function next_round_generation(')]
capture=callback[:callback.index('run_original_total_setup(original, known)')]
code='\n'.join(line.split('//')[0]for line in capture.splitlines())
assert 'waittill('not in code and 'wait 'not in code and ' thread 'not in code
steps=['generation = next_round_generation(level.eroded_coop_round_generation)',
 'level.eroded_coop_round_generation = generation','reason = round_controller_reason()',
 'count = level.players.size','round = level.round_number','total = level.zombie_total',
 'respawns = level.zombie_respawns','recycle_total = level.zombie_total_subtract',
 'target = normal_round_target(round, count)',
 'seed = make_round_seed(round, count, total, respawns, recycle_total, target, generation)']
positions=[capture.index(step)for step in steps];assert positions==sorted(positions)
assert source.count('level.eroded_coop_round_generation = generation')==1
assert 'function round_listener('not in source
assert callback.count('run_original_total_setup(original, known)')==1
assert callback.index('run_original_total_setup(original, known)')<callback.index('if (!isdefined(seed))')
assert callback.index('run_original_total_setup(original, known)')<callback.index('level thread track_round(epoch, map, seed)')
seed=source[source.index('function make_round_seed('):source.index('function valid_counters(')]
assert 'total != target || respawns != 0'in seed
assert 'seed.pending = total - respawns'in seed and 'seed.consumed = target - seed.pending'in seed
install=source[source.index('function install(limit,'):source.index('function diagnostic_heartbeat(')]
assert 'isdefined(level.eroded_coop_known_total_setup)'in install and '!isdefined(level.zombie_total_set_func)'in install
assert install.index('if (!install_round_seed_hook(epoch, map))')<install.index('level.player_too_many_players_check_func = &player_count_check')
assert install.index('if (!install_round_seed_hook(epoch, map))')<install.index('install_bgb_movement_hook()')<install.index('setdvar("zm_coopReadyEpoch", epoch)')
owns=source[source.index('function owns_callbacks('):source.index('// GobbleGum')]
assert 'round_setup_context_current() && round_setup_owner_owned()'in owns
worker=source[source.index('function track_round('):]
for field in ('count','round','target','pending','consumed','recycle_total'):assert f'{field} = seed.{field}'in worker
assert worker.count('round_generation_current(seed.generation, level.eroded_coop_round_generation')==2
assert worker.index('spawns = accounted_spawn_delta(')<worker.index('!stable_round_membership(')
assert worker.index('if (custom_round_controller())',worker.index('while (true)'))<worker.index('spawns = accounted_spawn_delta(')
assert worker.index('if (custom_round_controller())',worker.index('while (true)'))<worker.index('level.zombie_total = next_total')
reason=source[source.index('function round_controller_reason('):source.index('function track_round(')]
assert 'isdefined(level.kill_counter_hud)'in reason and 'kill_counter_budget_preserved'in reason
assert '!round_setup_owner_owned()'in reason and '[['not in reason
fixture=(root/'tests/gsc/eroded_coop_selftest.gsc').read_text()
for case in ('test_seed_callback_owners()','next_round_generation(2147483647)',
 'make_round_seed(3, 8, 99, 0, 17, 100, 8)',
 '!eroded_coop::total_setup_owner_owned(wrapper, known, foreign, true, true)',
 '!eroded_coop::total_setup_owner_owned(wrapper, known, known, true, false)',
 '!eroded_coop::total_setup_owner_owned(known, known, known, true, true)',
 'level.eroded_coop_seed_test_calls == 0','level.eroded_coop_seed_test_calls == 1',
 'spawns == 10 && credits == 2 && consumed == 10','remaining_budget(100, consumed, credits) == 86'):
 assert case in fixture,case
print('PASS owned pristine setup callback contracts; saved-owner/startup/worker guards and compiled VM helper cases present; scheduling requires actual retained runtime observation')
