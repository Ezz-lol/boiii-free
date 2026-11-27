"""Offline actual-body control-flow checks for the production seed callback.

Mechanically translates selected GSC bodies into JavaScript and supplies explicit
engine, original-callback and worker stand-ins. This checks ownership and branch
ordering, not GSC VM scheduling, pointer ABI, map gameplay or runtime safety.
"""
from pathlib import Path
import argparse
import datetime
import hashlib
import json
import os
import re
import subprocess
import sys

sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tests'))
from script_frame_diagnostics import function

HELPERS=('total_setup_original_owned','total_setup_owner_owned','round_setup_context_current',
    'round_setup_owner_owned','install_round_seed_hook','run_original_total_setup','round_total_setup',
    'next_round_generation','round_generation_current','make_round_seed','valid_counters',
    'count_callback_owned','normal_round_target','custom_round_controller','round_thinker_owned',
    'moon_round_budget_owned','round_controller_reason','island_trials_current', 'stalingrad_pools_current','owns_callbacks')


def mask(source):
    return re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"',lambda m:' '*len(m[0]),source)


def translate(body):
    parameters=set(filter(None,(item.strip() for item in re.search(r'function\s+\w+\(([^)]*)\)',body)[1].split(','))))
    locals_=set(re.findall(r'(?<![\w.])([A-Za-z_]\w*)\s*=(?!=)',mask(body)))-parameters
    result=body.replace('level.players.size','level.players.length')
    for original,replacement in (
        ('&zombie_utility::default_max_zombie_func','stock_count'),('&zm::round_think','stock_think'),
        ('&zm::round_spawning','stock_spawn'),('&round_total_setup','round_total_setup'),
        ('&player_count_check','player_count_check'),('&spawn_delay','spawn_delay'),
        ('&custom_total_setup','custom_total_setup'),('&custom_bgb_callback','custom_bgb_callback'),
        ('&custom_count_callback','custom_count_callback'),('eroded_coop::',''),
        ('zm::get_zombie_count_for_round','stock_target'),('flag::exists','flag_exists'),
        ('level flag::get','flag_get'),('level thread track_round','track_round')):
        result=result.replace(original,replacement)
    result=re.sub(r'\[\[\s*original\s*\]\]\(\)', 'original()',result)
    if locals_:
        opening=result.index('{')+1
        result=result[:opening]+'\nlet '+','.join(sorted(locals_))+';\n'+result[opening:]
    if '::' in mask(result) or '[[' in mask(result):raise ValueError('Unsupported translation seam')
    return result


STANDINS=r'''
const assert=require('node:assert/strict');
let checks=0;function check(value,message){++checks;assert.ok(value,message);}
let level={},events=[],dvars={},workers=[],original_calls=0,target_calls=0,foreign_calls=0;
let currentEpoch=1,currentMap='zm_zod',flags={},actualPlayers=[];
function isdefined(value){return value!==undefined;}
function isint(value){return Number.isInteger(value)&&value>=-2147483648&&value<=2147483647;}
function isarray(value){return Array.isArray(value);}
function spawnstruct(){return {};}
function startup_current(epoch,map){return epoch===currentEpoch&&map===currentMap;}
function getdvarint(name){return name==='zm_coopStartupEpoch'?currentEpoch:(dvars[name]??0);}
function getdvarstring(name){return name==='mapname'?currentMap:(dvars[name]??'');}
// Independently exercised by zombies_coop_disconnect_guards.py; this suite
// isolates round-seed ownership and supplies an already-owned guard context.
function disconnect_guards_owned(){return true;}
// Optional actor callbacks have an independent actual-body suite.
function bot_threat_guard_compatible(){return true;}
function fake_death_guard_compatible(){return true;}
function setdvar(key,value){dvars[key]=value;}
function diagnostic_event(value){events.push('event:'+value);}
function flag_exists(name){return Object.hasOwn(flags,name);}
function flag_get(name){return flags[name];}
function stock_count(){}function stock_think(){}function stock_spawn(){}
function player_count_check(){}function spawn_delay(){}function moon_think(){}
function foreign(){++foreign_calls;}
function temple(){++original_calls;events.push('original');}
function stock_target(round,count){++target_calls;return round*10*count;}
function track_round(epoch,map,seed){events.push('worker');workers.push({epoch,map,seed:{...seed}});}
'''

CASES=r'''
function reset(known){
 level={eroded_coop_limit:8,eroded_coop_stock_spawn_delay:stock_spawn,
  player_too_many_players_check_func:player_count_check,func_get_zombie_spawn_delay:spawn_delay,
  eroded_coop_total_setup_saved:true,eroded_coop_total_setup_epoch:1,eroded_coop_total_setup_map:'zm_zod',
  zombie_total_set_func:round_total_setup,eroded_coop_known_total_setup:known,
  eroded_coop_total_setup_original:known,players:Array(8).fill({}),round_number:1,
  zombie_total:80,zombie_respawns:0,zombie_total_subtract:17};
 events=[];dvars={};workers=[];original_calls=0;target_calls=0;foreign_calls=0;
 currentEpoch=1;currentMap='zm_zod';flags={};actualPlayers=Array(8).fill({});
}
// Actual pure predicates, including strict saved owner and stale context cases.
for(const [owner,known,expected]of [[undefined,undefined,true],[undefined,temple,true],
 [temple,temple,true],[foreign,temple,false],[foreign,undefined,false]])
 check(total_setup_original_owned(owner,known)===expected,'original ownership');
for(const [owner,known,original,installed,current,expected]of [
 [undefined,undefined,undefined,false,false,true],[temple,temple,undefined,false,false,true],
 [round_total_setup,undefined,undefined,true,true,true],[round_total_setup,temple,temple,true,true,true],
 [round_total_setup,temple,foreign,true,true,false],[round_total_setup,temple,temple,true,false,false],
 [temple,temple,temple,true,true,false],[foreign,temple,temple,true,true,false]])
 check(total_setup_owner_owned(owner,known,original,installed,current)===expected,'saved ownership');

// Installer: no foreign overwrite or self-wrapping on duplicate installation.
reset();delete level.eroded_coop_total_setup_saved;level.zombie_total_set_func=undefined;
check(install_round_seed_hook(1,'zm_zod')===true,'default installation');
check(level.zombie_total_set_func===round_total_setup&&level.eroded_coop_total_setup_original===undefined,'saved undefined original');
check(install_round_seed_hook(1,'zm_zod')===true&&level.eroded_coop_total_setup_original===undefined,'idempotent installation');
level.zombie_total_set_func=foreign;
check(install_round_seed_hook(1,'zm_zod')===false&&level.zombie_total_set_func===foreign,'do not reinstall after takeover');
check(!owns_callbacks(),'later takeover revokes callback readiness');
reset();delete level.eroded_coop_total_setup_saved;level.zombie_total_set_func=foreign;
check(install_round_seed_hook(1,'zm_zod')===false&&level.zombie_total_set_func===foreign&&foreign_calls===0,'initial unknown owner');
reset(temple);delete level.eroded_coop_total_setup_saved;level.zombie_total_set_func=temple;
check(install_round_seed_hook(1,'zm_zod')===true&&level.eroded_coop_total_setup_original===temple,'known Temple preserved');
reset();delete level.eroded_coop_total_setup_saved;level.zombie_total_set_func=undefined;currentEpoch=2;
check(install_round_seed_hook(1,'zm_zod')===false&&level.zombie_total_set_func===undefined,'stale installer');

// Normal pristine setup creates exactly one generation and immutable seed.
reset();round_total_setup();
check(level.eroded_coop_round_generation===1&&workers.length===1&&original_calls===0,'one default producer');
check(workers[0].seed.count===8&&workers[0].seed.pending===80&&workers[0].seed.consumed===0,'pristine stock seed');
const oldSeed=workers[0].seed;round_total_setup();
check(level.eroded_coop_round_generation===2&&workers.length===2,'one generation per callback');
check(!round_generation_current(oldSeed.generation,level.eroded_coop_round_generation,1,1),'old worker invalidated');
reset(temple);round_total_setup();
check(original_calls===1&&workers.length===1&&events.indexOf('original')<events.indexOf('worker'),'Temple once before worker');

// A failed seed must still preserve the actual known stock initializer once.
for(const change of [()=>level.zombie_total=79,()=>level.zombie_total=0,
 ()=>level.zombie_respawns=1,()=>level.zombie_total_subtract=-1,()=>level.players=[],
 ()=>level.players=undefined,()=>level.eroded_coop_round_generation=2147483647,
 ()=>level.player_too_many_players_check_func=foreign,
 ()=>level.max_zombie_func=foreign,()=>level.round_think_func=foreign,
 ()=>level.round_spawn_func=foreign,()=>level.fn_custom_round_ai_spawn=foreign]){
 reset(temple);change();const before=level.zombie_total;round_total_setup();
 check(original_calls===1&&workers.length===0&&foreign_calls===0&&level.zombie_total===before,'declined seed preserves original once');
}
for(const value of [false,0,'',{},1]){
 reset(temple);level.kill_counter_hud=value;round_total_setup();
 check(original_calls===1&&workers.length===0&&target_calls===0&&dvars.zm_coopRoundStatus==='kill_counter_budget_preserved','any-defined killHUD declines before getter');
}
reset(temple);level.zombie_total_set_func=foreign;round_total_setup();
check(original_calls===0&&workers.length===0&&level.eroded_coop_round_generation===undefined,'foreign current owner not invoked or replaced');
reset(temple);currentEpoch=2;round_total_setup();
check(original_calls===0&&workers.length===0,'stale epoch closed');
reset(temple);currentMap='zm_tomb';round_total_setup();
check(original_calls===0&&workers.length===0,'stale map closed');

// Verify post-original rechecks. The real Temple body is zero-yield and does not
// make these mutations; stand-ins ensure a changed owner/context cannot publish.
for(const mutation of [()=>level.zombie_total_set_func=foreign,()=>currentEpoch=2,()=>level.kill_counter_hud=0]){
 function mutatingKnown(){++original_calls;mutation();}
 reset(mutatingKnown);round_total_setup();
 check(original_calls===1&&workers.length===0,'post-original ownership/context guard');
}

// Capture uses the exact setter's level.players, not a newly joined/departed list.
reset();actualPlayers=Array(4).fill({});round_total_setup();
check(workers.length===1&&workers[0].seed.count===8,'transient actual roster does not change original seed');
reset();level.players=[];round_total_setup();
check(workers.length===0&&level.zombie_total===80,'empty roster never invents quota');

// Existing Moon ownership rules survive both restored-budget directions.
for(const total of [40,240,280]){
 reset();level.eroded_coop_profile_map='zm_moon';level.eroded_coop_known_round_think=moon_think;
 level.round_think_func=moon_think;level.on_the_moon=true;level._from_nml=true;
 level.round_number=3;level.nml_last_round=3;level.zombie_total=total;flags.teleporter_used=false;
 round_total_setup();check(workers.length===0&&target_calls===0&&level.zombie_total===total,'Moon restoration preserved');
}
reset();level.eroded_coop_profile_map='zm_moon';level.eroded_coop_known_round_think=moon_think;
level.round_think_func=moon_think;level.on_the_moon=true;level._from_nml=true;
level.round_number=3;level.nml_last_round=2;level.zombie_total=240;flags.teleporter_used=false;
round_total_setup();check(workers.length===1,'Moon fresh n+1 resumes');
level.on_the_moon=false;
check(round_controller_reason()==='moon_restored_budget_preserved','in-flight NML ownership recognized');
'''

LEGACY_CASES=r'''
let fixture_regression_checks=0,fixture_vm_checks=0;
function fixture_require(value,message){++fixture_regression_checks;assert.ok(value,message);}
function fixture_check(value){++fixture_vm_checks;return value?0:1;}
function custom_total_setup(){level.eroded_coop_test_setup_calls++;}
function custom_bgb_callback(){++foreign_calls;}
function custom_count_callback(maximum,round){level.eroded_coop_test_count_calls++;return maximum+round;}
function fixture_state(){return {...level};}
function same_fixture_state(before){
 for(const key of new Set([...Object.keys(before),...Object.keys(level)]))
  assert.equal(level[key],before[key],'Legacy fixture failed to restore '+key);
}
function fixture_context(installed,known,killhud){
 reset(known);
 if(!installed){
  level.eroded_coop_total_setup_saved=undefined;
  level.eroded_coop_total_setup_original=undefined;
  level.eroded_coop_total_setup_epoch=undefined;
  level.eroded_coop_total_setup_map=undefined;
  level.zombie_total_set_func=known;
 }
 if(killhud!==undefined)level.kill_counter_hud=killhud;
}
// The same old block can pass before installation and fail after installation.
// This is a deterministic control-flow regression, not attribution of a canary.
for(const known of [undefined,temple]){
 fixture_context(false,known);let before=fixture_state();
 fixture_require(legacy_fixture_without_isolation()===0,'Old block should pass before install');same_fixture_state(before);
 fixture_context(true,known);before=fixture_state();
 fixture_require(legacy_fixture_without_isolation()===2,'Old block must expose two installed-context failures');same_fixture_state(before);
 for(const installed of [false,true]){
  fixture_context(installed,known);before=fixture_state();const owned=owns_callbacks();
  fixture_require(legacy_fixture_with_isolation()===0,'Fixed legacy block must pass in both lifecycle contexts');
  same_fixture_state(before);
  fixture_require(owns_callbacks()===owned,'Fixed fixture changed live ownership');
  fixture_require(original_calls===0&&foreign_calls===0&&workers.length===0,'Fixture must not invoke game callbacks or launch a worker');
 }
}
for(const killhud of [false,0,{},1]){
 fixture_context(true,temple,killhud);const before=fixture_state();
 fixture_require(legacy_fixture_with_isolation()===0,'Known killHUD state must be isolated for legacy recognition tests');
 same_fixture_state(before);
 fixture_require(level.kill_counter_hud===killhud,'KillHUD state must be restored exactly');
 fixture_require(owns_callbacks(),'Installed setup context must remain live after fixture');
}
console.log(JSON.stringify({passed:true,checks,fixture_regression_checks,fixture_vm_checks,
 legacy_mutant_failures_before_install:0,legacy_mutant_failures_after_install:2,
 fixed_legacy_failures_before_and_after_install:0,
 scope:'Actual translated GSC bodies and exact legacy fixture block with explicit stand-ins; not VM scheduling, canary attribution or gameplay proof.'}));
'''


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--node',default=os.environ.get('NODE','node'),
                        help='Node executable, configured through --node, NODE or PATH')
    parser.add_argument('--output-dir',type=Path,default=ROOT/'test-output',
                        help='Parent of a new retained result directory; existing files are never removed')
    args=parser.parse_args()
    source_path=ROOT/'scripts/zombies_coop/eroded_coop.gsc'
    family='cb'
    if not source_path.is_file():
        source_path=ROOT/'data/custom_scripts/zm/eroded_coop.gsc'
        family='ezz'
    source=source_path.read_text(encoding='utf-8')
    bodies={name:function(source,'function '+name+'(') for name in HELPERS}
    if 'function round_listener(' in source or source.count('level.eroded_coop_round_generation = generation;')!=1:
        raise ValueError('Only the setup callback may produce round generations')
    callback=bodies['round_total_setup']
    for token in ('wait ','waittill(', 'getplayers()'):
        if token in mask(callback):raise ValueError('Capture introduces a yield or substitutes actual roster')
    if callback.index('run_original_total_setup(original, known)')>callback.index('if (!isdefined(seed))'):
        raise ValueError('Known original must run even on declined seed')
    if callback.count('run_original_total_setup(original, known)')!=1:
        raise ValueError('Original must be chained exactly once per current setup')
    installer=function(source,'function install(')
    if not (installer.index('if (!install_round_seed_hook(epoch, map))')<installer.index('level.player_too_many_players_check_func = &player_count_check;')
            <installer.index('setdvar("zm_coopReadyMap", map)')):
        raise ValueError('Failed seed installation must not publish expanded readiness')
    if 'round_setup_context_current() && round_setup_owner_owned()' not in bodies['owns_callbacks']:
        raise ValueError('Later owner/context loss must revoke heartbeat ownership')
    worker=function(source,'function track_round(')
    if not (worker.index('if (custom_round_controller())',worker.index('while (true)'))<worker.index('spawns = accounted_spawn_delta(')<worker.index('level.zombie_total = next_total;')):
        raise ValueError('KillHUD/Moon owner guard must precede accounting and writes')
    if 'if (isdefined(level.kill_counter_hud))' not in bodies['round_controller_reason']:
        raise ValueError('Any defined killHUD budget must retain ownership')
    if 'total != target || respawns != 0' not in bodies['make_round_seed']:
        raise ValueError('Never infer a consumed quota from a partial snapshot')
    fixture_path=ROOT/'tests/gsc/eroded_coop_selftest.gsc'
    fixture=fixture_path.read_text(encoding='utf-8')
    start=fixture.index('    saved_round_spawn = level.round_spawn_func;')
    end=fixture.index('    failures += test_round_thinker_ownership();',start)
    legacy=fixture[start:end]
    if re.search(r'\b(?:wait|waittill|thread|notify)\b',mask(legacy)):
        raise ValueError('Legacy fixture must restore state before any yield or callback thread')
    fields=('eroded_coop_total_setup_saved','eroded_coop_total_setup_original',
            'eroded_coop_total_setup_epoch','eroded_coop_total_setup_map','kill_counter_hud')
    for field in fields:
        if not re.search(r'\bsaved_\w+\s*=\s*level\.'+field+r'\s*;',legacy):
            raise ValueError('Legacy fixture does not save live '+field)
        if not re.search(r'level\.'+field+r'\s*=\s*undefined\s*;',legacy):
            raise ValueError('Legacy fixture does not isolate live '+field)
        if not re.search(r'level\.'+field+r'\s*=\s*saved_\w+\s*;',legacy):
            raise ValueError('Legacy fixture does not restore live '+field)
    old_legacy='\n'.join(line for line in legacy.splitlines() if not any('level.'+field in line for field in fields))
    legacy_functions=[]
    for name,body in [('legacy_fixture_with_isolation',legacy),('legacy_fixture_without_isolation',old_legacy)]:
        actual='function '+name+'() {\nfailures = 0;\n'+re.sub(r'\bcheck\(', 'fixture_check(',body)+'\nreturn failures;\n}'
        legacy_functions.append(translate(actual))
    stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    out=args.output_dir.resolve()/('seed-callbacks-'+family+'-'+stamp)
    out.mkdir(parents=True,exist_ok=False)
    js=out/'actual-functions.js'
    js.write_text(STANDINS+'\n'+'\n'.join(translate(body) for body in bodies.values())+'\n'+CASES+
                  '\n'+'\n'.join(legacy_functions)+'\n'+LEGACY_CASES,encoding='utf-8')
    result=subprocess.run([args.node,str(js)],capture_output=True,text=True,check=False)
    (out/'stdout.log').write_text(result.stdout,encoding='utf-8')
    (out/'stderr.log').write_text(result.stderr,encoding='utf-8')
    if result.returncode:raise RuntimeError('Offline actual-body test failed; see '+str(out))
    verdict=json.loads(result.stdout)
    report={**verdict,'family':family,'source':str(source_path),'source_sha256':hashlib.sha256(source_path.read_bytes()).hexdigest(),
        'test_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'generated_js_sha256':hashlib.sha256(js.read_bytes()).hexdigest(),
        'extracted_functions':list(bodies),'fixture_source_sha256':hashlib.sha256(fixture_path.read_bytes()).hexdigest(),
        'fixture_block_scope':'Exact selected legacy block, plus mutant removing only five state-isolation save/clear/restore fields',
        'proof':str(out/'RESULTS.private.json')}
    (out/'RESULTS.private.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
