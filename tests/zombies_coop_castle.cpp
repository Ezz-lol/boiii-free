// Synthetic objects only. No retail script bytes or native engine execution.
#include "../src/client/component/zombies/castle_selector.hpp"
#include "../src/client/component/zombies/schema_asset_store.hpp"
#include <atomic>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <new>
using namespace zombies::coop;
namespace d = castle_selector::detail;
static unsigned checks{};
static std::atomic_int fail_after{-1};
void* operator new(std::size_t n) {
  if (fail_after.load() >= 0 && fail_after.fetch_sub(1) == 0) throw std::bad_alloc();
  if (auto* p = std::malloc(n ? n : 1)) return p;
  throw std::bad_alloc();
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
static void check(bool value) { ++checks; assert(value); }
// Deterministic synthetic witness digest; production uses real SHA-256 and its
// fixed captured contract. This function is never supplied to production code.
static d::digest synthetic_hash(std::span<const std::uint8_t> bytes) {
  d::digest hash{}; std::uint32_t state = 0x811c9dc5;
  for (std::size_t i=0;i<bytes.size();++i) {
    state = (state ^ bytes[i]) * 0x1000193;
    hash[i % hash.size()] ^= static_cast<std::uint8_t>(state >> ((i % 4)*8));
  }
  return hash;
}
static std::vector<std::uint8_t> synthetic() {
  std::vector<std::uint8_t> b(0x300);
  std::copy(t7::object_magic.begin(),t7::object_magic.end(),b.begin());
  t7::write32(b,8,0x12345678);t7::write32(b,0x14,0x100);t7::write32(b,0x30,0x100);
  t7::write32(b,0x34,0x50);
  const std::string name="scripts/zm/__castle_import_synthetic.gsc";
  std::copy(name.begin(),name.end(),b.begin()+0x50);
  t7::write32(b,0x20,0x220);d::write16(b,0x3a,2);
  for (unsigned i=0;i<2;++i) {
    const auto p=0x220+20*i;t7::write32(b,p,0x34560000+i);t7::write32(b,p+4,0x100+0x80*i);
    t7::write32(b,p+8,i?0x87654321:d::target_function);t7::write32(b,p+12,0x98765432);b[p+16]=5;
  }
  t7::write32(b,0x24,0x260);d::write16(b,0x3c,1);
  t7::write32(b,0x260,d::fnv("get_all_closest"));t7::write32(b,0x264,d::fnv("array"));
  d::write16(b,0x268,2);b[0x26a]=5;b[0x26b]=2;
  t7::write32(b,0x26c,0x120);t7::write32(b,0x270,0x1b0);
  d::write16(b,0x120,0x888);d::write16(b,0x1b0,0x999);
  return b;
}
static d::contract contract(const std::vector<std::uint8_t>& b) {
  d::contract c{};c.independently_verified=true;c.logical_name="scripts/zm/__castle_import_synthetic.gsc";
  c.sha256=synthetic_hash(b);c.source_crc=0x12345678;c.code=0x100;c.code_size=0x100;
  c.owner={0x34560000,0x100,0x180,d::target_function,0x98765432,5,0};
  c.reference=0x120;c.opcode=0x888;c.site_parameter_byte=0;
  c.instruction.assign(b.begin()+0x120,b.begin()+0x130);return c;
}
int main() {
  using castle_selector::context;
  const context good{true,8,0,1,"custom_map","custom_map"};
  check(castle_selector::valid_context(good));
  for(int capacity=0;capacity<12;++capacity) {
    auto current=good;current.capacity=capacity;
    check(castle_selector::valid_context(current)==(capacity>=5 && capacity<=8));
  }
  for(const auto map:{"zm_castle","zm_zod","my_inherited_map","Mixed-Case_123"}) {
    auto current=good;current.current_map=map;current.frozen_map=map;check(castle_selector::valid_context(current));
  }
  for(const auto bad:{"","core_frontend","bad/map","bad map","../escape"}) {
    auto current=good;current.current_map=bad;current.frozen_map=bad;check(!castle_selector::valid_context(current));
  }
  const std::string long_map(65,'a');auto current=good;current.current_map=long_map;current.frozen_map=long_map;
  check(!castle_selector::valid_context(current));
  current=good;current.zombies=false;check(!castle_selector::valid_context(current));
  current=good;current.realm=1;check(!castle_selector::valid_context(current));
  current=good;current.realm=2;check(!castle_selector::valid_context(current));
  current=good;current.generation=0;check(!castle_selector::valid_context(current));
  current=good;current.frozen_map="other";check(!castle_selector::valid_context(current));
  current=good;check(castle_selector::same_context(good,current));
  ++current.generation;check(!castle_selector::same_context(good,current));
  current=good;current.capacity=7;check(!castle_selector::same_context(good,current));
  check(castle_selector::is_asset("SCRIPTS\\ZM\\ZM_CASTLE_VO.GSC"));
  for(const auto path:{"scripts/zm/zm_castle_vo.csc","scripts/zm/zm_castle_vo.gsc.bak","../scripts/zm/zm_castle_vo.gsc","scripts/mp/zm_castle_vo.gsc"})
    check(!castle_selector::is_asset(path));

  const auto input=synthetic();const auto c=contract(input);const auto result=d::transform(input,c,synthetic_hash);
  check(result.changed);check(t7::read32(result.bytes,8)==0x12345678);
  for(std::size_t i=0;i<input.size();++i)
    if(!((i>=0x0c&&i<0x10)||(i>=0x24&&i<0x28)||(i>=0x3c&&i<0x3e)||i==0x44)) check(input[i]==result.bytes[i]);
  const auto table=t7::read32(result.bytes,0x24);check(d::read16(result.bytes,0x3c)==2);
  check(t7::read32(result.bytes,table)==d::fnv("get_all_closest"));check(t7::read32(result.bytes,table+12)==0x1b0);
  check(t7::read32(result.bytes,table+16)==0x645eced3 && t7::read32(result.bytes,table+20)==0x33eb1daa);
  check(t7::read32(result.bytes,table+28)==0x120);
  check(!castle_selector::rewrite(input,synthetic_hash).changed); // synthetic data cannot enter fixed production contract
  auto changed=input;
  for(std::size_t i=0;i<input.size();++i) {changed[i]^=1;check(!d::transform(changed,c,synthetic_hash).changed);changed[i]^=1;}
  auto wrong=c;wrong.site_parameter_byte=5;check(!d::transform(input,wrong,synthetic_hash).changed);
  wrong=c;wrong.independently_verified=false;check(!d::transform(input,wrong,synthetic_hash).changed);
  auto bad=input;t7::write32(bad,0x270,0x122);wrong=c;wrong.sha256=synthetic_hash(bad);
  check(d::transform(bad,wrong,synthetic_hash).reason=="overlapping_reference_windows");
  bad=input;t7::write32(bad,0x270,0x140);wrong=c;wrong.sha256=synthetic_hash(bad);
  check(d::transform(bad,wrong,synthetic_hash).reason=="ambiguous_or_missing_target_import");
  bad=input;bad[0x26b]=0xff;wrong=c;wrong.sha256=synthetic_hash(bad);
  check(d::transform(bad,wrong,synthetic_hash).reason=="unknown_import_shape");

  struct asset{const char* name;int len;const char* buffer;};using store_type=t7::schema_asset_store<asset>;
  std::string error;store_type store;
  auto lease=store.capture_generation("scripts/zm/synthetic.gsc");
  auto* first=store.retain("scripts/zm/synthetic.gsc",result.bytes,error,lease,input);check(first);
  check(store.template_for(first)==std::optional<std::vector<std::uint8_t>>{input});
  const_cast<char*>(first->buffer)[0x128]=static_cast<char>(0xcc);
  check(store.begin_generation(false));
  bool stale_action=false;check(!store.if_current(lease,[&]{stale_action=true;}));check(!stale_action);
  check(!store.retain("scripts/zm/synthetic.gsc",result.bytes,error,lease,input));
  lease=store.capture_generation("scripts/zm/synthetic.gsc");
  auto* second=store.retain("scripts/zm/synthetic.gsc",result.bytes,error,lease,input);
  check(second && second!=first && second->buffer!=first->buffer);
  check(store.template_for(first)==std::optional<std::vector<std::uint8_t>>{input});
  check(static_cast<unsigned char>(first->buffer[0x128])==0xcc);
  check(store.cached("SCRIPTS\\ZM\\SYNTHETIC.GSC")==second);
  store_type defaults;auto* def=defaults.retain("scripts/zm/synthetic.gsc",result.bytes,error);check(def);
  check(defaults.template_for(def)==std::optional<std::vector<std::uint8_t>>{result.bytes});
  store_type charged;const auto baseline=charged.retained();check(charged.retain("scripts/zm/synthetic.gsc",result.bytes,error,{},input));
  const auto charge=charged.retained().second-baseline.second;
  store_type tight(baseline.second+charge-1);check(!tight.retain("scripts/zm/synthetic.gsc",result.bytes,error,{},input));
  check(tight.exhausted() && tight.retained()==baseline);
  for(unsigned fail_guard=1;fail_guard<=3;++fail_guard) {
    store_type guarded;const auto initial=guarded.retained();unsigned calls=0;
    check(!guarded.retain_if("scripts/zm/synthetic.gsc",result.bytes,error,{},input,[&]{return ++calls!=fail_guard;}));
    check(guarded.retained()==initial && !guarded.cached("scripts/zm/synthetic.gsc") && !guarded.exhausted());
  }
  {store_type guarded;const auto initial=guarded.retained();unsigned calls=0;bool threw=false;
    try{(void)guarded.retain_if("scripts/zm/synthetic.gsc",result.bytes,error,{},input,[&]{if(++calls==3)throw 1;return true;});}catch(int){threw=true;}
    check(threw && guarded.retained()==initial && !guarded.cached("scripts/zm/synthetic.gsc"));}
  unsigned allocation_failures=0,successes=0;
  for(int cutoff=0;cutoff<64;++cutoff) {
    store_type injected;const auto initial=injected.retained();bool failed=false;fail_after.store(cutoff);
    try{(void)injected.retain("scripts/zm/synthetic.gsc",result.bytes,error,{},input);}catch(const std::bad_alloc&){failed=true;}
    fail_after.store(-1);
    if(failed){++allocation_failures;check(injected.retained()==initial && !injected.cached("scripts/zm/synthetic.gsc"));}
    else{++successes;check(injected.retained().first==1 && injected.cached("scripts/zm/synthetic.gsc"));}
  }
  check(allocation_failures>0 && successes>0);
  std::cout<<"checks="<<checks<<" allocation_failures="<<allocation_failures<<" successes="<<successes<<'\n';
}
