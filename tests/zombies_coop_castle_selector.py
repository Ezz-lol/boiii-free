"""Offline actual-body Castle dialogue selector checks.

Mechanically translates production GSC and supplies explicit typed-array/entity,
vector, dvar and nearest-query stand-ins. This does not execute a GSC VM, native
asset rewrite, stock nearest helper, dialogue audio or game process.
"""
from pathlib import Path
import argparse,datetime,hashlib,json,os,re,subprocess,sys
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tests'))
from script_frame_diagnostics import function

HELPERS=('castle_selector_map_valid','castle_selector_bind','castle_selector_bound_current','castle_selector_decline',
         'castle_selector_table_valid','castle_selector_rows_current','castle_dialogue_get_all_closest')

def translate(body):
    masked=re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"',lambda m:' '*len(m[0]),body)
    parameters=set(filter(None,(v.strip() for v in re.search(r'function\s+\w+\(([^)]*)\)',body)[1].split(','))))
    variables=set(re.findall(r'(?<![\w.])([A-Za-z_]\w*)\s*=(?!=)',masked))-parameters
    body=body.replace('array::get_all_closest','stock_get_all_closest')
    body=re.sub(r'\b([A-Za-z_]\w*)\.size\b',r'gsc_size(\1)',body)
    body=re.sub(r'\btrue\b','1',re.sub(r'\bfalse\b','0',body))
    opening=body.index('{')+1
    if variables:body=body[:opening]+'\nlet '+','.join(sorted(variables))+';\n'+body[opening:]
    return body

STANDINS=r'''
const assert=require('node:assert/strict');let checks=0;
function check(value,message){checks++;assert.ok(value,message);}
const VECTOR=Symbol('vector'),KEYS=Symbol('keys');
function vec(x=0,y=0,z=0){const value=[x,y,z];value[VECTOR]=true;return value;}
function array(...values){return values;}
function isdefined(value){return value!==undefined;}
function isint(value){return Number.isInteger(value)&&value>=-2147483648&&value<=2147483647;}
function isstring(value){return typeof value==='string';}
function isvec(value){return value?.[VECTOR]===true;}
function isarray(value){return Array.isArray(value)&&!isvec(value);}
function isentity(value){return value?.entity===true;}
function isplayer(value){return isentity(value)&&value.player===true;}
function getarraykeys(value){return value[KEYS]??Object.keys(value).filter(k=>value[k]!==undefined).map(k=>/^(0|[1-9][0-9]*)$/.test(k)?Number(k):k);}
function gsc_size(value){return typeof value==='string'?value.length:getarraykeys(value).length;}
function getsubstr(value,start,end){return value.slice(start,end);}
function issubstr(value,part){return value.includes(part);}
let level={},dvars={},receipts=[],writes=[],calls=[];
function schema_revision(){return 4;}
function getdvarint(name){return dvars[name]??0;}
function getdvarstring(name){return dvars[name]??'';}
function setdvar(name,value){dvars[name]=value;writes.push([name,value]);}
function println(value){receipts.push(value);}
function player(characterindex,x,id){return {entity:true,player:true,characterindex,origin:vec(x),id};}
function table(){let t=[];for(const r of[0,1,3]){t[r]=[];for(let e=0;e<3;e++)t[r][e]=['synthetic_'+r+'_'+e+'_A','synthetic_'+r+'_'+e+'_B'];}return t;}
function closest(origin,candidates,exclusions,limit,radius){
 return candidates.map((p,i)=>({p,i,d:(p.origin[0]-origin[0])**2+(p.origin[1]-origin[1])**2+(p.origin[2]-origin[2])**2}))
  .filter(v=>!exclusions.includes(v.p)&&v.d<=radius*radius).sort((a,b)=>a.d-b.d||a.i-b.i).slice(0,limit).map(v=>v.p);
}
function stock_get_all_closest(origin,candidates,exclusions,limit,radius){calls.push({origin,candidates,exclusions,limit,radius,seen:level.eroded_coop_castle_selector_seen,status:dvars.zm_coopCastleVOStatus});return closest(origin,candidates,exclusions,limit,radius);}
function reset(capacity=8,map='zm_castle'){level={eroded_coop_fields_revision:4,var_4ea3bfd0:table()};dvars={zm_coopStartupEpoch:17,mapname:map,zm_coopSchemaSlots:capacity,zm_coopAppliedLimit:capacity,zm_coopSchemaServerRevision:4,zm_coopStatus:'experimental_callbacks_installed',zm_coopCastleVOStatus:'armed_for_known_module'};receipts=[];writes=[];calls=[];}
function bind(capacity=8,map='zm_castle'){reset(capacity,map);check(castle_selector_bind(17,map,capacity)==1,'native current binding');}
function invoke(candidates,origin=vec(),exclusions=[],limit=4,radius=900){return castle_dialogue_get_all_closest(origin,candidates,exclusions,limit,radius);}
function only_active_receipt(){return writes.length===1&&writes[0][0]==='zm_coopCastleVOStatus'&&writes[0][1]==='owned_selector_active'&&receipts.length===1&&receipts[0]==='[Zombies coop] castle_selector=active';}
function declined(reason,wasActive=false){
 check(level.eroded_coop_castle_selector_declined===1,'decline latched');
 check(dvars.zm_coopStatus==='declined_castle_dialogue_owner'&&dvars.zm_coopSchemaServerRevision===0,'native gate explicitly revoked');
 check(dvars.zm_coopCastleVOStatus===reason,'bounded diagnosis');
 const messages=wasActive?['[Zombies coop] castle_selector=active','[Zombies coop] castle_selector=declined']:['[Zombies coop] castle_selector=declined'];
 check(JSON.stringify(receipts)===JSON.stringify(messages),'exact one-shot active/decline receipts');
 const expectedWrites=[['zm_coopCastleVOStatus',reason],['zm_coopStatus','declined_castle_dialogue_owner'],['zm_coopSchemaServerRevision',0]];
 if(wasActive)expectedWrites.unshift(['zm_coopCastleVOStatus','owned_selector_active']);
 check(JSON.stringify(writes)===JSON.stringify(expectedWrites),'only specifically allowed diagnostic writes');
 check(wasActive?level.eroded_coop_castle_selector_seen===1:level.eroded_coop_castle_selector_seen===undefined,'failed validation never emits active receipt');
}
'''

CASES=r'''
for(let cap=5;cap<=8;cap++){
 bind(cap);const origin=vec(4,3,0),excluded=player(2,0,'speaker');
 const candidates=Array.from({length:cap},(_,i)=>player([0,1,3][i%3],i*10,'P'+i));
 const original=candidates.slice(),exclusions=[excluded],out=invoke(candidates,origin,exclusions);
 check(JSON.stringify(out.map(p=>p.id))===JSON.stringify(closest(origin,candidates,exclusions,4,900).map(p=>p.id)),'all authored rows preserve stock nearest-four');
 check(calls.length===1&&calls[0].origin===origin&&calls[0].exclusions===exclusions&&calls[0].limit===4&&calls[0].radius===900,'one delegate with same argument identity');
 check(calls[0].candidates!==candidates&&calls[0].candidates.every((v,i)=>v===candidates[i]),'private filtered input stable order');
 check(candidates.length===original.length&&candidates.every((v,i)=>v===original[i])&&only_active_receipt(),'inputs unchanged; only exact active receipt');
 check(castle_selector_bind(17,'zm_castle',cap)==1,'same current bind idempotent');
}
for(const cap of[0,1,4,9,18,'8',8.5,true,undefined]){reset();const before=JSON.stringify(level);check(castle_selector_bind(17,'zm_castle',cap)==0&&JSON.stringify(level)===before&&writes.length===0,'invalid capacity cannot bind');}
for(const epoch of[0,-1,'17',17.5,true,undefined]){reset();check(castle_selector_bind(epoch,'zm_castle',8)==0&&writes.length===0,'invalid epoch cannot bind');}
for(const map of['','core_frontend','bad/map','bad\\map','bad.name','x'.repeat(65),{},17,undefined]){reset();check(castle_selector_bind(17,map,8)==0&&writes.length===0,'invalid map cannot bind');}
for(const field of['zm_coopStartupEpoch','zm_coopSchemaSlots','zm_coopAppliedLimit']){reset();dvars[field]++;check(castle_selector_bind(17,'zm_castle',8)==0,'native authority mismatch cannot bind');}

bind();let a=player(0,80,'far0'),b=player(1,30,'near1'),c=player(3,50,'mid3'),d=player(2,1,'duplicate2'),s=player(2,0,'speaker');
let source=[s,d,a,b,c],excluded=[s];let result=invoke(source,vec(),excluded);
check(JSON.stringify(result.map(p=>p.id))===JSON.stringify(['near1','mid3','far0']),'nearer duplicate archetype2 never supplies nonexistent row');
check(calls[0].candidates.every(p=>p.characterindex!==2),'row eligibility filtered before stock ordering');
check(source[0]===s&&source[1]===d&&excluded[0]===s,'candidate/exclusion input not mutated');
bind();a=player(0,10,'a');b=player(1,20,'b');invoke([b,a,a]);check(calls[0].candidates[0]===b&&calls[0].candidates[1]===a&&calls[0].candidates[2]===a,'duplicates and original enumeration order retained');
bind();let sparse=[];sparse[1]=player(0,10,'one');sparse[7]=player(3,20,'seven');invoke(sparse);check(calls[0].candidates.length===2&&calls[0].candidates[0]===sparse[1]&&calls[0].candidates[1]===sparse[7],'sparse slots preserve defined members');
bind();invoke([undefined,null,4,'x',{}, {entity:true,player:false},player(1,20,'good')]);check(calls.length===1&&calls[0].candidates.length===1,'nonplayer/undefined values filtered without reading character fields');
bind();check(invoke([player(2,10,'only2')]).length===0&&calls.length===1&&only_active_receipt(),'recognized no-authored-responder result is not an error');
bind();check(invoke([]).length===0&&calls.length===1&&only_active_receipt(),'empty candidate list safely delegated');

// The original caller still applies is_player_valid AFTER closest-four. Our
// stand-in deliberately includes dead/spectating data to expose premature filtering.
bind();let invalidState=player(0,1,'dead');invalidState.alive=false;invalidState.sessionstate='spectator';
invoke([invalidState,player(1,2,'alive')]);check(calls[0].candidates[0]===invalidState,'stock post-validity ordering preserved');

const tableChanges=[t=>delete t[3],t=>t[2]=[],t=>t.bad=[],t=>t[KEYS]=['0',1,3],t=>t[0]='not-row',
 t=>delete t[1][2],t=>t[1][3]=['x','y'],t=>t[1].bad=['x','y'],t=>t[1][KEYS]=['0',1,2],
 t=>t[3][1]='not-sequence',t=>t[3][1]=['only-one'],t=>t[3][1]=['a','b','c'],t=>t[3][1][KEYS]=['0',1],
 t=>t[3][1][0]='',t=>t[3][1][0]=0,t=>t[3][1][1]={}];
for(const change of tableChanges){bind();change(level.var_4ea3bfd0);check(invoke([player(0,1,'good')]).length===0&&calls.length===0,'unknown/sparse malformed table refuses original call');declined('dialogue_table_unrecognized');invoke([player(0,1,'good')]);check(receipts.length===1&&calls.length===0,'decline repeats inert');check(castle_selector_bind(17,'zm_castle',8)==0,'idempotent bind never revives declined map');}
for(const bad of[undefined,[],{},'table',vec()]){bind();level.var_4ea3bfd0=bad;invoke([]);declined('dialogue_table_unrecognized');}
bind();invoke([player(0,1,'first')]);level.var_4ea3bfd0[1][0][0]='mutated valid-shaped alias';invoke([]);check(calls.length===1,'later content mutation does not delegate');declined('dialogue_table_unrecognized',true);
bind();invoke([player(0,1,'first')]);level.var_4ea3bfd0=table();check(invoke([player(1,1,'same-values')]).length===1&&only_active_receipt(),'equivalent authored data accepted without claiming first-owner attestation');

for(const bad of[-1,4,8,'0',true,1.5,undefined,{},[]]){bind();invoke([player(bad,10,'bad')]);check(calls.length===0,'unsafe character index never read as table key');declined('candidate_character_unrecognized');}
for(const key of[-1,8,'x','0']){bind();let input=[player(0,1,'a')];input[KEYS]=[key];invoke(input);declined('candidate_keys_unrecognized');}
bind(5);invoke(Array.from({length:6},(_,i)=>player(0,i,'a'+i)));declined('candidate_count_unrecognized');
for(const args of[[vec(),[],[],5,900],[vec(),[],[],4,901],[vec(),[],[],true,900],[vec(),[],[],4,'900'],[{},[],[],4,900],[vec(),{},[],4,900],[vec(),[],{},4,900]]){bind();castle_dialogue_get_all_closest(...args);check(calls.length===0,'unrecognized call contract refuses delegate');declined('selector_arguments_unrecognized');}
for(const value of[undefined,0,2,3,'4',true]){bind();level.eroded_coop_fields_revision=value;invoke([]);declined('schema_context_unavailable');}
bind();dvars.zm_coopSchemaServerRevision=0;invoke([]);declined('schema_context_unavailable');

// Missing/stale old callbacks are inert and cannot poison a fresh epoch/map.
for(const change of[()=>dvars.zm_coopStartupEpoch=18,()=>dvars.mapname='zm_tomb',()=>dvars.zm_coopSchemaSlots=7,()=>dvars.zm_coopAppliedLimit=7,()=>delete level.eroded_coop_castle_selector_epoch,()=>level.eroded_coop_castle_selector_map={},()=>level.eroded_coop_castle_selector_epoch='17']){
 bind();change();const before=JSON.stringify(dvars);check(invoke([player(0,1,'good')]).length===0&&calls.length===0&&writes.length===0&&receipts.length===0&&JSON.stringify(dvars)===before,'stale work cannot clear new receipts');
 castle_selector_decline('dialogue_table_unrecognized');check(writes.length===0,'stale explicit decline inert');
}
bind();level.eroded_coop_castle_selector_declined=0;check(invoke([]).length===0&&calls.length===0,'any-defined decline cannot be revived');

// Generalized bind is inert on all expanded native-authorized ZM map identities.
for(const map of['zm_castle','zm_custom_trials','workshop_alpha','mp_named_but_ZM','X_123-foo','Z'.repeat(64)]){
 for(let cap=5;cap<=8;cap++){
  reset(cap,map);const tableBefore=level.var_4ea3bfd0;
  check(castle_selector_map_valid(map)==1,'bounded native-valid map text accepted');
  check(castle_selector_bind(17,map,cap)==1&&writes.length===0&&calls.length===0&&receipts.length===0&&level.var_4ea3bfd0===tableBefore,'unused binding changes no table/worker/receipt');
  check(level.eroded_coop_castle_selector_rows===undefined,'no snapshot or selector work until verified import calls');
  const candidates=Array.from({length:cap},(_,i)=>player([0,1,3][i%3],i*10,'M'+i));
  check(invoke(candidates).length===4&&calls.length===1,'same stock module helper works on inherited valid current map');
 }
}
for(const value of[undefined,null,0,[],{},'', 'core_frontend','x'.repeat(65),'bad/map','bad\\map','bad.name','has space','bad\nmap','non_ascii_é'])
 check(castle_selector_map_valid(value)==0,'map validator rejects bad type/length/native-disallowed text');
for(const map of['a','A','0','_','-','z'.repeat(64)])check(castle_selector_map_valid(map)==1,'native-valid lower boundary and identifier characters');
bind(8,'zm_custom_A');dvars.mapname='zm_custom_B';dvars.zm_coopStartupEpoch=18;dvars.zm_coopRoundNumber=9;
const newReceipts=JSON.stringify(dvars);check(invoke([]).length===0&&writes.length===0&&JSON.stringify(dvars)===newReceipts,'old inherited module callback cannot poison new map/epoch');
bind(6,'zm_custom_B');check(invoke([player(1,1,'fresh')]).length===1&&only_active_receipt(),'fresh inherited bind after rotation works');

// A receipt proves this validated helper reached the stock delegation seam,
// not that audio played or that the external import transform was exercised.
bind();check(dvars.zm_coopCastleVOStatus==='armed_for_known_module'&&receipts.length===0,'binding is armed, not yet active');
invoke([player(0,1,'first-active')]);
check(level.eroded_coop_castle_selector_seen===1&&only_active_receipt(),'first valid call emits exactly one active receipt');
check(calls[0].seen===1&&calls[0].status==='owned_selector_active','active receipt precedes stock delegate');
invoke([player(1,2,'second-active')]);invoke([]);
check(calls.length===3&&only_active_receipt(),'subsequent valid calls delegate without extra writes or log spam');
check(dvars.zm_coopStatus==='experimental_callbacks_installed'&&dvars.zm_coopSchemaServerRevision===4,'active receipt does not alter ready schema/status');
level.var_4ea3bfd0[0][0][0]='late-mutation-after-active';invoke([]);
declined('dialogue_table_unrecognized',true);
const refusedWrites=JSON.stringify(writes),refusedMessages=JSON.stringify(receipts);invoke([]);
check(JSON.stringify(writes)===refusedWrites&&JSON.stringify(receipts)===refusedMessages&&calls.length===3,'later decline supersedes active and remains idempotent');
bind();invoke([player(0,1,'old-active')]);dvars.zm_coopStartupEpoch=18;dvars.mapname='zm_custom_next';dvars.zm_coopCastleVOStatus='armed_for_known_module';writes=[];receipts=[];
check(invoke([]).length===0&&writes.length===0&&receipts.length===0&&dvars.zm_coopCastleVOStatus==='armed_for_known_module','previously active stale callback cannot emit receipt in new map');

console.log(JSON.stringify({passed:true,checks,scope:'Actual mechanically translated production GSC bodies; typed entity/vector/array, dvar, and nearest-order delegates are explicit stand-ins. No GSC VM/native/game execution or captured import proof.'}));
'''

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--node',default=os.environ.get('NODE','node'))
    parser.add_argument('--output-dir',type=Path,default=ROOT/'test-output')
    args=parser.parse_args()
    source=ROOT/'scripts/zombies_coop/eroded_coop_fields.gsc'
    family='cb' if (ROOT/'src/client/component/zombies/diagnostic_console.hpp').is_file() else 'ezz'
    text=source.read_text(encoding='utf-8')
    functions=[function(text,'function '+name+'(') for name in HELPERS]
    bodies='\n\n'.join(functions)
    register=function(text,'function register_schema(')
    binding='if (!castle_selector_bind(getdvarint("zm_coopStartupEpoch"), getdvarstring("mapname"), getdvarint("zm_coopSchemaSlots")))'
    assert re.search(r'function\s+schema_revision\s*\(\s*\)\s*\{\s*return\s+4\s*;\s*\}', function(text,'function schema_revision('))
    assert '#using scripts/shared/array_shared;' in text
    assert '#using scripts/zm/zm_castle_vo' not in text
    assert not re.search(r'\b(wait|waittill|thread|endon)\b',re.sub(r'//[^\n]*','',bodies))
    assert not re.search(r'level\.var_4ea3bfd0\s*=',bodies)
    assert 'return array::get_all_closest(origin, filtered, exclusions, limit, radius);' in bodies
    assert all(alias not in bodies for alias in ('vox_plr_','vox_grop_','clientfield::register'))
    assert register.count(binding)==1 and re.search(re.escape(binding)+r'\s*return;',register)
    assert register.index(binding)<register.index('level.eroded_coop_fields_revision =')<register.index('setdvar("zm_coopSchemaServerRevision"')
    assert 'zm_castle' not in functions[1]+functions[2]
    assert 'level.eroded_coop_castle_selector_map' in functions[2] and 'getdvarstring("mapname")' in functions[2]
    receipt='setdvar("zm_coopCastleVOStatus", "owned_selector_active");'
    assert functions[-1].count(receipt)==1 and functions[-1].count('println("[Zombies coop] castle_selector=active");')==1
    assert 'if (!isdefined(level.eroded_coop_castle_selector_seen))' in functions[-1]
    assert functions[-1].index('filtered[filtered.size] = candidate;')<functions[-1].index(receipt)<functions[-1].index('return array::get_all_closest')
    actual='\n\n'.join(translate(body) for body in functions)
    out=args.output_dir.resolve()/('castle-selector-'+family+'-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'))
    out.mkdir(parents=True,exist_ok=False)
    js=out/'actual-functions.js';js.write_text(STANDINS+'\n'+actual+'\n'+CASES,encoding='utf-8')
    command=[args.node,str(js)]
    result=subprocess.run(command,cwd=ROOT,capture_output=True,text=True)
    (out/'stdout.log').write_text(result.stdout,encoding='utf-8');(out/'stderr.log').write_text(result.stderr,encoding='utf-8')
    report={'passed':result.returncode==0,'family':family,'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),
            'test_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'generated_js_sha256':hashlib.sha256(js.read_bytes()).hexdigest(),
            'functions':list(HELPERS),'source_contract_checks':14,'command':command,'exit_code':result.returncode}
    if result.returncode==0:report.update(json.loads(result.stdout))
    (out/'RESULTS.private.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    print(json.dumps({'proof':str(out/'RESULTS.private.json'),**report},indent=2))
    return result.returncode

if __name__=='__main__':raise SystemExit(main())
