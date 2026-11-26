"""Offline actual-body checks for the narrowly owned disconnect guards.

Selected production GSC bodies are mechanically translated into JavaScript.
Explicit sparse GSC-array and callback stand-ins check ownership, filtering and
delegation only; this does not prove native pointer ABI, VM scheduling, retail
callbacks, disconnect cleanup, map gameplay or client behavior.
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

HELPERS=('disconnect_endgame_pending','disconnect_player_list','disconnect_rosters_empty','disconnect_context_current',
    'disconnect_originals_stock','disconnect_guards_owned','endgame_guard_owned','melee_guard_owned','disconnect_guard_installable',
    'install_disconnect_guards','endgame_cleanup_context_current','endgame_cleanup_owned','guarded_endgame','guarded_window_melee')


def mask(source):
    return re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"',lambda m:' '*len(m[0]),source)


def translate(body):
    parameters=set(filter(None,(item.strip() for item in re.search(r'function\s+\w+\(([^)]*)\)',body)[1].split(','))))
    locals_=set(re.findall(r'(?<![\w.])([A-Za-z_]\w*)\s*=(?!=)',mask(body)))-parameters
    result=body
    for original,replacement in (('&zm_gametype::onendgame','stock_endgame'),
        ('&globallogic::blank','stock_blank'),
        ('&zm_behavior::notetrackboardmelee','stock_melee'),
        ('&guarded_endgame','guarded_endgame'),('&guarded_window_melee','guarded_window_melee')):
        result=result.replace(original,replacement)
    result=re.sub(r'\bself\s+\[\[\s*([A-Za-z_][\w.]*)\s*\]\]\s*\(([^;]*)\)',r'\1.call(this,\2)',result)
    result=re.sub(r'\[\[\s*([A-Za-z_][\w.]*)\s*\]\]\s*\(([^;]*)\)',r'\1.call(this,\2)',result)
    result=re.sub(r'\bself\b','this',result)
    result=re.sub(r'\bforeach\s*\(\s*(\w+)\s+in\s+([^)]*)\)',r'for (const \1 of arrayValues(\2))',result)
    result=re.sub(r'=\s*\[\s*\]\s*;', '= garray();',result)
    result=re.sub(r'return\s*\[\s*\]\s*;', 'return garray();',result)
    if locals_:
        opening=result.index('{')+1
        result=result[:opening]+'\nlet '+','.join(sorted(locals_))+';\n'+result[opening:]
    if '::' in mask(result) or '[[' in mask(result):raise ValueError('Unsupported translation seam')
    return result


STANDINS=r'''
const assert=require('node:assert/strict');
let checks=0;function check(value,message){++checks;assert.ok(value,message);}
const arrayMarker=Symbol('gsc-array'), arrayEntries=Symbol('gsc-entries'),arrayWrites=Symbol('gsc-writes');
function garray(entries=[]){
 const values=new Map(entries);
 const value={};Object.defineProperty(value,arrayMarker,{value:true});
 Object.defineProperty(value,arrayEntries,{value:values});
 Object.defineProperty(value,arrayWrites,{value:0,writable:true});
 return new Proxy(value,{
  get(target,key){if(key==='size')return values.size;
   if(typeof key==='symbol')return target[key];
   const integer=/^-?\d+$/.test(key)?Number(key):key;return values.get(integer);},
  set(target,key,entry){const integer=typeof key==='string'&&/^-?\d+$/.test(key)?Number(key):key;values.set(integer,entry);target[arrayWrites]++;return true;}
 });
}
function array(...values){return garray(values.map((entry,index)=>[index,entry]));}
function getarraykeys(value){assert.ok(isarray(value),'getarraykeys requires array');return garray([...value[arrayEntries].keys()].map((key,index)=>[index,key]));}
function arrayValues(value){assert.ok(isarray(value));return [...value[arrayEntries].values()];}
function isarray(value){return value!==undefined&&value!==null&&value[arrayMarker]===true;}
function isdefined(value){return value!==undefined;}
function isint(value){return Number.isInteger(value)&&value>=-2147483648&&value<=2147483647;}
function isentity(value){return value!==undefined&&value!==null&&value.entity===true;}
function isplayer(value){return isentity(value)&&value.player===true;}
let level={},currentEpoch=1,currentMap='zm_zod',actualPlayers=garray(),calls=[],events=[];
function startup_current(epoch,map){return epoch===currentEpoch&&map===currentMap;}
// Normal ownership fixtures never manufacture a terminal receipt.
function getdvarint(name){return 0;}
function getdvarstring(name){return "";}
function getplayers(){return actualPlayers;}
function diagnostic_event(value){events.push(value);}
function setdvar(){}
function stock_endgame(winner){calls.push({kind:'endgame',self:this,winner,players:level.players});}
function stock_melee(entity){calls.push({kind:'melee',self:this,entity,targets:entity.first_node===undefined?undefined:entity.player_targets,first_node:entity.first_node});}
function foreign(){throw new Error('Unknown/custom callback must not be invoked');}
function stock_blank(){throw new Error('Pending callback must never be invoked by readiness probe');}
'''

CASES=r'''
function player(id,fields={}){return {entity:true,player:true,id,...fields};}
function sameValues(actual,expected,message){check(isarray(actual),message+' array');check(arrayValues(actual).length===expected.length&&arrayValues(actual).every((value,index)=>value===expected[index]),message+' identities/order');}
const p0=player(0),p1=player(1),p3=player(3),p7=player(7);
for(const [owner,pending] of [[undefined,true],[stock_blank,true],[stock_endgame,false],
 [foreign,false],[guarded_endgame,false],[stock_melee,false]])
 check(disconnect_endgame_pending(owner)===pending,'only missing/temporary stock blank waits; unknown owners decline immediately');
function reset(){
 level={onendgame:stock_endgame,_notetrack_handler:garray([['zombie_window_melee',stock_melee]]),players:garray(),placement:garray([['all',garray()]])};
 currentEpoch=1;currentMap='zm_zod';actualPlayers=garray();calls=[];events=[];
}
function install(){check(install_disconnect_guards(1,'zm_zod')===true,'known owner installation');}
// Sparse slot keys are bounded and traversed in their original numeric slot
// order; gaps are not mistaken for dense .size bounds. No survivor deduplication.
for(const entries of [[],[[7,p7]],[[3,p3],[7,p7]],[[7,p7],[0,p0],[3,p3]],[[0,p0],[3,p0],[7,p7]]]){
 const input=garray(entries),before=[...input[arrayEntries].entries()];
 const expected=[...entries].sort((a,b)=>a[0]-b[0]).map(item=>item[1]);
 sameValues(disconnect_player_list(input),expected,'sparse original slot order');
 check([...input[arrayEntries].entries()].every((item,index)=>item[0]===before[index][0]&&item[1]===before[index][1]),'input not mutated');
}
for(const input of [undefined,0,false,'players',{},garray([['0',p0]]),garray([['extra',p0]]),
 garray([[-1,p0]]),garray([[8,p0]]),garray([[2147483647,p0]]),garray([[.5,p0]]),
 garray(Array.from({length:9},(_,i)=>[i,player(i)])),garray([[0,{}]]),
 garray([[0,{entity:true,player:false}]]),garray([[0,1]]),garray([[0,'player']])])
 check(disconnect_player_list(input)===undefined,'malformed array/key/player declines');
const tagged=player(2,{player_disconnected:1}),dead=player(4,{alive:false}),spectator=player(5,{playing:false}),downed=player(6,{downed:true});
sameValues(disconnect_player_list(garray([[0,undefined],[1,tagged],[3,dead],[5,spectator],[7,downed]])),[dead,spectator,downed],'prune undefined/explicit disconnect only');
for(const tag of [undefined,0,2,false]){
 const retained=player(2,{player_disconnected:tag});sameValues(disconnect_player_list(garray([[5,retained]])),[retained],'unflagged entity remains');
}
for(const [actual,listed,empty]of [[garray(),garray(),true],[garray([[7,tagged]]),garray([[3,undefined]]),true],
 [garray([[7,dead]]),garray(),false],[garray(),garray([[7,spectator]]),false],
 [garray(),undefined,false],[garray([['0',tagged]]),garray(),false]])
 check(disconnect_rosters_empty(actual,listed)===empty,'effective emptiness requires both valid lists');
for(const [endgame,melee,owned]of [[stock_endgame,stock_melee,true],[foreign,stock_melee,false],
 [stock_endgame,foreign,false],[undefined,stock_melee,false],[stock_endgame,undefined,false]])
 check(disconnect_originals_stock(endgame,melee)===owned,'exact stock original pointers');

// Installer preserves unknown owners, saves known pointers once and cannot
// silently reacquire a replaced callback or cross a map/epoch boundary.
for(const mutate of [()=>level.onendgame=foreign,()=>level._notetrack_handler['zombie_window_melee']=foreign,
 ()=>level._notetrack_handler=undefined,()=>level._notetrack_handler={},()=>currentEpoch=2,()=>currentMap='zm_tomb']){
 reset();mutate();const oldEnd=level.onendgame,oldTable=level._notetrack_handler;
 check(install_disconnect_guards(1,'zm_zod')===false,'unknown/stale installation declines');
 check(level.onendgame===oldEnd&&level._notetrack_handler===oldTable&&level.eroded_coop_disconnect_saved===undefined,'declined install untouched');
}
reset();install();check(disconnect_guards_owned(),'owned installed context');
check(level.eroded_coop_endgame_original===stock_endgame&&level.eroded_coop_melee_original===stock_melee,'exact originals saved');
check(install_disconnect_guards(1,'zm_zod')===true&&level.eroded_coop_endgame_original===stock_endgame,'idempotent installation no self-wrap');
for(const [mutate,expected] of [[()=>level.onendgame=foreign,'melee'],[()=>level._notetrack_handler['zombie_window_melee']=foreign,'endgame'],
 [()=>level.eroded_coop_endgame_original=foreign,'melee'],[()=>level.eroded_coop_melee_original=foreign,'endgame'],
 [()=>currentEpoch=2,undefined],[()=>currentMap='zm_tomb',undefined],[()=>level.eroded_coop_disconnect_saved=false,undefined]]){
 reset();install();mutate();const oldEnd=level.onendgame,oldMelee=level._notetrack_handler['zombie_window_melee'];
 check(!disconnect_guards_owned()&&!install_disconnect_guards(1,'zm_zod'),'owner/context loss cannot reacquire');
 guarded_endgame.call({id:'self'},p0);guarded_window_melee.call({id:'self'},{entity:true});
 check(calls.length===(expected===undefined?0:1)&&(expected===undefined||calls[0].kind===expected)&&level.onendgame===oldEnd&&level._notetrack_handler['zombie_window_melee']===oldMelee,'each still-owned callback preserved; no stale original or overwrite');
}

// Effective-empty endgame prunes placement once, yet always chains exactly one
// stock no-op callback with the actual self and unchanged winner argument.
for(const placements of [[[0,undefined],[4,tagged]],[[0,dead],[3,undefined],[6,spectator],[7,dead]]]){
 reset();install();actualPlayers=garray([[7,tagged]]);level.players=garray([[3,undefined]]);
 const prior=garray(placements);level.placement['all']=prior;const writes=level.placement[arrayWrites],self={id:'end-self'},winner={id:'winner'};
 guarded_endgame.call(self,winner);
 check(calls.length===1&&calls[0].kind==='endgame'&&calls[0].self===self&&calls[0].winner===winner,'one same-self/winner original');
 sameValues(level.placement['all'],placements.map(row=>row[1]).filter(value=>value!==undefined&&value.player_disconnected!==1),'valid empty cleanup');
 check(level.placement['all']!==prior,'cleanup assignment used retained array');
 check(level.placement[arrayWrites]===writes+1,'placement cleanup assigned exactly once');
}
for(const [actual,listed]of [[garray([[7,dead]]),garray()],[garray(),garray([[7,spectator]])],
 [garray([['bad',tagged]]),garray()],[undefined,garray()],[garray(),undefined]]){
 reset();install();actualPlayers=actual;level.players=listed;const placement=garray([[7,tagged]]);level.placement['all']=placement;
 const self={},winner={};guarded_endgame.call(self,winner);
 check(calls.length===1&&calls[0].self===self&&calls[0].winner===winner&&level.placement['all']===placement,'present/ambiguous rosters preserve placement and original');
}
for(const placement of [undefined,{},garray([['all',undefined]]),garray([['all',garray([['bad',tagged]])]])]){
 reset();install();level.placement=placement;const all=placement&&isarray(placement)?placement['all']:undefined;
 guarded_endgame.call({},undefined);
 check(calls.length===1&&calls[0].winner===undefined&&level.placement===placement&&(!isarray(placement)||placement['all']===all),'malformed placement unchanged yet original once');
}

// Window melee filters only the cached player list. Retail node/trigger/weapon
// semantics and the first_node-undefined fallback remain outside this guard.
for(const first of [undefined,{}, {trigger_location:{}}, {origin:[0,0,0]}]){
 reset();install();const previous=garray([[0,undefined],[1,tagged],[4,dead],[6,spectator],[7,dead]]);
 let targets=previous,writes=0;const npc={entity:true,first_node:first,meleeweapon:{retail:true}},self={id:'melee-self'};
 Object.defineProperty(npc,'player_targets',{get(){return targets;},set(value){targets=value;++writes;}});
 guarded_window_melee.call(self,npc);
 check(calls.length===1&&calls[0].self===self&&calls[0].entity===npc,'one same-self/entity melee original');
 check(npc.first_node===first&&npc.meleeweapon.retail===true,'retail node and weapon untouched');
 if(first===undefined)check(npc.player_targets===previous&&writes===0,'undefined-node fallback target cache untouched');
 else {sameValues(npc.player_targets,[dead,spectator,dead],'stale target cleanup retains valid identities');check(writes===1,'target cache assigned exactly once');}
}
reset();install();const fallback={entity:true};
Object.defineProperty(fallback,'player_targets',{get(){throw new Error('fallback must not inspect target cache');},set(){throw new Error('fallback must not write target cache');}});
guarded_window_melee.call({},fallback);check(calls.length===1&&calls[0].entity===fallback,'undefined-node fallback never reads malformed cache');
for(const invalid of [undefined,0,{}, {entity:false}, {entity:true,first_node:{},player_targets:undefined},
 {entity:true,first_node:{},player_targets:garray([['bad',p0]])},
 {entity:true,first_node:{},player_targets:garray([[0,{}]])}]){
 reset();install();const targets=invalid&&invalid.player_targets;guarded_window_melee.call({},invalid);
 check(calls.length===0&&(!invalid||invalid.player_targets===targets),'malformed NPC/targets skips only this callback');
}
reset();install();const emptyNpc={entity:true,first_node:{},player_targets:garray([[7,tagged]])};
guarded_window_melee.call({},emptyNpc);
check(calls.length===1&&emptyNpc.player_targets.size===0,'empty filtered cache still delegates stock callback');
console.log(JSON.stringify({passed:true,checks,scope:'Mechanically translated actual GSC guard bodies with explicit array/entity/callback stand-ins; no engine/VM/runtime proof.'}));
'''


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--node',default=os.environ.get('NODE','node'),help='Node executable via --node, NODE or PATH')
    parser.add_argument('--source',type=Path,help='Optional candidate GSC source; default is this repository production common controller')
    parser.add_argument('--output-dir',type=Path,default=ROOT/'test-output',help='Parent for a new retained result directory; no deletion')
    args=parser.parse_args()
    source_path=ROOT/'scripts/zombies_coop/eroded_coop.gsc';family='cb'
    if not source_path.is_file():source_path=ROOT/'data/custom_scripts/zm/eroded_coop.gsc';family='ezz'
    if args.source is not None:source_path=args.source.resolve()
    source=source_path.read_text(encoding='utf-8')
    bodies={name:function(source,'function '+name+'(') for name in HELPERS}
    for name,body in bodies.items():
        if re.search(r'\b(?:wait|waittill|thread|notify|endon)\b',mask(body)):
            raise ValueError('Guard introduces a yield/thread/event seam: '+name)
    melee=bodies['guarded_window_melee']
    if re.search(r'first_node\s*\.|\b(?:origin|trigger_location|meleeweapon|meleedamage)\b',mask(melee)):
        raise ValueError('Melee guard inspects retail-owned node, vector or weapon semantics')
    if re.search(r'\b(?:isalive|isplaying|isstruct|isvector|gettype)\s*\(',mask(bodies['disconnect_player_list'])):
        raise ValueError('List filtering substitutes playing/liveness or invented type APIs')
    install_body=function(source,'function install(')
    waiting=install_body[install_body.index('while ('):install_body.index('expected_delay =')]
    if ('disconnect_endgame_pending(level.onendgame)' not in waiting or
            'if (!startup_current(epoch, map))' not in waiting or
            'if (gettime() - started >= 30000)' not in waiting or 'wait 0.05;' not in waiting):
        raise ValueError('Temporary stock-owner wait must retain epoch, 30s timeout and bounded polling')
    if not (install_body.index('disconnect_guard_installable(epoch, map)')<install_body.index('install_round_seed_hook(epoch, map)')
            <install_body.index('install_disconnect_guards(epoch, map)')<install_body.index('level.player_too_many_players_check_func = &player_count_check;')
            <install_body.index('setdvar("zm_coopReadyMap", map)')):
        raise ValueError('Readiness/primary callbacks precede successful guard installation')
    if 'disconnect_guards_owned()' not in function(source,'function owns_callbacks('):
        raise ValueError('Later disconnect owner takeover does not revoke readiness')
    stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    out=args.output_dir.resolve()/('disconnect-guards-'+family+'-'+stamp);out.mkdir(parents=True,exist_ok=False)
    js=out/'actual-functions.js';js.write_text(STANDINS+'\n'+'\n'.join(translate(body) for body in bodies.values())+'\n'+CASES,encoding='utf-8')
    result=subprocess.run([args.node,str(js)],capture_output=True,text=True,check=False)
    (out/'stdout.log').write_text(result.stdout,encoding='utf-8');(out/'stderr.log').write_text(result.stderr,encoding='utf-8')
    if result.returncode:raise RuntimeError('Offline actual-body test failed; see '+str(out))
    verdict=json.loads(result.stdout)
    report={**verdict,'family':family,'source':str(source_path),'source_sha256':hashlib.sha256(source_path.read_bytes()).hexdigest(),
        'test_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'generated_js_sha256':hashlib.sha256(js.read_bytes()).hexdigest(),
        'extracted_functions':list(bodies),'proof':str(out/'RESULTS.private.json')}
    (out/'RESULTS.private.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
