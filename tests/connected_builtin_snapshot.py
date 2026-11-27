"""Compile actual readonly builtin snapshot header with a synthetic memory reader.

Requires Python3.9+ and configured C++20 compiler. --compiler/CXX and
--output-dir are supported; no SDK setup/downloads/cleanup or engine execution.
"""
from pathlib import Path
import argparse,datetime,hashlib,json,os,subprocess,tempfile
ROOT=Path(__file__).resolve().parents[1]
FAMILY="ezz"
CPP=r'''

#include <component/zombies/builtin_snapshot.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <string>
#include <thread>
#include <vector>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
static_assert(offsetof(IMAGE_DOS_HEADER,e_lfanew)==60);
static_assert(sizeof(IMAGE_NT_HEADERS64)==264);
static_assert(offsetof(IMAGE_NT_HEADERS64,FileHeader)+offsetof(IMAGE_FILE_HEADER,Machine)==4);
static_assert(offsetof(IMAGE_NT_HEADERS64,FileHeader)+offsetof(IMAGE_FILE_HEADER,NumberOfSections)==6);
static_assert(offsetof(IMAGE_NT_HEADERS64,FileHeader)+offsetof(IMAGE_FILE_HEADER,SizeOfOptionalHeader)==20);
static_assert(offsetof(IMAGE_NT_HEADERS64,OptionalHeader)==24);
static_assert(offsetof(IMAGE_NT_HEADERS64,OptionalHeader)+offsetof(IMAGE_OPTIONAL_HEADER64,ImageBase)==48);
static_assert(offsetof(IMAGE_NT_HEADERS64,OptionalHeader)+offsetof(IMAGE_OPTIONAL_HEADER64,SizeOfImage)==80);
static_assert(offsetof(IMAGE_NT_HEADERS64,OptionalHeader)+offsetof(IMAGE_OPTIONAL_HEADER64,CheckSum)==88);
static_assert(sizeof(IMAGE_SECTION_HEADER)==40&&offsetof(IMAGE_SECTION_HEADER,Misc)==8);
static_assert(offsetof(IMAGE_SECTION_HEADER,VirtualAddress)==12&&offsetof(IMAGE_SECTION_HEADER,Characteristics)==36);
#endif
namespace b=zombies::coop::builtin_snapshot;
unsigned checks=0;
void expect(bool value){if(!value){std::fputs("snapshot case FAILED\n",stderr);std::abort();}++checks;}
struct memory {
 std::uintptr_t base=b::preferred_base;
 std::vector<std::uint8_t> bytes=std::vector<std::uint8_t>(0x1200000);
 std::vector<b::memory_region> regions;
 std::function<void(std::uintptr_t,b::memory_region&)> alter;
 std::uintptr_t query_failure{},copy_failure{};
 unsigned queries=0,copies=0;std::size_t maximum_copy=0,last_length=0;std::uintptr_t last_address=0;
 memory(){regions.push_back({base,bytes.size(),base,true,true,true,false});}
 bool query(std::uintptr_t address,b::memory_region& result){
  ++queries;if(address==query_failure)return false;
  for(auto region:regions){if(address>=region.base&&address-region.base<region.size){result=region;if(alter)alter(address,result);return true;}}
  return false;
 }
 bool copy(std::uintptr_t address,void*dest,std::size_t length){
  ++copies;maximum_copy=std::max(maximum_copy,length);last_address=address;last_length=length;
  if(address==copy_failure||address<base||length>bytes.size()||address-base>bytes.size()-length)return false;
  std::memcpy(dest,bytes.data()+address-base,length);return true;
 }
 template<class T>void put(std::size_t at,T value){expect(at+sizeof(value)<=bytes.size());std::memcpy(bytes.data()+at,&value,sizeof(value));}
};
constexpr std::size_t fixture_nt=0x100,fixture_section=fixture_nt+24+240;
constexpr std::size_t table=b::common_table-b::preferred_base+b::expected_index*sizeof(b::builtin_entry);
constexpr std::size_t action=0x2CBA00;
memory image(std::uintptr_t base=b::preferred_base){
 memory m;m.base=base;m.regions={{base,m.bytes.size(),base,true,true,true,false}};
 m.put<std::uint16_t>(0,0x5A4D);m.put<std::uint32_t>(60,fixture_nt);
 m.put<std::uint32_t>(fixture_nt,0x4550);m.put<std::uint16_t>(fixture_nt+4,0x8664);m.put<std::uint16_t>(fixture_nt+6,1);
 m.put<std::uint16_t>(fixture_nt+20,240);m.put<std::uint16_t>(fixture_nt+24,0x20B);
 m.put<std::uint64_t>(fixture_nt+48,b::preferred_base);m.put<std::uint32_t>(fixture_nt+80,static_cast<std::uint32_t>(m.bytes.size()));
 m.put<std::uint32_t>(fixture_nt+88,b::dedicated_checksum);
 b::builtin_entry expected{b::expected_hash,0,0,0,base+b::expected_action-b::preferred_base,0,0};
 b::builtin_entry connected{b::connected_hash,0,0,0,base+action,0,0};
 m.put(table,expected);m.put(table+sizeof(expected),connected);
 m.put<std::uint32_t>(fixture_section+8,0x100000);m.put<std::uint32_t>(fixture_section+12,0x200000);
 m.put<std::uint32_t>(fixture_section+36,0x20000000);
 for(std::size_t i=0;i<b::code_limit;++i)m.bytes[action+i]=static_cast<std::uint8_t>(i);
 return m;
}
void failed(memory&m,const char*reason){auto s=b::capture(m,m.base);expect(!s.valid&&s.code_size==0);expect(std::strcmp(s.reason,reason)==0);}
int main(){
 static_assert(sizeof(b::builtin_entry)==32&&offsetof(b::builtin_entry,action)==16);
 static_assert(b::connected_hash==0x6C6E7B10&&b::expected_hash==0x4441FF8D&&b::code_limit==256);
 const auto limit=(std::numeric_limits<std::uintptr_t>::max)();
 expect(b::range_contains(100,100,100,1));expect(b::range_contains(100,100,199,1));
 expect(!b::range_contains(100,100,200,1));expect(!b::range_contains(100,100,99,1));
 expect(!b::range_contains(100,100,100,0));expect(!b::range_contains(limit-8,16,limit-8,1));
 expect(!b::range_contains(100,100,100,(std::numeric_limits<std::size_t>::max)()));
 std::array<std::uint8_t,256> destination{};
 {
  auto m=image();auto original=m.bytes;auto s=b::capture(m,m.base);
  expect(s.valid&&std::strcmp(s.reason,"ok")==0&&s.code_size==256);
  expect(s.entry_address==m.base+table+32&&s.action_address==m.base+action);
  expect(s.connected.canonical==b::connected_hash&&s.connected.minimum==0&&s.connected.maximum==0&&s.connected.type==0);
  expect(m.copies==5&&m.last_address==m.base+action&&m.last_length==256&&m.maximum_copy==264);
  expect(std::equal(s.code.begin(),s.code.end(),original.begin()+action));expect(m.bytes==original);
  std::array<char,b::json_limit> text{};
  int length=b::format(s,"ezz",text.data(),text.size());expect(length>0&&length<static_cast<int>(text.size()));
  std::fputs(text.data(),stdout);
  expect(b::format(s,"other",text.data(),text.size())==-1);
  expect(b::format(s,nullptr,text.data(),text.size())==-1);expect(b::format(s,"ezz",nullptr,text.size())==-1);
  expect(b::format(s,"ezz",text.data(),0)==-1);expect(b::format(s,"ezz",text.data(),b::json_limit+1)==-1);
  std::array<char,8> fixture_small{};fixture_small.fill('x');expect(b::format(s,"ezz",fixture_small.data()+1,4)==-1);expect(fixture_small[0]=='x'&&fixture_small[5]=='x'&&fixture_small[4]=='\0');
  s.code_size=257;expect(b::format(s,"ezz",text.data(),text.size())==-1);
 }
 // A zero-argument call remains valid with max1 in either registration. These
 // fixtures capture metadata only and never invoke either argument variant.
 for(unsigned variant=1;variant<4;++variant){auto m=image();
  if(variant&1)m.put<std::uint32_t>(table+8,1);
  if(variant&2)m.put<std::uint32_t>(table+32+8,1);
  auto s=b::capture(m,m.base);expect(s.valid&&s.expected.minimum==0&&s.connected.minimum==0);
  expect(s.expected.maximum==((variant&1)?1u:0u)&&s.connected.maximum==((variant&2)?1u:0u));
  std::array<char,b::json_limit> text{};expect(b::format(s,"ezz",text.data(),text.size())>0);std::fputs(text.data(),stdout);
 }
 {auto m=image(b::preferred_base+0x10000000);auto s=b::capture(m,m.base);expect(s.valid&&s.action_address==m.base+action);}
 {auto m=image();expect(!b::capture(m,0).valid);expect(m.queries==0&&m.copies==0);}
 for(unsigned variant=0;variant<3;++variant){auto m=image();if(variant==0)m.put<std::uint16_t>(0,0);else m.put<std::uint32_t>(60,variant==1?63:4096);failed(m,"invalid_dos_header");}
 for(unsigned variant=0;variant<8;++variant){auto m=image();switch(variant){
  case 0:m.put<std::uint32_t>(fixture_nt,0);break;case 1:m.put<std::uint16_t>(fixture_nt+4,0x14c);break;
  case 2:m.put<std::uint16_t>(fixture_nt+24,0x10b);break;case 3:m.put<std::uint16_t>(fixture_nt+20,239);break;
  case 4:m.put<std::uint64_t>(fixture_nt+48,0);break;case 5:m.put<std::uint16_t>(fixture_nt+6,0);break;
  case 6:m.put<std::uint16_t>(fixture_nt+6,97);break;case 7:m.put<std::uint32_t>(fixture_nt+80,4095);break;}
  failed(m,"invalid_pe_header");}
 {auto m=image();m.put<std::uint32_t>(fixture_nt+88,0);failed(m,"unsupported_checksum");}
 {auto m=image();m.put<std::uint32_t>(fixture_nt+80,0x10000);failed(m,"table_outside_image");}
 {auto m=image();m.query_failure=m.base+table;failed(m,"table_unreadable");expect(m.copies==2);}
 for(unsigned variant=0;variant<9;++variant){auto m=image();switch(variant){
  case 0:m.put<std::uint32_t>(table,0);break;case 1:m.put<std::uint32_t>(table+32,0);break;
  case 2:m.put<std::uint32_t>(table+4,1);break;case 3:m.put<std::uint32_t>(table+32+8,2);break;
  case 4:m.put<std::uint32_t>(table+24,1);break;case 5:m.put<std::uint32_t>(table+32+24,1);break;
  case 6:m.put<std::uint64_t>(table+16,m.base+action);break;case 7:m.put<std::uint32_t>(table+32+4,1);break;
  case 8:m.put<std::uint32_t>(table+8,2);break;}
  failed(m,"table_identity_mismatch");}
 {auto m=image();m.put<std::uint64_t>(table+32+16,m.base+m.bytes.size()-255);failed(m,"action_outside_image");}
 {auto m=image();m.put<std::uint32_t>(fixture_section+36,0);failed(m,"action_not_executable_section");}
 {auto m=image();m.put<std::uint32_t>(fixture_section+8,1);failed(m,"action_not_executable_section");}
 {auto m=image();m.query_failure=m.base+fixture_section;failed(m,"section_unreadable");}
 {auto m=image();m.copy_failure=m.base+action;failed(m,"action_unreadable");expect(m.last_length==256);}
 for(unsigned variant=0;variant<5;++variant){auto m=image();m.alter=[=](std::uintptr_t address,b::memory_region&r){if(address!=b::preferred_base+action)return;switch(variant){
  case 0:r.committed=false;break;case 1:r.readable=false;break;case 2:r.executable=false;break;case 3:r.guarded=true;break;case 4:r.allocation_base+=1;break;}};
  failed(m,"action_unreadable");expect(m.copies==4);}
 {auto m=image();expect(!b::read_checked(m,m.base,m.bytes.size(),m.base,nullptr,1,false));expect(m.queries==0);}
 {auto m=image();m.regions[0].executable=false;expect(b::read_checked(m,m.base,m.bytes.size(),m.base,destination.data(),8,false));auto count=m.copies;expect(!b::read_checked(m,m.base,m.bytes.size(),m.base,destination.data(),8,true));expect(m.copies==count);}
 {auto m=image();m.regions={{m.base,32,m.base,true,true,true,false},{m.base+33,32,m.base,true,true,true,false}};
  expect(!b::read_checked(m,m.base,m.bytes.size(),m.base,destination.data(),64,false));expect(m.copies==0);}
 for(unsigned regions:{8u,9u}){auto m=image();m.regions.clear();for(unsigned i=0;i<regions;++i)m.regions.push_back({m.base+i*16,16,m.base,true,true,true,false});
  bool valid=b::read_checked(m,m.base,m.bytes.size(),m.base,destination.data(),regions*16,false);expect(valid==(regions==8));expect(m.copies==(regions==8?1u:0u));}
 {auto m=image();m.alter=[&](std::uintptr_t,b::memory_region&r){r.base=m.base+1;};expect(!b::read_checked(m,m.base,m.bytes.size(),m.base,destination.data(),8,false));expect(m.copies==0);}
 {auto m=image();m.alter=[=](std::uintptr_t,b::memory_region&r){r.base=limit-8;r.size=16;};expect(!b::read_checked(m,m.base,m.bytes.size(),m.base,destination.data(),8,false));expect(m.copies==0);}
 {b::attempt_budget budget;expect(budget.claim()==1);expect(budget.claim()==0);budget.finish(false);expect(budget.claim()==2);budget.finish(false);expect(budget.claim()==0);}
 {b::attempt_budget budget;expect(budget.claim()==1);budget.finish(true);expect(budget.claim()==0);}
 {b::attempt_budget budget;expect(budget.claim()==1);std::atomic<unsigned>claimed{0};std::vector<std::thread>threads;
  for(unsigned i=0;i<8;++i)threads.emplace_back([&]{if(budget.claim())claimed.fetch_add(1);});for(auto&t:threads)t.join();expect(claimed.load()==0);budget.finish(true);expect(budget.claim()==0);}
 std::printf("actual builtin_snapshot header PASS %u checks; synthetic reader only, no builtin invocation\n",checks);
}
'''


def main():
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument("--compiler",default=os.environ.get("CXX","cl"))
 parser.add_argument("--output-dir",type=Path,default=Path(tempfile.gettempdir())/"boiii-builtin-snapshot-tests")
 args=parser.parse_args();stamp=datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%S%fZ")
 out=args.output_dir.resolve()/("connected-builtin-snapshot-"+stamp);out.mkdir(parents=True,exist_ok=False)
 header=ROOT/"src/client/component/zombies/builtin_snapshot.hpp"
 source=out/"connected_builtin_snapshot.cpp";source.write_text(CPP,encoding="utf-8")
 executable=out/("connected_builtin_snapshot"+(".exe" if os.name=="nt" else ""))
 if Path(args.compiler).name.lower() in ("cl","cl.exe","clang-cl","clang-cl.exe"):
  command=[args.compiler,"/nologo","/std:c++20","/EHsc","/MT","/W4","/WX","/I"+str(ROOT/"src/client"),"/Fe:"+str(executable),"/Fo:"+str(out/"connected_builtin_snapshot.obj"),str(source)]
 else:command=[args.compiler,"-std=c++20","-Wall","-Wextra","-Werror","-pthread","-I"+str(ROOT/"src/client"),str(source),"-o",str(executable)]
 with (out/"checks.log").open("x",encoding="utf-8") as log:
  log.write(json.dumps({"compile":command})+"\n");log.flush();subprocess.run(command,cwd=out,stdout=log,stderr=log,check=True)
  run=subprocess.run([str(executable)],cwd=out,stdout=subprocess.PIPE,stderr=log,text=True,check=True)
 (out/"run.log").write_text(run.stdout,encoding="utf-8")
 lines=run.stdout.splitlines();assert len(lines)==5 and lines[-1].startswith("actual builtin_snapshot header PASS")
 keys=set("version family role captured reason image_base image_size checksum entry_address canonical_hash min_args max_args type expected_canonical_hash expected_min_args expected_max_args expected_type expected_action action_address action_rva section_index code_bytes code_hex scope".split())
 for line,(expected_maximum,connected_maximum) in zip(lines[:-1],((0,0),(1,0),(0,1),(1,1))):
  pairs=json.loads(line,object_pairs_hook=list);assert len(pairs)==len(keys) and {k for k,v in pairs}==keys
  receipt=dict(pairs);assert receipt["captured"] is True and receipt["family"]==FAMILY and receipt["role"]=="dedicated"
  assert all(type(receipt[key]) is int for key in ("min_args","max_args","type","expected_min_args","expected_max_args","expected_type"))
  assert receipt["canonical_hash"]==0x6c6e7b10 and receipt["expected_canonical_hash"]==0x4441ff8d and receipt["expected_action"]=="0x1402cb900"
  assert receipt["expected_min_args"]==0 and receipt["expected_max_args"]==expected_maximum and receipt["expected_type"]==0
  assert receipt["min_args"]==0 and receipt["max_args"]==connected_maximum and receipt["type"]==0
  assert receipt["code_bytes"]==256 and len(receipt["code_hex"])==512 and bytes.fromhex(receipt["code_hex"])==bytes(range(256))
  assert len(line.encode())<2048 and receipt["scope"]=="read_only_no_invocation"
 report={"passed":True,"family":FAMILY,"scope":"Actual header with synthetic memory/query/copy fixtures; no game/engine/builtin invocation.","header_sha256":hashlib.sha256(header.read_bytes()).hexdigest(),"receipt_bytes":[len(line.encode()) for line in lines[:-1]],"unit_result":lines[-1],"log":str(out/"checks.log"),"run_log":str(out/"run.log")}
 (out/"RESULTS.json").write_text(json.dumps(report,indent=2)+"\n",encoding="utf-8");print(json.dumps(report,indent=2))

if __name__=="__main__":main()
