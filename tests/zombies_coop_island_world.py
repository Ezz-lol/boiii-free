"""Actual-source Island world glue checks with vector/engine stand-ins, not client or VM proof."""
from pathlib import Path
import argparse
import datetime
import hashlib
import json
import os
import re
import subprocess
import sys

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tests'))
from script_frame_diagnostics import function

NAMES = ('valid_capacity', 'valid_slot', 'bounded_world_point', 'rotate_world_offset',
         'derive_world_frame', 'placement_clear', 'prepare_world_templates', 'prepare_player_world', 'activate_player_world')


def translate(body):
    params = set(re.search(r'function\s+\w+\(([^)]*)\)', body)[1].replace(' ', '').split(','))
    masked = re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"', lambda m: ' ' * len(m[0]), body)
    locals_ = set(re.findall(r'(?<![\w.])([A-Za-z_]\w*)\s*=(?!=)', masked)) - params
    text = body.replace('struct::get_array', 'get_struct_array')
    text = re.sub(r'&zm_island_challenges::(\w+)', r'stock_\1', text)
    text = text.replace('zm_unitrigger::unitrigger_force_per_player_triggers', 'force_per_player').replace('zm_unitrigger::register_static_unitrigger', 'register_static')
    text = text.replace('(frames[pair[0]].origin + frames[pair[1]].origin) * 0.5', 'vecscale(vecadd(frames[pair[0]].origin, frames[pair[1]].origin), 0.5)')
    text = text.replace('vectortoangles(altar - origin)', 'vectortoangles(vecsub(altar, origin))')
    text = text.replace('return (offset[0] * cos(yaw) - offset[1] * sin(yaw),\n        offset[0] * sin(yaw) + offset[1] * cos(yaw), offset[2]);',
                        'return vec(offset[0] * cos(yaw) - offset[1] * sin(yaw), offset[0] * sin(yaw) + offset[1] * cos(yaw), offset[2]);')
    text = text.replace('(0, facing[1], 0)', 'vec(0, facing[1], 0)')
    text = text.replace('origin + rotate_world_offset(source.lookats[tier].origin - source.origin,\n            result.angles[1] - source.angles[1])',
                        'vecadd(origin, rotate_world_offset(vecsub(source.lookats[tier].origin, source.origin), result.angles[1] - source.angles[1]))')
    text = text.replace('origin + (0, 0, 32)', 'vecadd(origin, vec(0, 0, 32))').replace('origin - (0, 0, 96)', 'vecsub(origin, vec(0, 0, 96))')
    text = text.replace('origin + (0, 0, 48)', 'vecadd(origin, vec(0, 0, 48))').replace('altar + (0, 0, 24)', 'vecadd(altar, vec(0, 0, 24))')
    text = text.replace('pillar setmodel("tag_origin");', 'setmodel(pillar, "tag_origin");').replace('pillar notsolid();', 'notsolid(pillar);')
    text = text.replace('pillar clientfield::set("eroded_island_pillar", packed_pillar(slot, 1, 1, 1));', 'set_clientfield(pillar, "eroded_island_pillar", packed_pillar(slot, 1, 1, 1));')
    text = re.sub(r'player\s+flag::exists\(([^;]+?)\)', r'flag_exists(player, \1)', text)
    text = re.sub(r'(\b[A-Za-z_]\w*(?:(?:\[[A-Za-z_]\w*\])|(?:\.[A-Za-z_]\w*))*)\.size\b', r'gsc_size(\1)', text)
    if '::' in re.sub(r'//[^\n]*|/\*[\s\S]*?\*/', '', text):
        raise ValueError('Untranslated source namespace')
    position = text.index('{') + 1
    declarations = '\nlet ' + ','.join(sorted(locals_)) + ';\n' if locals_ else '\n'
    return text[:position] + declarations + text[position:]


STANDINS = r'''
const assert=require('node:assert/strict');let checks=0;
function check(v,label){++checks;assert.ok(v,label);}
function vec(...n){return n;}function vecadd(a,b){return a.map((v,i)=>v+b[i]);}function vecsub(a,b){return a.map((v,i)=>v-b[i]);}function vecscale(a,n){return a.map(v=>v*n);}
function cos(n){return Math.cos(n*Math.PI/180);}function sin(n){return Math.sin(n*Math.PI/180);}function abs(n){return Math.abs(n);}
function vectortoangles(v){return [0,(Math.atan2(v[1],v[0])*180/Math.PI+360)%360,0];}function distance(a,b){return Math.hypot(...vecsub(a,b));}
function isdefined(v){return v!==undefined;}function isint(v){return typeof v==='number'&&Number.isInteger(v)&&v>=-2147483648&&v<=2147483647;}
function isvec(v){return Array.isArray(v)&&v.length===3&&v.every(n=>typeof n==='number');}
function isarray(v){return Array.isArray(v);}function isentity(v){return v!==undefined&&v.entity===true;}function array(...v){return v;}function spawnstruct(){return {};}
function getarraykeys(v){return Object.keys(v).filter(k=>v[k]!==undefined).map(k=>/^(0|[1-9]\d*)$/.test(k)?Number(k):k);}function gsc_size(v){return getarraykeys(v).length;}
let level,owned,triggers,altars,lookats,floorBad,sightBad,spawns,calls,registerSuccess;
function registry_owned(){return owned;}function get_struct_array(key){return key==='s_challenge_trigger'?triggers:altars;}function getentarray(key,field){assert.equal(field,'targetname');return lookats[Number(key.at(-1))];}
function groundtrace(start,end,actors,ignore){calls.push('floor');assert.equal(actors,0);assert.equal(ignore,undefined);return floorBad??{fraction:48.5/128,position:[start[0],start[1],start[2]-48.5],normal:[0,0,1]};}
function bullettrace(start,end,actors,ignore){calls.push('sight');assert.equal(actors,0);assert.equal(ignore,undefined);return sightBad??{fraction:1};}
function spawn(type,origin){assert.equal(type,'script_model');const ent={entity:true,origin};spawns.push(ent);return ent;}function setmodel(ent,model){ent.model=model;}function notsolid(ent){ent.solid=false;}
function set_clientfield(ent,key,value){ent.field={key,value};}function flag_exists(player,key){return player.flags[key]!==undefined;}
function stock_function_3ae0d6d5(){}function stock_function_a00e23d0(){}
function force_per_player(stub,value){calls.push('per_player');assert.equal(value,1);stub.trigger_per_player=1;}
function register_static(stub,callback){calls.push('register');assert.equal(callback,stock_function_a00e23d0);assert.ok(gsc_size(level.zones)>0);stub.registered=registerSuccess?1:0;}
// Exact empty-server prestart metadata from independently captured Ezz raw source;
// no runtime location assumptions are made by these offline engine stand-ins.
const points=[[2377.47,-1376.64,-367.5],[2377.5,-1074,-367.5],[2742.76,-1070.12,-367.5],[2742.5,-1376.07,-367.5]];
const yaws=[0.199642,0,180.6,180.6];
const looks=[[2407,-1376],[2408,-1073],[2713,-1071],[2713,-1377]],heights=[0,-339,-314,-290];
function reset(){owned=true;level={eroded_island_trials_capacity:8,eroded_island_trials_epoch:7,_unitriggers:{trigger_stubs:[]},zones:{first:{}}};
 triggers=points.map((origin,slot)=>({origin:[...origin],angles:[0,yaws[slot],0],var_7cc697d0:slot}));altars=[{origin:[2559.5,-1224,-324.5]}];lookats={};
 for(let tier=1;tier<=3;tier++)lookats[tier]=[2,0,3,1].map(slot=>({entity:true,var_7cc697d0:slot,origin:[...looks[slot],heights[tier]]}));
 floorBad=undefined;sightBad=undefined;spawns=[];calls=[];registerSuccess=true;}
function near(a,b,label){check(a.length===b.length&&a.every((n,i)=>Math.abs(n-b[i])<1e-7),label);}
function playerRecord(slot){const tasks={},player={flags:{},_challenges:tasks,var_873a3e27:[]};for(let tier=1;tier<=3;tier++){player.flags['flag_player_completed_challenge_'+tier]=0;player.flags['flag_player_collected_reward_'+tier]=0;player.var_873a3e27[tier]=0;}return {player,record:{owner:player,epoch:7,tasks,trigger_active:false}};}
'''

CASES = r'''
reset();check(prepare_world_templates(),'measured geometry accepted');check(spawns.length===0,'early geometry creates no actors');check(level.eroded_island_trials_world.length===8,'eight independent frames');check(calls.filter(v=>v==='floor').length===4&&calls.filter(v=>v==='sight').length===4,'finite four floor/LOS validations');
for(let slot=0;slot<4;slot++){const f=level.eroded_island_trials_world[slot];near(f.origin,points[slot],'original trigger origin preserved');near(f.angles,[0,yaws[slot],0],'original trigger angles preserved');for(let tier=1;tier<=3;tier++)check(f.lookats[tier]===lookats[tier].find(v=>v.var_7cc697d0===slot),'actual original lookat entity preserved');}
const expected=[[2377.485,-1225.32,-367.5],[2742.63,-1223.095,-367.5],[2560.13,-1072.06,-367.5],[2559.985,-1376.355,-367.5]];
for(let slot=4;slot<8;slot++){const f=level.eroded_island_trials_world[slot];near(f.origin,expected[slot-4],'observed pair midpoint');const yaw=vectortoangles(vecsub(altars[0].origin,f.origin))[1];near(f.angles,[0,yaw,0],'faces existing altar');for(let tier=1;tier<=3;tier++){const delta=vecsub(lookats[tier].find(v=>v.var_7cc697d0===0).origin,points[0]);const radians=(yaw-yaws[0])*Math.PI/180;const ref=[f.origin[0]+delta[0]*Math.cos(radians)-delta[1]*Math.sin(radians),f.origin[1]+delta[0]*Math.sin(radians)+delta[1]*Math.cos(radians),f.origin[2]+delta[2]];near(f.lookats[tier].origin,ref,'actual source rotates source frame relative offset');check(!isentity(f.lookats[tier]),'extra lookat is private struct only');}}
check(!prepare_world_templates(),'no replacement of an existing owned world cache');
for(const mutate of [()=>owned=false,()=>triggers.pop(),()=>triggers[3].var_7cc697d0=0,()=>triggers[0].var_7cc697d0='0',()=>lookats[2].pop(),()=>lookats[1][0].var_7cc697d0=0,()=>altars=[],()=>altars[0].origin[0]=NaN,()=>lookats[1][0].origin=[9999,1,1],()=>triggers[0].angles[0]=45,()=>floorBad={fraction:NaN,position:points[0],normal:[0,0,1]},()=>floorBad={fraction:1,position:points[0],normal:[0,0,1]},()=>floorBad={fraction:.2,position:points[0],normal:[0,0,0]},()=>sightBad={fraction:.1},()=>sightBad={fraction:NaN}]){reset();mutate();check(!prepare_world_templates(),'invalid geometry/type/trace declines');check(level.eroded_island_trials_world===undefined&&spawns.length===0,'failed preflight publishes no cache/actors');}
for(const point of [undefined,[1,2],{},'123',[1,'2',3],[NaN,0,0],[Infinity,0,0],[0,0,40000]])check(!bounded_world_point(point),'unknown/nonvector/nonfinite/outside world position refused');
reset();check(prepare_world_templates(),'world prepared for occupancy');
for(let slot=0;slot<8;slot++){const {player,record}=playerRecord(slot);check(prepare_player_world(player,slot,record),'owned participant world constructed');check(spawns.length===0&&record.pillar===undefined,'all slots use private frames without server locators');check(record.stub.var_5fa98d68==='unitrigger_radius_use'&&record.stub.script_unitrigger_type===undefined,'canonical type alias only');check(record.stub.prompt_and_visibility_func===stock_function_3ae0d6d5,'exact retail prompt callback');for(let tier=1;tier<=3;tier++)check(record.stub.var_8e2d9e6f[tier]===tier,'independent real task indices');check(!record.trigger_active&&!calls.includes('register'),'world construction cannot activate prompt');check(!prepare_player_world(player,slot,record)&&spawns.length===0,'duplicate construct creates no actors');calls=[];check(activate_player_world(player,slot,record),'post-init activation succeeds');check(record.trigger_active&&calls.join(',')==='per_player,register','synchronous registration records active only after success');check(!activate_player_world(player,slot,record),'duplicate activation rejected');calls=[];}

for(const mutate of [v=>level.zones={},v=>level._unitriggers=undefined,v=>delete v.player.flags.flag_player_collected_reward_3,v=>delete v.player.var_873a3e27[2],v=>v.player._challenges={},v=>owned=false,v=>v.record.epoch=6]){reset();prepare_world_templates();const v=playerRecord(4);prepare_player_world(v.player,4,v.record);calls=[];mutate(v);check(!activate_player_world(v.player,4,v.record),'incomplete/foreign/deferred context never registers');check(!calls.includes('register')&&!v.record.trigger_active,'failed activation untouched');}
reset();prepare_world_templates();{const v=playerRecord(4);prepare_player_world(v.player,4,v.record);registerSuccess=false;check(!activate_player_world(v.player,4,v.record)&&!v.record.trigger_active,'only successful stock registered receipt marks active');}
console.log(JSON.stringify({passed:true,checks,scope:'Actual extracted source with vector/native stand-ins; no GSC VM/physics/client rendering proof.'}));
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--node', default=os.environ.get('NODE', 'node'))
    parser.add_argument('--output-dir', type=Path, default=ROOT / 'test-output')
    args = parser.parse_args()
    source = ROOT / 'scripts/zombies_coop/eroded_coop_island_trials.gsc'
    text = source.read_text(encoding='utf-8')
    code = re.sub(r'//[^\n]*|/\*[\s\S]*?\*/', '', text)
    if 'eroded_island_pillar' in code or 'packed_pillar' in code or re.search(r'\bspawn\s*\(', code):
        raise ValueError('No scriptmover allocation/transport or native locator actor allowed')
    if re.search(r'\.script_[A-Za-z_]+', code):
        raise ValueError('ACTS reserved script_* field emitted from text rather than canonical raw ID')
    bodies = {name: function(text, 'function ' + name + '(') for name in NAMES}
    for body in bodies.values():
        if re.search(r'\b(?:wait|waittill|thread|notify)\b', re.sub(r'//[^\n]*', '', body)):
            raise ValueError('World construction/activation must not yield or dispatch workers')
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    out = args.output_dir.resolve() / ('island-world-' + ROOT.name + '-' + stamp)
    out.mkdir(parents=True, exist_ok=False)
    js = out / 'actual-world.js'
    js.write_text(STANDINS + '\n' + '\n'.join(translate(body) for body in bodies.values()) + '\n' + CASES, encoding='utf-8')
    result = subprocess.run([args.node, str(js)], capture_output=True, text=True)
    (out / 'stdout.log').write_text(result.stdout, encoding='utf-8')
    (out / 'stderr.log').write_text(result.stderr, encoding='utf-8')
    if result.returncode:
        raise RuntimeError('Retained actual-world stand-in failure: ' + str(out))
    proof = {**json.loads(result.stdout), 'family': ROOT.name, 'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
             'test_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(), 'actual_functions': list(bodies),
             'generated_js_sha256': hashlib.sha256(js.read_bytes()).hexdigest()}
    path = out / 'RESULTS.private.json'
    path.write_text(json.dumps(proof, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(dict(proof, proof=str(path))))


if __name__ == '__main__':
    main()
