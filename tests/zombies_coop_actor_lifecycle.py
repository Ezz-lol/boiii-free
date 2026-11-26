"""Actual GSC actor-wrapper checks with explicit callback/cancellation stand-ins.

This does not execute the GSC VM or retail bodies. Stock bot/threat containers
are assumed to be the spawnstruct values from the verified bot on-connect path;
defined-container checks do not validate arbitrary custom-mod field types.
"""
from pathlib import Path
import argparse,datetime,hashlib,json,os,re,subprocess,sys

sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tests'))
from zombies_coop_disconnect_guards import function,mask,translate,STANDINS

HELPERS=('bot_threat_guard_owned','bot_threat_guard_compatible','install_bot_threat_guard',
         'guarded_bot_threat_engage','fake_death_guard_owned','fake_death_guard_compatible',
         'install_fake_death_guard','guarded_player_fake_death','install_actor_lifecycle_guards',
         'actor_lifecycle_on_connect')

def actor_translate(body):
    for a,b in (('&bot_combat::engage_threat','stock_engage'),('&zm::player_fake_death','stock_fake'),
        ('&guarded_bot_threat_engage','guarded_bot_threat_engage'),('&guarded_player_fake_death','guarded_player_fake_death'),
        ('&actor_lifecycle_on_connect','actor_lifecycle_on_connect'),('callback::on_connect','callback_on_connect'),
        ('level thread actor_lifecycle_observer','launch_observer')):body=body.replace(a,b)
    body=re.sub(r'\b(self|level)\s+endon\(#"([a-z_]+)"\);',r'register_endon(\1,"\2");',body)
    return translate(body)

ACTOR_STANDINS=r'''
let thread,connectCallbacks=[],observerLaunches=[],resumes=[],receipts=[];
function println(message){receipts.push(message);}
function isalive(value){return isentity(value)&&value.alive!==false;}
function register_endon(receiver,event){thread.endons.push({receiver,event});}
function invoke(fn,self,...args){thread={endons:[],cancelled:false};fn.apply(self,args);return thread;}
function emit(t,receiver,event){if(t.endons.some(s=>s.receiver===receiver&&s.event===event))t.cancelled=true;}
function resume(t){if(!t.cancelled)calls.push({kind:'stock-after-wait',thread:t});}
function stock_engage(){
 check(arguments.length===0,'stock engage gets zero args');
 check(thread.endons.some(s=>s.receiver===this&&s.event==='death')&&thread.endons.some(s=>s.receiver===this&&s.event==='disconnect'),'self cancellation installed before stock combat');
 check(thread.endons.some(s=>s.receiver===level&&s.event==='game_ended')&&thread.endons.some(s=>s.receiver===level&&s.event==='end_game'),'level cancellation installed before stock combat');
 check(thread.endons.every(s=>s.receiver===this||s.receiver===level),'no cancellation bound to initial threat target');
 calls.push({kind:'engage',self:this,args:arguments.length});resumes.push(thread);
}
function stock_fake(){
 check(arguments.length===0,'stock fake death gets zero args');
 check(thread.endons.length===1&&thread.endons[0].receiver===this&&thread.endons[0].event==='disconnect','fake death only disconnect cancellation before original');
 calls.push({kind:'fake',self:this,args:arguments.length});resumes.push(thread);
}
function callback_on_connect(fn){connectCallbacks.push(fn);}
function launch_observer(epoch,map){observerLaunches.push({epoch,map});}
function actor(id,extra={}){return {entity:true,player:true,id,bot:{threat:{entity:{entity:true,id:'enemy'}}},...extra};}
function resetActor(){level={botthreatengage:stock_engage,eroded_coop_started_epoch:1,eroded_coop_started_map:'zm_zod'};currentEpoch=1;currentMap='zm_zod';calls=[];events=[];connectCallbacks=[];observerLaunches=[];resumes=[];receipts=[];}
function installBoth(){check(install_bot_threat_guard(1,'zm_zod')===true,'install exact stock engage');check(install_fake_death_guard(1,'zm_zod')===true,'install absent-default fake death');}
'''

CASES=r'''
resetActor();delete level.botthreatengage;
check(!install_bot_threat_guard(1,'zm_zod')&&bot_threat_guard_compatible(),'missing stock callback does not block human readiness');
level.botthreatengage=stock_engage;
check(install_bot_threat_guard(1,'zm_zod')&&bot_threat_guard_owned(),'late known registration installed');
resetActor();level.botthreatengage=foreign;
check(!install_bot_threat_guard(1,'zm_zod')&&level.botthreatengage===foreign&&bot_threat_guard_compatible(),'foreign bot owner preserved');
level.botthreatengage=stock_engage;check(!install_bot_threat_guard(1,'zm_zod')&&level.botthreatengage===stock_engage,'unknown owner decline latched');
for(const custom of [foreign,stock_fake]){
 resetActor();level.custom_player_fake_death=custom;
 check(!install_fake_death_guard(1,'zm_zod')&&level.custom_player_fake_death===custom&&fake_death_guard_compatible(),'defined custom fake death untouched');
 delete level.custom_player_fake_death;check(!install_fake_death_guard(1,'zm_zod')&&level.custom_player_fake_death===undefined,'foreign fake decline latched');
}
resetActor();install_actor_lifecycle_guards(1,'zm_zod');install_actor_lifecycle_guards(1,'zm_zod');
check(connectCallbacks.length===1&&observerLaunches.length===1,'one connect listener/optional observer');
check(bot_threat_guard_owned()&&fake_death_guard_owned(),'known defaults installed synchronously');
check(level.eroded_coop_bot_threat_original===stock_engage&&level.eroded_coop_fake_death_original===stock_fake,'saved exact originals');
check(install_bot_threat_guard(1,'zm_zod')&&install_fake_death_guard(1,'zm_zod'),'duplicate install no self-wrap');
check(receipts.length===2&&receipts.includes('[Zombies coop] actor_guard=bot_threat installed=1')&&receipts.includes('[Zombies coop] actor_guard=fake_death installed=1'),'exact once-per-map successful receipts survive duplicate attempts');
check(receipts.every(value=>value.length<80&&!value.includes('id=')&&!value.includes('name=')),'install receipts bounded and contain no player data');
resetActor();delete level.botthreatengage;install_actor_lifecycle_guards(1,'zm_zod');level.botthreatengage=stock_engage;
connectCallbacks[0]();check(bot_threat_guard_owned(),'connect listener can install late stock callback without fake players');

for(const [mutate,botOk,fakeOk]of [[()=>level.botthreatengage=foreign,false,true],
 [()=>level.custom_player_fake_death=foreign,true,false],[()=>level.eroded_coop_bot_threat_original=foreign,false,true],
 [()=>level.eroded_coop_fake_death_original=foreign,true,false],[()=>currentEpoch=2,false,false],[()=>currentMap='zm_tomb',false,false]]){
 resetActor();installBoth();mutate();
 check(bot_threat_guard_compatible()===botOk&&fake_death_guard_compatible()===fakeOk,'independent owner/context readiness compatibility');
 const botBefore=level.botthreatengage,fakeBefore=level.custom_player_fake_death;
 check(install_bot_threat_guard(1,'zm_zod')===botOk&&install_fake_death_guard(1,'zm_zod')===fakeOk,'lost owner never reacquired');
 const self=actor('still-owned');invoke(guarded_bot_threat_engage,self);invoke(guarded_player_fake_death,self,{},'death');
 check(calls.filter(c=>c.kind==='engage').length===(botOk?1:0)&&calls.filter(c=>c.kind==='fake').length===(fakeOk?1:0),'counterpart still delegates exact stock original');
 check(level.botthreatengage===botBefore&&level.custom_player_fake_death===fakeBefore,'takeover pointers left unchanged');
}
for(const self of [undefined,{}, {entity:true,player:false},actor('dead',{alive:false}),actor('disconnect',{player_disconnected:1}),
 actor('no-container',{bot:undefined}),actor('no-threat',{bot:{}}),actor('no-target',{bot:{threat:{}}}),
 actor('bad-target',{bot:{threat:{entity:{}}}}),actor('disconnected-target',{bot:{threat:{entity:actor('target',{player_disconnected:1})}}})]){
 resetActor();installBoth();invoke(guarded_bot_threat_engage,self);
 check(calls.length===0,'invalid/stale bot or target does not reach retail engage');
}
for(const receiver of ['self','level'])for(const event of receiver==='self'?['death','disconnect']:['game_ended','end_game']){
 resetActor();installBoth();const self=actor('live');const t=invoke(guarded_bot_threat_engage,self);
 check(calls.length===1&&calls[0].self===self,'same self retained for combat');
 emit(t,receiver==='self'?self:level,event);resume(t);
 check(calls.length===1&&t.cancelled,'registered cancellation blocks modeled suspended continuation');
}
resetActor();installBoth();const self=actor('normal'),t=invoke(guarded_bot_threat_engage,self);resume(t);
check(calls.length===2&&!t.cancelled,'ordinary stock combat continuation preserved');
for(const deathEvent of ['death','game_ended','end_game']){
 resetActor();installBoth();const self=actor('intentionally-dead',{alive:false});const t=invoke(guarded_player_fake_death,self,{direction:1},'MOD_MELEE');
 check(calls.length===1&&calls[0].kind==='fake'&&calls[0].self===self,'intentional fake-death path retains dead player and ignores hook args');
 emit(t,deathEvent==='death'?self:level,deathEvent);resume(t);
 check(calls.length===2&&!t.cancelled,'fake death keeps original death/endgame/intermission semantics');
}
resetActor();installBoth();const fakeSelf=actor('disconnect-during-wait');const fakeThread=invoke(guarded_player_fake_death,fakeSelf,{},'MOD_MELEE');
emit(fakeThread,fakeSelf,'disconnect');resume(fakeThread);check(calls.length===1&&fakeThread.cancelled,'disconnect prevents modeled stock one-second freeze continuation');
for(const self of [undefined,{},actor('gone',{player_disconnected:1})]){
 resetActor();installBoth();invoke(guarded_player_fake_death,self,{},'MOD_MELEE');check(calls.length===0,'invalid/disconnected fake-death actor skipped');
}
console.log(JSON.stringify({passed:true,checks,scope:'Mechanically translated actual GSC wrappers/ownership plus explicit cancellation stand-ins; no GSC VM, opcode attribution or retail runtime proof.'}));
'''

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--node',default=os.environ.get('NODE','node'))
    p.add_argument('--output-dir',type=Path,default=ROOT/'test-output');args=p.parse_args()
    source=ROOT/'scripts/zombies_coop/eroded_coop.gsc';family='cb'
    if not source.is_file():source=ROOT/'data/custom_scripts/zm/eroded_coop.gsc';family='ezz'
    text=source.read_text(encoding='utf-8');bodies={name:function(text,'function '+name+'(') for name in HELPERS}
    for name in ('guarded_bot_threat_engage','guarded_player_fake_death'):
        body=mask(bodies[name])
        if re.search(r'\b(?:wait|waittill|thread|notify)\b',body):raise ValueError('Wrapper must delegate synchronously without new yield/thread')
        if re.search(r'\b(?:isstruct|isvector|gettype)\s*\(',body):raise ValueError('Unverified type API')
    fake=bodies['guarded_player_fake_death']
    if 'isalive' in fake or 'endon(#"death")' in fake or 'level endon' in fake:raise ValueError('Fake death changes intentional death/endgame behavior')
    if 'self [[level.eroded_coop_fake_death_original]]();' not in fake:raise ValueError('Fake stock delegate arity/context changed')
    observer=function(text,'function actor_lifecycle_observer(')
    if 'gettime() - started < 30000' not in observer or 'wait 0.05;' not in observer or 'startup_current(epoch, map)' not in observer:raise ValueError('Optional observer unbounded')
    install=function(text,'function install(')
    waiting=install[install.index('while ('):install.index('expected_delay =')]
    if 'botthreatengage' in waiting or 'custom_player_fake_death' in waiting:raise ValueError('Optional callback registration blocks human startup')
    if '#using scripts/shared/bots/_bot_combat;' not in text:raise ValueError('Verified stock dependency absent')
    for name,assignment,receipt in (('install_bot_threat_guard','level.botthreatengage = &guarded_bot_threat_engage;',
            'println("[Zombies coop] actor_guard=bot_threat installed=1");'),
        ('install_fake_death_guard','level.custom_player_fake_death = &guarded_player_fake_death;',
            'println("[Zombies coop] actor_guard=fake_death installed=1");')):
        if bodies[name].count(receipt)!=1 or bodies[name].index(receipt)<bodies[name].index(assignment):
            raise ValueError('Unthrottled bounded success receipt must follow actual owner assignment')
    stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    out=args.output_dir.resolve()/('actor-lifecycle-'+family+'-'+stamp);out.mkdir(parents=True,exist_ok=False)
    js=out/'actual-functions.js';js.write_text(STANDINS+'\n'+ACTOR_STANDINS+'\n'+'\n'.join(actor_translate(body) for body in bodies.values())+'\n'+CASES,encoding='utf-8')
    result=subprocess.run([args.node,str(js)],capture_output=True,text=True)
    (out/'stdout.log').write_text(result.stdout,encoding='utf-8');(out/'stderr.log').write_text(result.stderr,encoding='utf-8')
    if result.returncode:raise RuntimeError('See retained actual-body failure: '+str(out))
    report={**json.loads(result.stdout),'family':family,'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),
        'test_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'generated_js_sha256':hashlib.sha256(js.read_bytes()).hexdigest(),
        'extracted_functions':list(bodies),'proof':str(out/'RESULTS.private.json')}
    (out/'RESULTS.private.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report,indent=2))

if __name__=='__main__':main()
