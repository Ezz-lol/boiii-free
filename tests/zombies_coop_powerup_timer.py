"""Actual GSC powerup timer and ownership regressions under explicit stand-ins.

The unchanged timer body is pinned to the primary stock reference by SHA-256;
no private stock file is needed. Node executes actual selected GSC bodies after
mechanical translation. These checks do not prove native linking or game VM
scheduling, remote clients, or a universal crash cure.
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


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source', type=Path)
parser.add_argument('--baseline', type=Path, help='Optional prior controller source; all its function bodies must remain unchanged')
parser.add_argument('--tests-dir', type=Path, default=Path(__file__).resolve().parent)
parser.add_argument('--output-dir', type=Path)
parser.add_argument('--node', default=os.environ.get('NODE', 'node'))
args = parser.parse_args()
sys.dont_write_bytecode = True
sys.path.insert(0, str(args.tests_dir.resolve()))
from script_frame_diagnostics import function
import zombies_coop_disconnect_guards as existing

root = Path(__file__).resolve().parents[1]
if args.source is not None:
    source_path = args.source.resolve(strict=True)
else:
    source_path = root / 'scripts/zombies_coop/eroded_coop.gsc'
    if not source_path.is_file():
        source_path = root / 'data/custom_scripts/zm/eroded_coop.gsc'
source = source_path.read_text(encoding='utf-8')
sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
names = ('startup_identity_current', 'ending_latched', 'startup_current', 'disconnect_context_current',
         'endgame_cleanup_context_current', 'endgame_cleanup_owned', 'disconnect_player_list', 'disconnect_rosters_empty',
         'powerup_timer_ending_empty_current', 'guarded_powerup_timer')
bodies = {name: function(source, 'function ' + name + '(') for name in names}
guard = bodies['powerup_timer_ending_empty_current']
assert not re.search(r'\b(wait|waittill|thread|notify|endon|setdvar|spawn|delete)\b', existing.mask(guard))
guarded = bodies['guarded_powerup_timer']
stock_sound = 'getplayers()[ 0 ] playsoundtoteam( str_sound_off, player_team );'
copied = guarded[guarded.index('    str_index_on = "zombie_powerup_"'):]
stock_body = '{\n' + copied.replace('    if (!powerup_timer_ending_empty_current(epoch, map))\n        ' + stock_sound,
                                    '    ' + stock_sound)
# Independently pinned full primary timer body. The executable stock comparator
# comes only from this byte-verified body, never an unchecked copy of the change.
PRIMARY_BODY_SHA256 = '6a065db0603de61b50ecc851347c445d894e98bf6eab10f7f017d5875c1f6de9'
PRIMARY_FILE_SHA256 = '9b60593b77d81377dec7e282fbea0208facca782bd7216e726ec35e57a62ccdf'
PRIMARY_SOURCE = 'https://github.com/ate47/bo3-source/blob/fe040b1187b4850fe8da3c78b703c986c87d1028/scripts/zm/_zm_powerups.gsc'
assert hashlib.sha256(stock_body.encode()).hexdigest() == PRIMARY_BODY_SHA256, 'Copied timer differs from the pinned primary stock body beyond the single expiry guard'
retail = 'function time_remaining_on_powerup( player_team, str_powerup )\n' + stock_body
assert guarded.count('zm_powerups::time_remaining_on_powerup') == 1
assert 'guarded_powerup_timer(' not in retail
for include in ('#using scripts/zm/_zm_bgb;', '#using scripts/zm/_zm_powerups;'):
    assert source.count(include) == 1, include
unchanged = {}
if args.baseline:
    old = args.baseline.read_text(encoding='utf-8')
    old_names = re.findall(r'(?m)^function (?:autoexec )?(\w+)\(', old)
    new_names = re.findall(r'(?m)^function (?:autoexec )?(\w+)\(', source)
    for name in old_names:
        needle = 'function autoexec main(' if name == 'main' else 'function ' + name + '('
        before = function(old, needle)
        assert before == function(source, needle), name
        unchanged[name] = hashlib.sha256(before.encode()).hexdigest()
    assert set(new_names) - set(old_names) == {'powerup_timer_ending_empty_current', 'guarded_powerup_timer'}
    assert [line for line in source.splitlines() if line.startswith('#using') and line not in old.splitlines()] == [
        '#using scripts/zm/_zm_bgb;', '#using scripts/zm/_zm_powerups;']

base = existing.STANDINS
for stub in ("function startup_current(epoch,map){return epoch===currentEpoch&&map===currentMap;}",
             "function getdvarint(name){return 0;}", 'function getdvarstring(name){return "";}'):
    assert base.count(stub) == 1
    base = base.replace(stub, "")
base += r'''
let dvars={},gift=false,trace=[],temp,fallbacks=[];
function isstring(v){return typeof v==='string';}
function getdvarint(name){return name==='zm_coopStartupEpoch'?currentEpoch:(dvars[name]??0);}
function getdvarstring(name){return name==='mapname'?currentMap:(dvars[name]??'');}
function guarded_endgame(){throw new Error('Pointer identity only; timer must not invoke callback');}
function temporal_gift(name){assert.equal(name,'zm_bgb_temporal_gift');return gift;}
function spawn(kind,origin){assert.equal(kind,'script_origin');assert.deepEqual(origin,[0,0,0]);temp={entity:true,temp:true,deleted:false};trace.push('spawn');return temp;}
function native_method(entity,method,...args){if(!entity||entity.entity!==true||entity.deleted)throw new TypeError('undefined is not an entity');trace.push([method,entity.temp?'temp':entity.id,...args]);if(method==='delete')entity.deleted=true;}
function* fallback_stock(team,power){fallbacks.push({self:this,team,power});yield* stock_timer.call(this,team,power);}
'''


def timer_js(body, name):
    body = re.sub(r"function (?:guarded_powerup_timer|time_remaining_on_powerup)\(", "function " + name + "(", body, count=1)
    body = body.replace("bgb::is_team_enabled", "temporal_gift").replace("wait 0.05;", "yield 0.05;")
    body = body.replace("self zm_powerups::time_remaining_on_powerup(player_team, str_powerup);",
                        "yield* fallback_stock.call(this,player_team,str_powerup);")
    body = body.replace('spawn( "script_origin", ( 0, 0, 0 ) )', 'spawn("script_origin", [0,0,0])')
    body = body.replace(stock_sound, 'native_method(getplayers()[0], "playsoundtoteam", str_sound_off, player_team);')
    for method in ("playloopsound", "stoploopsound", "delete"):
        body = re.sub(r"temp_ent\s+" + method + r"\(\s*([^)]*)\)",
                      lambda match: 'native_method(temp_ent,"' + method + '"' + ("," + match[1] if match[1].strip() else "") + ")", body)
    return existing.translate(body).replace("function " + name + "(", "function* " + name + "(", 1)

CASES = r'''

const receiver={id:'original-level-receiver'};
function player(id,extra={}){return {entity:true,player:true,id,...extra};}
function reset(count,timer,temporal,ending=false,capacity=8,owned=true){
 currentEpoch=5;currentMap='zm_zod';gift=temporal;trace=[];temp=undefined;fallbacks=[];calls=[];events=[];
 const entries=Array.from({length:count},(_,i)=>[i,player(i)]);actualPlayers=garray(entries);
 level={players:garray(entries),zombie_vars:{allies:{zombie_powerup_insta_kill_on:1,zombie_powerup_insta_kill_time:timer}},
  eroded_coop_limit:capacity,eroded_coop_started_epoch:5,eroded_coop_started_map:'zm_zod',
  eroded_coop_disconnect_saved:owned,eroded_coop_disconnect_epoch:5,eroded_coop_disconnect_map:'zm_zod',
  eroded_coop_endgame_original:stock_endgame,onendgame:owned?guarded_endgame:stock_endgame};
 dvars={zm_coopAppliedLimit:capacity,zm_coopSchemaSlots:capacity,zm_coopStatus:'experimental_callbacks_installed',
  zm_coopReadyEpoch:5,zm_coopReadyMap:'zm_zod',zm_coopHeartbeat:10};if(ending)markTerminal();
}
function markTerminal(){level.eroded_coop_ending_epoch=5;level.eroded_coop_ending_map='zm_zod';Object.assign(dvars,
 {zm_coopEndingRevision:1,zm_coopEndingEpoch:5,zm_coopEndingMap:'zm_zod',zm_coopStatus:'ending',zm_coopReadyEpoch:0,zm_coopReadyMap:'',zm_coopHeartbeat:0});}
function run(worker,afterFirst){let waits=0,error=false;
 while(true){try{const step=worker.next();if(step.done)break;assert.equal(step.value,.05);waits++;if(waits===1&&afterFirst)afterFirst();assert.ok(waits<1400);}
 catch(e){error=e instanceof TypeError;if(!error)throw e;break;}}
 return {waits,error,trace:[...trace],on:level.zombie_vars.allies.zombie_powerup_insta_kill_on,
  time:level.zombie_vars.allies.zombie_powerup_insta_kill_time,deleted:!!temp&&temp.deleted};}
for(let count=1;count<=8;count++)for(const timer of [0,.1,1,30])for(const temporal of [false,true]){
 reset(count,timer,temporal);const stock=run(stock_timer.call(receiver,'allies','insta_kill'));
 reset(count,timer,temporal);check(endgame_cleanup_owned(),'actual normal ownership used');const next=run(guarded_powerup_timer.call(receiver,'allies','insta_kill'));
 check(JSON.stringify(stock)===JSON.stringify(next),'occupied1..8 complete timer/audio/cleanup trace identical');
 check(!next.error&&next.deleted&&next.on===0&&next.time===30&&fallbacks.length===0,'known occupied uses exact copy once');
}
for(let capacity=5;capacity<=8;capacity++)for(const timer of [0,.1,1,30])for(const temporal of [false,true]){
 reset(0,timer,temporal,true,capacity);check(powerup_timer_ending_empty_current(5,'zm_zod'),'actual strict terminal effective-empty guard');
 const before=JSON.stringify(dvars);const next=run(guarded_powerup_timer.call(receiver,'allies','insta_kill'));
 check(!next.error&&next.deleted&&next.on===0&&next.time===30,'terminal empty keeps timer reset loop stop delete');
 check(!next.trace.some(e=>Array.isArray(e)&&e[0]==='playsoundtoteam')&&JSON.stringify(dvars)===before,'skip only team sound without readiness writes');
}
for(let capacity=1;capacity<=4;capacity++)for(let count=0;count<=capacity;count++)for(const temporal of [false,true]){
 reset(count,.1,temporal,false,capacity,false);const stock=run(stock_timer.call(receiver,'allies','insta_kill'));
 reset(count,.1,temporal,false,capacity,false);const next=run(guarded_powerup_timer.call(receiver,'allies','insta_kill'));
 check(JSON.stringify(stock)===JSON.stringify(next),'default<=4 original worker behavior exactly retained');
 check(fallbacks.length===1&&fallbacks[0].self===receiver&&fallbacks[0].team==='allies'&&fallbacks[0].power==='insta_kill','fallback preserves receiver arguments exactly once');
}
for(const mutate of [()=>level.onendgame=foreign,()=>level.eroded_coop_endgame_original=foreign,
 ()=>level.eroded_coop_disconnect_saved=false,()=>currentEpoch=6,()=>currentMap='zm_tomb']){
 reset(1,.1,true);mutate();const next=run(guarded_powerup_timer.call(receiver,'allies','insta_kill'));
 check(!next.error&&next.deleted&&fallbacks.length===1,'unknown entry uses original worker once, no recursion');
}
for(const mutate of [()=>currentEpoch=6,()=>currentMap='zm_tomb',()=>level.eroded_coop_disconnect_epoch=6,
 ()=>level.eroded_coop_disconnect_map='zm_tomb',()=>level.eroded_coop_started_epoch=6,
 ()=>level.eroded_coop_ending_epoch=6,()=>level.eroded_coop_ending_map='zm_tomb',()=>level.onendgame=foreign,
 ()=>level.eroded_coop_endgame_original=foreign,()=>level.eroded_coop_disconnect_saved=false,
 ()=>dvars.zm_coopEndingRevision=0,()=>dvars.zm_coopEndingEpoch=6,()=>dvars.zm_coopEndingMap='zm_tomb',
 ()=>dvars.zm_coopReadyEpoch=5,()=>dvars.zm_coopReadyMap='zm_zod',()=>dvars.zm_coopHeartbeat=10,
 ()=>dvars.zm_coopSchemaSlots=7,()=>dvars.zm_coopAppliedLimit=4,()=>level.eroded_coop_limit=4]){
 reset(1,.1,false);const next=run(guarded_powerup_timer.call(receiver,'allies','insta_kill'),()=>{markTerminal();mutate();});
 check(!next.error&&next.trace.some(e=>Array.isArray(e)&&e[0]==='playsoundtoteam'),'expiry stale/foreign/mismatched state cannot suppress occupied stock sound');
 reset(0,.1,false);const empty=run(guarded_powerup_timer.call(receiver,'allies','insta_kill'),()=>{markTerminal();mutate();});
 check(empty.error&&!empty.deleted,'unknown empty expiry intentionally retains stock hazard, no universal cure claim');
}
reset(8,.1,false);const depart=run(guarded_powerup_timer.call(receiver,'allies','insta_kill'),()=>{actualPlayers=garray();level.players=garray();markTerminal();});
check(!depart.error&&depart.deleted,'occupied entry rechecks actual current ending-empty at expiry');
reset(0,.1,false,true);const join=run(guarded_powerup_timer.call(receiver,'allies','insta_kill'),()=>{actualPlayers=garray([[7,player(7)]]);level.players=garray([[7,actualPlayers[7]]]);});
check(join.error,'sparse getplayers array retains stock first-element assumption; malformed engine roster not silently repaired');
reset(0,.1,false,true);const joinDense=run(guarded_powerup_timer.call(receiver,'allies','insta_kill'),()=>{actualPlayers=garray([[0,player(7)]]);level.players=garray([[7,actualPlayers[0]]]);});
check(!joinDense.error&&joinDense.trace.some(e=>Array.isArray(e)&&e[0]==='playsoundtoteam'&&e[1]===7),'new occupied expiry uses stock actual first player');
for(const extra of [{alive:false},{downed:true},{playing:false},{}])for(const side of ['actual','listed']){
 reset(0,0,false,true);const survivor=player(7,extra);if(side==='actual')actualPlayers=garray([[0,survivor]]);else level.players=garray([[7,survivor]]);
 check(!powerup_timer_ending_empty_current(5,'zm_zod'),'dead/downed/spectator/unflagged survivor is never treated empty');
}
reset(0,.1,false,true);actualPlayers=garray([[0,player(0,{player_disconnected:1})],[7,player(7,{player_disconnected:1})]]);
level.players=garray([[3,player(3,{player_disconnected:1})]]);
check(powerup_timer_ending_empty_current(5,'zm_zod'),'only explicit disconnected entity entries may be effectively pruned');
check(!run(guarded_powerup_timer.call(receiver,'allies','insta_kill')).error,'effective empty tagged entities suppress only terminal sound');
for(const malformed of [undefined,{},garray([['foreign',player(0)]])]){
 reset(0,0,false,true);level.players=malformed;check(!powerup_timer_ending_empty_current(5,'zm_zod'),'unknown listed roster cannot grant suppression');
}
reset(0,0,false,false);check(run(guarded_powerup_timer.call(receiver,'allies','insta_kill')).error,'normal empty outside strict ending proof retains original hazard');
console.log(JSON.stringify({passed:true,checks,scope:'Actual helper dependencies and proposed timer, mechanically translated with explicit GSC sparse-array, ownership/dvar, scheduler, entity/audio and original-worker stand-ins. No native VM or raw import/link proof.'}));

'''

stamp = datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
output = (args.output_dir or root / 'test-output') / ('powerup-timer-' + stamp)
output.mkdir(parents=True, exist_ok=False)
js = output / 'actual-helper.cjs'
translated = [existing.translate(body) for name, body in bodies.items() if name != 'guarded_powerup_timer']
js.write_text(base + '\n' + '\n'.join(translated) + '\n' + timer_js(retail, 'stock_timer') + '\n'
              + timer_js(guarded, 'guarded_powerup_timer') + '\n' + CASES, encoding='utf-8')
run = subprocess.run([args.node, str(js)], capture_output=True, text=True, timeout=60)
(output / 'stdout.log').write_text(run.stdout, encoding='utf-8')
(output / 'stderr.log').write_text(run.stderr, encoding='utf-8')
assert run.returncode == 0, (output, run.stderr)
record = json.loads(run.stdout)
record.update(source_sha256=sha(source_path), test_sha256=sha(Path(__file__)), generated_js_sha256=sha(js),
              entire_copied_retail_body_matches_after_one_guard_removal=True,
              primary_body_sha256=PRIMARY_BODY_SHA256, primary_reference_file_sha256=PRIMARY_FILE_SHA256,
              primary_reference_source=PRIMARY_SOURCE, unchanged_baseline_function_count=len(unchanged),
              unchanged_baseline_functions=unchanged, production_changed=False, activation_performed=False,
              runtime_limitations=['No actual GSC VM link/autoexec cycle or native import dispatch proof.',
                 'Stale, unknown/custom or normal-empty expiry deliberately keeps original first-player sound behavior and can retain the original empty receiver hazard.',
                 'A sparse actual getplayers result is not repaired; the retained retail first-element assumption remains.',
                 'Fallback recursion avoidance requires exact caller-only metadata redirection preserving the original timer and helper fallback imports.'])
if args.baseline:
    record['baseline_sha256'] = sha(args.baseline)
(output / 'RESULTS.json').write_text(json.dumps(record, indent=2) + '\n', encoding='utf-8')
print(json.dumps(dict(proof=str(output / 'RESULTS.json'), passed=True, checks=record['checks'])))
