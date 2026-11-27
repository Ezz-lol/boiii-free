"""Actual terminal endgame cleanup ownership tests; no native/gameplay proof.

Uses the existing disconnect-guard translator and array/callback stand-ins.
Run with --source for a private proposal, or use repository defaults after review.
"""
from pathlib import Path
import argparse,datetime,hashlib,json,os,re,subprocess,sys
parser=argparse.ArgumentParser();parser.add_argument('--source',type=Path);parser.add_argument('--baseline',type=Path);parser.add_argument('--retail-source',type=Path);parser.add_argument('--tests-dir',type=Path,default=Path(__file__).resolve().parent);parser.add_argument('--output-dir',type=Path);parser.add_argument('--node',default=os.environ.get('NODE','node'));args=parser.parse_args()
sys.dont_write_bytecode=True;sys.path.insert(0,str(args.tests_dir.resolve()))
import zombies_coop_disconnect_guards as existing
from script_frame_diagnostics import function
root=Path(__file__).resolve().parents[1]
source=args.source or root/'scripts/zombies_coop/eroded_coop.gsc'
if not source.is_file():source=root/'data/custom_scripts/zm/eroded_coop.gsc'
text=source.read_text();stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');out=(args.output_dir or root/'test-output')/('terminal-endgame-'+stamp);out.mkdir(parents=True)
names=(*existing.HELPERS,'startup_identity_current','ending_latched','startup_current','endgame_cleanup_context_current','endgame_cleanup_owned')
bodies={name:function(text,'function '+name+'(')for name in names}
for name in('endgame_cleanup_context_current','endgame_cleanup_owned','guarded_endgame'):
    assert not re.search(r'\b(?:wait|waittill|thread|notify|endon|setdvar)\b',existing.mask(bodies[name])),name
base=existing.STANDINS
for normal_stub in ('function startup_current(epoch,map){return epoch===currentEpoch&&map===currentMap;}',
                    'function getdvarint(name){return 0;}', 'function getdvarstring(name){return "";}'):
    assert base.count(normal_stub)==1,normal_stub
    base=base.replace(normal_stub,'')
base+='''let dvars={};
function isstring(value){return typeof value==='string';}
function getdvarint(name){if(name==='zm_coopStartupEpoch')return currentEpoch;return dvars[name]===undefined?0:dvars[name];}
function getdvarstring(name){if(name==='mapname')return currentMap;return dvars[name]===undefined?'':dvars[name];}
'''
cases=existing.CASES[:existing.CASES.index('console.log(')]
cases+=r'''
function terminal(){reset();install();level.eroded_coop_limit=8;level.eroded_coop_started_epoch=1;level.eroded_coop_started_map='zm_zod';level.eroded_coop_ending_epoch=1;level.eroded_coop_ending_map='zm_zod';dvars={zm_coopAppliedLimit:8,zm_coopSchemaSlots:8,zm_coopEndingRevision:1,zm_coopEndingEpoch:1,zm_coopEndingMap:'zm_zod',zm_coopStatus:'ending',zm_coopReadyEpoch:0,zm_coopReadyMap:'',zm_coopHeartbeat:0};}
for(let capacity=5;capacity<=8;capacity++){
 terminal();level.eroded_coop_limit=capacity;dvars.zm_coopAppliedLimit=dvars.zm_coopSchemaSlots=capacity;
 check(!startup_current(1,'zm_zod')&&!disconnect_context_current()&&!endgame_guard_owned()&&!melee_guard_owned()&&!disconnect_guards_owned(),'terminal must preserve all normal ownership closure');
 check(endgame_cleanup_context_current()&&endgame_cleanup_owned(),'terminal current identity permits only cleanup');
 const before=JSON.stringify(dvars),self={},winner={};level.placement['all']=garray([[0,undefined],[7,tagged]]);
 guarded_endgame.call(self,winner);
 check(level.placement['all'].size===0&&calls.length===1&&calls[0].self===self&&calls[0].winner===winner,'terminal effective-empty prune and original exactly once');
 check(JSON.stringify(dvars)===before&&!startup_current(1,'zm_zod'),'cleanup cannot republish readiness/admission');
}
for(const mutate of [()=>currentEpoch=2,()=>currentMap='zm_tomb',()=>level.eroded_coop_disconnect_saved=false,
 ()=>level.eroded_coop_disconnect_epoch=2,()=>level.eroded_coop_disconnect_epoch='1',()=>level.eroded_coop_disconnect_epoch=true,()=>level.eroded_coop_disconnect_map='zm_tomb',()=>level.eroded_coop_disconnect_map=1,
 ()=>level.eroded_coop_started_epoch=2,()=>level.eroded_coop_started_epoch=undefined,()=>level.eroded_coop_started_map='zm_tomb',
 ()=>level.eroded_coop_ending_epoch=2,()=>level.eroded_coop_ending_map='zm_tomb',
 ()=>dvars.zm_coopAppliedLimit=4,()=>dvars.zm_coopAppliedLimit=9,()=>dvars.zm_coopSchemaSlots=7,
 ()=>level.eroded_coop_limit=undefined,()=>level.eroded_coop_limit='8',()=>level.eroded_coop_limit=true,
 ()=>level.eroded_coop_limit=4,()=>level.eroded_coop_limit=9,()=>{dvars.zm_coopAppliedLimit=dvars.zm_coopSchemaSlots=7;},
 ()=>dvars.zm_coopEndingRevision=0,()=>dvars.zm_coopEndingRevision=2,()=>dvars.zm_coopEndingEpoch=2,
 ()=>dvars.zm_coopEndingMap='zm_tomb',()=>dvars.zm_coopStatus='callback_lost',()=>dvars.zm_coopStatus='initializing',
 ()=>dvars.zm_coopReadyEpoch=1,()=>dvars.zm_coopReadyMap='zm_zod',()=>dvars.zm_coopHeartbeat=1,
 ()=>level.eroded_coop_endgame_original=foreign,()=>level.onendgame=foreign]){
 terminal();const placement=garray([[0,undefined],[7,tagged]]);level.placement['all']=placement;mutate();
 guarded_endgame.call({},undefined);check(!endgame_cleanup_owned()&&calls.length===0&&level.placement['all']===placement,'terminal stale/malformed/owner/positive-ready state must not prune or call');
}
for(const survivor of [dead,downed,spectator,p7])for(const side of ['actual','listed']){
 terminal();if(side==='actual')actualPlayers=garray([[7,survivor]]);else level.players=garray([[7,survivor]]);
 const placement=garray([[0,undefined],[7,tagged]]);level.placement['all']=placement;
 const self={},winner={};guarded_endgame.call(self,winner);
 check(level.placement['all']===placement&&calls.length===1&&calls[0].self===self&&calls[0].winner===winner,'nonempty/dead/downed/spectating human and order remain untouched');
}
for(const malformed of [undefined,{},garray([['bad',tagged]])]){
 terminal();actualPlayers=malformed;const placement=garray([[0,undefined],[7,tagged]]);level.placement['all']=placement;
 guarded_endgame.call({},undefined);check(level.placement['all']===placement&&calls.length===1,'unknown rosters preserve data and one known original chain');
}
'''
retail_sha=None
if args.retail_source:
    retail=args.retail_source.read_text();retail_sha=hashlib.sha256(args.retail_source.read_bytes()).hexdigest()
    retail_body=function(retail,'function gameend(').replace('function gameend(','function* stock_challenge_gameend(').replace('wait 0.05;','yield 0.05;').replace('scoreevents::processscoreevent','stock_score_event')
    body_js=existing.translate(retail_body.replace('function*','function',1)).replace('function stock_challenge_gameend(','function* stock_challenge_gameend(',1)
    cases+='\nfunction spawnstruct(){return {};}function gettime(){return 1000;}let challengeCalls=[];function dochallengecallback(name,data){challengeCalls.push([name,data.player]);}function stock_score_event(name,player){challengeCalls.push([name,player]);}\n'+body_js+r'''
// Pinned retail source exposes the stale-placement hazard; its exact raw opcode
// has not been captured, so this is a source stand-in rather than opcode proof.
terminal();level.teambased=false;level.placement['all']=garray([[0,undefined]]);let worker=stock_challenge_gameend(undefined);check(worker.next().value===.05,'actual retail worker yields first');let fault=false;try{worker.next();}catch(error){fault=error instanceof TypeError;}check(fault,'pinned retail source faults on undefined placement outside guarded callback');
terminal();level.teambased=false;level.placement['all']=garray([[0,undefined],[7,tagged]]);guarded_endgame.call({},undefined);worker=stock_challenge_gameend(undefined);worker.next();check(worker.next().done,'terminal effective-empty cleanup prevents this synthetic stale-placement access');
reset();dvars={};install();level.teambased=false;actualPlayers=garray([[0,p0],[7,p7]]);level.players=garray([[0,p0],[1,p7]]);level.placement['all']=garray([[0,p0],[1,p7]]);challengeCalls=[];guarded_endgame.call({},p0);worker=stock_challenge_gameend(p0);worker.next();worker.next();check(calls.length===1&&p0.completedgame===1&&p7.completedgame===1&&challengeCalls.length===2&&challengeCalls.every(row=>row[0]==='gameEnd'),'occupied human challenge callbacks and completed flags preserved');
'''
cases+='\nconsole.log(JSON.stringify({passed:true,checks,scope:"Actual GSC helper bodies and optional pinned retail source under explicit array/entity/scheduler stand-ins; no raw opcode, game VM, engine lifetime or runtime cure proof."}));\n'
def run_code(label,selected):
    js=out/(label+'.cjs');js.write_text(base+'\n'+'\n'.join(existing.translate(value)for value in selected.values())+'\n'+cases)
    run=subprocess.run([args.node,str(js)],capture_output=True,text=True,timeout=60);(out/(label+'.stdout.log')).write_text(run.stdout);(out/(label+'.stderr.log')).write_text(run.stderr);return run,js
run,js=run_code('proposal',bodies);assert run.returncode==0,(out,run.stderr)
record=json.loads(run.stdout);record.update(private=True,source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),generated_js_sha256=hashlib.sha256(js.read_bytes()).hexdigest(),retail_source_sha256=retail_sha,production_edited=False)
if args.baseline:
    old=args.baseline.read_text();unchanged={}
    for name in existing.HELPERS:
        if name in ('guarded_endgame','endgame_cleanup_context_current','endgame_cleanup_owned'):continue
        a=function(old,'function '+name+'(');assert a==bodies[name],name;unchanged[name]=hashlib.sha256(a.encode()).hexdigest()
    old_bodies=dict(bodies);old_bodies['guarded_endgame']=function(old,'function guarded_endgame(')
    negative,_=run_code('baseline27-negative',old_bodies);assert negative.returncode!=0,'Old27 must fail terminal fixture'
    record.update(unchanged_normal_helpers=unchanged,baseline27_negative_failed_as_expected=True,baseline_sha256=hashlib.sha256(args.baseline.read_bytes()).hexdigest())
(out/'RESULTS.private.json').write_text(json.dumps(record,indent=2)+'\n');print(json.dumps(dict(proof=str(out/'RESULTS.private.json'),passed=True,checks=record['checks'])))
