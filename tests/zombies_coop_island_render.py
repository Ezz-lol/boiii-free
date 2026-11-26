"""Execute actual CSC codec/locator/clone/lifetime bodies with native stand-ins.

No BO3/client image is loaded. Explicit native stand-ins record resource
ownership; extracted production CSC bodies make every lifecycle decision.
"""
from pathlib import Path
import argparse, datetime, hashlib, json, re, shutil, subprocess


def body(source, name):
    match = re.search(r"\bfunction\s+" + re.escape(name) + r"\s*\([^)]*\)\s*\{", source)
    assert match, name
    start, depth, quote, escape = match.start(), 1, None, False
    index = match.end()
    while depth:
        char = source[index]
        if quote:
            if escape: escape = False
            elif char == "\\": escape = True
            elif char == quote: quote = None
        elif char in "\"'": quote = char
        elif char == "{": depth += 1
        elif char == "}": depth -= 1
        index += 1
    return source[start:index]


def translate(text):
    text = re.sub(r'(?m)^(\s*)([a-z_0-9]+) =(?!=)', r'\1var \2 =', text)
    text = re.sub(r'for \(([a-z_0-9]+) =', r'for (var \1 =', text)
    text = text.replace('.size', '.length').replace('struct::get_array', 'struct_get_array').replace('util::spawn_model', 'spawn_model')
    text = text.replace('self thread island_pillar_render(localclientnum, decoded, generation);',
                        'enqueue_render(self, island_pillar_render, localclientnum, decoded, generation);')
    text = text.replace('model thread island_pillar_model_lifetime(localclientnum, self, record);',
                        'enqueue_lifetime(model, island_pillar_model_lifetime, localclientnum, self, record);')
    text = text.replace('record.model island_pillar_apply_parts(localclientnum, decoded, player);',
                        'with_self(record.model, () => island_pillar_apply_parts(localclientnum, decoded, player));')
    for target in ('record.model', 'self'):
        text = text.replace(target + ' hasdobj(', 'hasdobj(' + target + ', ')
        text = text.replace(target + ' delete();', 'delete_model(' + target + ');')
    text = text.replace('model hide();', 'hide_model(model);').replace('record.model show();', 'show_model(record.model);')
    text = text.replace('player getentitynumber()', 'getentitynumber(player)')
    text = text.replace('self hidepart(', 'hidepart(self, ').replace('self showpart(', 'showpart(self, ')
    text = text.replace('self endon(#"entityshutdown");', 'native_endon(self, "entityshutdown");')
    text = text.replace('wait 0.05;', 'step_wait(0.05);')
    text = text.replace('while (island_pillar_model_current(localclientnum, locator, record)) wait 0.1;',
                        'while (island_pillar_model_current(localclientnum, locator, record)) yield 0.1;')
    text = text.replace('function island_pillar_model_lifetime(', 'function* island_pillar_model_lifetime(')
    text = text.replace('result.origin = (x, y, z);', 'result.origin = vec(x, y, z);')
    text = text.replace('result.angles = (template_angles[0], template_angles[1] + yaw, template_angles[2]);',
                        'result.angles = vec(template_angles[0], template_angles[1] + yaw, template_angles[2]);')
    return text


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    source_path = args.root / 'scripts/zombies_coop/eroded_coop_fields.csc'
    source = source_path.read_text(encoding='utf-8')
    names = ('schema_capacity_current', 'island_pillar_decode', 'island_pillar_valid_vector',
             'island_pillar_clone_pose', 'island_pillar_render_current', 'island_pillar_retire',
             'island_pillar_clientfield', 'island_pillar_template', 'island_pillar_model_current',
             'island_pillar_model_lifetime', 'island_pillar_render', 'island_pillar_apply_parts')
    extracted = '\n\n'.join(translate(body(source, name)) for name in names)
    code = r'''"use strict";
let checks=0,self,env,calls,queue,lifetimes,models,nextModel;
const isint=v=>typeof v==='number'&&Number.isInteger(v),isfloat=v=>typeof v==='number';
const isdefined=v=>v!==undefined&&v!==null&&v.alive!==false;
const isarray=Array.isArray,array=(...v)=>v,spawnstruct=()=>({});
const vec=(...v)=>Object.assign(v,{isvector:true}),isvec=v=>Array.isArray(v)&&v.isvector===true&&v.length===3;
const cos=v=>Math.cos(v*Math.PI/180),sin=v=>Math.sin(v*Math.PI/180);
const getdvarstring=()=>env.map,getdvarint=()=>env.capacity;
const getlocalplayer=()=>env.player,getentitynumber=p=>p.slot;
const getarraykeys=a=>Object.keys(a).filter(k=>k!=='isvector').map(k=>/^\d+$/.test(k)?Number(k):k);
const struct_get_array=(name,key)=>{if(name!=='s_challenge_trigger'||key!=='targetname')throw Error('foreign struct lookup');return env.triggers;};
const getentarray=(local,name,key)=>{if(name!=='challenge_pillar_0'||key!=='targetname'||local<0||local>3)throw Error('foreign model lookup');return env.templates;};
function check(v,n){if(!v)throw Error(n);checks++;}
function model_ok(m){if(!m||m.owned!==true||m.alive!==true)throw Error('deleted/foreign model native call');}
function spawn_model(local,name,origin,angles){
 if(env.failspawn)return undefined;
 if(typeof name!=='string'||!isvec(origin)||!isvec(angles))throw Error('invalid spawn arguments');
 const m={owned:true,alive:true,id:nextModel++,local,model:name,origin,angles,hidden:false};models.push(m);calls.push(['spawn',m.id,local,name,origin,angles]);return m;
}
function hasdobj(m,local){model_ok(m);check(local===m.local,'DObj exact local client');return env.dobj;}
function hide_model(m){model_ok(m);m.hidden=true;calls.push(['model-hide',m.id]);}
function show_model(m){model_ok(m);m.hidden=false;calls.push(['model-show',m.id]);}
function delete_model(m){model_ok(m);m.alive=false;calls.push(['delete',m.id]);}
function hidepart(m,local,part,model){model_ok(m);calls.push(['hide',local,part,model,m.id]);}
function showpart(m,local,part,model){model_ok(m);calls.push(['show',local,part,model,m.id]);}
const native_endon=(m,name)=>{model_ok(m);if(name!=='entityshutdown')throw Error('unexpected lifetime endon');};
function with_self(entity,fn){const previous=self;self=entity;try{return fn();}finally{self=previous;}}
const enqueue_render=(entity,fn,...args)=>queue.push({entity,fn,args});
const enqueue_lifetime=(entity,fn,...args)=>lifetimes.push({entity,generator:fn(...args)});
function runqueue(){const jobs=queue;queue=[];for(const j of jobs)with_self(j.entity,()=>j.fn(...j.args));}
function lifetime_step(){const keep=[];for(const j of lifetimes){if(j.entity.alive===false)continue;const result=with_self(j.entity,()=>j.generator.next());if(!result.done){check(result.value===.1,'bounded lifetime poll');keep.push(j);}}lifetimes=keep;}
const step_wait=t=>{if(t!==.05)throw Error('unexpected render wait');env.waits++;if(env.onwait)env.onwait();};
function setup(slot=4,capacity=8){
 self={alive:true,origin:vec(10,20,30),angles:vec(0,90,0),eroded_coop_pillar_render_generation:[1,1,1,1],eroded_coop_pillar_models:[]};
 env={map:'zm_island',capacity,player:{slot},dobj:true,waits:0,
 templates:[{model:'p7_zm_isl_ritual_pillar_symbol',origin:vec(2,0,8),angles:vec(0,10,0)}],
 triggers:Array.from({length:4},(_,i)=>({var_7cc697d0:i,origin:vec(0,0,3),angles:vec(0,0,0)}))};
 calls=[];queue=[];lifetimes=[];models=[];nextModel=1;
}
function update(value,local=0){island_pillar_clientfield(local,0,value,false,true,'eroded_island_pillar',false);runqueue();}
const count=k=>calls.filter(x=>x[0]===k).length,near=(a,b)=>Math.abs(a-b)<1e-8;
''' + extracted + r'''
const encode=(slot,states)=>(slot-4)|(states[0]<<2)|(states[1]<<5)|(states[2]<<8);
for(let capacity=5;capacity<=8;capacity++){
 const valid=new Map();
 for(let slot=4;slot<capacity;slot++)for(let a=1;a<=4;a++)for(let b=1;b<=4;b++)for(let c=1;c<=4;c++)valid.set(encode(slot,[a,b,c]),{slot,states:[a,b,c]});
 for(let v=0;v<=2047;v++)check(JSON.stringify(island_pillar_decode(v,capacity))===JSON.stringify(valid.get(v)),'wire domain '+capacity+':'+v);
}
for(const v of [-1,2048,Infinity,NaN,1.5,true,null,undefined,'292'])check(island_pillar_decode(v,8)===undefined,'invalid scalar');
for(const capacity of [0,1,4,9,8.5,true,null,'8'])check(island_pillar_decode(292,capacity)===undefined,'invalid capacity');
// Preserve parts coverage independently of constructor/lifetime behavior.
for(let slot=4;slot<8;slot++)for(let local=0;local<8;local++){
 setup(local);const model=spawn_model(0,'p7_zm_isl_ritual_pillar_symbol',vec(0,0,0),vec(0,0,0));
 with_self(model,()=>island_pillar_apply_parts(0,{slot,states:[1,2,4]},env.player));
 check(count('hide')===18,'clear six parts per tier');const shown=calls.filter(x=>x[0]==='show');check(shown.length===2,'idle tier blank');const role=local===slot?'player':'ally';
 check(shown[0][2]===`j_${role}_started_02`&&shown[1][2]===`j_${role}_claimed_03`,'real owner and tier');
 check(calls.filter(x=>x[0]==='show'||x[0]==='hide').every(x=>x[1]===0&&x[3]==='p7_zm_isl_ritual_pillar_symbol'),'native parts arguments');
}
setup();let template=island_pillar_template(0);
check(template.model==='p7_zm_isl_ritual_pillar_symbol'&&near(template.pose.origin[0],10)&&near(template.pose.origin[1],22)&&near(template.pose.origin[2],35),'actual template relative offset');
check(near(template.pose.angles[1],100),'actual template relative yaw');
for(let yaw=-720;yaw<=720;yaw+=15){
 const pose=island_pillar_clone_pose(vec(12,24,38),vec(5,17,7),vec(10,20,30),vec(0,20,0),vec(-100,200,300),vec(0,yaw,0)),r=(yaw-20)*Math.PI/180;
 check(isdefined(pose)&&near(pose.origin[0],-100+2*Math.cos(r)-4*Math.sin(r))&&near(pose.origin[1],200+2*Math.sin(r)+4*Math.cos(r))&&pose.origin[2]===308,'relative rotation/Z');
 check(pose.angles[0]===5&&pose.angles[2]===7&&pose.angles[1]===17+yaw-20,'relative angles');
}
for(const v of [undefined,null,[],[1,2,3],vec(1,2),vec(1,2,3,4),vec(NaN,0,0),vec(Infinity,0,0),vec(true,0,0),vec(1000000,0,0),vec(-1000000,0,0)])check(!island_pillar_valid_vector(v,1000000),'typed finite bounded vector');
for(const bound of [0,-1,1.1,1000001,true,'100'])check(!island_pillar_valid_vector(vec(0,0,0),bound),'typed bound');
for(let position=0;position<6;position++){const v=Array.from({length:6},()=>vec(0,0,0));v[position]=vec(NaN,0,0);check(island_pillar_clone_pose(...v)===undefined,'every vector validated');}
for(const [base,locator]of [[vec(1,0,0),vec(0,0,0)],[vec(0,0,1),vec(0,0,0)],[vec(0,0,0),vec(1,0,0)],[vec(0,0,0),vec(0,0,1)]])check(island_pillar_clone_pose(vec(0,0,0),vec(0,0,0),vec(0,0,0),base,vec(0,0,0),locator)===undefined,'tilted frame declined');
check(island_pillar_clone_pose(vec(999999,0,0),vec(0,0,0),vec(0,0,0),vec(0,0,0),vec(999999,0,0),vec(0,0,0))===undefined,'transformed origin bounded');
check(island_pillar_clone_pose(vec(0,0,0),vec(0,3599,0),vec(0,0,0),vec(0,0,0),vec(0,0,0),vec(0,3599,0))===undefined,'transformed angle bounded');
for(const mutate of [()=>env.templates=[],()=>env.templates.push(env.templates[0]),()=>env.templates[0]=undefined,()=>delete env.templates[0].model,()=>env.triggers=[],()=>env.triggers[0]=undefined,()=>env.triggers[0].var_7cc697d0=undefined,()=>env.triggers[0].var_7cc697d0='0',()=>env.triggers[1].var_7cc697d0=0,()=>env.triggers[0].var_7cc697d0=1,()=>env.triggers[2].var_7cc697d0=1,()=>env.triggers[2].var_7cc697d0=-1,()=>env.triggers[2].var_7cc697d0=4,()=>self.origin=vec(NaN,0,0)]){setup();mutate();check(island_pillar_template(0)===undefined,'ambiguous/invalid template declined');}
setup();env.triggers[0].script_special=0;delete env.triggers[0].var_7cc697d0;check(island_pillar_template(0)===undefined,'literal script_special cannot substitute raw field ID');
setup();update(292);check(count('spawn')===1&&count('model-hide')===1&&count('model-show')===1,'one hidden then ready clone');
check(models[0]!==self&&self.eroded_coop_pillar_models[0].model===models[0],'locator never treated as DObj');check(count('hide')===18&&count('show')===0,'all-idle parts');lifetime_step();
update(encode(4,[3,3,3]));check(count('spawn')===1&&count('delete')===0&&count('show')===3,'state callback reuses owned clone');
update(encode(5,[2,3,4]));check(count('spawn')===2&&count('delete')===1&&!models[0].alive&&models[1].alive,'new owner retires old clone');
lifetime_step();check(count('delete')===1&&self.eroded_coop_pillar_models[0].model===models[1],'old lifetime cannot clear replacement');
update(0);check(count('delete')===2&&self.eroded_coop_pillar_models[0]===undefined,'inactive update retires clone');lifetime_step();check(count('delete')===2,'no double delete after shutdown');
setup();env.templates[0].model='custom_composite_pillar';update(encode(4,[2,3,4]));check(count('spawn')===1&&count('hide')===0&&count('show')===0&&count('model-show')===1,'unknown model joint calls guarded');
setup();env.dobj=false;update(292);check(env.waits===120&&count('spawn')===1&&count('delete')===1&&count('model-show')===0,'finite DObj wait retires hidden clone');
setup();env.dobj=false;env.onwait=()=>{if(env.waits===3)env.dobj=true;};update(292);check(env.waits===3&&count('spawn')===1&&count('model-show')===1,'delayed DObj reuses clone');
setup();env.failspawn=true;update(292);check(env.waits===120&&count('spawn')===0&&count('delete')===0,'native spawn failure bounded');
setup();env.templates=[];update(292);check(env.waits===120&&count('spawn')===0,'missing template bounded');
setup();env.player=undefined;update(292);check(env.waits===120&&count('spawn')===0,'missing player bounded');
for(const change of ['map','capacity','locator','record','owner']){
 setup();update(encode(7,[2,3,4]));const locator=self,record=self.eroded_coop_pillar_models[0],owned=models[0];lifetime_step();
 if(change==='map')env.map='zm_zod';if(change==='capacity')env.capacity=5;if(change==='locator')locator.alive=false;if(change==='record')locator.eroded_coop_pillar_models[0]={active:true,model:{owned:false},slot:7};if(change==='owner')record.active=false;
 lifetime_step();check(!owned.alive&&count('delete')===1,'lifetime releases on '+change);
 if(change==='record')check(locator.eroded_coop_pillar_models[0]!==undefined&&locator.eroded_coop_pillar_models[0].model.owned===false,'foreign replacement untouched');else if(change!=='locator')check(locator.eroded_coop_pillar_models[0]===undefined,'owned record cleared');
}
setup();env.dobj=false;env.onwait=()=>{if(env.waits===2)self.eroded_coop_pillar_render_generation[0]++;};update(292);check(env.waits===2&&count('model-show')===0&&models[0].alive,'generation cancels delayed render preserving reusable clone');
for(const change of ['map','capacity','locator']){
 setup();env.dobj=false;const locator=self;
 env.onwait=()=>{if(env.waits===2){if(change==='map')env.map='zm_zod';if(change==='capacity')env.capacity=4;if(change==='locator')locator.alive=false;}};
 update(292);check(env.waits===2&&count('spawn')===1&&count('model-show')===0,'waiting render cancels on '+change);
 lifetime_step();check(count('delete')===1&&!models[0].alive,'waiting clone reclaimed on '+change);
}
setup();update(292);delete_model(models[0]);update(encode(4,[2,3,4]));check(count('spawn')===2&&count('delete')===1&&models[1].alive,'externally destroyed clone replaced without duplicate deletion');lifetime_step();check(self.eroded_coop_pillar_models[0].model===models[1],'old shutdown preserves replacement record');
setup();island_pillar_clientfield(0,0,292,false,true,'eroded_island_pillar',false);island_pillar_clientfield(0,292,encode(4,[3,3,3]),false,false,'eroded_island_pillar',false);runqueue();check(count('spawn')===1&&count('show')===3&&calls.filter(x=>x[0]==='show').every(x=>x[2].includes('_completed_')),'only newest queued generation renders');
setup();env.dobj=false;env.onwait=()=>{if(env.waits===2){env.dobj=true;island_pillar_clientfield(0,292,encode(4,[3,3,3]),false,false,'eroded_island_pillar',false);runqueue();}};update(292);check(count('spawn')===1&&count('delete')===0&&count('show')===3&&count('model-show')===1,'callback during wait reuses atomic record');
setup();island_pillar_clientfield(0,0,292,false,true,'eroded_island_pillar',false);island_pillar_clientfield(0,292,0,false,false,'eroded_island_pillar',false);runqueue();check(count('spawn')===0,'inactive cancels queued old callback');
for(const local of [-1,4,7,true,'0',undefined]){setup();island_pillar_clientfield(local,0,292,false,true,'eroded_island_pillar',false);runqueue();check(queue.length===0&&count('spawn')===0,'local index distinct from network slot');}
setup();for(let local=0;local<4;local++)update(292,local);check(count('spawn')===4&&new Set(self.eroded_coop_pillar_models.map(x=>x.model.id)).size===4,'one clone per local client');
console.log(JSON.stringify({passed:true,checks,scope:'Actual CSC bodies with explicit native/entity/scheduler stand-ins; no game, rendering, replication, client image, or engine compatibility proof.'}));
'''
    args.output_dir.mkdir(parents=True, exist_ok=False)
    js = args.output_dir / 'actual-functions.cjs'
    js.write_text(code, encoding='utf-8')
    node = shutil.which('node')
    assert node, 'Node.js required for standalone source-body checks'
    result = subprocess.run([node, str(js)], text=True, capture_output=True, timeout=30)
    (args.output_dir/'stdout.log').write_text(result.stdout,encoding='utf-8')
    (args.output_dir/'stderr.log').write_text(result.stderr,encoding='utf-8')
    assert result.returncode == 0, result.stderr
    proof=json.loads(result.stdout)
    proof.update({'source_sha256':hashlib.sha256(source_path.read_bytes().replace(b'\r\n',b'\n')).hexdigest(),
                  'test_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                  'extracted_functions':names,'created_utc':datetime.datetime.now(datetime.timezone.utc).isoformat()})
    (args.output_dir/'RESULTS.private.json').write_text(json.dumps(proof,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(proof))


if __name__ == '__main__': main()
