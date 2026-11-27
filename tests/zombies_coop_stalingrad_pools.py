"""Execute the actual flat Stalingrad receipt predicate with typed stand-ins.

This is a source/control-flow test, not GSC scheduling or engine proof.
"""
from pathlib import Path
import argparse,datetime,hashlib,json,os,re,subprocess,sys
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tests'))
from script_frame_diagnostics import function
CASES=r"""
const assert=require('node:assert/strict');let checks=0,level={},dvars={},now=10000;
function check(v,m){checks++;assert.ok(v,m);}
function isdefined(v){return v!==undefined;}
function isint(v){return Number.isInteger(v)&&v>=-2147483648&&v<=2147483647;}
function isstring(v){return typeof v==='string';}
function gettime(){return now;}
function getdvarint(k){return dvars[k]??0;}function getdvarstring(k){return dvars[k]??'';}
function seed(cap=8,epoch=17){now=10000;dvars={mapname:'zm_stalingrad',zm_coopStartupEpoch:epoch,zm_coopAppliedLimit:cap,zm_coopSchemaSlots:cap};level={eroded_stalingrad_pool_revision:1,eroded_stalingrad_pool_map:'zm_stalingrad',eroded_stalingrad_pool_epoch:epoch,eroded_stalingrad_pool_capacity:cap,eroded_stalingrad_pool_catalog_size:Math.max(6,cap),eroded_stalingrad_pool_tick:now,eroded_stalingrad_pool_ready:1};}
for(const map of ['zm_zod','zm_factory','zm_tomb','usermap']){level={};check(stalingrad_pools_current(17,map,8),'unrelated map needs no Stalingrad module');}
for(let cap=5;cap<=8;cap++){seed(cap);check(stalingrad_pools_current(17,'zm_stalingrad',cap),'current catalog capacity');}
seed();for(const key of Object.keys(level)){const saved=level[key];delete level[key];check(!stalingrad_pools_current(17,'zm_stalingrad',8),'missing receipt field');level[key]=saved;}
for(const key of ['eroded_stalingrad_pool_revision','eroded_stalingrad_pool_epoch','eroded_stalingrad_pool_capacity','eroded_stalingrad_pool_catalog_size','eroded_stalingrad_pool_tick','eroded_stalingrad_pool_ready'])for(const value of ['1',[],{},undefined,1.5]){seed();level[key]=value;check(!stalingrad_pools_current(17,'zm_stalingrad',8),'wrong typed scalar');}
for(const [key,value]of [['eroded_stalingrad_pool_revision',0],['eroded_stalingrad_pool_revision',2],['eroded_stalingrad_pool_epoch',16],['eroded_stalingrad_pool_epoch',18],['eroded_stalingrad_pool_capacity',4],['eroded_stalingrad_pool_capacity',9],['eroded_stalingrad_pool_catalog_size',6],['eroded_stalingrad_pool_catalog_size',9],['eroded_stalingrad_pool_ready',0],['eroded_stalingrad_pool_ready',2],['eroded_stalingrad_pool_map','zm_zod'],['eroded_stalingrad_pool_map',[]],['eroded_stalingrad_pool_tick',-1],['eroded_stalingrad_pool_tick',10001],['eroded_stalingrad_pool_tick',8999]]){seed();level[key]=value;check(!stalingrad_pools_current(17,'zm_stalingrad',8),'drift or stale receipt');}
seed();level.eroded_stalingrad_pool_tick=9000;check(stalingrad_pools_current(17,'zm_stalingrad',8),'exact1000ms boundary');now=10001;check(!stalingrad_pools_current(17,'zm_stalingrad',8),'aged receipt expires');
for(const [key,value]of [['mapname','zm_zod'],['zm_coopStartupEpoch',18],['zm_coopAppliedLimit',4],['zm_coopSchemaSlots',4]]){seed();dvars[key]=value;check(!stalingrad_pools_current(17,'zm_stalingrad',8),'current native tuple drift');}
for(const epoch of [-1,0,1.5,'17',undefined,{}]){seed();check(!stalingrad_pools_current(epoch,'zm_stalingrad',8),'bad expected epoch');}
for(const cap of [-1,0,1,4,9,18,5.5,'8',undefined,[]]){seed();check(!stalingrad_pools_current(17,'zm_stalingrad',cap),'bad expected capacity');}
seed();const before=JSON.stringify({level,dvars,now});check(stalingrad_pools_current(17,'zm_stalingrad',8),'ready before read-only check');check(JSON.stringify({level,dvars,now})===before,'predicate has no writes');
console.log(JSON.stringify({passed:true,checks,scope:'Actual flat receipt predicate and source wiring under typed stand-ins; no game/GSC VM/client proof.'}));
"""
def main():
 p=argparse.ArgumentParser();p.add_argument('--node',default=os.environ.get('NODE','node'));p.add_argument('--output-dir',type=Path,default=ROOT/'test-output');a=p.parse_args()
 source=ROOT/'scripts/zombies_coop/eroded_coop.gsc'
 if not source.is_file():source=ROOT/'data/custom_scripts/zm/eroded_coop.gsc'
 text=source.read_text();body=function(text,'function stalingrad_pools_current(');install=function(text,'function install(');owned=function(text,'function owns_callbacks(')
 if '#using custom_scripts/zm/eroded_coop_stalingrad_challenges_entry' in text or '#using scripts/zm/zm_stalingrad_challenges' in text:raise ValueError('Map-only module imported into common controller')
 if '!stalingrad_pools_current(epoch, map, limit)' not in install[install.index('while ('):install.index('expected_delay =')]:raise ValueError('Initial common gate missing')
 if 'stalingrad_pools_current(getdvarint("zm_coopStartupEpoch"), getdvarstring("mapname"), level.eroded_coop_limit)' not in owned:raise ValueError('Ongoing common gate missing')
 if install.count('!stalingrad_pools_current(epoch, map, limit)')<4:raise ValueError('Ready publication lacks pre/post heartbeat map receipt guards')
 translated=re.sub(r'\btrue\b','1',re.sub(r'\bfalse\b','0',body));opening=translated.index('{')+1;translated=translated[:opening]+'\nlet expected_size,now;\n'+translated[opening:]
 marker=CASES.index('for(const map');out=a.output_dir/('stalingrad-pool-gate-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));out.mkdir(parents=True,exist_ok=False)
 js=out/'actual-gate.js';js.write_text(CASES[:marker]+translated+'\n'+CASES[marker:]);run=subprocess.run([a.node,str(js)],capture_output=True,text=True)
 (out/'stdout.log').write_text(run.stdout);(out/'stderr.log').write_text(run.stderr)
 if run.returncode:raise RuntimeError('Retained actual-gate failure: '+str(out))
 result={**json.loads(run.stdout),'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'test_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'proof':str(out/'RESULTS.private.json')};(out/'RESULTS.private.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
if __name__=='__main__':main()
