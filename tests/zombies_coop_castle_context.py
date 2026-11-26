"""Run actual asset-component bodies under native stand-ins; no game/runtime claim.

The default test commits no retail bytes. --retail-path optionally checks a
private, user-supplied exact raw object against the production fixed contract.
"""
from pathlib import Path
import argparse, hashlib, json, subprocess

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--compiler',required=True)
parser.add_argument('--output',type=Path,required=True)
parser.add_argument('--retail-path',type=Path)
args=parser.parse_args()
root=Path(__file__).resolve().parents[1]
source=root/'src/client/component/zombies_schema_assets.cpp'
original=source.read_text()
guard=original.split('bool castle_current(const castle_request& request) noexcept {',1)[1].split('\nstd::optional<castle_request>',1)[0]
assert 'frozen_schema_map(' not in guard and 'refresh_schema_session(' not in guard
assert 'schema_asset_failure(' not in guard
body='\n'.join(line for line in original.splitlines() if not line.startswith('#include'))
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
#include <vector>
#include "HEADER_ROOT/zombies/schema_asset_store.hpp"
#include "HEADER_ROOT/zombies/schema_context_policy.hpp"
#include "HEADER_ROOT/zombies/castle_selector.hpp"
namespace mock {
std::atomic_bool failed{};
std::atomic_int failures{},capacity{8},frozen_calls{};
std::string actual_map="custom_map",frozen_map="custom_map";
std::mutex context_mutex;
std::function<void()> after_hash;
}
namespace game {
enum class eModes{MP,CP,ZOMBIES};
inline constexpr auto MODE_ZOMBIES=eModes::ZOMBIES;
eModes mode=eModes::ZOMBIES;
struct dvar{};dvar map_dvar;
std::function<void()> read_hook;
const dvar* Dvar_FindVar(const char*){return &map_dvar;}
const char* Dvar_GetString(const dvar*){if(read_hook)read_hook();return mock::actual_map.c_str();}
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
struct script_asset {const char* name; LENGTH_TYPE len; const char* buffer;};
bool prepare_client_generation(std::string_view,bool,bool&) noexcept;
}
}
'''
prelude=prelude.replace('HEADER_ROOT',(root/'src/client/component').as_posix())
prelude=prelude.replace('LENGTH_TYPE','std::size_t' if '#define BOIII_COOP_EZZ' in original else 'int')
tests=r'''
unsigned checks{};void check(bool ok){++checks;assert(ok);}
int main(int argc,char** argv){
  using namespace zombies::coop;using namespace zombies::coop::schema_assets;
  std::vector<char> raw(castle_selector::captured_size,0);
  std::copy(t7::object_magic.begin(),t7::object_magic.end(),raw.begin());
  script_asset invalid{castle_selector::asset_name.data(),static_cast<decltype(script_asset::len)>(raw.size()),raw.data()};
  const auto reset=[&]{game::read_hook={};mock::after_hash={};mock::failed=false;mock::failures=0;mock::capacity=8;
    game::mode=game::eModes::ZOMBIES;mock::actual_map="custom_map";mock::frozen_map="custom_map";begin_vm_generation(false);};
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
    script_asset original{invalid.name,static_cast<decltype(script_asset::len)>(object.size()),object.data()};
    auto* first=transform(original.name,&original);check(first!=&original && mock::failures==0);
    check(transform("SCRIPTS\\ZM\\ZM_CASTLE_VO.GSC",&original)==first);
    const auto expected=std::vector<char>(first->buffer,first->buffer+first->len);
    const_cast<char*>(first->buffer)[0x61d8]=static_cast<char>(0xcc);begin_vm_generation(false);
    auto* second=transform(original.name,first);check(second!=first && second!=&original && mock::failures==0);
    check(std::vector<char>(second->buffer,second->buffer+second->len)==expected);
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
    lock_order_adversary=True,guard_has_no_frozen_context_getter=True)
(out/'RESULTS.private.json').write_text(json.dumps(proof,indent=2)+'\n')
print(json.dumps(proof))
