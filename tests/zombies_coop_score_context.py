"""Exercise actual score asset/context/readiness/diagnostic bodies with native stand-ins.

No game execution or retail bytes are committed. --retail-path is optional and
accepts only a private exact stock score object through the production digest gate.
"""
from pathlib import Path
import argparse, hashlib, json, re, subprocess

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--compiler',required=True)
parser.add_argument('--output',type=Path,required=True)
parser.add_argument('--retail-path',type=Path)
args=parser.parse_args()
root=Path(__file__).resolve().parents[1]
source=root/'src/client/component/zombies_schema_assets.cpp'
companion=root/'src/client/component/zombies_companion.cpp'
diagnostics=root/'src/client/component/zombies_diagnostics.cpp'
original=source.read_text()

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

guard=function_text(original,r'\bbool\s+castle_current\s*\(\s*const\s+castle_request\s*&\s*request\s*\)\s*noexcept\s*\{')
assert not re.search(r'\b(?:frozen_schema_map|refresh_schema_session|schema_asset_failure)\s*\(',guard)
ready=function_text(companion.read_text(),r'\bbool\s+schema_server_ready\s*\(\s*\)\s*noexcept\s*\{')
assert re.search(r'\bschema_assets::score_producer_ready\s*\(',ready), 'actual server admission must require native proof'
registration=re.search(r'command::add\s*\(\s*"coopScoreSourceStatus"\s*,[\s\S]*?\}\s*\)\s*;',diagnostics.read_text())
assert registration, 'actual compact server diagnostic command missing'
body='\n'.join(line for line in original.splitlines() if not line.lstrip().startswith('#include'))
out=args.output.resolve();out.mkdir(parents=True,exist_ok=False)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
inputs={str(p):sha(p) for p in (source,companion,diagnostics)}
prelude=r'''
#define NOMINMAX
#include <Windows.h>
#include <bcrypt.h>
#include <array>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <mutex>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>
#include "HEADER_ROOT/zombies/schema_asset_store.hpp"
#include "HEADER_ROOT/zombies/schema_context_policy.hpp"
#include "HEADER_ROOT/zombies/castle_selector.hpp"
#include "HEADER_ROOT/zombies/score_events.hpp"
#include "HEADER_ROOT/zombies/companion.hpp"
namespace mock {
std::atomic_bool failed{};
std::atomic_int failures{},capacity{8},frozen_calls{};
std::string actual_map="custom_map",frozen_map="custom_map";
std::mutex context_mutex;
std::function<void()> after_hash;
bool dvar_present=true,dvar_string=true,dvar_session_specific=true;
unsigned dvar_resolutions{};
}
namespace game {
enum class eModes{MP,CP,ZOMBIES};
inline constexpr auto MODE_ZOMBIES=eModes::ZOMBIES;
eModes mode=eModes::ZOMBIES;
std::function<void()> read_hook;
struct dvar_t{};struct encryptedDvar_t{};
dvar_t server_dvar;encryptedDvar_t client_dvar;
enum class dvarType_t{INT,STRING,SESSIONMODE_BASE_DVAR};
// Match Ezz's important overload shape: bool is non-const, both pointer
// conversions are const, and operator! is const. A const ternary is ambiguous.
struct EngineDependentDvarMut {
  bool present{},resolved{};
  operator bool() noexcept{return present;}
  operator dvar_t*() const noexcept{return present?&server_dvar:nullptr;}
  operator encryptedDvar_t*() const noexcept{return present?&client_dvar:nullptr;}
  bool operator!() const noexcept{return !present;}
  EngineDependentDvarMut resolve() const{++mock::dvar_resolutions;return {present,true};}
  dvarType_t type() const{return !resolved&&mock::dvar_session_specific?dvarType_t::SESSIONMODE_BASE_DVAR:
      mock::dvar_string?dvarType_t::STRING:dvarType_t::INT;}
  const char* get_cstring() const{assert(present && resolved);if(read_hook)read_hook();return mock::actual_map.c_str();}
};
template<class T>concept const_ternary=requires(const T& value){value?true:false;};
template<class T>concept mutable_ternary=requires(T& value){value?true:false;};
static_assert(!const_ternary<EngineDependentDvarMut> && mutable_ternary<EngineDependentDvarMut>);
EngineDependentDvarMut Dvar_FindVar(const char*){return {mock::dvar_present,false};}
EngineDependentDvarMut get_dvar(const char* name){return Dvar_FindVar(name);}
namespace scr {
struct GSC_OBJ{std::uint8_t header[0x48];};
static_assert(sizeof(GSC_OBJ)==0x48 && !std::is_convertible_v<GSC_OBJ,std::uint8_t>);
struct ScriptParseTree{const char* name;std::size_t len;GSC_OBJ* buffer;};
static_assert(sizeof(ScriptParseTree)==0x18);
}
eModes Com_SessionMode_GetMode(){return mode;}
namespace com{eModes Com_SessionMode_GetMode(){return mode;}}
}
namespace utils::cryptography::sha256 {
std::string compute(const std::uint8_t* data,std::size_t length,bool hex){
  assert(!hex);BCRYPT_ALG_HANDLE algorithm{};std::string result(32,'\0');
  assert(BCryptOpenAlgorithmProvider(&algorithm,BCRYPT_SHA256_ALGORITHM,nullptr,0)==0);
  assert(BCryptHash(algorithm,nullptr,0,const_cast<PUCHAR>(data),static_cast<ULONG>(length),
      reinterpret_cast<PUCHAR>(result.data()),32)==0);
  BCryptCloseAlgorithmProvider(algorithm,0);if(mock::after_hash)mock::after_hash();return result;
}
}
namespace zombies::coop {
int frozen_schema_capacity() noexcept{return mock::capacity.load();}
std::string frozen_schema_map(){std::lock_guard lock(mock::context_mutex);++mock::frozen_calls;return mock::frozen_map;}
bool schema_assets_failed() noexcept{return mock::failed.load();}
void schema_asset_failure(std::string_view){mock::failed=true;++mock::failures;}
void diagnostic_event(std::string_view,std::int64_t=0,std::int64_t=0) noexcept{}
void refresh_schema_session() noexcept{}
namespace schema_assets {
using script_asset=game::scr::ScriptParseTree;
bool prepare_client_generation(std::string_view,bool,bool&) noexcept;
}
}
'''
prelude=prelude.replace('HEADER_ROOT',(root/'src/client/component').as_posix())
assert '#define BOIII_COOP_EZZ' in original

ready_support=r'''
namespace mock {
int script_revision=zombies::coop::companion_revision,script_mask=3,mask_expected=3;
std::string script_contract(zombies::coop::companion_contract);
bool script_read_throws{},ending{};std::string script_status="experimental_callbacks_installed";
}
namespace command {
struct params{int count{};int size()const{return count;}};
std::function<void(const params&)> score_handler;
template<class F>void add(const char* name,F callback){assert(std::string_view(name)=="coopScoreSourceStatus");score_handler=callback;}
}
namespace scheduler {
enum class pipeline{server=1};
inline constexpr auto server=pipeline::server;
std::vector<std::function<void()>> pending;
void once(std::function<void()> callback,pipeline selected){assert(selected==server);pending.push_back(std::move(callback));}
}
namespace zombies::coop {
bool ending_session_current() noexcept{return mock::ending;}
namespace {
int read_int(const char* name){
 if(mock::script_read_throws)throw std::runtime_error("script marker reader");
 if(std::string_view(name)=="zm_coopSchemaServerRevision")return mock::script_revision;
 if(std::string_view(name)=="zm_coopSchemaServerMask")return mock::script_mask;
 throw std::runtime_error("unexpected script int");
}
std::string read_string(const char* name){
 if(mock::script_read_throws)throw std::runtime_error("script marker reader");
 if(std::string_view(name)=="zm_coopStatus")return mock::script_status;
 assert(std::string_view(name)=="zm_coopSchemaServerContract");return mock::script_contract;
}
int expected_mask(std::string_view){return mock::mask_expected;}
}
READY_BODY
void install_actual_score_command(){COMMAND_BODY}
}
'''

tests=r'''
unsigned checks{};void check(bool ok){++checks;assert(ok);}
int main(int argc,char** argv){
 using namespace zombies::coop;using namespace zombies::coop::schema_assets;
 const auto asset=[](const char* name,std::vector<char>& bytes){return script_asset{name,static_cast<decltype(script_asset::len)>(bytes.size()),reinterpret_cast<decltype(script_asset::buffer)>(bytes.data())};};
 const auto byte_buffer=[](const script_asset* value){return reinterpret_cast<const char*>(value->buffer);};
 static_assert(sizeof(script_asset)==24 && offsetof(script_asset,buffer)==16);
 std::vector<char> raw(score_events::captured_size,0);std::copy(t7::object_magic.begin(),t7::object_magic.end(),raw.begin());
 auto invalid=asset(score_events::asset_name.data(),raw);
 const auto reset=[&]{game::read_hook={};mock::after_hash={};mock::failed=false;mock::failures=0;mock::capacity=8;game::mode=game::eModes::ZOMBIES;mock::actual_map="custom_map";mock::frozen_map="custom_map";
 mock::dvar_present=true;mock::dvar_string=true;mock::dvar_session_specific=true;mock::dvar_resolutions=0;
 mock::ending=false;mock::script_status="experimental_callbacks_installed";mock::script_revision=companion_revision;mock::script_mask=3;mock::mask_expected=3;mock::script_contract=std::string(companion_contract);mock::script_read_throws=false;scheduler::pending.clear();begin_vm_generation(false);};
 const auto receipt=[&](const char* label,bool expected){const auto failures=mock::failures.load();const auto retained=assets.retained();const auto text=score_source_diagnostic();check(mock::failures==failures && assets.retained()==retained);check(text.size()<600);std::cout<<"RECEIPT|"<<label<<"|"<<(expected?"true":"false")<<"|"<<text<<"\n";};
 reset();check(!score_producer_ready());check(!schema_server_ready());receipt("missing_cache",false);
 for(auto mode:{game::eModes::MP,game::eModes::CP}){reset();game::mode=mode;check(transform(invalid.name,&invalid)==&invalid);check(mock::failures==0&&!score_producer_ready()&&!schema_server_ready());receipt("pending_peer_wrong_mode",false);}
 for(int cap:{0,1,4,9,16}){reset();mock::capacity=cap;check(transform(invalid.name,&invalid)==&invalid);check(mock::failures==0&&!score_producer_ready()&&!schema_server_ready());}
 for(auto map:{"","core_frontend","bad/map","bad map","bad\\map","bad\"map"}){reset();mock::actual_map=map;mock::frozen_map=map;check(transform(invalid.name,&invalid)==&invalid);check(mock::failures==0&&!score_producer_ready());receipt("invalid_map",false);}
 reset();mock::actual_map=std::string(65,'a');mock::frozen_map=mock::actual_map;check(transform(invalid.name,&invalid)==&invalid && mock::failures==0);
 reset();mock::frozen_map="different";check(transform(invalid.name,&invalid)==&invalid && mock::failures==0&&!score_producer_ready());
 reset();check(!capture_castle_request("scripts/zm/_zm_score.csc"));check(transform_score("scripts/zm/_zm_score.csc",&invalid)==&invalid && mock::failures==0);
 for(auto name:{"scripts/zm/_zm_score.gsc.bak","scripts/mp/_zm_score.gsc","../scripts/zm/_zm_score.gsc"}){reset();check(transform(name,&invalid)==&invalid && mock::failures==0);}
 reset();check(transform(invalid.name,nullptr)==nullptr && mock::failures==1);check(!score_producer_ready());receipt("missing_raw_declined",false);
 reset();auto missing=invalid;missing.buffer=nullptr;check(transform(invalid.name,&missing)==&missing && mock::failures==1);
 reset();missing=invalid;missing.len=0;check(transform(invalid.name,&missing)==&missing && mock::failures==1);
 reset();missing=invalid;missing.len=static_cast<decltype(missing.len)>(-1);check(transform(invalid.name,&missing)==&missing && mock::failures==1);
 reset();check(transform(invalid.name,&invalid)==&invalid && mock::failures==1&&mock::failed);check(!score_producer_ready()&&!schema_server_ready());
 reset();auto request=capture_castle_request(invalid.name);check(request.has_value());begin_vm_generation(false);castle_failure(*request,"old VM");check(mock::failures==0);
 reset();request=capture_castle_request(invalid.name);game::mode=game::eModes::MP;castle_failure(*request,"old mode");check(mock::failures==0);
 reset();request=capture_castle_request(invalid.name);mock::actual_map="next_map";castle_failure(*request,"old map");check(mock::failures==0);
 reset();request=capture_castle_request(invalid.name);mock::capacity=7;castle_failure(*request,"old cap");check(mock::failures==0);
 reset();mock::after_hash=[&]{begin_vm_generation(false);};check(transform(invalid.name,&invalid)==&invalid && mock::failures==0);check(!score_producer_ready());
 reset();mock::after_hash=[&]{game::mode=game::eModes::CP;};check(transform(invalid.name,&invalid)==&invalid && mock::failures==0);
 reset();mock::after_hash=[&]{mock::actual_map="next_map";};check(transform(invalid.name,&invalid)==&invalid && mock::failures==0);
 reset();mock::after_hash=[&]{mock::capacity=7;};check(transform(invalid.name,&invalid)==&invalid && mock::failures==0);
 reset();mock::after_hash=[]{throw std::bad_alloc();};check(transform(invalid.name,&invalid)==&invalid && mock::failures==1);check(!score_producer_ready());
 check(reinterpret_cast<std::uintptr_t>(invalid.buffer+1)-reinterpret_cast<std::uintptr_t>(invalid.buffer)==0x48);
 check(reinterpret_cast<std::uintptr_t>(reinterpret_cast<const std::uint8_t*>(invalid.buffer)+invalid.len)-reinterpret_cast<std::uintptr_t>(invalid.buffer)==raw.size());
 reset();mock::dvar_present=false;check(transform(invalid.name,&invalid)==&invalid&&mock::failures==0&&!score_producer_ready());check(mock::dvar_resolutions==0);
 reset();mock::dvar_string=false;check(transform(invalid.name,&invalid)==&invalid&&mock::failures==0&&!score_producer_ready());check(mock::dvar_resolutions>0);
 reset();install_actual_score_command();check(static_cast<bool>(command::score_handler));command::score_handler({0});command::score_handler({2});check(scheduler::pending.empty());command::score_handler({1});check(scheduler::pending.size()==1);game::mode=game::eModes::MP;std::cout<<"COMMAND_EXPECT|false\n";scheduler::pending.front()();scheduler::pending.clear();check(mock::failures==0);
 if(argc==2){
  reset();std::ifstream file(argv[1],std::ios::binary);check(file.good());std::vector<char> object{std::istreambuf_iterator<char>(file),{}};auto original=asset(invalid.name,object);const auto original_bytes=object;
  auto* first=transform(original.name,&original);check(first!=&original && mock::failures==0);check(score_producer_ready()&&schema_server_ready());receipt("verified_cache",true);
  check(transform("SCRIPTS\\ZM\\_ZM_SCORE.GSC",&original)==first);mock::after_hash=[]{throw std::bad_alloc();};check(transform(original.name,&original)==first && mock::failures==0);mock::after_hash={};check(object==original_bytes);
  const auto expected=std::vector<char>(byte_buffer(first),byte_buffer(first)+first->len);const_cast<char*>(byte_buffer(first))[0xc28]=static_cast<char>(0xcc);check(score_producer_ready());
  begin_vm_generation(true);check(score_producer_ready()&&schema_server_ready());receipt("client_vm_does_not_grant_or_revoke_server",true);
  begin_vm_generation(false);check(!score_producer_ready()&&!schema_server_ready());receipt("new_server_vm_revokes",false);
  auto* second=transform(original.name,first);check(second!=first && second!=&original && mock::failures==0);check(std::vector<char>(byte_buffer(second),byte_buffer(second)+second->len)==expected);check(static_cast<std::size_t>(second->len)==expected.size());check(score_producer_ready()&&schema_server_ready());receipt("new_vm_pristine_rebound",true);
  mock::ending=true;check(score_producer_ready()&&!schema_server_ready());mock::ending=false;mock::script_status="callback_ownership_lost";check(!schema_server_ready());mock::script_status="experimental_callbacks_installed";
  mock::script_revision=0;check(score_producer_ready()&&!schema_server_ready());mock::script_revision=companion_revision;mock::script_mask=0;check(!schema_server_ready());mock::script_mask=3;mock::script_contract="other";check(!schema_server_ready());mock::script_contract=std::string(companion_contract);mock::script_read_throws=true;check(!schema_server_ready());mock::script_read_throws=false;check(schema_server_ready());
  mock::failed=true;check(!score_producer_ready()&&!schema_server_ready());receipt("failure_latch_revokes",false);mock::failed=false;
  game::mode=game::eModes::MP;check(!score_producer_ready()&&!schema_server_ready());game::mode=game::eModes::ZOMBIES;mock::frozen_map="other_map";check(!score_producer_ready());mock::frozen_map=mock::actual_map;
  int reads{};game::read_hook=[&]{if(++reads==2)mock::actual_map="travel_pending";};check(!score_producer_ready()&&mock::failures==0);game::read_hook={};mock::actual_map="custom_map";check(score_producer_ready());
  command::score_handler({1});check(scheduler::pending.size()==1);begin_vm_generation(false);std::cout<<"COMMAND_EXPECT|false\n";scheduler::pending.front()();scheduler::pending.clear();check(!score_producer_ready());
  auto* third=transform(original.name,second);check(third!=second&&score_producer_ready());command::score_handler({1});check(scheduler::pending.size()==1);std::cout<<"COMMAND_EXPECT|true\n";scheduler::pending.front()();scheduler::pending.clear();check(mock::failures==0);
  for(int cap=5;cap<=8;++cap){reset();mock::capacity=cap;check(!score_producer_ready());auto* held=transform(original.name,&original);check(held!=&original&&mock::failures==0);check(score_producer_ready()&&schema_server_ready());receipt("all_supported_capacities",true);}
  reset();mock::after_hash=[&]{begin_vm_generation(false);};check(transform(original.name,&original)==&original&&mock::failures==0&&!score_producer_ready());
 }
 std::cout<<"checks="<<checks<<" actual_score_component_bodies=true native_engine=false\n";
}
'''

ready_support=ready_support.replace('READY_BODY',ready).replace('COMMAND_BODY',registration.group())
generated=out/'score_context_actual.cpp'
generated.write_text(prelude+'\n'+body+'\n'+ready_support+'\n'+tests)
binary=out/'score_context_actual.exe'
command=[args.compiler,'/nologo','/std:c++20','/EHsc','/W4','/WX','/Fe:'+str(binary),'/Fo:'+str(binary.with_suffix('.obj')),str(generated),'bcrypt.lib']
with(out/'compile.log').open('x',encoding='utf-8')as log:result=subprocess.run(command,cwd=out,stdout=log,stderr=subprocess.STDOUT)
assert result.returncode==0,out/'compile.log'
run_command=[str(binary)]+([str(args.retail_path.resolve(strict=True))]if args.retail_path else[])
run=subprocess.run(run_command,cwd=out,capture_output=True,text=True,timeout=25)
(out/'stdout.log').write_text(run.stdout);(out/'stderr.log').write_text(run.stderr)
assert run.returncode==0,(out,run.stderr)
receipts=[];command_expected=None;command_receipts=[]
allowed={'version','verified','map','capacity','realm','generation','revision','contract'}
for line in run.stdout.splitlines():
    if line.startswith('RECEIPT|'):
        _,label,expected,raw=line.split('|',3);value=json.loads(raw)
        assert value['verified'] is (expected=='true'),(label,value)
        assert set(value)==allowed and value['version']==1 and value['realm']==0
        assert type(value['capacity'])is int and type(value['generation'])is int
        assert value['revision']==4 and re.fullmatch('[0-9a-f]{64}',value['contract'])
        assert not value['map'] or re.fullmatch('[A-Za-z0-9_-]{1,64}',value['map'])
        if value['verified']:assert value['capacity']in range(5,9) and value['generation']>0 and value['map']
        receipts.append({'case':label,'value':value})
    elif line.startswith('COMMAND_EXPECT|'):
        assert command_expected is None;command_expected=line.endswith('|true')
    elif line.startswith('COOP_SCORE_SOURCE|'):
        value=json.loads(line.split('|',1)[1]);assert command_expected is not None
        assert value['verified'] is command_expected and set(value)==allowed
        command_receipts.append(value);command_expected=None
assert command_expected is None and command_receipts and receipts
if args.retail_path:assert any(value['verified']for value in command_receipts)
assert inputs=={str(p):sha(p)for p in(source,companion,diagnostics)},'actual source changed during scoped proof'
proof=dict(passed=True,scope='Actual component/readiness/readonly command C++ bodies under native state/lock/hash stand-ins; no engine execution.',
    source_inputs=inputs,ending_observer_mocked_for_score_scope=True,generated_sha256=sha(generated),test_sha256=sha(Path(__file__)),command=command,run_command=run_command,
    stdout=run.stdout,retail_private_supplied=bool(args.retail_path),native_json_receipts=receipts,actual_command_receipts=command_receipts,
    viewer_or_script_markers_cannot_grant_native_cache=True,guard_has_no_frozen_context_getter=True,abi_family='ezz')
(out/'RESULTS.private.json').write_text(json.dumps(proof,indent=2)+'\n');print(json.dumps(proof))
