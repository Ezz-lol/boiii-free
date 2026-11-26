// Synthetic tests contain no retail bytes. Optional private Windows fixture mode.
#if defined(BOIII_SCORE_PRIVATE_TEST)
#include "score_events.hpp"
#else
#include "../src/client/component/zombies/score_events.hpp"
#endif
#include <atomic>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <new>
#include <stdexcept>
#ifdef BOIII_SCORE_RETAIL_TEST
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#endif
using namespace zombies::coop;
namespace d = score_events::detail;
static unsigned checks{}, allocation_failures{};
static std::atomic_int fail_after{-1};
void* operator new(std::size_t n) {
  if (fail_after.load() >= 0 && fail_after.fetch_sub(1) == 0) throw std::bad_alloc();
  if (auto* p=std::malloc(n?n:1)) return p;
  throw std::bad_alloc();
}
void operator delete(void* p) noexcept {std::free(p);}
void operator delete(void* p,std::size_t) noexcept {std::free(p);}
static void check(bool value) {++checks;if(!value)throw std::runtime_error("score-event structural test failed");}
static d::digest witness(std::span<const std::uint8_t> b) {
 d::digest h{};std::uint32_t v=0x811c9dc5;
 for(std::size_t i=0;i<b.size();++i){v=(v^b[i])*0x1000193;h[i%32]^=static_cast<std::uint8_t>(v>>((i%4)*8));}return h;
}
static std::vector<std::uint8_t> synthetic() {
 std::vector<std::uint8_t> b(0x340);std::copy(t7::object_magic.begin(),t7::object_magic.end(),b.begin());
 t7::write32(b,8,0x12345678);t7::write32(b,0x14,0x100);t7::write32(b,0x30,0x100);t7::write32(b,0x34,0x50);
 const std::string name="scripts/zm/__score_events_synthetic.gsc";std::copy(name.begin(),name.end(),b.begin()+0x50);
 t7::write32(b,0x20,0x220);d::write16(b,0x3a,2);
 for(unsigned i=0;i<2;++i){auto p=0x220+20*i;t7::write32(b,p,0x34560000+i);t7::write32(b,p+4,0x100+0x80*i);t7::write32(b,p+8,i?0x87654321:d::target_function);t7::write32(b,p+12,0x98765432);b[p+16]=2;}
 t7::write32(b,0x24,0x260);d::write16(b,0x3c,2);
 t7::write32(b,0x260,d::fnv("increment_uimodel"));t7::write32(b,0x264,d::fnv("clientfield"));d::write16(b,0x268,1);b[0x26a]=1;b[0x26b]=4;t7::write32(b,0x26c,0x120);
 t7::write32(b,0x270,0xaaaabbbb);t7::write32(b,0x274,0xccccdddd);d::write16(b,0x278,1);b[0x27a]=0;b[0x27b]=0x24;t7::write32(b,0x27c,0x1b0);
 d::write16(b,0x120,0x888);d::write16(b,0x1b0,0x999);return b;
}
static d::contract fixture_contract(const std::vector<std::uint8_t>& b) {
 d::contract c{};c.independently_verified=true;c.logical_name="scripts/zm/__score_events_synthetic.gsc";c.sha256=witness(b);c.source_crc=0x12345678;c.code=0x100;c.code_size=0x100;
 c.owner={0x34560000,0x100,0x180,d::target_function,0x98765432,2,0};c.reference=0x120;c.opcode=0x888;c.site_parameter_byte=0;c.instruction.assign(b.begin()+0x120,b.begin()+0x130);
 c.original_name=d::fnv("increment_uimodel");c.original_space=d::fnv("clientfield");c.destination_name=d::fnv("score_event_increment_uimodel");c.destination_space=d::fnv("eroded_coop_fields");c.import_params=1;c.import_flags=4;return c;
}
static void preservation(const std::vector<std::uint8_t>& b,const std::vector<std::uint8_t>& transformed,std::size_t row) {
 check(transformed.size()>=b.size());
 for(std::size_t i=0;i<b.size();++i)if(!((i>=0xc&&i<0x10)||i==0x44||(i>=row&&i<row+8)))check(b[i]==transformed[i]);
 check(t7::read32(b,8)==t7::read32(transformed,8));check(t7::read32(b,0x24)==t7::read32(transformed,0x24));check(d::read16(b,0x3c)==d::read16(transformed,0x3c));
 auto code=t7::read32(b,0x14),size=t7::read32(b,0x30);check(std::equal(b.begin()+code,b.begin()+code+size,transformed.begin()+code));
 auto table=t7::read32(transformed,0xc);check(transformed[0x44]==b[0x44]+1);check(table>=b.size());
 for(unsigned i=0;i<b[0x44];++i)check(t7::read32(b,t7::read32(b,0xc)+i*4)==t7::read32(transformed,table+i*4));
 const auto dep=t7::bounded_name(transformed,t7::read32(transformed,table+b[0x44]*4));check(dep&&*dep=="scripts/zm/eroded_coop_fields");
}
static void synthetic_tests() {
 const auto b=synthetic();const auto c=fixture_contract(b);auto r=d::transform(b,c,witness);check(r.changed);check(r.reason=="score_import_redirected");preservation(b,r.bytes,0x260);check(t7::read32(r.bytes,0x260)==0xb7cb798e);check(t7::read32(r.bytes,0x264)==0x33eb1daa);check(r.bytes[0x26a]==1&&r.bytes[0x26b]==4);
 auto bad=c;bad.independently_verified=false;check(!d::transform(b,bad,witness).changed);bad=c;bad.sha256={};check(!d::transform(b,bad,witness).changed);check(!d::transform(b,c,nullptr).changed);
 bad=c;bad.owner.checksum++;check(!d::transform(b,bad,witness).changed);bad=c;bad.reference++;check(!d::transform(b,bad,witness).changed);bad=c;bad.reference=0xffffffff;check(!d::transform(b,bad,witness).changed);
 bad=c;bad.instruction.pop_back();check(!d::transform(b,bad,witness).changed);bad=c;bad.opcode++;check(!d::transform(b,bad,witness).changed);bad=c;bad.site_parameter_byte=1;check(!d::transform(b,bad,witness).changed);
 auto declined=[&](auto change){auto m=b;change(m);auto mc=c;mc.sha256=witness(m);const auto before=m;auto q=d::transform(m,mc,witness);check(!q.changed);check(q.bytes.empty());check(m==before);};
 declined([](auto&m){m[0]^=1;});declined([](auto&m){t7::write32(m,8,3);});declined([](auto&m){t7::write32(m,0x14,0xffffffff);});declined([](auto&m){t7::write32(m,0x30,0xffffffff);});declined([](auto&m){t7::write32(m,0x34,0xffffffff);});declined([](auto&m){m[0x50]='X';});
 declined([](auto&m){t7::write32(m,0x20,0x100);});declined([](auto&m){d::write16(m,0x3a,4097);});declined([](auto&m){t7::write32(m,0x238,0x100);});
 declined([](auto&m){t7::write32(m,0x24,0xffffffff);});declined([](auto&m){d::write16(m,0x3c,0);});declined([](auto&m){d::write16(m,0x3c,4097);});declined([](auto&m){d::write16(m,0x268,0);});
 declined([](auto&m){m[0x26a]=0;});declined([](auto&m){m[0x26b]=5;});declined([](auto&m){m[0x26b]=0;});declined([](auto&m){m[0x26b]=0xc4;});declined([](auto&m){m[0x26a]=1;m[0x26b]=1;});
 declined([](auto&m){t7::write32(m,0x27c,0x120);});declined([](auto&m){t7::write32(m,0x27c,0x124);});declined([](auto&m){t7::write32(m,0x26c,0x121);});declined([](auto&m){t7::write32(m,0x26c,0xfe);});declined([](auto&m){t7::write32(m,0x26c,0x1fe);});declined([](auto&m){t7::write32(m,0x26c,0x130);});
 declined([](auto&m){t7::write32(m,0x270,0xb7cb798e);t7::write32(m,0x274,0x33eb1daa);});declined([](auto&m){m[0x44]=1;t7::write32(m,0xc,0x100);});
 declined([](auto&m){t7::write32(m,0x34,0x180);const std::string n="scripts/zm/__score_events_synthetic.gsc";std::copy(n.begin(),n.end(),m.begin()+0x180);});
 // A previously shared target row cannot silently redirect another owner.
 declined([](auto&m){d::write16(m,0x3c,1);d::write16(m,0x268,2);t7::write32(m,0x270,0x1b0);});
 for(std::size_t n=0;n<b.size();++n){std::vector<std::uint8_t> short_b(b.begin(),b.begin()+n);check(!d::transform(short_b,c,witness).changed);}
 for(std::size_t i=0;i<b.size();++i){auto m=b;m[i]^=1;check(!d::transform(m,c,witness).changed);}
 auto included=t7::clone_with_include(b,"scripts/zm/eroded_coop_fields");check(included.result==t7::include_result::inserted);auto ic=c;ic.sha256=witness(included.bytes);auto ir=d::transform(included.bytes,ic,witness);check(ir.changed);check(ir.bytes.size()==included.bytes.size());
 for(std::size_t i=0;i<included.bytes.size();++i)if(!(i>=0x260&&i<0x268))check(included.bytes[i]==ir.bytes[i]);
 for(int cutoff=0;cutoff<96;++cutoff){fail_after=cutoff;try{auto q=d::transform(b,c,witness);fail_after=-1;check(q.changed);}catch(const std::bad_alloc&){fail_after=-1;++allocation_failures;}check(b==synthetic());}check(allocation_failures>5);
 check(score_events::is_asset("SCRIPTS\\ZM\\_ZM_SCORE.GSC"));for(auto name:{"scripts/zm/_zm_score.csc","scripts/zm/_zm_score.gsc.bak","../scripts/zm/_zm_score.gsc","scripts/mp/_zm_score.gsc"})check(!score_events::is_asset(name));
}
#ifdef BOIII_SCORE_RETAIL_TEST
static d::digest real_sha(std::span<const std::uint8_t> b) {
 BCRYPT_ALG_HANDLE alg{};BCRYPT_HASH_HANDLE h{};DWORD len{},used{};d::digest digest{};
 if(BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)<0)throw std::runtime_error("sha provider");
 if(BCryptGetProperty(alg,BCRYPT_OBJECT_LENGTH,reinterpret_cast<PUCHAR>(&len),sizeof(len),&used,0)<0)throw std::runtime_error("sha size");
 std::vector<std::uint8_t> obj(len);if(BCryptCreateHash(alg,&h,obj.data(),len,nullptr,0,0)<0)throw std::runtime_error("sha create");
 if(BCryptHashData(h,const_cast<PUCHAR>(b.data()),static_cast<ULONG>(b.size()),0)<0||BCryptFinishHash(h,digest.data(),static_cast<ULONG>(digest.size()),0)<0)throw std::runtime_error("sha data");
 BCryptDestroyHash(h);BCryptCloseAlgorithmProvider(alg,0);return digest;
}
static void retail_tests(const char* path,const char* output) {
 std::ifstream file(path,std::ios::binary);std::vector<std::uint8_t>b{std::istreambuf_iterator<char>(file),{}};check(b.size()==10998);const auto original=b;const auto c=d::captured_contract();check(real_sha(b)==c.sha256);
 const auto r=score_events::rewrite(b,real_sha);check(r.changed);preservation(b,r.bytes,8798);check(b==original);check(t7::read32(r.bytes,8798)==0xb7cb798e&&t7::read32(r.bytes,8802)==0x33eb1daa);check(r.bytes[8808]==1&&r.bytes[8809]==4);check(t7::read32(r.bytes,8810)==0xc22);
 check(std::equal(b.begin()+0xc22,b.begin()+0xc30,r.bytes.begin()+0xc22));check(d::read16(r.bytes,0xc22)==0x1d3e&&r.bytes[0xc24]==0);
 auto exports=t7::read32(b,0x20);check(std::equal(b.begin()+exports,b.begin()+exports+22*20,r.bytes.begin()+exports));check(d::read16(r.bytes,0x3c)==86);
 for(std::size_t i=0;i<b.size();++i){auto m=b;m[i]^=1;auto q=score_events::rewrite(m,real_sha);check(!q.changed);check(q.bytes.empty());}
 for(std::size_t n=0;n<b.size();++n)check(!score_events::rewrite(std::span(b).first(n),real_sha).changed);
 auto repeated=score_events::rewrite(r.bytes,real_sha);check(!repeated.changed);check(repeated.bytes.empty());
 std::ofstream saved(output,std::ios::binary);saved.write(reinterpret_cast<const char*>(r.bytes.data()),static_cast<std::streamsize>(r.bytes.size()));check(saved.good());
}
#endif
int main(int argc,char**argv){synthetic_tests();
#ifdef BOIII_SCORE_RETAIL_TEST
 if(argc!=3)throw std::runtime_error("private exact asset and output required");retail_tests(argv[1],argv[2]);
#else
 (void)argc;(void)argv;
#endif
 std::cout<<"checks="<<checks<<" allocation_failures="<<allocation_failures<<"\n";
}
