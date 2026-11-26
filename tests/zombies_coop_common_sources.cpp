#include "common_helper_source_pair.hpp"
#include <cassert>
#include <barrier>
#include <thread>
#include <fstream>
#include <iostream>
#include <iterator>
namespace c = zombies::coop::common_helper;
namespace t7 = zombies::coop::t7;
using bytes = std::vector<std::uint8_t>;
static unsigned checks = 0;
static void check(bool b) { ++checks; assert(b); }
// Synthetic metadata fixture only. Production uses SHA-256; optional actual
// artifacts are SHA-256 checked independently by the Python runner first.
static bytes fixture_object, fixture_source;
static c::digest hash(std::span<const std::uint8_t> input) {
  if(input.size()==fixture_object.size()&&std::equal(input.begin(),input.end(),fixture_object.begin()))return c::cb_compiled_pin;
  if(input.size()==fixture_source.size()&&std::equal(input.begin(),input.end(),fixture_source.begin()))return c::source_pin;
  c::digest out{};std::uint32_t value=0x4b9ace2f;
  for(auto ch:input)value=(value^ch)*0x1000193;
  for(std::size_t i=0;i<out.size();++i){value=(value^static_cast<std::uint32_t>(i))*0x1000193;out[i]=static_cast<std::uint8_t>(value>>(i%4*8));}
  return out;
}
static bytes synthetic_object() {
 bytes out(512,0);std::copy(t7::object_magic.begin(),t7::object_magic.end(),out.begin());
 t7::write32(out,0x14,0x80);t7::write32(out,0x30,0x40);t7::write32(out,0x20,0x100);t7::write32(out,0x34,0x140);
 out[0x3a]=1;t7::write32(out,0x104,0x80);t7::write32(out,0x108,c::helper_name);t7::write32(out,0x10c,c::helper_space);out[0x110]=2;
 std::copy(c::logical_name.begin(),c::logical_name.end(),out.begin()+0x140);
 std::fill(out.begin()+0x80,out.begin()+0xc0,static_cast<std::uint8_t>(0xa5));return out;
}
static bytes read(const char* path) { std::ifstream stream(path,std::ios::binary); assert(stream); return {std::istreambuf_iterator<char>(stream),{}}; }
struct cb_asset { const char* name{}; int len{}; const char* buffer{}; };
struct ezz_gsc_obj { std::uint8_t bytes[0x48]; };
struct ezz_asset { const char* name{}; std::size_t len{}; ezz_gsc_obj* buffer{}; };
static_assert(sizeof(cb_asset)==0x18 && sizeof(ezz_asset)==0x18 && offsetof(cb_asset,buffer)==0x10 && offsetof(ezz_asset,buffer)==0x10);
template <class Asset> static void cases(const bytes& object,const bytes& source,c::family family) {
  using gate_type = c::lifecycle<Asset>; using store_type = t7::schema_asset_store<Asset>;
  static_assert(t7::companion_count == 3 && t7::retention_limit == 32*1024*1024);
  Asset winner{}; winner.name=c::logical_name.data();winner.len=static_cast<decltype(winner.len)>(object.size());winner.buffer=reinterpret_cast<decltype(winner.buffer)>(const_cast<std::uint8_t*>(object.data()));
  auto bound=c::bind_loader_object(family,&winner,object,source,source,true,hash);check(bound.has_value());
  check(bound->helper.name==c::helper_name&&bound->helper.params==2&&bound->helper.flags==0&&bound->helper.start<bound->helper.end);
  check(c::selected_winner_matches(*bound,&winner,hash));auto sealed=c::seal_selected_winner(*bound,&winner,hash);check(sealed.has_value());
  auto foreign=winner;check(!c::selected_winner_matches(*bound,&foreign,hash));
  foreign.buffer=nullptr;check(!c::selected_winner_matches(*bound,&foreign,hash));
  if(family==c::family::ezz){
    check(!c::bind_loader_object(family,&winner,object,source,source,false,hash));
    auto wrong=source;wrong[0]^=1;check(!c::bind_loader_object(family,&winner,object,wrong,source,true,hash));
    check(!c::bind_loader_object(family,&winner,object,source,wrong,true,hash));
  }
  check(!c::bind_loader_object(static_cast<c::family>(-1),&winner,object,source,source,true,hash));
  auto bad_seal=*sealed;bad_seal.bytes[bad_seal.bound.helper.start]^=1;check(!c::valid_seal(bad_seal,hash));
  // Once sealed under loader protection, later native source reads use owned
  // backing even if the original allocator no longer presents that tree.
  auto saved_buffer=winner.buffer;winner.buffer=nullptr;check(c::valid_seal(*sealed,hash));winner.buffer=saved_buffer;
  for(int capacity=5;capacity<=8;++capacity){
    store_type store;gate_type gate;auto lease=store.capture_generation(c::logical_name);c::native_publication publication;check(publication.publish(7,capacity,"zm_zod"));auto ticket=publication.capture();int native_epoch=7;typename gate_type::request req{lease,7,capacity,"zm_zod",ticket};
    bool native=true;auto current=[&]{return native&&publication.provenance_current(ticket,native_epoch,capacity,false);};std::string error;
    auto* common=gate.preregister(store,req,*sealed,hash,error,current);check(common!=nullptr);check(gate.lookup_common(store,req,current)==common);
    check(!gate.ready(store,req,current));
    auto* power=store.retain("scripts/zm/_zm_powerups.gsc",bytes(128,0x7f),error,lease);
    const auto pb=reinterpret_cast<std::uintptr_t>(power->buffer);
    std::vector<typename gate_type::range> ranges{{pb+8,pb+24},{pb+48,pb+80}};
    check(!gate.pair_transformed_powerup(store,req,power,{},current));
    check(!gate.pair_transformed_powerup(store,req,power,{{pb+8,pb+24},{pb+16,pb+40}},current));
    check(!gate.pair_transformed_powerup(store,req,power,{{pb,pb+200},{pb+48,pb+80}},current));
    check(gate.pair_transformed_powerup(store,req,power,ranges,current));check(!gate.ready(store,req,current));
    auto* entry=reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(common->buffer)+bound->helper.start);
    check(!gate.finish_loader(store,req,current));check(gate.note_common_load_returned(store,req,current));check(gate.finish_loader(store,req,current));check(!gate.ready(store,req,current));
    check(store.retain("scripts/zm/_zm_score.gsc",bytes(32,0x3f),error,lease)!=nullptr);
    check(gate.ready(store,req,current));
    check(gate.preregister(store,req,*sealed,hash,error,current)==common);
    check(gate.ready(store,req,current));
    auto successor_ticket=std::make_shared<const c::native_identity>(7,capacity,"zm_zod");
    auto successor=req;successor.session=successor_ticket;check(!gate.ready(store,successor,current));
    auto missing=req;missing.session.reset();check(!gate.preregister(store,missing,*sealed,hash,error,current));
    auto other=req;other.native_epoch++;check(!gate.ready(store,other,current));other=req;other.map="zm_tomb";check(!gate.ready(store,other,current));
    other=req;other.capacity=capacity==8?7:8;check(!gate.ready(store,other,current));native=false;check(!gate.ready(store,req,current));native=true;
    unsigned revoked=0,normal_detours=0;
    check(!gate.observe_detour(store,1,current,[&](int){++revoked;}));++normal_detours;check(revoked==0&&gate.ready(store,req,current));
    check(gate.observe_detour(store,reinterpret_cast<std::uintptr_t>(entry),current,[&](int epoch){check(epoch==7);++revoked;}));
    ++normal_detours;check(normal_detours==2&&revoked==1&&!gate.ready(store,req,current));
    check(!gate.preregister(store,req,*sealed,hash,error,current));
    check(!gate.ready(store,req,current));check(gate.lookup_common(store,req,current)==common);check(gate.lookup_powerup(store,req,current)==power);
    const auto old=bytes(reinterpret_cast<const std::uint8_t*>(common->buffer),reinterpret_cast<const std::uint8_t*>(common->buffer)+common->len);
    check(store.begin_generation(false));auto fresh=store.capture_generation(c::logical_name);
    check(publication.publish(8,capacity,"zm_zod"));ticket=publication.capture();native_epoch=8;typename gate_type::request next{fresh,8,capacity,"zm_zod",ticket};
    auto* next_common=gate.preregister(store,next,*sealed,hash,error,current);check(next_common&&next_common!=common);
    check(bytes(reinterpret_cast<const std::uint8_t*>(common->buffer),reinterpret_cast<const std::uint8_t*>(common->buffer)+common->len)==old);
    check(!gate.preregister(store,req,*sealed,hash,error,current));
    check(!gate.ready(store,req,current)&&!gate.ready(store,next,current));
    check(!gate.observe_detour(store,reinterpret_cast<std::uintptr_t>(entry),current,[&](int){assert(false);}));
  }
  // A protected common detour BEFORE the stock powerup module is loaded must
  // revoke this lease too; missing pair is never permission to forget detours.
  {store_type store;gate_type gate;c::native_publication pub;check(pub.publish(7,8,"zm_zod"));auto ticket=pub.capture();auto current=[&]{return pub.provenance_current(ticket,7,8,false);};auto lease=store.capture_generation(c::logical_name);typename gate_type::request req{lease,7,8,"zm_zod",ticket};std::string error;
   auto* common=gate.preregister(store,req,*sealed,hash,error,current);check(common!=nullptr);unsigned revoked=0;
   check(gate.observe_detour(store,reinterpret_cast<std::uintptr_t>(common->buffer)+bound->helper.start,current,[&](int e){check(e==7);++revoked;}));
   check(revoked==1&&!gate.preregister(store,req,*sealed,hash,error,current));}

  // Entry barrier is outside lifecycle/store locks. Duplicate receipt work must
  // serialize without a false decline, lost load flag or revoked current proof.
  for(unsigned iteration=0;iteration<16;++iteration){
    store_type store;gate_type gate;c::native_publication pub;check(pub.publish(7,8,"zm_zod"));auto ticket=pub.capture();auto current=[&]{return pub.provenance_current(ticket,7,8,false);};
    auto lease=store.capture_generation(c::logical_name);typename gate_type::request req{lease,7,8,"zm_zod",ticket};std::string error;
    check(gate.preregister(store,req,*sealed,hash,error,current)!=nullptr);
    auto* power=store.retain("scripts/zm/_zm_powerups.gsc",bytes(128,0x7f),error,lease);auto pb=reinterpret_cast<std::uintptr_t>(power->buffer);
    std::barrier start(3);bool results[2]{};
    const auto work=[&](unsigned index){start.arrive_and_wait();results[index]=gate.pair_transformed_powerup(store,req,power,{{pb+8,pb+24},{pb+48,pb+80}},current);};
    std::thread a(work,0u),b(work,1u);start.arrive_and_wait();a.join();b.join();check(results[0]&&results[1]);
    check(gate.note_common_load_returned(store,req,current));check(gate.finish_loader(store,req,current));check(store.retain("scripts/zm/_zm_score.gsc",bytes(32,0x3f),error,lease)!=nullptr);check(gate.ready(store,req,current));
  }

  for(const int capacity : {0,1,4,9}){
    store_type store;gate_type gate;auto lease=store.capture_generation(c::logical_name);typename gate_type::request req{lease,7,capacity,"zm_zod",std::make_shared<const c::native_identity>(7,8,"zm_zod")};std::string error;
    check(!gate.preregister(store,req,*sealed,hash,error,[]{return true;}));
  }
  {store_type store(1);gate_type gate;auto lease=store.capture_generation(c::logical_name);typename gate_type::request req{lease,7,8,"zm_zod",std::make_shared<const c::native_identity>(7,8,"zm_zod")};std::string error;
   check(!gate.preregister(store,req,*sealed,hash,error,[]{return true;}));check(store.exhausted());}
}
int main(int argc,char**argv){assert(argc==1||argc==3);
 fixture_object=argc==3?read(argv[1]):synthetic_object();
 const std::string fixture_text="known-readable-compiler-source-fixture";
 fixture_source=argc==3?read(argv[2]):bytes(fixture_text.begin(),fixture_text.end());
 const auto object=fixture_object,source=fixture_source;
 check(hash(object)==c::cb_compiled_pin);check(hash(source)==c::source_pin);
 cases<cb_asset>(object,source,c::family::cb);cases<ezz_asset>(object,source,c::family::ezz);
 std::cout<<"checks="<<checks<<" metadata_fixture=true optional_actual_artifacts=runner_verified_if_supplied ezz_compiler_binding=simulated linked_body_authenticated=false\n";
}
