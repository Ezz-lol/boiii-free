"""Private actual-body ending lifecycle checks with explicit scheduler stand-ins."""
from pathlib import Path
import argparse
import datetime
import os
import hashlib
import json
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(Path(__file__).resolve().parent))
from script_frame_diagnostics import function

DEFAULT_SOURCE = ROOT / "scripts/zombies_coop/eroded_coop.gsc"
if not DEFAULT_SOURCE.is_file():
    DEFAULT_SOURCE = ROOT / "data/custom_scripts/zm/eroded_coop.gsc"
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
NAMES = ("main", "clear_readiness", "startup_identity_current", "ending_latched", "startup_current", "mark_ending",
         "watch_pre_end_game", "watch_direct_end_game", "official_map", "island_trials_current", "stalingrad_pools_current", "map_profile_current",
         "diagnostic_round_values", "install", "diagnostic_heartbeat")


def translate(body):
    body = body.replace("function autoexec main(", "function main(")
    masked = re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"', lambda m: " " * len(m[0]), body)
    parameters = set(filter(None, (value.strip() for value in re.search(r"function\s+\w+\(([^)]*)\)", body)[1].split(","))))
    variables = set(re.findall(r"(?<![\w.])([A-Za-z_]\w*)\s*=(?!=)", masked)) - parameters
    for old, new in (("&zm::get_zombie_spawn_delay", "stock_delay"), ("&zm::player_too_many_players_check", "stock_count"),
                     ("&player_count_check", "player_count_check"), ("&spawn_delay", "spawn_delay")):
        body = body.replace(old, new)
    body = body.replace("getplayers().size", "getplayers().length")
    body = body.replace("level thread watch_pre_end_game(epoch, map);", "launch_pre(epoch,map);")
    body = body.replace("level thread watch_direct_end_game(epoch, map);", "launch_end(epoch,map);")
    body = body.replace("level thread install(limit, epoch, map);", "launch_install(limit,epoch,map);")
    body = body.replace("level thread diagnostic_heartbeat(epoch, map);", "launch_heartbeat(epoch,map);")
    body = body.replace('level waittill(#"pre_end_game");', 'yield "pre_end_game";')
    body = body.replace('level waittill(#"end_game");', 'yield "end_game";')
    body = re.sub(r'level\s+endon\(#"([^"\r\n]+)"\);', r'record_endon("\1");', body)
    body = body.replace("wait 0.05;", "yield 0.05;").replace("wait 1;", "yield 1;")
    for name in ("watch_pre_end_game", "watch_direct_end_game", "install", "diagnostic_heartbeat"):
        body = body.replace("function " + name + "(", "function* " + name + "(")
    body = re.sub(r"\btrue\b", "1", re.sub(r"\bfalse\b", "0", body))
    opening = body.index("{") + 1
    if variables:
        body = body[:opening] + "\nlet " + ",".join(sorted(variables)) + ";\n" + body[opening:]
    return body


STANDINS = r'''
"use strict";
const assert=require('node:assert/strict');let checks=0;
function check(v,m){checks++;assert.ok(v,m);}
function isdefined(v){return v!==undefined&&v!==null;}
function isint(v){return Number.isInteger(v)&&v>=-2147483648&&v<=2147483647;}
function isstring(v){return typeof v==='string';}
function isarray(v){return Array.isArray(v);}
let level={},dvars={},writes=[],order=[],pre=[],end=[],jobs=[],beats=[],ended=[],messages=[],now=0,owns=true,injectEnding=false;
function stock_delay(){}function stock_count(){}function player_count_check(){}function spawn_delay(){}
function getdvarint(k){return dvars[k]??0;}function getdvarstring(k){return dvars[k]??'';}
function setdvar(k,v){dvars[k]=v;writes.push([k,v]);}
function gettime(){return now;}function getplayers(){return [];}
function println(s){messages.push(s);}function diagnostic_event(s){messages.push(s);}
function record_endon(s){ended.push(s);}function owns_callbacks(){return owns;}
function disconnect_endgame_pending(){return false;}
function disconnect_guard_installable(){return true;}function install_round_seed_hook(){return true;}
function install_disconnect_guards(){return true;}function install_actor_lifecycle_guards(){}
function install_bgb_movement_hook(){}function install_character_hook(){}
function install_spawn_hook(){if(injectEnding)mark_ending(17,'zm_castle');}
function launch_pre(epoch,map){order.push('pre');const g=watch_pre_end_game(epoch,map);assert.equal(g.next().value,'pre_end_game');pre.push(g);}
function launch_end(epoch,map){order.push('end');const g=watch_direct_end_game(epoch,map);assert.equal(g.next().value,'end_game');end.push(g);}
function launch_install(limit,epoch,map){order.push('install');jobs.push(install(limit,epoch,map));}
function launch_heartbeat(epoch,map){order.push('heartbeat');beats.push(diagnostic_heartbeat(epoch,map));}
function reset(capacity=8,epoch=17,map='zm_castle'){
 level={player_too_many_players_check_func:stock_count,func_get_zombie_spawn_delay:stock_delay,
  _notetrack_handler:{zombie_window_melee(){}},scr_zm_ui_gametype:'zclassic',
  eroded_coop_profile_map:map,eroded_coop_profile_epoch:epoch};
 dvars={zm_coopAppliedLimit:capacity,zm_coopStartupEpoch:epoch,mapname:map,zm_coopReadyEpoch:epoch,
  zm_coopReadyMap:map,zm_coopHeartbeat:10,zm_coopStatus:'experimental_callbacks_installed'};
 writes=[];order=[];pre=[];end=[];jobs=[];beats=[];ended=[];messages=[];now=0;owns=true;injectEnding=false;
}
function started(status='experimental_callbacks_installed'){
 reset();level.eroded_coop_started_epoch=17;level.eroded_coop_started_map='zm_castle';dvars.zm_coopStatus=status;
}
function terminalWrites(){return [['zm_coopEndingRevision',0],['zm_coopEndingMap','zm_castle'],['zm_coopEndingEpoch',17],
 ['zm_coopEndingRevision',1],['zm_coopStatus','ending'],['zm_coopReadyEpoch',0],['zm_coopReadyMap',''],['zm_coopHeartbeat',0]];}
'''

CASES = r'''
for(const status of ['initializing','experimental_callbacks_installed','ending']){
 started(status);mark_ending(17,'zm_castle');
 check(JSON.stringify(writes)===JSON.stringify(terminalWrites()),'complete ordered terminal tuple before readiness withdrawal');
 check(ending_latched(17,'zm_castle')&&startup_identity_current(17,'zm_castle')&&!startup_current(17,'zm_castle'),'terminal latch closes ordinary workers without losing observer identity');
 const before=JSON.stringify([dvars,level,writes]);mark_ending(17,'zm_castle');
 check(JSON.stringify([dvars,level,writes])===before,'repeat/pre+direct event idempotent one shot');
}
for(const status of ['custom_spawn_delay_preserved','custom_player_check_preserved','callback_ownership_lost',
 'custom_total_setup_preserved','declined_score_event_owner','declined_castle_dialogue_owner','initialization_timeout','disabled','',undefined,{},1]){
 started();dvars.zm_coopStatus=status;const before=JSON.stringify([dvars,level]);mark_ending(17,'zm_castle');
 check(JSON.stringify([dvars,level])===before&&writes.length===0,'unknown/declined status preserved rather than benign ending');
}
for(const change of [()=>dvars.zm_coopStartupEpoch=18,()=>dvars.mapname='zm_tomb',()=>delete level.eroded_coop_started_epoch,
 ()=>level.eroded_coop_started_epoch=18,()=>level.eroded_coop_started_epoch='17',()=>delete level.eroded_coop_started_map,
 ()=>level.eroded_coop_started_map='zm_tomb',()=>level.eroded_coop_started_map={}]){
 started();change();const before=JSON.stringify([level,dvars]);mark_ending(17,'zm_castle');
 check(JSON.stringify([level,dvars])===before&&writes.length===0,'stale/unbound observer cannot write current VM state');
}
for(const epoch of [0,-1,18,'17',true,undefined,2147483648]){
 started();mark_ending(epoch,'zm_castle');check(writes.length===0,'bad identity epoch does not publish');
}
for(const map of ['',undefined,{},17,'zm_tomb']){
 started();mark_ending(17,map);check(writes.length===0,'bad identity map does not publish');
}
for(const name of ['pre','end']){
 reset();main();check(order.join(',')==='pre,end,install'&&pre.length===1&&end.length===1&&jobs.length===1,'both observers registered before installer starts');
 writes=[];(name==='pre'?pre[0]:end[0]).next();check(JSON.stringify(writes)===JSON.stringify(terminalWrites()),'actual single event observer marks initializing startup');
 const before=JSON.stringify([writes,dvars]);(name==='pre'?end[0]:pre[0]).next();
 check(JSON.stringify([writes,dvars])===before,'other actual observer does not republish tuple');
 check(jobs[0].next().done&&beats.length===0&&dvars.zm_coopReadyEpoch===0,'installer cannot publish ready after pre/direct terminal event');
 const duplicateBefore=JSON.stringify([writes,order,dvars]);main();
 check(JSON.stringify([writes,order,dvars])===duplicateBefore,'duplicate autoexec never clears terminal receipt or creates new watchers');
}
reset();main();delete level._notetrack_handler;const loading=jobs[0];
check(loading.next().value===.05,'actual installer yields while known stock setup is pending');
pre[0].next();level._notetrack_handler={zombie_window_melee(){}};writes=[];
check(loading.next().done&&writes.length===0&&beats.length===0,'waiting installer stops after terminal event before finish');
reset();main();injectEnding=true;check(jobs[0].next().done&&dvars.zm_coopStatus==='ending'&&dvars.zm_coopReadyEpoch===0&&beats.length===0,'final guard prevents ready publication after helper-time terminal transition');
reset();main();check(jobs[0].next().done&&dvars.zm_coopReadyEpoch===17&&dvars.zm_coopReadyMap==='zm_castle'&&dvars.zm_coopStatus==='experimental_callbacks_installed'&&beats.length===1,'ordinary startup retains readiness behavior');
check(beats[0].next().value===1,'ordinary heartbeat can publish before ending');end[0].next();writes=[];
check(beats[0].next().done&&writes.length===0&&dvars.zm_coopHeartbeat===0,'heartbeat cannot overwrite terminal clear on resume');

// Same map with a NEW epoch: an old still-referenced observer remains inert.
reset();main();const oldPre=pre[0],oldEnd=end[0];dvars.zm_coopStartupEpoch=18;
level.eroded_coop_started_epoch=18;level.eroded_coop_started_map='zm_castle';
dvars.zm_coopStatus='initializing';dvars.zm_coopEndingRevision=0;writes=[];
oldPre.next();oldEnd.next();check(writes.length===0&&dvars.zm_coopStatus==='initializing','old same-map VM observers cannot poison newer epoch');
for(let cap=1;cap<=4;cap++){
 reset(cap);main();check(order.length===0&&jobs.length===0&&dvars.zm_coopStatus==='disabled','stock capacities do not create ending observers');
 check(!writes.some(([name])=>name.startsWith('zm_coopEnding')),'stock mode adds no ending receipt writes');
}
for(const cap of [0,9]){reset(cap);main();check(order.length===0&&dvars.zm_coopStatus==='invalid_applied_limit','invalid capacity retains original decline');}
reset(8,0);main();check(order.length===0&&dvars.zm_coopStatus==='invalid_startup_epoch','invalid startup retains original decline');
reset();level.eroded_coop_started_epoch=17;level.eroded_coop_started_map='zm_castle';
const duplicate=JSON.stringify([level,dvars]);main();check(writes.length===0&&order.length===0&&JSON.stringify([level,dvars])===duplicate,'preexisting duplicate installer keeps all prior readiness/heartbeat');

// Actual installer cannot publish through a missing or withdrawn map receipt.
function seed_stalingrad_receipt(){
 dvars.zm_coopSchemaSlots=8;
 Object.assign(level,{eroded_stalingrad_pool_revision:1,eroded_stalingrad_pool_map:'zm_stalingrad',
  eroded_stalingrad_pool_epoch:17,eroded_stalingrad_pool_capacity:8,eroded_stalingrad_pool_catalog_size:8,
  eroded_stalingrad_pool_tick:now,eroded_stalingrad_pool_ready:1});
}
reset(8,17,'zm_stalingrad');main();
check(jobs[0].next().value===.05&&beats.length===0&&dvars.zm_coopReadyEpoch===0,'missing Stalingrad catalog keeps actual installer pending');
seed_stalingrad_receipt();check(jobs[0].next().done&&beats.length===1&&dvars.zm_coopReadyEpoch===17,'fresh Stalingrad catalog releases installer');
const originalSpawn=install_spawn_hook;
reset(8,17,'zm_stalingrad');main();seed_stalingrad_receipt();
install_spawn_hook=function(){level.eroded_stalingrad_pool_ready=0;};
check(jobs[0].next().done&&beats.length===0&&dvars.zm_coopReadyEpoch===0,'helper-time pool withdrawal stops ready publication');
install_spawn_hook=originalSpawn;
const originalHeartbeat=launch_heartbeat;
reset(8,17,'zm_stalingrad');main();seed_stalingrad_receipt();
launch_heartbeat=function(epoch,map){originalHeartbeat(epoch,map);level.eroded_stalingrad_pool_ready=0;};
check(jobs[0].next().done&&beats.length===1&&dvars.zm_coopReadyEpoch===0,'post-heartbeat withdrawal cannot be revived by installer');
launch_heartbeat=originalHeartbeat;

console.log(JSON.stringify({passed:true,checks,scope:'Mechanically translated actual private GSC lifecycle bodies with explicit coroutine/engine stand-ins. No engine scheduling, native ending admission, map-end or crash proof.'}));
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--output-dir", type=Path, default=ROOT / "test-output")
    parser.add_argument("--node", default=os.environ.get("NODE", "node"))
    args = parser.parse_args()
    PROPOSAL = args.source.resolve(strict=True)
    text = PROPOSAL.read_text(encoding="utf-8")
    normalized = text.replace("function autoexec main(", "function main(")
    bodies = {name: function(normalized, "function " + name + "(") for name in NAMES}
    mark = bodies["mark_ending"]
    executable = re.sub(r"//[^\n]*|/\*[\s\S]*?\*/", "", mark)
    assert not re.search(r"\b(?:wait|waittill|thread|notify|endon|freeze_players)\b", executable)
    assert not re.search(r"\b(?:flag|setclient|godmode|invulnerable|ai_ignore)\b", executable)
    assert re.findall(r'setdvar\("([^"\r\n]+)"', mark) == ["zm_coopEndingRevision", "zm_coopEndingMap", "zm_coopEndingEpoch", "zm_coopEndingRevision", "zm_coopStatus"]
    assert mark.index('setdvar("zm_coopEndingRevision", 1)') < mark.index('setdvar("zm_coopStatus", "ending")') < mark.index("clear_readiness();")
    for name, event in (("watch_pre_end_game", "pre_end_game"), ("watch_direct_end_game", "end_game")):
        body = re.sub(r"//[^\n]*", "", bodies[name])
        assert body.count('level waittill(#"' + event + '");') == 1 and "endon" not in body
        assert body.index("waittill") < body.index("mark_ending(epoch, map)")
    main_body = bodies["main"]
    assert main_body.index("level thread watch_pre_end_game") < main_body.index("level thread watch_direct_end_game") < main_body.index("level thread install")
    assert main_body.index("if (limit <= 4)") < main_body.index('setdvar("zm_coopEndingRevision"')
    install = bodies["install"]
    assert install.index("if (!startup_current(epoch, map) || !stalingrad_pools_current(epoch, map, limit)) return;") < install.index("level thread diagnostic_heartbeat") < install.index('setdvar("zm_coopReadyMap", map)')
    heartbeat = bodies["diagnostic_heartbeat"]
    assert heartbeat.index("if (!startup_current(epoch, map))") < heartbeat.index('setdvar("zm_coopHeartbeat"')
    output = args.output_dir.resolve() / ("ending-gsc-" + datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%S%fZ"))
    output.mkdir(parents=True, exist_ok=False)
    js = output / "actual-functions.cjs"
    js.write_text(STANDINS + "\n" + "\n\n".join(translate(body) for body in bodies.values()) + "\n" + CASES, encoding="utf-8")
    result = subprocess.run([args.node, str(js)], text=True, capture_output=True, timeout=60)
    (output / "stdout.log").write_text(result.stdout, encoding="utf-8")
    (output / "stderr.log").write_text(result.stderr, encoding="utf-8")
    if result.returncode:
        raise RuntimeError("Retained actual-body failure: " + str(output))
    report = json.loads(result.stdout)
    report.update(proposal=str(PROPOSAL), proposal_sha256=sha(PROPOSAL), test_sha256=sha(Path(__file__)),
                  generated_js_sha256=sha(js), functions=list(NAMES), no_game_or_native_proof=True)
    path = output / "RESULTS.private.json"
    path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"passed": True, "checks": report["checks"], "proof": str(path)}))


if __name__ == "__main__":
    main()
