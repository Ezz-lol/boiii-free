"""Execute actual CSC world-field/template/clone bodies with native stand-ins.

No game/client image is loaded. The explicit stand-ins record native operations;
extracted production CSC bodies make all state and resource-lifetime decisions.
"""
from pathlib import Path
import argparse, datetime, hashlib, json, re, shutil, subprocess


def body(source, name):
    match = re.search(r"\bfunction\s+" + re.escape(name) + r"\s*\([^)]*\)\s*\{", source)
    assert match, name
    start, depth, quote, escape, comment = match.start(), 1, None, False, None
    index = match.end()
    while depth:
        char = source[index]
        if comment == 'line':
            if char == '\n': comment = None
        elif comment == 'block':
            if source[index:index+2] == '*/': comment = None; index += 1
        elif quote:
            if escape: escape = False
            elif char == "\\": escape = True
            elif char == quote: quote = None
        elif source[index:index+2] == '//': comment = 'line'; index += 1
        elif source[index:index+2] == '/*': comment = 'block'; index += 1
        elif char in "\"'": quote = char
        elif char == "{": depth += 1
        elif char == "}": depth -= 1
        index += 1
    return source[start:index]


def translate(text):
    text = re.sub(r'(?m)^(\s*)([a-z_0-9]+) =(?!=)', r'\1var \2 =', text)
    text = re.sub(r'for \(([a-z_0-9]+) =', r'for (var \1 =', text)
    text = re.sub(r'&(island_pillar_[4-7]_[1-3])', r'\1', text)
    text = text.replace('.size', '.length').replace('struct::get_array', 'struct_get_array').replace('util::spawn_model', 'spawn_model')
    text = text.replace('level clientfield::get(', 'world_get(').replace('clientfield::register(', 'native_register(')
    text = text.replace('level thread island_pillar_render(localclientnum, slot, generation);', 'enqueue_render(level, island_pillar_render, localclientnum, slot, generation);')
    text = text.replace('model thread island_pillar_model_lifetime(localclientnum, slot, record);', 'enqueue_lifetime(model, island_pillar_model_lifetime, localclientnum, slot, record);')
    text = text.replace('record.model island_pillar_apply_parts(localclientnum, decoded, player);', 'with_self(record.model, () => island_pillar_apply_parts(localclientnum, decoded, player));')
    for target in ('record.model', 'self'):
        text = text.replace(target + ' hasdobj(', 'hasdobj(' + target + ', ').replace(target + ' delete();', 'delete_model(' + target + ');')
    text = text.replace('model hide();', 'hide_model(model);').replace('record.model show();', 'show_model(record.model);')
    text = text.replace('player getentitynumber()', 'getentitynumber(player)').replace('self hidepart(', 'hidepart(self, ').replace('self showpart(', 'showpart(self, ')
    text = text.replace('self endon(#"entityshutdown");', 'native_endon(self, "entityshutdown");').replace('wait 0.05;', 'step_wait(0.05);')
    text = text.replace('while (island_pillar_model_current(localclientnum, slot, record)) wait 0.1;', 'while (island_pillar_model_current(localclientnum, slot, record)) yield 0.1;')
    text = text.replace('function island_pillar_model_lifetime(', 'function* island_pillar_model_lifetime(')
    text = text.replace('result.origin = (x, y, z);', 'result.origin = vec(x, y, z);')
    text = text.replace('result.angles = (template_angles[0], template_angles[1] + yaw, template_angles[2]);', 'result.angles = vec(template_angles[0], template_angles[1] + yaw, template_angles[2]);')
    text = text.replace('(frames[pair[0]].origin + frames[pair[1]].origin) * 0.5', 'scale(add(frames[pair[0]].origin, frames[pair[1]].origin), 0.5)')
    text = text.replace('vectortoangles(altars[0].origin - origin)', 'vectortoangles(sub(altars[0].origin, origin))').replace('angles = (0, facing[1], 0);', 'angles = vec(0, facing[1], 0);')
    return text


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    source_path = args.root / 'scripts/zombies_coop/eroded_coop_fields.csc'
    source = source_path.read_text(encoding='utf-8')
    wrappers = tuple('island_pillar_' + str(slot) + '_' + str(tier) for slot in range(4,8) for tier in range(1,4))
    names = ('schema_revision', 'schema_capacity_current', 'register_map_slot_fields', 'island_pillar_valid_vector',
             'island_pillar_clone_pose', 'island_pillar_read_state', 'island_pillar_render_current', 'island_pillar_retire',
             'island_pillar_world_update', 'island_pillar_template', 'island_pillar_model_current', 'island_pillar_model_lifetime',
             'island_pillar_render', 'island_pillar_apply_parts') + wrappers
    extracted = '\n\n'.join(translate(body(source,name)) for name in names)
    code = r'''"use strict";
let checks=0,self,level,env,calls,queue,lifetimes,models,nextModel,registrations;
const isint=v=>typeof v==='number'&&Number.isInteger(v),isfloat=v=>typeof v==='number';
const isdefined=v=>v!==undefined&&v!==null&&v.alive!==false,isarray=Array.isArray,array=(...v)=>v,spawnstruct=()=>({});
const vec=(...v)=>Object.assign(v,{isvector:true}),isvec=v=>Array.isArray(v)&&v.isvector===true&&v.length===3;
const add=(a,b)=>vec(...a.map((v,i)=>v+b[i])),sub=(a,b)=>vec(...a.map((v,i)=>v-b[i])),scale=(a,n)=>vec(...a.map(v=>v*n));
const distance=(a,b)=>Math.hypot(...a.map((v,i)=>v-b[i])),vectortoangles=v=>vec(0,Math.atan2(v[1],v[0])*180/Math.PI,0);
const cos=v=>Math.cos(v*Math.PI/180),sin=v=>Math.sin(v*Math.PI/180);
const getdvarstring=()=>env.map,getdvarint=()=>env.capacity,getlocalplayer=n=>env.players[n],getentitynumber=p=>p.slot;
const getarraykeys=a=>Object.keys(a).filter(k=>k!=='isvector').map(k=>/^\d+$/.test(k)?Number(k):k);
const struct_get_array=(name,key)=>{if(key!=='targetname')throw Error('foreign struct key');if(name==='s_challenge_trigger')return env.triggers;if(name==='s_challenge_altar')return env.altars;throw Error('foreign struct lookup');};
const getentarray=(local,name,key)=>{if(name!=='challenge_pillar_0'||key!=='targetname'||local<0||local>3)throw Error('foreign model lookup');return env.templates;};
const world_get=name=>{if(!/^pillar_challenge_[4-7]_[1-3]$/.test(name))throw Error('foreign world read');env.reads.push(name);return env.fields[name]??0;};
const native_register=(...args)=>registrations.push(args);
function check(v,n){if(!v)throw Error(n);checks++;}
function model_ok(m){if(!m||m.owned!==true||m.alive!==true)throw Error('deleted/foreign model native call');}
function spawn_model(local,name,origin,angles){if(env.failspawn)return undefined;if(typeof name!=='string'||!isvec(origin)||!isvec(angles))throw Error('invalid spawn arguments');const m={owned:true,alive:true,id:nextModel++,local,model:name,origin,angles};models.push(m);calls.push(['spawn',m.id,local,name,origin,angles]);return m;}
function hasdobj(m,local){model_ok(m);check(local===m.local,'DObj exact local client');return env.dobj;}
const hide_model=m=>{model_ok(m);calls.push(['model-hide',m.id]);},show_model=m=>{model_ok(m);calls.push(['model-show',m.id]);};
const delete_model=m=>{model_ok(m);m.alive=false;calls.push(['delete',m.id]);};
const hidepart=(m,l,p,n)=>{model_ok(m);calls.push(['hide',l,p,n,m.id]);},showpart=(m,l,p,n)=>{model_ok(m);calls.push(['show',l,p,n,m.id]);};
const native_endon=(m,n)=>{model_ok(m);if(n!=='entityshutdown')throw Error('unexpected lifetime endon');};
function with_self(entity,fn){const old=self;self=entity;try{return fn();}finally{self=old;}}
const enqueue_render=(entity,fn,...args)=>queue.push({entity,fn,args}),enqueue_lifetime=(entity,fn,...args)=>lifetimes.push({entity,generator:fn(...args)});
function runqueue(){const jobs=queue;queue=[];for(const j of jobs)with_self(j.entity,()=>j.fn(...j.args));}
function lifetime_step(){const keep=[];for(const j of lifetimes){if(j.entity.alive===false)continue;const r=with_self(j.entity,()=>j.generator.next());if(!r.done){check(r.value===.1,'bounded lifetime poll');keep.push(j);}}lifetimes=keep;}
const step_wait=t=>{if(t!==.05)throw Error('unexpected wait');env.waits++;if(env.onwait)env.onwait();};
function setup(capacity=8){
 level={eroded_coop_fields_revision:3};self=level;
 env={map:'zm_island',capacity,players:[{slot:4},{slot:0},{slot:1},{slot:2}],dobj:true,waits:0,fields:{},reads:[],
 templates:[{model:'p7_zm_isl_ritual_pillar_symbol',origin:vec(-30,0,5),angles:vec(0,0,0)}],
 triggers:[[0,0,0],[0,300,0],[300,300,0],[300,0,0]].map((o,i)=>({var_7cc697d0:i,origin:vec(...o),angles:vec(0,i>1?180:0,0)})),
 altars:[{origin:vec(150,150,0)}]};
 calls=[];queue=[];lifetimes=[];models=[];nextModel=1;registrations=[];
}
function setstates(slot,states){for(let tier=1;tier<=3;tier++)env.fields[`pillar_challenge_${slot}_${tier}`]=states[tier-1];}
function update(slot=4,tier=1,value=1,local=0){island_pillar_world_update(local,slot,tier,value);runqueue();}
const count=k=>calls.filter(x=>x[0]===k).length,near=(a,b)=>Math.abs(a-b)<1e-8;
''' + extracted + r'''
// Exhaustive fixed three-bit field domain: only actual states1..4 are active.
for(let capacity=5;capacity<=8;capacity++)for(let slot=4;slot<8;slot++)for(let a=0;a<8;a++)for(let b=0;b<8;b++)for(let c=0;c<8;c++){
 setup(capacity);setstates(slot,[a,b,c]);const actual=island_pillar_read_state(slot),valid=slot<capacity&&[a,b,c].every(v=>v>=1&&v<=4);
 check(JSON.stringify(actual)===JSON.stringify(valid?{slot,states:[a,b,c]}:undefined),'world triplet domain');
}
for(const value of [true,'1',1.5,NaN,Infinity,null,undefined,-1,8])for(let tier=0;tier<3;tier++){setup();const s=[1,2,3];s[tier]=value;setstates(4,s);check(island_pillar_read_state(4)===undefined,'typed actual world values');}
setup();register_map_slot_fields('zm_island');check(registrations.length===16&&registrations.every(r=>r[0]==='world'),'no scriptmover field or extra mover bits');
const callbacks=new Map(registrations.filter(r=>r[1].startsWith('pillar_')).map(r=>[r[1],r[5]]));
check(callbacks.size===12&&registrations.filter(r=>r[1].startsWith('pillar_')).every(r=>r[2]===9000&&r[3]===3&&r[4]==='int'&&r[6]===false&&r[7]===true),'existing12three-bit world fields callbacks including zero');
for(let slot=4;slot<8;slot++)for(let tier=1;tier<=3;tier++){
 setup();setstates(slot,[2,3,4]);const fn=callbacks.get(`pillar_challenge_${slot}_${tier}`);fn(0,7,2,false,true,'ignored',false);runqueue();
 check(count('spawn')===1&&level.eroded_coop_pillar_models[slot].slot===slot,'each actual wrapper targets own slot');
 check(calls.filter(c=>c[0]==='show').map(c=>c[2]).join('|').endsWith('_claimed_03'),'wrapper uses native current triplet rather than callbackvalue');
}
// Retain actual parts helper owner/tier behavior independently of construction.
for(let slot=4;slot<8;slot++)for(let player=0;player<8;player++){
 setup();const m=spawn_model(0,'p7_zm_isl_ritual_pillar_symbol',vec(0,0,0),vec(0,0,0));with_self(m,()=>island_pillar_apply_parts(0,{slot,states:[1,2,4]},{slot:player}));
 const shown=calls.filter(c=>c[0]==='show'),role=slot===player?'player':'ally';check(count('hide')===18&&shown.length===2,'six parts per tier and idle blank');
 check(shown[0][2]===`j_${role}_started_02`&&shown[1][2]===`j_${role}_claimed_03`,'owner and tier match');
}
// Client-only midpoint frames preserve the actual retail template offset.
setup();const expected=[[-30,150,5],[330,150,5],[150,330,5],[150,-30,5]];
for(let slot=4;slot<8;slot++){const t=island_pillar_template(0,slot);check(isdefined(t)&&t.pose.origin.every((v,i)=>near(v,expected[slot-4][i])),'altar-facing midpoint and template offset');}
for(let yaw=-720;yaw<=720;yaw+=15){const p=island_pillar_clone_pose(vec(12,24,38),vec(5,17,7),vec(10,20,30),vec(0,20,0),vec(-100,200,300),vec(0,yaw,0)),r=(yaw-20)*Math.PI/180;check(isdefined(p)&&near(p.origin[0],-100+2*Math.cos(r)-4*Math.sin(r))&&near(p.origin[1],200+2*Math.sin(r)+4*Math.cos(r))&&p.origin[2]===308,'relative rotation/Z');check(p.angles[0]===5&&p.angles[2]===7&&p.angles[1]===17+yaw-20,'relative angles');}
for(const v of [undefined,null,[],[1,2,3],vec(1,2),vec(1,2,3,4),vec(NaN,0,0),vec(Infinity,0,0),vec(true,0,0),vec(1000000,0,0),vec(-1000000,0,0)])check(!island_pillar_valid_vector(v,1000000),'typed finite bounded vector');
for(const mutate of [()=>env.templates=[],()=>env.templates.push(env.templates[0]),()=>delete env.templates[0].model,()=>env.triggers=[],()=>env.triggers[0]=undefined,()=>env.triggers[0].var_7cc697d0='0',()=>env.triggers[1].var_7cc697d0=0,()=>env.triggers[2].var_7cc697d0=1,()=>env.triggers[2].var_7cc697d0=4,()=>env.triggers[2].var_7cc697d0=-1,()=>env.triggers[2].angles=vec(1,0,0),()=>env.triggers[2].origin=vec(32769,0,0),()=>env.altars=[],()=>env.altars.push(env.altars[0]),()=>env.altars[0].origin=vec(NaN,0,0),()=>env.altars[0].origin=vec(0,150,0),()=>env.altars[0].origin=vec(9999,0,0)]){setup();mutate();check(island_pillar_template(0,4)===undefined,'ambiguous/unsafe client geometry declined');}
setup();env.triggers[0].script_special=0;delete env.triggers[0].var_7cc697d0;check(island_pillar_template(0,4)===undefined,'literal script_special cannot substitute raw ID');
setup();update();check(count('spawn')===0,'default all-zero absent slot');
env.fields.pillar_challenge_4_1=1;update();env.fields.pillar_challenge_4_2=1;update(4,2);check(count('spawn')===0,'partial snapshot not visible');
env.fields.pillar_challenge_4_3=1;update(4,3);check(count('spawn')===1&&count('model-hide')===1&&count('model-show')===1,'first complete triplet creates hidden then ready clone');
setup();callbacks.get('pillar_challenge_4_1')(0,0,1,false,true,'ignored',false);check(env.reads.length===0&&queue.length===1,'callback defers native triplet read until committed snapshot');
env.onwait=()=>{if(env.waits===1)setstates(4,[1,1,1]);};runqueue();check(env.waits===1&&count('spawn')===1&&count('model-show')===1,'native snapshot committed on first deferred tick is rendered');
setup();setstates(4,[2,3,4]);update(4,1,0);check(count('spawn')===1&&count('delete')===0&&count('show')===3,'stale zero callback cannot hide a currently active triplet');
update(4,2,7);check(count('spawn')===1&&count('delete')===0&&count('show')===6,'outdated callback payload never replaces native current states');
setup();setstates(4,[1,1,1]);update();setstates(4,[2,3,4]);update(4,2,1);check(count('spawn')===1&&count('delete')===0&&count('show')===3,'state callback reuses clone and reads actual triplet');
setstates(4,[0,0,0]);update(4,1,4);check(count('delete')===1&&level.eroded_coop_pillar_models[4]===undefined,'actualzero wins over stale nonzero callback and disconnect retires');
setstates(4,[1,1,1]);update();check(count('spawn')===2&&models[1].alive,'same slot reconnect creates one fresh clone');lifetime_step();check(level.eroded_coop_pillar_models[4].model===models[1],'old shutdown cannot clear reconnected model');
setup();setstates(4,[2,3,4]);env.templates[0].model='custom_composite_pillar';update();check(count('spawn')===1&&count('hide')===0&&count('show')===0&&count('model-show')===1,'unknown model has no assumed joints');
for(const failure of ['dobj','template','spawn','player']){setup();setstates(4,[1,1,1]);if(failure==='dobj')env.dobj=false;if(failure==='template')env.templates=[];if(failure==='spawn')env.failspawn=true;if(failure==='player')env.players[0]=undefined;update();check(env.waits===120&&count('model-show')===0,'bounded six-second failure '+failure);if(failure==='dobj')check(count('spawn')===1&&count('delete')===1,'DObj timeout releases owned hidden clone');}
setup();setstates(4,[1,1,1]);env.dobj=false;env.onwait=()=>{if(env.waits===3)env.dobj=true;};update();check(env.waits===3&&count('spawn')===1&&count('model-show')===1,'delayed DObj reuses hidden clone');
setup();setstates(4,[1,1,1]);env.dobj=false;env.onwait=()=>{if(env.waits===2){setstates(4,[3,3,3]);env.dobj=true;island_pillar_world_update(0,4,1,3);runqueue();}};update();check(count('spawn')===1&&count('delete')===0&&count('show')===3&&count('model-show')===1,'callback during wait reuses one atomic record and only newest generation shows');
setup();setstates(4,[1,1,1]);island_pillar_world_update(0,4,1,1);setstates(4,[0,0,0]);island_pillar_world_update(0,4,1,0);runqueue();check(count('spawn')===0,'disconnect cancels stale queued render');
setup();setstates(4,[1,1,1]);env.dobj=false;env.onwait=()=>{if(env.waits===2)setstates(4,[0,1,1]);};update();check(env.waits===2&&count('delete')===1&&count('model-show')===0,'state loss during wait retires before rendering');
for(const change of ['map','capacity','revision','zero','record','active']){setup();setstates(7,[2,3,4]);update(7);const record=level.eroded_coop_pillar_models[7],owned=models[0];lifetime_step();if(change==='map')env.map='zm_zod';if(change==='capacity')env.capacity=5;if(change==='revision')level.eroded_coop_fields_revision=2;if(change==='zero')setstates(7,[1,0,1]);if(change==='record')level.eroded_coop_pillar_models[7]={active:true,slot:7,model:{owned:false}};if(change==='active')record.active=false;lifetime_step();check(!owned.alive&&count('delete')===1,'lifetime releases '+change);if(change==='record')check(level.eroded_coop_pillar_models[7].model.owned===false,'foreign replacement preserved');else check(level.eroded_coop_pillar_models[7]===undefined,'own record cleared');}
setup();setstates(4,[1,1,1]);update();delete_model(models[0]);update();check(count('spawn')===2&&count('delete')===1&&models[1].alive,'externally destroyed model safely replaced');
for(const args of [[-1,4,1],[4,4,1],[true,4,1],['0',4,1],[0,3,1],[0,8,1],[0,4,0],[0,4,4],[0,4,'1']]){setup();setstates(4,[1,1,1]);island_pillar_world_update(...args,1);runqueue();check(count('spawn')===0,'invalid local/slot/tier has no allocation');}
setup();for(let local=0;local<4;local++)for(let slot=4;slot<8;slot++){setstates(slot,[1,1,1]);update(slot,1,1,local);}check(count('spawn')===16&&new Set(level.eroded_coop_pillar_models.filter(Boolean).map(x=>x.model.id)).size===16,'four slots independently owned per local client');
setstates(5,[0,0,0]);for(let local=0;local<4;local++)update(5,1,0,local);check(count('delete')===4&&models.filter(m=>m.alive).length===12,'one disconnected slot cannot delete other slots or local clients');
console.log(JSON.stringify({passed:true,checks,scope:'Actual CSC source bodies with explicit native world-field/entity/scheduler stand-ins; no game, client rendering, replication or engine compatibility proof.'}));
'''
    args.output_dir.mkdir(parents=True,exist_ok=False)
    js=args.output_dir/'actual-functions.cjs';js.write_text(code,encoding='utf-8')
    node=shutil.which('node');assert node,'Node.js required for standalone source-body checks'
    result=subprocess.run([node,str(js)],text=True,capture_output=True,timeout=30)
    (args.output_dir/'stdout.log').write_text(result.stdout,encoding='utf-8');(args.output_dir/'stderr.log').write_text(result.stderr,encoding='utf-8')
    assert result.returncode==0,result.stderr
    assert source_path.read_text(encoding='utf-8') == source, 'CSC changed while its extracted bodies were tested'
    proof=json.loads(result.stdout);proof.update({'source_sha256':hashlib.sha256(source_path.read_bytes().replace(b'\r\n',b'\n')).hexdigest(),
        'test_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'extracted_functions':names,'created_utc':datetime.datetime.now(datetime.timezone.utc).isoformat()})
    (args.output_dir/'RESULTS.private.json').write_text(json.dumps(proof,indent=2)+'\n',encoding='utf-8');print(json.dumps(proof))


if __name__=='__main__':main()
