"""Offline actual-body Island readiness gate checks; not engine/VM proof."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,re,subprocess,sys
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tests'))
from script_frame_diagnostics import function

CASES=r'''
const assert=require('node:assert/strict');let checks=0,level={};
function check(value,message){++checks;assert.ok(value,message);}
function isdefined(value){return value!==undefined;}
function isint(value){return Number.isInteger(value)&&value>=-2147483648&&value<=2147483647;}
function isarray(value){return Array.isArray(value);}
function ready(epoch=17,capacity=8){level={eroded_island_trials_revision:3,eroded_island_trials_epoch:epoch,eroded_island_trials_capacity:capacity,eroded_island_trials_ready:1};}
for(const map of ['zm_zod','zm_factory','zm_tomb','usermap']){level={};check(island_trials_current(17,map,8),'other maps unaffected');}
for(let capacity=5;capacity<=8;capacity++){ready(17,capacity);check(island_trials_current(17,'zm_island',capacity),'current ready Island capacity');}
ready();level={};check(!island_trials_current(17,'zm_island',8),'missing early module closed');
for(const field of ['eroded_island_trials_revision','eroded_island_trials_epoch','eroded_island_trials_capacity','eroded_island_trials_ready']){
 ready();delete level[field];check(!island_trials_current(17,'zm_island',8),'missing field closed: '+field);
}
for(const [field,bad] of [
 ['eroded_island_trials_revision',1],['eroded_island_trials_revision',2],['eroded_island_trials_revision','3'],['eroded_island_trials_revision',[]],
 ['eroded_island_trials_epoch',16],['eroded_island_trials_epoch',18],['eroded_island_trials_epoch','17'],['eroded_island_trials_epoch',{}],
 ['eroded_island_trials_capacity',4],['eroded_island_trials_capacity',9],['eroded_island_trials_capacity','8'],['eroded_island_trials_capacity',[8]],
 ['eroded_island_trials_ready',0],['eroded_island_trials_ready',2],['eroded_island_trials_ready',{}],['eroded_island_trials_ready','1'],['eroded_island_trials_ready',[1]],['eroded_island_trials_ready',1.5]]){
 ready();level[field]=bad;check(!island_trials_current(17,'zm_island',8),'malformed/stale early metadata closed');
}
for(const epoch of [-1,0,1.25,'17',undefined,{}]){ready(epoch);check(!island_trials_current(epoch,'zm_island',8),'invalid matching epoch cannot bypass gate');}
for(const capacity of [-1,0,1,4,9,18,5.5,'8',undefined,[]]){ready(17,capacity);check(!island_trials_current(17,'zm_island',capacity),'invalid matching capacity cannot bypass gate');}
ready();check(island_trials_current(17,'zm_island',8),'ready before first player/pool initialization');
level.eroded_island_trials_ready=0;check(!island_trials_current(17,'zm_island',8),'revoked readiness closes active owner gate');
console.log(JSON.stringify({passed:true,checks,scope:'Actual readiness predicate with explicit GSC integer/undefined stand-ins and source wiring checks; no engine, client bootstrap or GSC VM proof.'}));
'''

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--node',default=os.environ.get('NODE','node'));p.add_argument('--output-dir',type=Path,default=ROOT/'test-output');a=p.parse_args()
    source=ROOT/'scripts/zombies_coop/eroded_coop.gsc';family='cb'
    if not source.is_file():source=ROOT/'data/custom_scripts/zm/eroded_coop.gsc';family='ezz'
    text=source.read_text(encoding='utf-8');helper=function(text,'function island_trials_current(')
    install=function(text,'function install(');owned=function(text,'function owns_callbacks(');heartbeat=function(text,'function diagnostic_heartbeat(')
    wait_start=install.index('while (');wait_end=install.index('expected_delay =')
    if '!island_trials_current(epoch, map, limit)' not in install[wait_start:wait_end]:raise ValueError('Startup no longer waits for early Island ownership/world readiness')
    if 'island_trials_current(getdvarint("zm_coopStartupEpoch"), getdvarstring("mapname"), level.eroded_coop_limit)' not in owned:raise ValueError('Ongoing ownership lost early Island gate')
    if '!owns_callbacks()' not in heartbeat or 'clear_readiness();' not in heartbeat:raise ValueError('Heartbeat cannot revoke native readiness')
    if any(token in helper for token in ('_challenges','flag_init_player_challenges','pools')):raise ValueError('Readiness must not await post-player stock pool initialization')
    # GSC boolean literals are integer values; this stand-in does not model VM float tagging.
    translated=re.sub(r'\btrue\b','1',re.sub(r'\bfalse\b','0',helper))
    stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');out=a.output_dir.resolve()/('island-gate-'+family+'-'+stamp);out.mkdir(parents=True,exist_ok=False)
    js=out/'actual-gate.js';js.write_text(CASES[:CASES.index('for(const map')]+'\n'+translated+'\n'+CASES[CASES.index('for(const map'):],encoding='utf-8')
    run=subprocess.run([a.node,str(js)],capture_output=True,text=True);(out/'stdout.log').write_text(run.stdout,encoding='utf-8');(out/'stderr.log').write_text(run.stderr,encoding='utf-8')
    if run.returncode:raise RuntimeError('Retained gate failure: '+str(out))
    report={**json.loads(run.stdout),'family':family,'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'test_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'generated_js_sha256':hashlib.sha256(js.read_bytes()).hexdigest(),'proof':str(out/'RESULTS.private.json')}
    (out/'RESULTS.private.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report,indent=2))
if __name__=='__main__':main()
