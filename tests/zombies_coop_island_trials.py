"""Offline checks of actual Island allocator bodies, not GSC VM/gameplay proof."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,re,subprocess,sys
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tests'))
from script_frame_diagnostics import function

HELPERS=('valid_capacity','valid_slot','valid_task','clone_task','make_pool','take_task','return_task','packed_pillar','registry_count',
         'roster_by_slot','participant_complete','completed_roster','player_challenge_action')
LIFECYCLE=('schema_current','context_current','registry_owned','decline','early_install',
           'publish_pool_api','initialize_pools','sync_stock_pools','reserve_player','release_player')

def translate(body):
    params=set(filter(None,(p.strip() for p in re.search(r'function\s+\w+\(([^)]*)\)',body)[1].split(','))))
    masked=re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"',lambda m:' '*len(m[0]),body)
    locals_=set(re.findall(r'(?<![\w.])([A-Za-z_]\w*)\s*=(?!=)',masked))-params
    loops=set(re.findall(r'foreach\s*\(\s*(\w+)\s+in\s+',body))
    result=re.sub(r'&zm_island_challenges::(\w+)',r'stock_\1',body)
    for name in ('on_player_connect','on_player_disconnect','make_pool','take_task','return_task'):
        result=result.replace('&'+name,name)
    result=result.replace('#"','"').replace('level thread completion_monitor();','launch_monitor();').replace('level thread ownership_watcher();','launch_ownership_watcher();')
    result=re.sub(r'level\s+endon\("([a-z_]+)"\);',r'register_endon("\1");',result)
    for old,new in (('callback::remove_on_connect','remove_on_connect'),('callback::remove_on_disconnect','remove_on_disconnect'),
                    ('callback::on_connect','add_on_connect'),('callback::on_disconnect','add_on_disconnect'),
                    ('zm_unitrigger::unregister_unitrigger','unregister_stub'),('record.pillar delete();','delete_model(record.pillar);')):
        result=result.replace(old,new)
    result=re.sub(r'foreach\s*\(\s*(\w+)\s+in\s+([^\)]+)\)',r'for (const \1 of \2)',result)
    result=re.sub(r'player\s+getentitynumber\(\)',r'entity_number(player)',result)
    result=re.sub(r'\[\[api\.(\w+)\]\]',r'api.\1',result)
    result=re.sub(r'getarraykeys\((\w+(?:\.\w+)*)\)\.size',r'gsc_size(getarraykeys(\1))',result)
    result=result.replace('player flag::exists(flag)','flag_exists(player,flag)').replace('player flag::get(flag)','flag_get(player,flag)')
    result=re.sub(r'(\b[A-Za-z_]\w*(?:\.[A-Za-z_]\w*)*)\.size\b',r'gsc_size(\1)',result)
    if locals_-loops:
        pos=result.index('{')+1;result=result[:pos]+'\nlet '+','.join(sorted(locals_-loops))+';\n'+result[pos:]
    if '::' in re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',result):raise ValueError('Unsupported translation seam')
    return result

STANDINS=r'''
const assert=require('node:assert/strict');let checks=0;
function check(value,label){++checks;assert.ok(value,label);}
function isdefined(value){return value!==undefined;}
function isint(value){return Number.isInteger(value)&&value>=-2147483648&&value<=2147483647;}
function isstring(value){return typeof value==='string';}
function isarray(value){return Array.isArray(value);}
function isentity(value){return value!==undefined&&value.entity===true;}function isplayer(value){return isentity(value)&&value.player===true;}
function entity_number(value){return value.slot;}
function flag_exists(value,key){return value.flag!==undefined&&value.flag[key]!==undefined;}
function flag_get(value,key){return value.flag[key];}
function array(...items){return items;}function spawnstruct(){return {};}
function getarraykeys(items){return Object.keys(items).filter(k=>items[k]!==undefined).map(k=>/^(0|[1-9]\d*)$/.test(k)?Number(k):k);}
function gsc_size(items){return getarraykeys(items).length;}
function arrayremoveindex(items,index,preserve){assert.equal(preserve,0);items.splice(index,1);}
let holeMode=false;
function arrayremovevalue(items,value){const index=items.indexOf(value);assert.ok(index>=0);if(holeMode)delete items[index];else items.splice(index,1);}
function stock_function_2dbc7cd3(){}function stock_function_25c1bab7(){}
function stock_function_5a96677a(){}function stock_function_26c58398(){}function foreign(){}
// Pinned stock catalog count values include the tier-three thirty-kill task.
const stockCounts={1:[1,1,5,5,5],2:[1,1,15,10,20,20],3:[8,3,1,30,5]};
function templates(tier){return stockCounts[tier].map((count,i)=>({n_index:tier,str_info:'localized-'+i,n_count:count,str_notify:'stock_'+tier+'_'+i,func_think:undefined}));}
let level={},dvars={},playersNow=[],worldSupported=false,receipts=[],monitors=0,deleted=[],unregistered=[],watcherStarts=0,endons=[];
function getdvarint(key){return dvars[key]??0;}function getdvarstring(key){return dvars[key]??'';}
function setdvar(key,value){dvars[key]=value;}function getplayers(){return playersNow;}
function prepare_world_templates(){return worldSupported;}
function println(message){receipts.push(message);}function randomint(count){assert.ok(count>0&&count<=8);return 0;}
function on_player_connect(){}function on_player_disconnect(){}
function stock_on_player_connect(){}function stock_on_player_disconnect(){}function stock_function_b1cd865a(){}
const CONNECT='hash_eaffea17',DISCONNECT='hash_aebdd257';
function remove_callback(key,fn){const entries=level._callbacks[key],index=entries.findIndex(e=>e[0]===fn&&e[1]===undefined);assert.ok(index>=0);entries.splice(index,1);}
function remove_on_connect(fn){remove_callback(CONNECT,fn);}function remove_on_disconnect(fn){remove_callback(DISCONNECT,fn);}
function add_on_connect(fn){level._callbacks[CONNECT].push([fn,undefined]);}function add_on_disconnect(fn){level._callbacks[DISCONNECT].push([fn,undefined]);}
function launch_monitor(){++monitors;}
function launch_ownership_watcher(){++watcherStarts;}function register_endon(event){endons.push(event);}
function unregister_stub(stub){unregistered.push(stub);}function delete_model(model){deleted.push(model);}
function resetEarly(){
 level={_callbacks:{[CONNECT]:[[foreign,undefined],[stock_on_player_connect,undefined]],[DISCONNECT]:[[stock_on_player_disconnect,undefined],[foreign,{}]]}};
 dvars={mapname:'zm_island',zm_coopSchemaSlots:8,zm_coopSchemaServerRevision:2,zm_coopStartupEpoch:3};
 playersNow=[];worldSupported=false;receipts=[];monitors=0;deleted=[];unregistered=[];watcherStarts=0;endons=[];
}
function beginPools(){resetEarly();worldSupported=true;early_install();level._challenges={challenge_1:templates(1),challenge_2:templates(2),challenge_3:templates(3)};level._callbacks[DISCONNECT].push([stock_function_b1cd865a,undefined]);check(initialize_pools(),'stock pools initialized after real init boundary');}
'''

CASES=r'''
for(const capacity of [-1,0,9,18,1.5,true,undefined])check(!valid_capacity(capacity),'reject invalid capacity');
for(let capacity=1;capacity<=8;capacity++)check(valid_capacity(capacity),'1..8 capacities valid');
for(const mode of [false,true]){holeMode=mode;
for(let tier=1;tier<=3;tier++)for(const capacity of [1,4,5,6,7,8]){
 const source=templates(tier),pool=make_pool(source,tier,capacity),owners=Array.from({length:capacity},()=>({}));
 check(pool!==undefined&&pool.allocated===source.length,'stock catalog shape preserved');
 const records=[];
 for(let slot=0;slot<capacity;slot++){
  const record=take_task(pool,slot,owners[slot],0);records.push(record);
  check(record!==undefined,'each actual slot gets task');
  check(take_task(pool,slot,owners[slot],0)===record,'same owner repeated reservation idempotent');
  check(take_task(pool,slot,{},0)===undefined,'slot reuse cannot steal old owner');
  check(pool.allocated<=8,'at most eight assignment records');
 }
 check(new Set(records).size===capacity,'all participants own independent record objects');
 check(records.slice(0,Math.min(4,capacity)).every((r,i)=>r===source[i]),'first four original random selections unchanged');
 for(let slot=0;slot<capacity;slot++){
  check(!return_task(pool,slot,{}),'foreign return refused');
  check(return_task(pool,slot,owners[slot]),'matching return succeeds once');
  check(!return_task(pool,slot,owners[slot]),'duplicate disconnect does not inflate pool');
 }
 check(gsc_size(pool.available)===pool.allocated,'every record returned exactly once');
 for(let cycle=0;cycle<4;cycle++){
  const fresh=Array.from({length:capacity},()=>({}));
  for(let slot=0;slot<capacity;slot++)check(take_task(pool,slot,fresh[slot],0)!==undefined,'rejoin gets returned record');
  for(let slot=0;slot<capacity;slot++)check(return_task(pool,slot,fresh[slot]),'rejoined owner returns');
 }
 check(pool.allocated===Math.max(source.length,capacity)&&gsc_size(pool.available)===pool.allocated,'rejoin never grows or leaks tasks');
 check(source.every((record,index)=>record.n_count===stockCounts[tier][index]),'template records never mutated');
}
for(let tier=1;tier<=3;tier++){
 const source=templates(tier),pool=make_pool(source,tier,8),owners=Array.from({length:8},()=>({}));
 for(let slot=0;slot<8;slot++)take_task(pool,slot,owners[slot],0);
 for(let slot=4;slot<8;slot++)check(return_task(pool,slot,owners[slot]),'8 to4 returns departed tasks');
 const kept=pool.assigned.slice(0,4);
 for(let slot=4;slot<8;slot++){owners[slot]={};check(take_task(pool,slot,owners[slot],0)!==undefined,'4 to8 reuses pool');}
 check(kept.every((r,i)=>pool.assigned[i]===r),'connected players keep original tasks on population change');
}
}holeMode=false;
const template=templates(1)[0],copy=clone_task(template,1);
check(copy!==template&&Object.keys(copy).sort().join(',')==='func_think,n_count,n_index,str_info,str_notify','clone only five stock fields');
copy.n_count=20;check(template.n_count===1,'clone independent mutable object');
for(const mutate of [r=>r.n_index=2,r=>r.n_index='1',r=>r.n_count=0,r=>r.n_count=31,r=>r.n_count=true,r=>r.str_notify=1,r=>r.str_info=undefined,r=>r.func_think=foreign]){
 const bad={...template};mutate(bad);check(clone_task(bad,1)===undefined,'unknown task contract rejected');
}
const malformed=templates(1);malformed.extra=template;check(make_pool(malformed,1,8)===undefined,'associative pool keys rejected');
const sparse=templates(1);delete sparse[0];check(make_pool(sparse,1,8)===undefined,'missing template rejected');
for(const slot of [-1,8,true,1.2])check(take_task(make_pool(templates(1),1,8),slot,{},0)===undefined,'invalid actual slot refused');
const p=make_pool(templates(1),1,8);check(take_task(p,0,{},7)===undefined&&p.available.length===5,'invalid selection is nonmutating');
{
 const player={entity:true,player:true},tasks={},record={owner:player,epoch:7,tasks};
 check(player_challenge_action(player,undefined,7)==='new','new player may initialize only absent challenge field');
 player._challenges={};check(player_challenge_action(player,undefined,7)==='foreign_challenges','foreign challenge owner preserved');
 check(player_challenge_action(player,record,7)==='stale_slot_record','same owner record cannot conceal changed challenge object');
 player._challenges=tasks;check(player_challenge_action(player,record,7)==='owned','exact same owner/epoch/tasks duplicate idempotent');
 check(player_challenge_action(player,{...record,owner:{}},7)==='stale_slot_record','foreign slot record closes readiness');
 check(player_challenge_action(player,{...record,epoch:6},7)==='stale_slot_record','stale epoch record closes readiness');
 check(player_challenge_action(player,record,0)==='stale_slot_record','invalid context closes readiness');
 delete player._challenges;player.var_873a3e27=[];
 check(player_challenge_action(player,undefined,7)==='foreign_trial_progress','existing custom trial progress container preserved');
}
check(registry_count([[foreign,undefined]],foreign)===1,'exact registry pointer recognized');
check(registry_count([[foreign]],foreign)===1,'undefined optional receiver may be absent in stock array');
check(registry_count([[foreign,{}]],foreign)===0,'object-bound callback not overwritten');
check(registry_count([[]],foreign)===-1,'malformed registry closed');
for(let slot=4;slot<8;slot++)for(let a=1;a<=4;a++)for(let b=1;b<=4;b++)for(let c=1;c<=4;c++){
 const packed=packed_pillar(slot,a,b,c);
 check(packed>0&&packed<2048&&((packed&3)+4)===slot&&((packed>>2)&7)===a&&((packed>>5)&7)===b&&((packed>>8)&7)===c,'11bit packed state roundtrip');
}
for(const args of [[3,1,1,1],[8,1,1,1],[4,0,1,1],[4,1,5,1],[4,1,1,true]])check(packed_pillar(...args)===undefined,'invalid packed state refused');
function completedPlayers(count){
 const players=[],records=[];
 for(let slot=0;slot<count;slot++){
  const tasks={},player={entity:true,player:true,slot,_challenges:tasks,flag:{},bot:slot%2===1};
  for(let tier=1;tier<=3;tier++)player.flag['flag_player_completed_challenge_'+tier]=1;
  players.push(player);records[slot]={owner:player,epoch:7,tasks};
 }
 return {players,records};
}
check(!completed_roster([],[],[],7),'empty server cannot earn all-complete');
for(let count=1;count<=8;count++){
 const {players,records}=completedPlayers(count);
 check(completed_roster(players,[...players].reverse(),records,7),'actual human/bot completed set independent of array order');
 players[0].sessionstate='spectator';players[0].alive=false;
 check(completed_roster(players,players,records,7),'downed/dead/spectating participants retain completed flags');
 players[0].flag.flag_player_completed_challenge_2=0;
 check(!completed_roster(players,players,records,7),'uncompleted actual participant blocks unlock');
 players[0].flag.flag_player_completed_challenge_2=1;
 check(!completed_roster(players,players,records,8),'stale map assignment cannot earn next map unlock');
 const listed=[...players];listed[0]={...players[0]};
 check(!completed_roster(players,listed,records,7),'same slot different entity snapshot refused');
}
{
 const {players,records}=completedPlayers(8),four=players.slice(0,4);
 players[7].flag.flag_player_completed_challenge_1=0;
 check(!completed_roster(players,players,records,7),'uncompleted eighth blocks full roster');
 check(completed_roster(four,four,records,7),'8 to4 uses current four completed participants');
 check(!completed_roster(players,players,records,7),'4 to8 includes current uncompleted participant again');
 const duplicate=[...four,four[0]];
 check(roster_by_slot(duplicate)===undefined,'duplicate slot/entity snapshot refused');
 four[0].player_disconnected=1;
 check(completed_roster(four,four,records,7),'tagged departed player excluded from current set');
 for(const player of four)player.player_disconnected=1;
 check(!completed_roster(four,four,records,7),'all departing entities are not an earned empty roster');
}
for(const actual of [[{}],[{entity:true,player:false,slot:0}],[{entity:true,player:true,slot:8}]])check(roster_by_slot(actual)===undefined,'invalid current participant snapshot refused');
resetEarly();early_install();
check(registry_owned()&&level.eroded_island_trials_ready===false&&dvars.zm_coopIslandTrialsStatus==='geometry_pending','unknown geometry cannot open admission');
check(level._callbacks[CONNECT][0][0]===foreign&&level._callbacks[DISCONNECT][0][0]===foreign,'unrelated callback ownership retained');
const connectCount=level._callbacks[CONNECT].length,disconnectCount=level._callbacks[DISCONNECT].length;
early_install();check(receipts.length===1&&level._callbacks[CONNECT].length===connectCount&&level._callbacks[DISCONNECT].length===disconnectCount,'duplicate preinit does not replace or register twice');
check(watcherStarts===1,'ownership watcher installed once independently of pools');
check(receipts[0]==='[Zombies coop] island_trials=early_registry installed=1','fixed bounded install receipt');
resetEarly();worldSupported=true;early_install();check(registry_owned()&&level.eroded_island_trials_ready===true,'verified world preflight can become early ready without pool startup deadlock');
check(Object.keys(level.eroded_island_trials_pool_api).sort().join(',')==='capacity,epoch,make_pool,map,return_task,revision,take_task','only exact per-map pure helper table fields published');
check(level.eroded_island_trials_pool_api.make_pool===make_pool&&level.eroded_island_trials_pool_api.take_task===take_task&&level.eroded_island_trials_pool_api.return_task===return_task,'actual helper references published');
dvars.zm_coopStartupEpoch=4;const oldStatus=dvars.zm_coopIslandTrialsStatus;early_install();
check(!registry_owned()&&dvars.zm_coopIslandTrialsStatus===oldStatus,'stale VM cannot reacquire callback or overwrite next map diagnostics');
for(const mutate of [()=>playersNow=[{entity:true,player:true,slot:0}],()=>level._callbacks[CONNECT]=[[foreign]],()=>level._callbacks[DISCONNECT]=[[]]]){
 resetEarly();mutate();const before=JSON.stringify(level._callbacks);early_install();
 check(level.eroded_island_trials_revision===undefined&&JSON.stringify(level._callbacks)===before,'nonpristine or unknown registry declined without partial replacement');
}
for(const mutate of [()=>dvars.zm_coopSchemaSlots=4,()=>dvars.zm_coopSchemaSlots=9,()=>dvars.zm_coopSchemaServerRevision=1,()=>dvars.mapname='zm_zod']){
 resetEarly();mutate();early_install();check(level.eroded_island_trials_revision===undefined&&receipts.length===0,'nonexpanded wrong schema/map path untouched');
}
beginPools();check(monitors===1&&initialize_pools()&&monitors===1,'one pool/completion initialization per map');
level._callbacks[DISCONNECT].push([stock_function_b1cd865a,undefined]);
check(!registry_owned()&&!initialize_pools(),'later historical-counter callback reappearance revokes ownership');
beginPools();const owners=Array.from({length:8},(_,slot)=>({entity:true,player:true,slot}));
for(let slot=0;slot<8;slot++){
 const record=reserve_player(owners[slot],slot);check(record!==undefined&&record.owner===owners[slot],'three tiers reserved for actual owner');
 owners[slot]._challenges=record.tasks;record.stub={slot};record.trigger_active=true;record.pillar={slot};
 check(reserve_player(owners[slot],slot)===undefined,'duplicate initializer cannot replace active record');
}
const kept=level.eroded_island_trials_records.slice(0,4);
for(let slot=4;slot<8;slot++)check(release_player(owners[slot],slot),'shrunk owner returned all three tiers');
check(deleted.length===4&&unregistered.length===4,'each owned mover and stub retired once');
for(let slot=4;slot<8;slot++){
 const old=owners[slot];owners[slot]={entity:true,player:true,slot};check(reserve_player(owners[slot],slot)!==undefined,'new entity may rejoin released slot');
 check(!release_player(old,slot),'old entity cannot release new slot occupant');
}
check(kept.every((r,i)=>r===level.eroded_island_trials_records[i]),'8 to4 to8 does not reset retained player progress');
check(deleted.length===4&&unregistered.length===4,'stale release has no world side effects');
beginPools();const owner={entity:true,player:true,slot:0},other={};
take_task(level.eroded_island_trials_pools[1],0,other,0);
check(reserve_player(owner,0)===undefined&&level.eroded_island_trials_pools[0].assigned[0]===undefined,'partial three-tier reservation rolls earlier tier back');
check(level.eroded_island_trials_records[0]===undefined&&level.eroded_island_trials_pools[1].owners[0]===other,'failed reservation preserves existing foreign lease');
beginPools();const notActivatedOwner={entity:true,player:true,slot:0};
const notActivated=reserve_player(notActivatedOwner,0);notActivated.stub={prepared:true};notActivated.pillar={prepared:true};
check(release_player(notActivatedOwner,0)&&unregistered.length===0&&deleted.length===1,'constructed but unregistered stub does not call retail unregister');
// Step the actual watcher body as a generator; waits are explicit scheduler boundaries.
for(const afterCompletion of [false,true]){
 beginPools();level.all_challenges_completed=afterCompletion;
 const watcher=ownership_watcher();check(watcher.next().value===0.1,'watcher waits without mutating gameplay');
 check(endons.includes('end_game')&&endons.includes('intermission'),'watcher terminates with VM gameplay lifecycle');
 level._callbacks[CONNECT]=[[foreign]];
 check(watcher.next().done&&level.eroded_island_trials_ready===false&&dvars.zm_coopIslandTrialsStatus==='registry_ownership_lost','takeover closes readiness even after one-shot completion monitor ends');
 check(level.eroded_island_trials_pool_api===undefined,'decline revokes helper table');
}
resetEarly();worldSupported=true;early_install();
const beforePools=ownership_watcher();beforePools.next();level._callbacks[DISCONNECT].push([stock_on_player_disconnect]);
check(beforePools.next().done&&!level.eroded_island_trials_ready,'takeover before first pool/participant closes readiness');
resetEarly();worldSupported=true;early_install();
const staleWatcher=ownership_watcher();staleWatcher.next();dvars.zm_coopStartupEpoch=4;dvars.zm_coopIslandTrialsStatus='new-map-status';
check(staleWatcher.next().done&&!level.eroded_island_trials_ready&&dvars.zm_coopIslandTrialsStatus==='new-map-status','old watcher cannot overwrite later map diagnostics');
resetEarly();worldSupported=true;early_install();
const capacityWatcher=ownership_watcher();capacityWatcher.next();dvars.zm_coopSchemaSlots=7;
check(capacityWatcher.next().done&&!level.eroded_island_trials_ready,'changed frozen native capacity closes watcher context');
for(const mode of [false,true]){
 holeMode=mode;beginPools();const api=level.eroded_island_trials_pool_api;
 for(let capacity=1;capacity<=8;capacity++)for(let tier=1;tier<=3;tier++)check(exercise_pool(api,tier,capacity)===0,'actual private VM fixture exercise passes explicit dense/hole model');
 for(let tier=1;tier<=3;tier++)check(exercise_transition(api,tier)===0,'actual private VM fixture transition body preserves leases');
}holeMode=false;
console.log(JSON.stringify({passed:true,checks,scope:'Mechanically translated actual allocator/packing/registry helpers; no GSC VM, world binding or client proof.'}));
'''

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--node',default=os.environ.get('NODE','node'));p.add_argument('--output-dir',type=Path,default=ROOT/'test-output');a=p.parse_args()
    source=ROOT/'scripts/zombies_coop/eroded_coop_island_trials.gsc';text=source.read_text(encoding='utf-8')
    bodies={name:function(text,'function '+name+'(') for name in HELPERS}
    bodies.update({name:function(text,'function '+name+'(') for name in LIFECYCLE})
    fixture_path=ROOT/'tests/gsc/eroded_coop_island_selftest.gsc';fixture=fixture_path.read_text(encoding='utf-8')
    fixture_code=re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',fixture)
    if re.search(r'\bforeach\b',fixture_code) or re.search(r'\bforeach\b',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',text)):raise ValueError('ACTS BO3 does not implement foreach; use explicit key snapshots')
    if '#using' in fixture_code:raise ValueError('Private fixture must not import pre-anchor Island asset')
    if re.search(r'\b(?:self|level|player)\.\w+\s*=(?!=)',fixture_code):raise ValueError('Private fixture cannot mutate production object fields')
    if set(re.findall(r'setdvar\("([^"]+)"',fixture_code))!={'zm_coopIslandPoolTestStatus','zm_coopIslandPoolTestFailures','zm_coopIslandPoolTestEpoch'}:raise ValueError('Unexpected fixture dvar writes')
    run=function(fixture,'function run(')
    if 'attempt < 600' not in run or 'wait 0.05;' not in run or 'api_current(api, epoch, capacity)' not in run:raise ValueError('Fixture wait/context must be finite')
    if re.search(r'\b(?:wait|waittill|thread|notify)\b',re.sub(r'//[^\n]*','',run[run.index('failures = 0;'):])):raise ValueError('Private fixture yielded after API snapshot')
    fixture_helpers=('check','templates','unique_records','template_member','exercise_pool','exercise_transition')
    translated_fixture=[]
    for name in fixture_helpers:
        code=translate(function(fixture,'function '+name+'('))
        code=re.sub(r'\bcheck\(', 'fixture_check(',code)
        translated_fixture.append(code)
    watcher=function(text,'function ownership_watcher(')
    if 'wait 0.1;' not in watcher or 'while (context_current())' not in watcher or '!registry_owned()' not in watcher:raise ValueError('Independent ownership watcher missing or unbounded busy loop')
    if 'all_challenges_completed' in watcher or 'eroded_island_trials_pools' in watcher:raise ValueError('Ownership watcher must not depend on completion or initialized pools')
    translated_watcher=translate(watcher).replace('function ownership_watcher(', 'function* ownership_watcher(').replace('wait 0.1;', 'yield 0.1;')
    connect=function(text,'function on_player_connect(')
    if connect.index('decline("registry_ownership_lost")')>connect.index('level flag::wait_till'):raise ValueError('Connect ownership loss must revoke before any yield')
    if connect.index('player_challenge_action(')>connect.index('reserve_player('):raise ValueError('Challenge ownership preflight must precede reservation/world writes')
    if connect.index('if (isdefined(self._challenges))')>connect.index('self._challenges = record.tasks;'):raise ValueError('Late foreign challenge ownership must be preserved')
    if connect.index('if (isdefined(self._challenges))')<connect.index('prepare_player_world('):raise ValueError('Final challenge ownership recheck must follow world setup')
    if connect.index('activate_player_world(')<connect.index('self thread pillar_state_worker('):raise ValueError('Prompt activation must follow all task worker startup')
    if connect.index('self.var_873a3e27[tier] = 0;')>connect.index('self thread zm_island_challenges::function_2ce855f3('):raise ValueError('Stock prompt progress must exist before any worker/activation')
    if connect.index('if (isdefined(self.var_873a3e27))')>connect.index('self.var_873a3e27 = array();'):raise ValueError('Foreign progress must not be overwritten')
    for name in HELPERS:
        if re.search(r'\b(?:wait|waittill|thread|notify)\b',re.sub(r'//[^\n]*','',bodies[name])):raise ValueError('Pure helper yielded')
    if 'array("zm_laststand", "eroded_coop_fields")' not in text:raise ValueError('Stock dependency execution is nontransitive')
    monitor=function(text,'function completion_monitor(')
    if 'completed_roster(getplayers(), level.players' not in monitor or 'level flag::set("all_challenges_completed")' not in monitor:raise ValueError('Completion unlock lost actual current-roster predicate')
    code=re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',text)
    if 'var_c28313cd' in code or 'hash_41370469' in code or '::function_974d5f1d(' in code:raise ValueError('Historical completion counter/notification must not be forged')
    if 'clientfield::set("eroded_island_pillar", 0)' in code:raise ValueError('Visible mover must be deleted, not reset to zero')
    if re.search(r'\b(?:isstruct|isvector|gettype)\s*\(',text):raise ValueError('Unverified type API')
    stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');family=ROOT.name
    out=a.output_dir.resolve()/('island-trials-'+family+'-'+stamp);out.mkdir(parents=True,exist_ok=False)
    js=out/'actual-helpers.js';js.write_text(STANDINS+'\n'+'\n'.join(translate(b) for b in bodies.values())+'\n'+translated_watcher+'\n'+'\n'.join(translated_fixture)+'\n'+CASES,encoding='utf-8')
    run=subprocess.run([a.node,str(js)],capture_output=True,text=True);(out/'stdout.log').write_text(run.stdout,encoding='utf-8');(out/'stderr.log').write_text(run.stderr,encoding='utf-8')
    if run.returncode:raise RuntimeError('Retained failure: '+str(out))
    report={**json.loads(run.stdout),'family':family,'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'fixture_sha256':hashlib.sha256(fixture_path.read_bytes()).hexdigest(),'test_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'generated_js_sha256':hashlib.sha256(js.read_bytes()).hexdigest(),'extracted_functions':list(bodies)+['ownership_watcher'],'fixture_helpers':list(fixture_helpers),'proof':str(out/'RESULTS.private.json')}
    (out/'RESULTS.private.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report,indent=2))
if __name__=='__main__':main()
