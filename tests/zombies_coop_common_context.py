"""Portable actual native provider/generation lifecycle regression.

Engine APIs are explicit stand-ins. No private files, game or network required.
"""
from pathlib import Path
import argparse,hashlib,json,os,shlex,subprocess,tempfile

PRELUDE='\n#include <algorithm>\n#include <cassert>\n#include <chrono>\n#include <cstdint>\n#include <iostream>\n#include <map>\n#include <mutex>\n#include <optional>\n#include <string>\n#include <string_view>\n#include "HEADERS/coop_policy.hpp"\n#include "HEADERS/startup_gate.hpp"\n#include "common_helper_native_context.hpp"\n#include "schema_asset_store.hpp"\nstatic unsigned checks=0;static void check(bool v){++checks;assert(v);}\nstatic bool in_store=false,ui=false,zm=true,prepare_ok=true,failed=false;static int cap=8;static std::string map="zm_zod";\nstatic std::map<std::string,int> ints;static std::map<std::string,std::string> strings;\nstatic void engine_guard(){check(!in_store);}\nnamespace game{\nenum class eModes{MP,ZOMBIES};inline constexpr auto MODE_ZOMBIES=eModes::ZOMBIES;\nstatic eModes Com_SessionMode_GetMode(){engine_guard();return zm?eModes::ZOMBIES:eModes::MP;}\nstatic bool Com_IsRunningUILevel(){engine_guard();return ui;}\nnamespace com{inline auto Com_SessionMode_GetMode(){return game::Com_SessionMode_GetMode();}inline bool Com_IsRunningUILevel(){return game::Com_IsRunningUILevel();}}\nstatic bool is_client(){return false;}inline bool is_server_running(){return true;}inline bool server_running(){return true;}\nREADERS\n// The real startup helper resolves and type-checks the integer receipt dvar.\nenum class dvarType_t { INT };\nstruct mock_dvar {\n const char* name{};\n explicit operator bool() const{return name!=nullptr;}\n mock_dvar resolve() const{engine_guard();return *this;}\n dvarType_t type() const{engine_guard();return dvarType_t::INT;}\n int get_int() const{engine_guard();return ints[name];}\n};\nstatic mock_dvar get_dvar(const char*n){engine_guard();return {std::string_view(n)=="zm_coopTemplePapEpoch"?n:nullptr};}\nstatic void Dvar_SetFromStringByName(const char*n,const char*v,bool){engine_guard();strings[n]=v;try{ints[n]=std::stoi(v);}catch(...){}}\ninline void set_dvar_int(const char*n,int v,bool){engine_guard();ints[n]=v;strings[n]=std::to_string(v);}\n}\nnamespace utils::flags{static bool has_flag(const char*){return true;}}\nnamespace zombies::coop{\nnamespace{std::mutex state_mutex;startup_gate gate;std::atomic_bool blocked{false};ending_policy::admission_lifecycle admission_state;int epoch=0,applied_limit=0;bool reported=false,map_armed=false;std::string active_map;std::chrono::steady_clock::time_point next_poll{};common_helper::native_publication common_native_context;}\nstatic bool client_lobby_hooks_ready(){return true;}\nstatic int frozen_schema_capacity(){return cap;}\nstatic bool schema_assets_failed(){return failed;}\nstatic bool prepare_schema_session(int c,const std::string&,std::string&){engine_guard();cap=c;return prepare_ok;}\nstatic void schema_asset_failure(const std::string&){failed=true;}\nstatic void schema_begin_server_vm(){engine_guard();}\nstatic void reset_schema_session(){cap=4;failed=false;}\nstatic bool ending_session_current(){return admission_state.ending();}\nstatic bool schema_server_ready(){return true;}\nstatic void diagnostic_event(std::string_view,int=0){}\nPREFLIGHT\nHELPERS\nnamespace common_helper {NATIVE_WRAPPERS}\n\nADMISSION\nBEGIN\nSHUTDOWN\nnamespace{POLL}\nGENERATION\n}\n'
TESTS='\nstruct Asset{const char*name{};int len{};std::uint8_t*buffer{};};\nint main(){using namespace zombies::coop;using Store=t7::schema_asset_store<Asset>;Store store;\n const auto reset=[](){ui=false;zm=true;prepare_ok=true;failed=false;cap=8;applied_limit=0;epoch=0;reported=false;map_armed=false;active_map.clear();gate.reset();admission_state.reset();blocked=false;next_poll={};ints.clear();strings.clear();strings["mapname"]=map;ints["zm_coopPlayerLimit"]=8;ints["com_maxclients"]=8;common_native_context.invalidate();};\n const auto atomic_current=[&](auto ticket){return common_helper::native_session_current(ticket);};\n for(int capacity=1;capacity<=8;++capacity){reset();ints["zm_coopPlayerLimit"]=capacity;ints["com_maxclients"]=capacity;check(begin_startup()==nullptr);auto ticket=common_native_context.capture();check(bool(ticket)==(capacity>=5));if(capacity<=4)check(admission_ready());\n  if(capacity>=5){check(ticket->epoch==epoch&&ticket->capacity==capacity&&ticket->map==map);check(strings["zm_coopReadyEpoch"]=="0"&&strings["zm_coopReadyMap"].empty()&&strings["zm_coopEndingRevision"]=="0");\n   check(common_native_context.sampled(ticket,true,false,map,map,active_startup_epoch(),cap,false,false));\n   auto lease=store.capture_generation("scripts/zm/_zm_powerups.gsc");bool saw=false;check(store.if_current(lease,[&]{in_store=true;std::lock_guard lock(state_mutex);saw=atomic_current(ticket);in_store=false;}));check(saw);\n   check(!common_native_context.sampled(ticket,false,false,map,map,epoch,cap,false,false));check(!common_native_context.sampled(ticket,true,true,map,map,epoch,cap,false,false));check(!common_native_context.sampled(ticket,true,false,"zm_tomb",map,epoch,cap,false,false));check(!common_native_context.sampled(ticket,true,false,map,"zm_tomb",epoch,cap,false,false));\n   check(!common_native_context.current(ticket,epoch+1,cap,false,false));check(!common_native_context.current(ticket,epoch,cap-1,false,false));check(!common_native_context.current(ticket,epoch,cap,true,false));check(!common_native_context.current(ticket,epoch,cap,false,true));\n   blocked=false;check(admission_ready());check(mark_session_ending(epoch));check(atomic_current(ticket)&&!admission_ready());admission_state.arm(epoch);check(atomic_current(ticket)&&admission_ready());\n   check(common_helper::revoke_native_session(ticket));check(!atomic_current(ticket)&&!admission_ready()&&common_helper::native_session_owned(ticket));\n   shutdown_map();check(!atomic_current(ticket)&&!common_native_context.capture());\n  }\n }\n reset();check(begin_startup()==nullptr);auto old=common_native_context.capture();check(begin_startup()==nullptr);check(!atomic_current(old)&&atomic_current(common_native_context.capture()));\n // Same reported native epoch/map/cap still has a different immutable ticket.\n auto same=common_native_context.capture();check(common_native_context.publish(same->epoch,same->capacity,same->map));check(!atomic_current(same));check(!common_helper::revoke_native_session(same)&&atomic_current(common_native_context.capture()));\n {auto live=common_native_context.capture();auto live_lease=store.capture_generation("scripts/zm/_zm_powerups.gsc");check(store.if_current(live_lease,[&]{in_store=true;std::lock_guard lock(state_mutex);check(common_helper::revoke_native_session(live));check(!common_helper::native_admission_current());in_store=false;}));}\n for(int kind=0;kind<4;++kind){reset();check(begin_startup()==nullptr);auto ticket=common_native_context.capture();if(kind==0)zm=false;if(kind==1)ints["zm_coopPlayerLimit"]=99;if(kind==2)prepare_ok=false;if(kind==3)strings["mapname"]="invalid map";const auto result=begin_startup();check(!common_native_context.capture()&&!atomic_current(ticket));check((result==nullptr)==(kind==0));}\n for(int frontend=0;frontend<2;++frontend){reset();check(begin_startup()==nullptr);auto ticket=common_native_context.capture();if(frontend)ui=true;else zm=false;check(poll_startup()==nullptr);check(!common_native_context.capture()&&!atomic_current(ticket));}\n reset();check(begin_startup()==nullptr);auto ticket=common_native_context.capture();auto lease=store.capture_generation("scripts/zm/_zm_powerups.gsc");check(store.begin_generation(false));check(!store.if_current(lease,[&]{check(false);}));check(atomic_current(ticket)); // Store lease independently rejects old VM even before native shutdown.\n for(int capacity:{0,4,9}){check(!common_native_context.publish(1,capacity,map));check(!common_native_context.capture());}\n for(auto invalid:{"","core_frontend","bad map"})check(!common_native_context.publish(1,8,invalid));\n check(!common_native_context.publish(0,8,map));\n reset();check(begin_startup()==nullptr);blocked=false;auto before_client=common_helper::capture_native_session();schema_assets::begin_vm_generation(true);check(common_helper::native_session_owned(before_client)&&admission_ready());schema_assets::begin_vm_generation(false);check(!common_helper::capture_native_session()&&!admission_ready());\n reset();check(begin_startup()==nullptr);blocked=false;schema_assets::exhausted_for_test=true;schema_assets::begin_vm_generation(false);check(!common_helper::capture_native_session()&&!admission_ready());schema_assets::exhausted_for_test=false;\n std::cout<<"checks="<<checks<<" actual_begin_shutdown_frontend=true store_callback_atomics_only=true engine_link_authentication=false\\n";\n}\n'

def function(text,marker):
    start=text.find(marker)
    if start<0 or text.find(marker,start+1)>=0:raise ValueError('Expected one actual function: '+marker)
    pos=text.find('{',start);depth=1;end=pos+1
    while end<len(text):
        if text.startswith('//',end):
            end=text.find('\n',end)
            if end<0:break
            continue
        if text.startswith('/*',end):end=text.find('*/',end)+2;continue
        if text[end]in('"',"'"):
            q=text[end];end+=1
            while end<len(text):
                if text[end]=='\\':end+=2;continue
                if text[end]==q:end+=1;break
                end+=1
            continue
        if text[end]=='{':depth+=1
        elif text[end]=='}':
            depth-=1
            if depth==0:return text[start:end+1]
        end+=1
    raise ValueError('Unclosed actual body')

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,default=Path(__file__).resolve().parents[1])
    parser.add_argument('--compiler',default=os.environ.get('CXX','c++'))
    parser.add_argument('--compiler-arg',action='append',default=[])
    parser.add_argument('--output-dir',type=Path)
    args=parser.parse_args();repo=args.repo.resolve()
    source=repo/'src/client/component/zombies_coop.cpp';asset_source=repo/'src/client/component/zombies_schema_assets.cpp'
    text=source.read_text();asset_text=asset_source.read_text()
    ezz='game::com::Com_SessionMode_GetMode()'in text
    readers='static std::optional<int> get_dvar_int(const char*n){engine_guard();return ints[n];}static std::optional<std::string_view> get_dvar_string(const char*n){engine_guard();return strings[n];}'if ezz else'static int get_dvar_int(const char*n){engine_guard();return ints[n];}static std::string get_dvar_string(const char*n){engine_guard();return strings[n];}'
    helpers='\n'.join(function(text,m)for m in('bool reset_temple_pap_receipt(','int active_startup_epoch(','bool session_ending(','bool mark_session_ending('))
    wrappers='\n'.join(function(text,m)for m in('void invalidate_native_session(','native_ticket capture_native_session(','bool native_session_owned(','bool native_session_current(','bool native_admission_current(','bool revoke_native_session('))
    generation='''
namespace schema_assets {
bool exhausted_for_test=false;
struct test_store {
  bool exhausted(){return exhausted_for_test;}
  bool begin_generation(bool client){if(!client)check(!common_helper::capture_native_session());return true;}
}assets;
std::mutex client_context_mutex;std::string client_context_map;
void fail(std::string_view){failed=true;}
'''+function(asset_text,'void begin_vm_generation(')+'\n}'
    generated=PRELUDE.replace('HEADERS',(repo/'src/client/component/zombies').as_posix()).replace('READERS',readers).replace('PREFLIGHT',function(text,'const char *preflight_error(')).replace('HELPERS',helpers).replace('NATIVE_WRAPPERS',wrappers).replace('ADMISSION',function(text,'bool admission_ready(')).replace('BEGIN',function(text,'const char *begin_startup(')).replace('SHUTDOWN',function(text,'void shutdown_map(')).replace('POLL',function(text,'const char *poll_startup(')).replace('GENERATION',generation)+TESTS
    output=args.output_dir
    if output:output.mkdir(parents=True,exist_ok=False)
    else:output=Path(tempfile.mkdtemp(prefix='coop-common-context-'))
    cpp=output/'actual_context.cpp';cpp.write_text(generated);exe=output/('context.exe'if os.name=='nt'else'context')
    compiler=[args.compiler]if Path(args.compiler).is_file()else shlex.split(args.compiler,posix=os.name!='nt');msvc=Path(compiler[0]).name.lower()in('cl','cl.exe','clang-cl','clang-cl.exe');command=compiler+args.compiler_arg
    if msvc:command+=['/nologo','/std:c++20','/EHsc','/W4','/WX','/DNOMINMAX','/I'+str(repo/'src/client/component/zombies'),'/Fe:'+str(exe),'/Fo:'+str(exe.with_suffix('.obj')),str(cpp)]
    else:command+=['-std=c++20','-Wall','-Wextra','-Werror','-I',str(repo/'src/client/component/zombies'),str(cpp),'-o',str(exe)]
    result=subprocess.run(command,cwd=output,capture_output=True,text=True,timeout=120);(output/'compile.log').write_text(result.stdout+result.stderr)
    if result.returncode:raise SystemExit('Compile failed: '+str(output/'compile.log'))
    result=subprocess.run([str(exe)],cwd=output,capture_output=True,text=True,timeout=30);(output/'stdout.log').write_text(result.stdout);(output/'stderr.log').write_text(result.stderr)
    if result.returncode:raise SystemExit('Context body regression: '+str(output))
    sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    record=dict(passed=True,stdout=result.stdout,actual_source_sha256=sha(source),actual_generation_source_sha256=sha(asset_source),provider_sha256=sha(repo/'src/client/component/zombies/common_helper_native_context.hpp'),scope='Actual native provider, begin/shutdown/frontend/admission and server/client generation bodies under engine/state stand-ins; no imports, client image, native timing or gameplay proof.')
    (output/'RESULTS.json').write_text(json.dumps(record,indent=2)+'\n');print(json.dumps(dict(proof=str(output/'RESULTS.json'),stdout=result.stdout)))

if __name__=='__main__':main()
