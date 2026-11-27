"""Actual ending observer/readiness/main-poll bodies under native state stand-ins.

This checks local lifecycle control only. It does not run the game or establish
native event scheduling, client gameplay, HUD cleanup or a crash cause.
"""
from pathlib import Path
import argparse,hashlib,json,re,subprocess
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--compiler',required=True)
parser.add_argument('--output',type=Path,required=True)
args=parser.parse_args();root=Path(__file__).resolve().parents[1]
coop_path=root/'src/client/component/zombies_coop.cpp'
companion_path=root/'src/client/component/zombies_companion.cpp'
coop=coop_path.read_text();companion=companion_path.read_text()
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
inputs={str(p):sha(p)for p in(coop_path,companion_path)}

def function_text(text, pattern):
    matches=list(re.finditer(pattern,text))
    assert len(matches)==1, (pattern,len(matches))
    start=matches[0].start();pos=matches[0].end();depth=1
    while pos<len(text):
        if text.startswith('//',pos):
            end=text.find('\n',pos);pos=len(text) if end<0 else end+1;continue
        if text.startswith('/*',pos):
            end=text.find('*/',pos+2);assert end>=0;pos=end+2;continue
        if text[pos] in ('"',"'"):
            quote=text[pos];pos+=1
            while pos<len(text):
                if text[pos]=='\\':pos+=2;continue
                if text[pos]==quote:pos+=1;break
                pos+=1
            continue
        if text[pos]=='{':depth+=1
        elif text[pos]=='}':
            depth-=1
            if depth==0:return text[start:pos+1]
        pos+=1
    raise AssertionError('unclosed actual C++ body')
prelude=r'''#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include "PRIVATE_HEADERS/ending_policy.hpp"
#include "PRIVATE_HEADERS/startup_gate.hpp"
#include "PRIVATE_HEADERS/coop_runtime.hpp"
#include "PRIVATE_HEADERS/companion.hpp"
namespace mock {bool failed{},cache{true},ui{},running{true},zombies{true};int capacity{8},events{};std::string frozen="zm_zod";std::map<std::string,int> ints;std::map<std::string,std::string> strings;std::function<void()> on_ending_map;std::function<void()> on_second_ending_revision;int ending_revision_reads{};bool throw_read{};}
namespace game {
enum class eModes{MP,ZOMBIES};inline constexpr auto MODE_ZOMBIES=eModes::ZOMBIES;
bool Com_IsRunningUILevel(){return mock::ui;}eModes Com_SessionMode_GetMode(){return mock::zombies?eModes::ZOMBIES:eModes::MP;}
namespace com {bool Com_IsRunningUILevel(){return game::Com_IsRunningUILevel();}eModes Com_SessionMode_GetMode(){return game::Com_SessionMode_GetMode();}}
bool is_server_running(){return mock::running;}bool server_running(){return mock::running;}
GAME_READERS
}
namespace zombies::coop {
namespace schema_assets {bool score_producer_ready() noexcept{return mock::cache;}}
int frozen_schema_capacity() noexcept{return mock::capacity;}
std::string frozen_schema_map(){return mock::frozen;}
bool schema_assets_failed() noexcept{return mock::failed;}
void diagnostic_event(std::string_view,std::int64_t=0,std::int64_t=0) noexcept{++mock::events;}
namespace {
GLOBALS
int read_int(const char* name){if(std::string_view(name)=="zm_coopEndingRevision"&&++mock::ending_revision_reads==2&&mock::on_second_ending_revision){auto callback=std::move(mock::on_second_ending_revision);mock::on_second_ending_revision={};callback();}if(mock::throw_read)throw std::runtime_error("marker read");return mock::ints[name];}
std::string read_string(const char* name){if(mock::throw_read)throw std::runtime_error("marker read");if(std::string_view(name)=="zm_coopEndingMap"&&mock::on_ending_map)mock::on_ending_map();return mock::strings[name];}
int expected_mask(std::string_view){return 3;}
bool zombies_mode(){return mock::zombies;}
}
HELPERS
ENDING
READY
namespace {
POLL
}
}
'''
tests=r'''
unsigned checks{};void check(bool ok){++checks;assert(ok);}
int main(){using namespace zombies::coop;
 const auto reset=[&](bool ready=true){mock::on_ending_map={};mock::on_second_ending_revision={};mock::ending_revision_reads=0;mock::throw_read=false;mock::failed=false;mock::cache=true;mock::ui=false;mock::running=true;mock::zombies=true;mock::capacity=8;mock::frozen="zm_zod";mock::events=0;mock::ints.clear();mock::strings.clear();epoch=11;applied_limit=8;reported=false;map_armed=true;active_map="zm_zod";blocked=false;next_poll={};gate.arm(epoch,8);admission_state.arm(epoch);
 mock::ints["zm_coopSchemaServerRevision"]=companion_revision;mock::ints["zm_coopSchemaServerMask"]=3;mock::ints["zm_coopHeartbeat"]=1;mock::ints["zm_coopReadyEpoch"]=ready?epoch:0;mock::strings["zm_coopReadyMap"]=ready?"zm_zod":"";mock::strings["mapname"]="zm_zod";mock::strings["zm_coopSchemaServerContract"]=std::string(companion_contract);mock::strings["zm_coopStatus"]=ready?"experimental_callbacks_installed":"initializing";};
 const auto packet=[&]{mock::ints["zm_coopEndingRevision"]=1;mock::ints["zm_coopEndingEpoch"]=epoch;mock::strings["zm_coopEndingMap"]="zm_zod";mock::strings["zm_coopStatus"]="ending";};
 reset();check(schema_server_ready());check(poll_startup()==nullptr&&gate.ready()&&admission_ready());packet();next_poll=std::chrono::steady_clock::now()+std::chrono::hours(1);check(!schema_server_ready()&&session_ending()&&!admission_ready());check(poll_startup()==nullptr&&gate.ending()&&!gate.ready()&&!reported);check(mock::events==1);check(poll_startup()==nullptr&&mock::events==1);
 mock::ints["zm_coopEndingRevision"]=0;mock::ints["zm_coopEndingEpoch"]=0;mock::strings["zm_coopEndingMap"]="";mock::strings["zm_coopStatus"]="experimental_callbacks_installed";check(!schema_server_ready()&&!admission_ready());check(poll_startup()==nullptr&&!reported);
 reset();check(poll_startup()==nullptr&&gate.ready());mock::ending_revision_reads=0;mock::on_second_ending_revision=packet;next_poll={};std::cout<<"REGRESSION_CASE|ending_during_schema_sample\n"<<std::flush;check(poll_startup()==nullptr&&gate.ending()&&!reported&&!admission_ready());
 reset(false);packet();mock::ints["zm_coopReadyEpoch"]=0;mock::strings["zm_coopReadyMap"]="";check(ending_session_current()&&!schema_server_ready());check(poll_startup()==nullptr&&gate.ending()&&!reported);
 for(int rev:{0,2,4}){reset();packet();mock::ints["zm_coopEndingRevision"]=rev;check(!ending_session_current()&&!session_ending());check(!schema_server_ready());check(poll_startup()!=nullptr&&reported);}
 reset();packet();mock::ints["zm_coopEndingEpoch"]=epoch-1;mock::ints["zm_coopStartupEpoch"]=epoch-1;check(!ending_session_current()&&!session_ending()&&!schema_server_ready());check(poll_startup()!=nullptr);
 reset();packet();mock::strings["zm_coopEndingMap"]="zm_tomb";check(!ending_session_current()&&!session_ending()&&!schema_server_ready());check(poll_startup()!=nullptr);
 reset();packet();mock::strings["mapname"]="zm_tomb";check(!ending_session_current()&&!session_ending());reset();packet();mock::frozen="zm_tomb";check(!ending_session_current()&&!session_ending());
 for(auto status:{"callback_ownership_lost","custom_spawn_delay_preserved","custom_total_setup_preserved"}){reset();packet();mock::strings["zm_coopStatus"]=status;check(!ending_session_current()&&!session_ending()&&!schema_server_ready());check(poll_startup()!=nullptr&&reported);}
 reset();packet();mock::on_ending_map=[&]{admission_state.arm(epoch+1);};check(!ending_session_current()&&active_startup_epoch()==epoch+1&&!session_ending());mock::on_ending_map={};check(!mark_session_ending(epoch)&&!session_ending());
 reset();packet();check(mark_session_ending(epoch));mock::failed=true;check(!schema_server_ready());next_poll={};check(poll_startup()!=nullptr&&reported);
 reset();packet();mock::throw_read=true;check(!ending_session_current()&&!schema_server_ready()&&!session_ending());mock::throw_read=false;
 for(int limit=1;limit<=4;++limit){reset();mock::capacity=limit;applied_limit=limit;admission_state.arm(epoch,false);gate.arm(epoch,limit);packet();check(!ending_session_current()&&!session_ending());check(poll_startup()==nullptr&&admission_ready());}
 reset();packet();mock::zombies=false;check(!ending_session_current());check(poll_startup()==nullptr&&!session_ending()&&active_startup_epoch()==0&&admission_ready());reset();packet();mock::ui=true;check(poll_startup()==nullptr&&active_startup_epoch()==0&&admission_ready());
 reset();packet();check(ending_session_current());admission_state.reset();gate.reset();++epoch;admission_state.arm(epoch);gate.arm(epoch,8);mock::strings["zm_coopStatus"]="experimental_callbacks_installed";mock::ints["zm_coopReadyEpoch"]=epoch;next_poll={};check(!session_ending()&&!mark_session_ending(epoch-1));check(poll_startup()==nullptr&&admission_ready()&&gate.ready());
 std::cout<<"checks="<<checks<<" actual_native_bodies=true no_game_execution=true\n";
}
'''

start=re.search(r'\bstd::mutex\s+state_mutex\s*;',coop)
end=re.search(r'const\s+char\s*\*\s*preflight_error',coop)
assert start and end and start.end()<end.start()
globals_text=coop[start.start():end.start()]
helpers='\n'.join(function_text(coop,r'\b(?:int|bool)\s+'+name+r'\s*\([^)]*\)\s*noexcept\s*\{')for name in('active_startup_epoch','session_ending','mark_session_ending','admission_ready'))
ending=function_text(companion,r'\bbool\s+ending_session_current\s*\(\s*\)\s*noexcept\s*\{')
ready=function_text(companion,r'\bbool\s+schema_server_ready\s*\(\s*\)\s*noexcept\s*\{')
poll=function_text(coop,r'const\s+char\s*\*\s*poll_startup\s*\(\s*\)\s*\{')
ezz='#define BOIII_COOP_EZZ'in companion
readers='std::optional<int> get_dvar_int(const char*n){return mock::ints[n];}std::optional<std::string_view>get_dvar_string(const char*n){return mock::strings[n];}'if ezz else'int get_dvar_int(const char*n){return mock::ints[n];}std::string get_dvar_string(const char*n){return mock::strings[n];}'
prelude=prelude.replace('PRIVATE_HEADERS',(root/'src/client/component/zombies').as_posix()).replace('GAME_READERS',readers).replace('GLOBALS',globals_text).replace('HELPERS',helpers).replace('ENDING',ending).replace('READY',ready).replace('POLL',poll)
prelude='#include "'+(root/'src/client/component/zombies/coop_policy.hpp').as_posix()+'"\n'+prelude
out=args.output.resolve();out.mkdir(parents=True,exist_ok=False)
generated=out/'ending_actual.cpp';generated.write_text(prelude+'\n'+tests)
binary=out/'ending_actual.exe';command=[args.compiler,'/nologo','/std:c++20','/EHsc','/W4','/WX','/Fe:'+str(binary),'/Fo:'+str(binary.with_suffix('.obj')),str(generated)]
with(out/'compile.log').open('x',encoding='utf-8')as log:result=subprocess.run(command,cwd=out,stdout=log,stderr=subprocess.STDOUT)
assert result.returncode==0,out/'compile.log'
run=subprocess.run([str(binary)],cwd=out,capture_output=True,text=True,timeout=30)
(out/'stdout.log').write_text(run.stdout);(out/'stderr.log').write_text(run.stderr)
assert run.returncode==0,(out,run.stderr)
assert inputs=={str(p):sha(p)for p in(coop_path,companion_path)},'source changed during proof'
proof=dict(passed=True,scope='Actual new native ending/session/readiness/main-poll functions under state/API stand-ins; no game or crash-cause proof.',source_inputs=inputs,test_sha256=sha(Path(__file__)),generated_sha256=sha(generated),command=command,stdout=run.stdout,ezz_api_shape=ezz)
(out/'RESULTS.private.json').write_text(json.dumps(proof,indent=2)+'\n');print(json.dumps(proof))
