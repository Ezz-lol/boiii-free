"""Offline actual-body round-number diagnostic checks.

Executes mechanically translated production main/startup/heartbeat bodies using
explicit dvar, ownership and scheduler-yield stand-ins. No native/GSC VM or game
process is executed; endon cancellation and VM numeric tagging are not modeled.
"""
from pathlib import Path
import argparse,datetime,hashlib,json,os,re,subprocess,sys
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tests'))
from script_frame_diagnostics import function

def translate(body):
    masked=re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"',lambda m:' '*len(m[0]),body)
    parameters=set(filter(None,(v.strip() for v in re.search(r'function\s+(?:autoexec\s+)?\w+\(([^)]*)\)',body)[1].split(','))))
    variables=set(re.findall(r'(?<![\w.])([A-Za-z_]\w*)\s*=(?!=)',masked))-parameters
    body=body.replace('function autoexec main','function main').replace('level thread install','install')
    body=body.replace('getplayers().size','getplayers().length')
    body=re.sub(r'\s*level endon\(#"(?:intermission|end_game)"\);','',body)
    if 'wait 1;' in body:
        body=body.replace('function diagnostic_heartbeat','function* diagnostic_heartbeat').replace('wait 1;','yield 1;')
    opening=body.index('{')+1
    if variables:body=body[:opening]+'\nlet '+','.join(sorted(variables))+';\n'+body[opening:]
    return body

STANDINS=r'''
const assert=require('node:assert/strict');let checks=0,level={},dvars={},writes=[],installs=[],owned=true;
function check(value,message){checks++;assert.ok(value,message);}
function isdefined(value){return value!==undefined;}
function isint(value){return Number.isInteger(value)&&value>=-2147483648&&value<=2147483647;}
function getdvarint(name){return dvars[name]??0;}
function getdvarstring(name){return dvars[name]??'';}
function setdvar(name,value){writes.push([name,value]);dvars[name]=value;}
function clear_readiness(){setdvar('zm_coopReadyEpoch',0);setdvar('zm_coopReadyMap','');setdvar('zm_coopHeartbeat',0);}
function owns_callbacks(){return owned;}
function getplayers(){return [{},{}];}
function install(...args){installs.push(args);}
function reset(){level={};dvars={zm_coopStartupEpoch:17,mapname:'zm_castle',zm_coopAppliedLimit:8,zm_coopRoundNumber:91};writes=[];installs=[];owned=true;}
'''

CASES=r'''
reset();main();check(dvars.zm_coopRoundNumber===0,'new startup resets stale prior round to explicit unknown');
check(installs.length===1&&installs[0][0]===8&&installs[0][1]===17&&installs[0][2]==='zm_castle','startup installation inputs preserved');
check(!Object.hasOwn(level,'round_number'),'diagnostic startup never creates gameplay round');
for(const prior of[0,1,9,999]){reset();level.eroded_coop_started_epoch=17;level.eroded_coop_started_map='zm_castle';dvars.zm_coopRoundNumber=prior;main();check(writes.length===0&&dvars.zm_coopRoundNumber===prior&&installs.length===0,'duplicate autoexec preserves current diagnostic');}
for(const map of['zm_zod','zm_temple','zm_moon']){reset();dvars.mapname=map;level.round_number=7;main();check(dvars.zm_coopRoundNumber===0&&level.round_number===7,'new map reset is diagnostic only');}
for(const limit of[0,4,9]){reset();dvars.zm_coopAppliedLimit=limit;level.round_number=4;main();check(dvars.zm_coopRoundNumber===0&&level.round_number===4&&installs.length===0,'inactive/invalid startup keeps gameplay untouched and unknown round');}

for(const value of[undefined,-1,-20,0,1.5,'5',true,{},[],null]){
 reset();if(value!==undefined)level.round_number=value;const before=level.round_number;const it=diagnostic_heartbeat(17,'zm_castle');const yielded=it.next();
 check(!yielded.done&&yielded.value===1&&dvars.zm_coopRoundNumber===0,'missing/invalid observed round publishes unknown');
 check(level.round_number===before,'round observation never rewrites invalid gameplay value');
 it.return();
}
for(const map of['zm_zod','zm_castle','zm_moon','zm_temple','zm_island']){
 for(const value of[1,5,27,2147483647]){
  reset();dvars.mapname=map;level.round_number=value;level.special_round_type='owned-special';
  const before=JSON.stringify(level);const it=diagnostic_heartbeat(17,map);it.next();
  check(dvars.zm_coopRoundNumber===value&&JSON.stringify(level)===before,'normal/special current observed round published without gameplay mutation');
  it.return();
 }
}
for(const change of[()=>dvars.zm_coopStartupEpoch=18,()=>dvars.mapname='zm_tomb']){
 reset();level.round_number=10;change();const it=diagnostic_heartbeat(17,'zm_castle');
 check(it.next().done&&writes.length===0&&dvars.zm_coopRoundNumber===91,'stale entry cannot overwrite new diagnostic');
}
for(const change of[()=>dvars.zm_coopStartupEpoch=18,()=>dvars.mapname='zm_tomb']){
 reset();level.round_number=10;const it=diagnostic_heartbeat(17,'zm_castle');it.next();
 change();level.round_number=99;dvars.zm_coopRoundNumber=7;writes=[];
 check(it.next().done&&writes.length===0&&dvars.zm_coopRoundNumber===7,'old resumed worker cannot overwrite new epoch/map');
}
reset();level.round_number=11;owned=false;let it=diagnostic_heartbeat(17,'zm_castle');check(it.next().done&&!writes.some(w=>w[0]==='zm_coopRoundNumber'),'unknown ownership exits before publication');
reset();level.round_number=2;it=diagnostic_heartbeat(17,'zm_castle');it.next();level.round_number=3;it.next();check(dvars.zm_coopRoundNumber===3,'successive owned observations progress');
delete level.round_number;it.next();check(dvars.zm_coopRoundNumber===0,'missing round in later special/teardown observation becomes unknown');it.return();
console.log(JSON.stringify({passed:true,checks,scope:'Actual production main/startup_current/heartbeat bodies with explicit dvar/ownership/array/scheduler stand-ins. No GSC VM, engine scheduling or runtime diagnosis claim.'}));
'''

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--node',default=os.environ.get('NODE','node'))
    parser.add_argument('--output-dir',type=Path,default=ROOT/'test-output')
    args=parser.parse_args()
    source=ROOT/'scripts/zombies_coop/eroded_coop.gsc';family='cb'
    if not source.is_file():source=ROOT/'data/custom_scripts/zm/eroded_coop.gsc';family='ezz'
    text=source.read_text(encoding='utf-8')
    actual={name:function(text,signature) for name,signature in [('main','function autoexec main('),('startup','function startup_current('),('heartbeat','function diagnostic_heartbeat(')]}
    assert actual['main'].index('return;')<actual['main'].index('setdvar("zm_coopRoundNumber", 0)')
    assert actual['heartbeat'].index('!startup_current')<actual['heartbeat'].index('!owns_callbacks')<actual['heartbeat'].index('setdvar("zm_coopRoundNumber"')<actual['heartbeat'].index('wait 1;')
    assert not re.search(r'level\.round_number\s*=',actual['main']+actual['heartbeat'])
    assert text.count('setdvar("zm_coopRoundNumber"')==2
    assert 'isdefined(level.round_number) && isint(level.round_number) && level.round_number > 0' in actual['heartbeat']
    assert not re.search(r'level\.[A-Za-z_]\w*\s*=',actual['heartbeat'])
    out=args.output_dir.resolve()/('round-diagnostics-'+family+'-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'))
    out.mkdir(parents=True,exist_ok=False)
    js=out/'actual-functions.js';js.write_text(STANDINS+'\n'+'\n\n'.join(translate(v) for v in actual.values())+'\n'+CASES,encoding='utf-8')
    command=[args.node,str(js)]
    run=subprocess.run(command,cwd=ROOT,capture_output=True,text=True)
    (out/'stdout.log').write_text(run.stdout,encoding='utf-8');(out/'stderr.log').write_text(run.stderr,encoding='utf-8')
    result={'passed':run.returncode==0,'family':family,'exit_code':run.returncode,'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'test_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'actual_functions':list(actual),'source_contract_checks':6,'command':command}
    if run.returncode==0:result.update(json.loads(run.stdout))
    (out/'RESULTS.private.json').write_text(json.dumps(result,indent=2),encoding='utf-8');print(json.dumps({'proof':str(out/'RESULTS.private.json'),**result},indent=2));return run.returncode

if __name__=='__main__':raise SystemExit(main())
