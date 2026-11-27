#include "schema_asset_store.hpp"
#include <atomic>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <new>

std::atomic_bool fail_all{false};
std::atomic_int fail_after{-1};
std::atomic_size_t allocation_calls{0};
void* operator new(std::size_t size) {
  ++allocation_calls;
  if (fail_all.load(std::memory_order_relaxed)) throw std::bad_alloc();
  if (fail_after.load(std::memory_order_relaxed)>=0 && fail_after.fetch_sub(1,std::memory_order_relaxed)==0) throw std::bad_alloc();
  if (auto* value=std::malloc(size?size:1)) return value;
  throw std::bad_alloc();
}
void operator delete(void* value) noexcept {std::free(value);}
void operator delete(void* value,std::size_t) noexcept {std::free(value);}

using namespace zombies::coop::t7;
struct Asset {const char* name{}; int len{}; const char* buffer{};};
using Store=schema_asset_store<Asset>;
std::size_t checks=0,failures=0,collision_failures=0;
void check(bool value,const char* label,bool collision=false) {
  ++checks;
  if(!value){++failures;if(collision)++collision_failures;std::cerr<<"FAIL "<<label<<'\n';}
}
std::string load(const char* path) {
  std::ifstream input(path,std::ios::binary);if(!input)throw std::runtime_error("fixture missing");
  return {std::istreambuf_iterator<char>(input),{}};
}
std::string renamed(std::string_view source,std::string_view name) {
  std::vector<std::uint8_t> result(source.begin(),source.end());
  const auto offset=static_cast<std::uint32_t>(result.size());
  result.insert(result.end(),name.begin(),name.end());result.push_back(0);write32(result,0x34,offset);
  return {reinterpret_cast<const char*>(result.data()),result.size()};
}
std::vector<std::uint8_t> bytes(std::string_view source){return {source.begin(),source.end()};}
std::string_view contents(const Asset* asset){return asset?std::string_view(asset->buffer,static_cast<std::size_t>(asset->len)):std::string_view{};}
std::string alias(std::string_view name){std::string value(name);for(auto& c:value){if(c=='/')c='\\';else if(c>='a'&&c<='z')c=static_cast<char>(c-('a'-'A'));}return value;}

int main(int argc,char** argv) {
  if(argc!=3)return 2;
  static_assert(companion_count==3);
  static_assert(map_companion_names.size()==8);
  static_assert(retention_limit==32u*1024u*1024u);
  const auto raw=load(argv[1]);const auto raw_client=load(argv[2]);
  const auto wire_server=renamed(raw,server_companion),wire_client=renamed(raw_client,client_companion),wire_island=renamed(raw,island_companion);
  std::array<std::string,map_companion_names.size()> payloads,aliases;
  for(unsigned i=0;i<map_companion_names.size();++i){payloads[i]=renamed(raw,map_companion_names[i]);aliases[i]=alias(map_companion_names[i]);}
  std::string error;error.reserve(1024);

  // Identity table remains separate from the original wire schema table.
  for(unsigned i=0;i<map_companion_names.size();++i){
    const auto name=map_companion_names[i];
    check(map_companion_identity(name)==i,"eight exact map identities");
    check(map_companion_identity(aliases[i])==i,"case/separator identity alias");
    check(!fixed_companion_identity(name),"map code is not old wire companion");
    check(!map_companion_identity(std::string(name)+".bak"),"suffix rejected");
    check(!map_companion_identity(std::string(name.substr(0,name.size()-4))+".csc"),"client type name rejected");
    check(!map_companion_identity("/"+std::string(name)),"absolute-like path rejected");
    check(!map_companion_identity("../"+std::string(name)),"parent path rejected");
  }
  check(!map_companion_identity(server_companion)&&!map_companion_identity(client_companion)&&!map_companion_identity(island_companion),"wire names absent from map table");
  for(unsigned i=0;i<map_companion_names.size();++i){
    Store store;const auto before=store.retained();
    check(!store.pin_map_companion(static_cast<unsigned>(map_companion_names.size()),payloads[i],error),"out-of-range index rejected");
    check(!store.pin_map_companion(i,raw,error),"unrenamed own compiled filename rejected");
    const auto wrong_type=renamed(raw,std::string(map_companion_names[i].substr(0,map_companion_names[i].size()-4))+".csc");
    check(!store.pin_map_companion(i,wrong_type,error),"wrong realm extension in object filename rejected");
    check(!store.pin_map_companion(i,payloads[(i+1)%map_companion_names.size()],error),"different fixed map filename rejected");
    check(!store.pin_map_companion(i,"truncated",error),"truncated payload rejected");
    auto bad=payloads[i];bad[0]^=1;check(!store.pin_map_companion(i,bad,error),"non-PC1C magic rejected");
    check(store.retained()==before&&!store.exhausted(),"validation failures do not publish or poison");
    check(store.pin_map_companion(i,payloads[i],error),"all exact names pin");
    const auto pinned=store.retained();check(pinned.first==0&&pinned.second>before.second,"pin reserves budget but no exposed slot");
    check(store.pin_map_companion(i,payloads[i],error)&&store.retained()==pinned,"identical pin idempotent");
    auto replacement=payloads[i];replacement[8]^=1;
    check(!store.pin_map_companion(i,replacement,error)&&store.retained()==pinned,"replacement immutable bytes rejected");
    const auto lease=store.capture_generation(map_companion_names[i]);
    auto* first=store.map_companion(aliases[i],error,lease);
    check(first&&contents(first)==payloads[i]&&std::string_view(first->name)==map_companion_names[i],"lookup canonical asset and exact pinned bytes");
    const auto exposed=store.retained();
    for(int repeat=0;repeat<20;++repeat)check(store.map_companion(map_companion_names[i],error,lease)==first,"recursive lookup identity stable");
    check(store.retained()==exposed,"repeated lookup no retained growth");
    const auto client=store.capture_generation(client_companion);
    check(!store.map_companion(map_companion_names[i],error,client),"client lease cannot access server map code");
    check(!store.map_companion(map_companion_names[i],error,{2,lease.generation}),"invalid realm rejected");
    check(store.begin_generation(true)&&store.map_companion(map_companion_names[i],error,lease)==first,"client generation preserves server backing");
    if(first)const_cast<char*>(first->buffer)[8]^=1;
    check(store.begin_generation(false),"server generation advances");
    check(!store.map_companion(map_companion_names[i],error,lease),"stale server lease rejected");
    const auto fresh_lease=store.capture_generation(map_companion_names[i]);
    auto* fresh=store.map_companion(map_companion_names[i],error,fresh_lease);
    check(fresh&&first&&fresh!=first&&fresh->buffer!=first->buffer&&contents(fresh)==payloads[i],"new server generation never reuses mutable old backing");
    check(first&&contents(first)!=payloads[i],"retired old backing remains alive and mutated");
    check(store.template_for(first)==bytes(payloads[i]),"old immutable prelink template remains pinned bytes");
  }
  {Store store;check(store.pin(wire_server,wire_client,wire_island,error),"old exact three-payload pin unchanged");
    for(unsigned i=0;i<map_companion_names.size();++i)check(store.pin_map_companion(i,payloads[i],error),"map and wire share store without identity collision");
    const auto lease=store.capture_generation(server_companion);check(store.authorize_island_server(lease),"existing island authorization unchanged");
    for(const auto name:companion_names)check(store.companion(name,error)!=nullptr,"all original three wire lookups work");
    for(const auto name:map_companion_names)check(store.map_companion(name,error,lease)!=nullptr,"all eight server code lookups work");
  }
  // Local allocation rollback at every allocation point, followed by retry.
  int failed_pins=0,failed_lookups=0;
  for(int point=0;point<32;++point){
    Store store;const auto before=store.retained();bool pinned=false;
    fail_after=point;try{pinned=store.pin_map_companion(0,payloads[0],error);}catch(const std::bad_alloc&){++failed_pins;}fail_after=-1;
    if(!pinned)check(store.retained()==before&&!store.exhausted(),"pin allocation failure atomically rolls back");
    check(store.pin_map_companion(0,payloads[0],error),"pin retry succeeds after allocation rollback");
    check(contents(store.map_companion(map_companion_names[0],error,store.capture_generation(server_companion)))==payloads[0],"retry exposes exact bytes");
  }
  check(failed_pins>=8,"multiple independent pin allocation failures exercised");
  for(int point=0;point<16;++point){
    Store store;check(store.pin_map_companion(0,payloads[0],error),"lookup injection pin");const auto before=store.retained();
    const auto lease=store.capture_generation(server_companion);Asset* result=nullptr;
    fail_after=point;try{result=store.map_companion(map_companion_names[0],error,lease);}catch(const std::bad_alloc&){++failed_lookups;}fail_after=-1;
    if(!result)check(store.retained()==before&&!store.cached(map_companion_names[0]),"lookup allocation failure leaves no half-published cache node");
    if(!result){const auto calls=allocation_calls.load();fail_all=true;result=store.emergency_map_companion(aliases[0]);fail_all=false;check(allocation_calls==calls,"emergency after lookup allocation failure allocates nothing");}
    check(result&&contents(result)==payloads[0],"lookup failure emergency exact pinned object");
  }
  check(failed_lookups>=5,"multiple independent lookup allocation failures exercised");
  // Pin budget exact boundary, shared wire+map accounting and no partial pin.
  Store probe;const auto base=probe.retained().second;check(probe.pin_map_companion(0,payloads[0],error),"budget probe pin");const auto map_charge=probe.retained().second-base;
  {Store exact(base+map_charge);check(exact.pin_map_companion(0,payloads[0],error),"exact map reservation budget accepted");}
  {Store short_store(base+map_charge-1);const auto before=short_store.retained();check(!short_store.pin_map_companion(0,payloads[0],error)&&short_store.retained()==before&&!short_store.exhausted(),"one-byte-short pin budget fails without publication or poison");}
  Store wire_probe;check(wire_probe.pin(wire_server,wire_client,wire_island,error),"wire budget probe");const auto wire_charge=wire_probe.retained().second-base;
  {Store shared(base+wire_charge+map_charge-1);check(shared.pin(wire_server,wire_client,wire_island,error),"wire fits shared budget");const auto before=shared.retained();check(!shared.pin_map_companion(0,payloads[0],error)&&shared.retained()==before,"map reservations use same remaining wire budget");}
  {Store bounded(base+map_charge+payloads[0].size()*5);check(bounded.pin_map_companion(0,payloads[0],error),"bounded pin");
    Asset* oldest=nullptr;for(int generation=0;generation<8;++generation){auto* asset=bounded.map_companion(map_companion_names[0],error,bounded.capture_generation(server_companion));if(!asset)break;if(!oldest)oldest=asset;check(bounded.retained().second<=base+map_charge+payloads[0].size()*5,"retention stays within budget");check(bounded.begin_generation(false),"unexhausted generation changes");}
    check(bounded.exhausted()&&oldest&&contents(oldest)==payloads[0],"budget exhaustion preserves earlier exposed bytes");
    const auto calls=allocation_calls.load();fail_all=true;auto* emergency=bounded.emergency_map_companion(aliases[0]);fail_all=false;
    check(emergency&&contents(emergency)==payloads[0]&&allocation_calls==calls,"exhaustion emergency pre-reserved exact bytes without allocation");
    check(!bounded.begin_generation(false)&&!bounded.begin_generation(true),"exhausted store refuses both future realms");
    check(!bounded.pin_map_companion(0,payloads[0],error),"idempotent pin cannot unpoison store");
    check(bounded.map_companion(map_companion_names[0],error,bounded.capture_generation(server_companion))==emergency,"fatal current generation keeps emergency identity");
  }
  {Store reserved;for(unsigned i=0;i<map_companion_names.size();++i)check(reserved.pin_map_companion(i,payloads[i],error),"all emergencies reserved");
    std::array<Asset*,map_companion_names.size()> emergency{};const auto calls=allocation_calls.load();fail_all=true;
    for(unsigned i=0;i<map_companion_names.size();++i)emergency[i]=reserved.emergency_map_companion(aliases[i]);
    auto* unknown=reserved.emergency_map_companion("unknown");fail_all=false;
    check(allocation_calls==calls&&!unknown,"all eight and unknown emergency paths allocate nothing");
    for(unsigned i=0;i<map_companion_names.size();++i){check(emergency[i]&&contents(emergency[i])==payloads[i],"every emergency keeps its own canonical object");for(unsigned j=0;j<i;++j)check(emergency[i]!=emergency[j],"emergency identities never alias other map slots");}
  }
  {Store count_bound;check(count_bound.pin_map_companion(0,payloads[0],error),"count exhaustion pin");
    for(int i=0;i<4096;++i)check(count_bound.retain("scripts/zm/synthetic_"+std::to_string(i)+".gsc",{1},error)!=nullptr,"generic retained entry budget slot");
    check(!count_bound.map_companion(map_companion_names[0],error,count_bound.capture_generation(server_companion))&&count_bound.exhausted(),"map clone obeys shared4096 exposed slot limit");
    fail_all=true;auto* emergency=count_bound.emergency_map_companion(map_companion_names[0]);fail_all=false;check(emergency&&contents(emergency)==payloads[0],"slot exhaustion emergency still available");
  }
  // Producer collision: fixed map names cannot inherit arbitrary cached bytes.
  for(unsigned i=0;i<map_companion_names.size();++i){
    Store store;auto lease=store.capture_generation(server_companion);
    auto* injected=store.retain(map_companion_names[i],{1,2,3},error,lease);
    check(!injected,"generic producer cannot preseed unpinned map name",true);
    check(!store.map_companion(map_companion_names[i],error,lease),"map lookup cannot return unpinned producer cache hit",true);
    check(store.pin_map_companion(i,payloads[i],error),"pin after rejected producer");
    auto* result=store.map_companion(map_companion_names[i],error,lease);
    check(result&&contents(result)==payloads[i],"map lookup gets pinned bytes despite prior producer attempt",true);
    Store pinned;check(pinned.pin_map_companion(i,payloads[i],error),"pinned collision setup");lease=pinned.capture_generation(server_companion);
    check(!pinned.retain(map_companion_names[i],{4,5,6},error,lease),"wrong producer bytes rejected after pin",true);
    auto* good=pinned.map_companion(map_companion_names[i],error,lease);check(good&&contents(good)==payloads[i],"postpin producer cannot replace immutable map bytes",true);
    Store wrong_pristine;check(wrong_pristine.pin_map_companion(i,payloads[i],error),"pristine collision setup");
    check(!wrong_pristine.retain(map_companion_names[i],bytes(payloads[i]),error,wrong_pristine.capture_generation(server_companion),std::vector<std::uint8_t>{9}),"producer cannot publish wrong immutable template",true);
  }
  std::cout<<"{\"checks\":"<<checks<<",\"failures\":"<<failures<<",\"collision_failures\":"<<collision_failures<<",\"injected_pin_failures\":"<<failed_pins<<",\"injected_lookup_failures\":"<<failed_lookups<<",\"scope\":\"map code store implementation and renamed own compiled metadata fixtures only; no engine/runtime/semantic proof\"}\n";
  return failures?1:0;
}
