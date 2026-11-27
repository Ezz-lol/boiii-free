"""Run actual asset-component bodies under native stand-ins; no game/runtime claim.

The default test commits no retail bytes. --retail-path optionally checks a
private, user-supplied exact raw object against the production fixed contract.
"""
from pathlib import Path
import argparse, hashlib, json, re, subprocess

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--compiler',required=True)
parser.add_argument('--output',type=Path,required=True)
parser.add_argument('--retail-path',type=Path)
parser.add_argument('--repo',type=Path,default=Path(__file__).resolve().parents[1])
args=parser.parse_args()
root=args.repo.resolve()
source=root/'src/client/component/zombies_schema_assets.cpp'
original=source.read_text()
guard_start=re.search(r'\bbool\s+castle_current\s*\(\s*const\s+castle_request\s*&\s*request\s*\)\s*noexcept\s*\{',original)
assert guard_start, 'actual castle_current function missing'
guard_end=re.search(r'\bstd::optional\s*<\s*castle_request\s*>\s+capture_castle_request',original[guard_start.end():])
assert guard_end, 'actual request capture boundary missing'
guard=original[guard_start.end():guard_start.end()+guard_end.start()]
assert not re.search(r'\bfrozen_schema_map\s*\(',guard) and not re.search(r'\brefresh_schema_session\s*\(',guard)
assert not re.search(r'\bschema_asset_failure\s*\(',guard)
body='\n'.join(line for line in original.splitlines() if not line.lstrip().startswith('#include'))
out=args.output.resolve();out.mkdir(parents=True,exist_ok=False)
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
#include "HEADER_ROOT/zombies/common_helper_source_pair.hpp"
#include "HEADER_ROOT/zombies/powerup_events.hpp"
#include "HEADER_ROOT/zombies/coop_runtime.hpp"
#include "HEADER_ROOT/zombies/lobby_policy.hpp"
#include "HEADER_ROOT/zombies/map_runtime_bridge.hpp"
#include "HEADER_ROOT/zombies/map_mechanics_contracts.hpp"
#include "HEADER_ROOT/zombies/map_mechanics_payloads.hpp"
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
  bool present{},resolved{},integer{};
  operator bool() noexcept{return present;}
  operator dvar_t*() const noexcept{return present?&server_dvar:nullptr;}
  operator encryptedDvar_t*() const noexcept{return present?&client_dvar:nullptr;}
  bool operator!() const noexcept{return !present;}
  EngineDependentDvarMut resolve() const{++mock::dvar_resolutions;return {present,true,integer};}
  dvarType_t type() const{return integer?dvarType_t::INT:!resolved&&mock::dvar_session_specific?dvarType_t::SESSIONMODE_BASE_DVAR:
      mock::dvar_string?dvarType_t::STRING:dvarType_t::INT;}
  int get_int() const{return 0;}
  const char* get_cstring() const{assert(present && resolved);if(read_hook)read_hook();return mock::actual_map.c_str();}
};
template<class T>concept const_ternary=requires(const T& value){value?true:false;};
template<class T>concept mutable_ternary=requires(T& value){value?true:false;};
static_assert(!const_ternary<EngineDependentDvarMut> && mutable_ternary<EngineDependentDvarMut>);
EngineDependentDvarMut Dvar_FindVar(const char* name){
  return {mock::dvar_present,false,std::strcmp(name,"zm_coopTemplePapEpoch")==0};
}
EngineDependentDvarMut get_dvar(const char* name){return Dvar_FindVar(name);}
namespace scr {
struct GSC_OBJ{std::uint8_t header[0x48];};
static_assert(sizeof(GSC_OBJ)==0x48 && !std::is_convertible_v<GSC_OBJ,std::uint8_t>);
struct ScriptParseTree{const char* name;std::size_t len;GSC_OBJ* buffer;};
static_assert(sizeof(ScriptParseTree)==0x18);
}
bool Com_IsRunningUILevel(){return false;}
eModes Com_SessionMode_GetMode(){return mode;}
namespace com{eModes Com_SessionMode_GetMode(){return mode;}bool Com_IsRunningUILevel(){return false;}}
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
// This legacy Castle fixture intentionally has no common-source session.
// Positive provider/pair behavior is covered by common_context/common_sources.
namespace common_helper {
native_ticket capture_native_session() noexcept {return {};}
void invalidate_native_session() noexcept {}
bool native_session_current(const native_ticket&) noexcept {return false;}
bool native_session_owned(const native_ticket&) noexcept {return false;}
bool native_admission_current() noexcept {return false;}
bool revoke_native_session(const native_ticket&) noexcept {return false;}
}
int frozen_schema_capacity() noexcept{return mock::capacity.load();}
std::string frozen_schema_map(){std::lock_guard lock(mock::context_mutex);++mock::frozen_calls;return mock::frozen_map;}
bool schema_assets_failed() noexcept{return mock::failed.load();}
bool latch_schema_asset_failure() noexcept{return !mock::failed.exchange(true);}
void report_schema_asset_failure(std::string_view){++mock::failures;}
void schema_asset_failure(std::string_view){mock::failed=true;++mock::failures;}
int active_startup_epoch() noexcept{return 1;}
void diagnostic_event(std::string_view,std::int64_t=0,std::int64_t=0) noexcept{}
void refresh_schema_session() noexcept{}
namespace schema_assets {
using script_asset=game::scr::ScriptParseTree;
COMMON_ALIASES
MAP_DECLARATIONS
bool prepare_client_generation(std::string_view,bool,bool&) noexcept;
}
}
'''
public_interface=root/'src/client/component/zombies/schema_assets.hpp'
public_text=public_interface.read_text()
common_start=public_text.index('using common_lifecycle')
common_end=public_text.index('bool common_required()',common_start)
common_aliases=public_text[common_start:common_end]
prelude=prelude.replace('COMMON_ALIASES',common_aliases)
map_start=public_text.index('// Retained map module/helper code dependencies')
map_end=public_text.index('// Pin the exact bytes',map_start)
prelude=prelude.replace('MAP_DECLARATIONS',public_text[map_start:map_end])
prelude=prelude.replace('HEADER_ROOT',(root/'src/client/component').as_posix())
assert '#define BOIII_COOP_EZZ' in original
tests=r'''
unsigned checks{};void check(bool ok){++checks;assert(ok);}
int main(int argc,char** argv){
  using namespace zombies::coop;using namespace zombies::coop::schema_assets;
  std::vector<char> raw(castle_selector::captured_size,0);
  std::copy(t7::object_magic.begin(),t7::object_magic.end(),raw.begin());
  script_asset invalid{castle_selector::asset_name.data(),raw.size(),reinterpret_cast<game::scr::GSC_OBJ*>(raw.data())};
  const auto byte_buffer=[](const script_asset* asset){return reinterpret_cast<const char*>(asset->buffer);};
  check(reinterpret_cast<std::uintptr_t>(invalid.buffer+1)-reinterpret_cast<std::uintptr_t>(invalid.buffer)==0x48);
  check(reinterpret_cast<std::uintptr_t>(reinterpret_cast<const std::uint8_t*>(invalid.buffer)+invalid.len)-
      reinterpret_cast<std::uintptr_t>(invalid.buffer)==raw.size());
  const auto reset=[&]{game::read_hook={};mock::after_hash={};mock::failed=false;mock::failures=0;mock::capacity=8;
    game::mode=game::eModes::ZOMBIES;mock::actual_map="custom_map";mock::frozen_map="custom_map";
    mock::dvar_present=true;mock::dvar_string=true;mock::dvar_session_specific=true;mock::dvar_resolutions=0;
    begin_vm_generation(false);};
  reset();mock::dvar_present=false;check(transform(invalid.name,&invalid)==&invalid && mock::failures==0);
  check(mock::dvar_resolutions==0);
  reset();mock::dvar_string=false;check(transform(invalid.name,&invalid)==&invalid && mock::failures==0);
  check(mock::dvar_resolutions>0);
  for(const auto mode:{game::eModes::MP,game::eModes::CP}){reset();game::mode=mode;
    check(transform(invalid.name,&invalid)==&invalid);check(mock::failures==0);}
  for(int cap=0;cap<=4;++cap){reset();mock::capacity=cap;check(transform(invalid.name,&invalid)==&invalid);check(mock::failures==0);}
  for(const auto map:{"","core_frontend","bad/map","bad map"}){reset();mock::actual_map=map;mock::frozen_map=map;
    check(transform(invalid.name,&invalid)==&invalid);check(mock::failures==0);}
  reset();mock::actual_map=std::string(65,'a');mock::frozen_map=mock::actual_map;
  check(transform(invalid.name,&invalid)==&invalid && mock::failures==0);
  reset();mock::frozen_map="other_map";check(transform(invalid.name,&invalid)==&invalid && mock::failures==0);
  for(const auto path:{"scripts/zm/zm_castle_vo.csc","scripts/zm/zm_castle_vo.gsc.bak","scripts/mp/zm_castle_vo.gsc"}){
    reset();check(transform(path,&invalid)==&invalid && mock::failures==0);}
  // An inherited exact module on a custom ZM map is eligible; invalid raw bytes
  // must latch current-context failure rather than silently broaden identity.
  reset();check(transform(invalid.name,&invalid)==&invalid);check(mock::failures==1 && mock::failed);
  reset();auto request=capture_castle_request(invalid.name);check(request.has_value());
  begin_vm_generation(false);castle_failure(*request,"stale VM");check(mock::failures==0);
  reset();request=capture_castle_request(invalid.name);game::mode=game::eModes::MP;
  castle_failure(*request,"stale mode");check(mock::failures==0);
  reset();request=capture_castle_request(invalid.name);mock::actual_map="next_map";
  castle_failure(*request,"stale map");check(mock::failures==0);
  reset();request=capture_castle_request(invalid.name);mock::capacity=7;
  castle_failure(*request,"stale capacity");check(mock::failures==0);
  reset();mock::after_hash=[&]{begin_vm_generation(false);};
  check(transform(invalid.name,&invalid)==&invalid && mock::failures==0);
  reset();mock::after_hash=[&]{game::mode=game::eModes::CP;};
  check(transform(invalid.name,&invalid)==&invalid && mock::failures==0);
  reset();mock::after_hash=[] {throw std::bad_alloc();};
  check(transform(invalid.name,&invalid)==&invalid && mock::failures==1);
  // Adversarial lock order: reset owns companion context -> waits for store;
  // Castle failure owns store -> samples native state. Calling the frozen-map
  // getter in that second path would deadlock. Process timeout detects it.
  reset();request=capture_castle_request(invalid.name);const auto frozen_before=mock::frozen_calls.load();
  std::atomic_bool store_held{},context_held{};
  game::read_hook=[&]{store_held=true;while(!context_held.load())std::this_thread::yield();};
  std::thread failure_thread([&]{castle_failure(*request,"current failure");});
  std::thread reset_thread([&]{while(!store_held.load())std::this_thread::yield();
    std::lock_guard lock(mock::context_mutex);context_held=true;begin_vm_generation(true);});
  failure_thread.join();reset_thread.join();game::read_hook={};
  check(mock::failures==1 && mock::frozen_calls==frozen_before);
  if(argc==2){
    reset();std::ifstream file(argv[1],std::ios::binary);check(file.good());
    std::vector<char> object{std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>()};
    script_asset original{invalid.name,object.size(),reinterpret_cast<game::scr::GSC_OBJ*>(object.data())};
    auto* first=transform(original.name,&original);check(first!=&original && mock::failures==0);
    check(transform("SCRIPTS\\ZM\\ZM_CASTLE_VO.GSC",&original)==first);
    check(mock::dvar_resolutions>0);
    const auto expected=std::vector<char>(byte_buffer(first),byte_buffer(first)+first->len);
    reinterpret_cast<char*>(first->buffer)[0x61d8]=static_cast<char>(0xcc);begin_vm_generation(false);
    auto* second=transform(original.name,first);check(second!=first && second!=&original && mock::failures==0);
    check(std::vector<char>(byte_buffer(second),byte_buffer(second)+second->len)==expected);
    check(expected.size()==static_cast<std::size_t>(second->len));
    reset();mock::after_hash=[&]{mock::actual_map="travel_pending";};
    check(transform(original.name,&original)==&original && mock::failures==0);
    reset();mock::after_hash=[&]{begin_vm_generation(false);};
    check(transform(original.name,&original)==&original && mock::failures==0);
  }
  std::cout<<"checks="<<checks<<" actual_component_bodies=true native_engine=false\n";
}
'''
generated=out/'castle_context_actual.cpp';generated.write_text(prelude+'\n'+body+'\n'+tests)
binary=out/'castle_context_actual.exe'
command=[args.compiler,'/nologo','/std:c++20','/EHsc','/W4','/WX','/Fe:'+str(binary),'/Fo:'+str(binary.with_suffix('.obj')),str(generated),'bcrypt.lib']
with(out/'compile.log').open('x',encoding='utf-8')as log:result=subprocess.run(command,cwd=out,stdout=log,stderr=subprocess.STDOUT)
assert result.returncode==0,out/'compile.log'
run_command=[str(binary)]+([str(args.retail_path.resolve(strict=True))] if args.retail_path else [])
run=subprocess.run(run_command,cwd=out,capture_output=True,text=True,timeout=20)
(out/'stdout.log').write_text(run.stdout);(out/'stderr.log').write_text(run.stderr)
assert run.returncode==0,(out,run.stderr)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
proof=dict(passed=True,scope='Actual component C++ bodies with native state/lock/hash stand-ins. No engine execution.',
    source=str(source),source_sha256=sha(source),generated_sha256=sha(generated),test_sha256=sha(Path(__file__)),
    command=command,run_command=run_command,stdout=run.stdout,retail_private_supplied=bool(args.retail_path),
    lock_order_adversary=True,guard_has_no_frozen_context_getter=True,
    common_session_standin='intentionally_absent',public_interface_sha256=sha(public_interface),
    ezz_dvar_const_ambiguity_mirrored=True,ezz_gsc_object_size=72,ezz_buffer_type='GSC_OBJ*',byte_stride_verified=True)
(out/'RESULTS.private.json').write_text(json.dumps(proof,indent=2)+'\n')
print(json.dumps(proof))
