"""Actual GSC grenade-alias helpers plus explicit weapon/callback stand-ins.

No retail VM, native IsWeapon implementation, live loadout or gameplay proof.
The captured retail faults are two grenade-ammo queries, not target-yield sites.
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

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tests"))
from zombies_coop_actor_lifecycle import actor_translate, function, mask

HELPERS = ("bot_grenade_alias_action", "bridge_bot_grenade_aliases", "guarded_bot_threat_engage")
STANDINS = r'''
const assert=require('node:assert/strict');
let checks=0,level,events,calls,owner,receipts;
function check(v,m){++checks;assert.ok(v,m);}
function isdefined(v){return v!==undefined&&v!==null;}
function isint(v){return Number.isInteger(v);}
function isweapon(v){return Boolean(v&&v.weapon===true);}
function isentity(v){return Boolean(v&&v.entity===true);}
function isplayer(v){return Boolean(v&&v.player===true);}
function isalive(v){return isentity(v)&&v.alive!==false;}
function register_endon(){}
function diagnostic_event(v){events.push(v);}
function println(v){receipts.push(v);}
function bot_threat_guard_owned(){return owner;}
function weapon(id){return Object.freeze({weapon:true,id});}
const none=weapon('none'),frag=weapon('frag'),monkey=weapon('monkey'),other=weapon('other');
function original(){
 check(this===expectedSelf,'original receiver preserved');
 check(arguments.length===0,'original arity preserved');
 check(isweapon(this.grenadetypeprimary)&&isweapon(this.grenadetypesecondary),'original never sees invalid grenade args');
 ++calls;
}
let expectedSelf;
function reset(){level={weaponnone:none,eroded_coop_bot_threat_original:original};events=[];calls=0;owner=true;receipts=[];}
function actor(extra={}){return {entity:true,player:true,bot:{threat:{entity:{entity:true}}},...extra};}
function invoke(self){expectedSelf=self;guarded_bot_threat_engage.call(self);}
'''

CASES = r'''
// Exercise every pure ownership/weapon combination against an independent table.
const states=[undefined,1,2,0,true,'owned'];
const values=[undefined,null,none,frag,{},'frag',0,false,{entity:true}];
for(const state of states)for(const alias of values)for(const previous of [undefined,none,frag])for(const candidate of values){
 let expected;
 if(!isdefined(state))expected=isdefined(alias)?(isweapon(alias)?2:-2):(isweapon(candidate)?1:-1);
 else if(!isint(state)||state!==1||!isdefined(alias)||!isdefined(previous)||alias!==previous)expected=isweapon(alias)?2:-2;
 else expected=isweapon(candidate)?1:-1;
 check(bot_grenade_alias_action(state,alias,previous,candidate)===expected,'ownership matrix');
}
reset();let self=actor({current_lethal_grenade:frag,current_tactical_grenade:monkey});
invoke(self);check(calls===1,'missing MP aliases do not permanently disable stock combat');
check(self.grenadetypeprimary===frag&&self.grenadetypesecondary===monkey,'uses actual ZM weapons');
check(self.eroded_coop_primary_alias_state===1&&self.eroded_coop_secondary_alias_state===1,'owned only after bridge');
check(self.current_lethal_grenade===frag&&self.current_tactical_grenade===monkey,'real ZM loadout unchanged');
check(receipts.length===1&&receipts[0]==='[Zombies coop] bot_grenade_alias=zm_loadout ready=1','first actual alias adoption emits fixed receipt');
self.current_lethal_grenade=other;self.current_tactical_grenade=undefined;invoke(self);
check(calls===2&&self.grenadetypeprimary===other&&self.grenadetypesecondary===none,'owned aliases refresh with actual loadout and valid none');
check(receipts.length===1,'loadout refresh emits no continuous receipt');
for(const current of [undefined,null]){
 reset();self=actor({current_lethal_grenade:current,current_tactical_grenade:current});invoke(self);
 check(calls===1&&self.grenadetypeprimary===none&&self.grenadetypesecondary===none,'missing current slots use stock valid none fallback');
}
reset();self=actor({grenadetypeprimary:other,grenadetypesecondary:monkey,current_lethal_grenade:frag});invoke(self);
check(calls===1&&self.grenadetypeprimary===other&&self.grenadetypesecondary===monkey,'valid custom aliases preserved');
check(self.eroded_coop_primary_alias_state===2&&self.eroded_coop_secondary_alias_state===2,'initial foreign aliases latched');
check(receipts.length===0,'entirely foreign aliases do not claim bridge adoption');
self.current_lethal_grenade=monkey;self.current_tactical_grenade=frag;invoke(self);
check(calls===2&&self.grenadetypeprimary===other&&self.grenadetypesecondary===monkey,'custom aliases never refreshed by our ZM values');
reset();self=actor({current_lethal_grenade:frag});invoke(self);self.grenadetypeprimary=other;invoke(self);
check(calls===2&&self.grenadetypeprimary===other&&self.eroded_coop_primary_alias_state===2,'later custom takeover relinquishes ownership');
self.current_lethal_grenade=monkey;self.grenadetypeprimary=undefined;invoke(self);
check(calls===2&&!isdefined(self.grenadetypeprimary),'foreign owner removal never silently reacquired');
self.grenadetypeprimary=frag;invoke(self);check(calls===3&&self.grenadetypeprimary===frag,'foreign owner repair resumes combat');
for(const key of ['grenadetypeprimary','grenadetypesecondary'])for(const invalid of [{},'frag',1,true,{entity:true}]){
 reset();self=actor({current_lethal_grenade:frag,[key]:invalid});invoke(self);
 check(calls===0&&self[key]===invalid,'invalid custom alias never overwritten or delegated');
 check(receipts.length===0,'failed bridge emits no successful adoption');
 self[key]=monkey;invoke(self);check(calls===1&&self[key]===monkey,'custom repair recovers without reinstall');
}
for(const key of ['current_lethal_grenade','current_tactical_grenade'])for(const invalid of [{},'frag',1,true,{entity:true}]){
 reset();self=actor({[key]:invalid});invoke(self);check(calls===0,'invalid current weapon not converted to guessed asset');
 check(!isdefined(self.grenadetypeprimary)&&!isdefined(self.grenadetypesecondary),'candidate failure makes no partial alias assignment');
 self[key]=frag;invoke(self);check(calls===1,'later valid ZM loadout recovers automatically');
}
for(const invalidNone of [undefined,null,{},'none',0,false]){
 reset();level.weaponnone=invalidNone;self=actor();invoke(self);check(calls===0,'invalid or not-yet-created none never passed to native ammo');
 level.weaponnone=none;invoke(self);check(calls===1,'stock none readiness permits later retry');
}
reset();delete level.weaponnone;self=actor({grenadetypeprimary:frag,grenadetypesecondary:monkey});invoke(self);
check(calls===1,'fully custom valid aliases do not depend on unavailable stock fallback');
reset();self=actor({current_lethal_grenade:frag});owner=false;invoke(self);
check(calls===0&&!isdefined(self.grenadetypeprimary),'lost callback owner does not mutate aliases');
for(const extra of [{alive:false},{player_disconnected:1},{bot:undefined},{bot:{threat:{}}}]){
 reset();self=actor(extra);invoke(self);check(calls===0&&!isdefined(self.grenadetypeprimary),'invalid actor/target stays untouched');
}
console.log(JSON.stringify({passed:true,checks,scope:'Actual GSC alias/ownership/wrapper bodies with weapon/callback stand-ins; no native game or real loadout test.'}));
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--node", default=os.environ.get("NODE", "node"))
    parser.add_argument("--output-dir", type=Path, default=ROOT / "test-output")
    args = parser.parse_args()
    source = ROOT / "scripts/zombies_coop/eroded_coop.gsc"
    family = "cb"
    if not source.is_file():
        source = ROOT / "data/custom_scripts/zm/eroded_coop.gsc"
        family = "ezz"
    text = source.read_text(encoding="utf-8")
    bodies = {name: function(text, "function " + name + "(") for name in HELPERS}
    combined = mask("\n".join(bodies.values()))
    if re.search(r"\b(?:wait|waittill|thread|notify|giveweapon|takeweapon|removeweapon|switchtoweapon|setweaponammostock)\b", combined):
        raise ValueError("Grenade bridge gained yield/inventory writes")
    if 'getweapon(' in combined or 'getweaponammostock(' in combined:
        raise ValueError("Bridge must not create guessed weapons or probe native ammo")
    body = bodies["guarded_bot_threat_engage"]
    if body.index("self bridge_bot_grenade_aliases()") > body.index("self [[level.eroded_coop_bot_threat_original]]();"):
        raise ValueError("Alias validation must precede retail delegate")
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%S%fZ")
    out = args.output_dir.resolve() / ("bot-grenades-" + family + "-" + stamp)
    out.mkdir(parents=True, exist_ok=False)
    js = out / "actual-functions.js"
    js.write_text(STANDINS + "\n" + "\n".join(actor_translate(b) for b in bodies.values()) + "\n" + CASES, encoding="utf-8")
    run = subprocess.run([args.node, str(js)], capture_output=True, text=True)
    (out / "stdout.log").write_text(run.stdout, encoding="utf-8")
    (out / "stderr.log").write_text(run.stderr, encoding="utf-8")
    if run.returncode:
        raise RuntimeError("Retained actual-body failure: " + str(out))
    result = {**json.loads(run.stdout), "family": family,
              "source_sha256": hashlib.sha256(source.read_bytes()).hexdigest(),
              "test_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
              "generated_js_sha256": hashlib.sha256(js.read_bytes()).hexdigest(), "extracted_functions": list(bodies)}
    (out / "RESULTS.private.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
