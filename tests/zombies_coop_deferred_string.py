"""Check actual deferred EndingMap registration with configured C++ toolchain.

Run from a developer environment with --compiler cl (or its absolute path).
--output retains a new artifact directory; otherwise a fresh system temporary
folder is used. Engine execution and native registration atomicity are separate.
"""
from pathlib import Path
import argparse
import hashlib
import json
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]

def function_text(text, pattern):
 matches=list(re.finditer(pattern,text));assert len(matches)==1
 start=matches[0].start();pos=matches[0].end();depth=1
 while pos<len(text):
  if text.startswith('//',pos):
   end=text.find('\n',pos);pos=len(text)if end<0 else end+1;continue
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
   if not depth:return text[start:pos+1]
  pos+=1
 raise AssertionError('unclosed body')

PRELUDE = r'''
#include <cassert>
#include <climits>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>
namespace mock {
enum class phase{unpack,main};phase where=phase::unpack;
struct dvar {bool string_type{};std::string current;};
std::map<std::string,dvar>vars;std::map<std::string,int>ints;
std::vector<std::string>int_calls;unsigned string_calls{},early_calls{},find_calls{},checks{};bool destructive{};
struct early_string{};
void check(bool v){++checks;assert(v);}
void reset(){where=phase::unpack;vars.clear();ints.clear();int_calls.clear();string_calls=early_calls=find_calls=0;destructive=false;}
}
namespace scheduler {
enum class pipeline{main};std::vector<std::function<void()>>queued;
void once(const std::function<void()>&f,pipeline p){mock::check(p==pipeline::main);queued.push_back(f);}
void run(){mock::where=mock::phase::main;auto callbacks=std::move(queued);queued.clear();for(auto&f:callbacks)f();}
}
namespace utils::flags {bool has_flag(const char*){return true;}}
namespace game {
constexpr unsigned DVAR_ARCHIVE=1u;enum class dvarFlags_e:unsigned{ARCHIVE=1u};
#ifdef EZZ_API
struct dependent_dvar{mock::dvar*p{};explicit operator bool()const{return p!=nullptr;}};
using result=dependent_dvar;result wrap(mock::dvar*p){return {p};}
#else
using result=mock::dvar*;result wrap(mock::dvar*p){return p;}
#endif
result Dvar_FindVar(const char*name){++mock::find_calls;const auto it=mock::vars.find(name);return wrap(it==mock::vars.end()?nullptr:&it->second);}
template<class Flags>void*register_dvar_int(const char*name,int value,int,int,Flags,const char*){mock::int_calls.emplace_back(name);mock::ints[name]=value;return nullptr;}
result register_dvar_string(const char*name,const char*value,unsigned,const char*){
 ++mock::string_calls;if(mock::where!=mock::phase::main){++mock::early_calls;throw mock::early_string{};}
 auto[it,inserted]=mock::vars.try_emplace(name,mock::dvar{true,value});if(!inserted&&mock::destructive)it->second={true,value};return wrap(&it->second);
}
void Dvar_SetFromStringByName(const char*name,const char*value,bool){mock::vars[name]={true,value};}
}
namespace zombies::coop {constexpr int stock_limit=4,player_limit=8;namespace ending_policy{constexpr int receipt_revision=1;}
struct generic_component{virtual void post_unpack()=0;virtual~generic_component()=default;};
struct proposed:generic_component{PROPOSED_BODY};
}

'''

TESTS = r'''

void reset(){mock::reset();scheduler::queued.clear();}
void publish_ending(){mock::ints["zm_coopEndingRevision"]=1;mock::ints["zm_coopEndingEpoch"]=17;mock::vars["zm_coopEndingMap"]={true,"zm_zod"};mock::vars["zm_coopStatus"]={true,"ending"};}
int main(){using mock::check;

 for(bool destructive:{false,true}){
  reset();mock::destructive=destructive;zombies::coop::proposed c;c.post_unpack();check(mock::early_calls==0&&mock::string_calls==0&&mock::find_calls==0);check(mock::int_calls.size()==7&&scheduler::queued.size()==1);scheduler::run();check(mock::string_calls==1&&mock::vars["zm_coopEndingMap"].string_type&&mock::vars["zm_coopEndingMap"].current.empty());check(mock::find_calls==1&&scheduler::queued.empty());scheduler::run();check(mock::string_calls==1);
  // Realistic early command order: post_unpack queued, begin_startup reset/arm,
  // controller emitted a valid ending BEFORE the first main callback.
  reset();mock::destructive=destructive;c.post_unpack();game::Dvar_SetFromStringByName("zm_coopEndingMap","",true);mock::ints["zm_coopEndingRevision"]=0;mock::ints["zm_coopEndingEpoch"]=0;publish_ending();scheduler::run();check(mock::string_calls==0&&mock::early_calls==0&&mock::find_calls==1);check(mock::vars["zm_coopEndingMap"].current=="zm_zod"&&mock::ints["zm_coopEndingRevision"]==1&&mock::ints["zm_coopEndingEpoch"]==17&&mock::vars["zm_coopStatus"].current=="ending");
  reset();mock::destructive=destructive;c.post_unpack();game::Dvar_SetFromStringByName("zm_coopEndingMap","",true);mock::ints["zm_coopEndingRevision"]=0;mock::ints["zm_coopEndingEpoch"]=0;scheduler::run();check(mock::string_calls==0&&mock::vars["zm_coopEndingMap"].current.empty()&&mock::ints["zm_coopEndingRevision"]==0&&mock::ints["zm_coopEndingEpoch"]==0);
  reset();mock::destructive=destructive;mock::vars["zm_coopEndingMap"]={true,"zm_tomb"};c.post_unpack();scheduler::run();check(mock::string_calls==0&&mock::vars["zm_coopEndingMap"].current=="zm_tomb");
  reset();mock::destructive=destructive;mock::vars["zm_coopEndingMap"]={false,"wrong_type"};c.post_unpack();scheduler::run();check(mock::string_calls==0&&!mock::vars["zm_coopEndingMap"].string_type&&mock::vars["zm_coopEndingMap"].current=="wrong_type");
  reset();mock::destructive=destructive;c.post_unpack();c.post_unpack();check(mock::string_calls==0&&scheduler::queued.size()==2);scheduler::run();check(mock::string_calls==1&&mock::find_calls==2);
 }
 std::cout<<"checks="<<mock::checks<<" exact_actual_post_unpack=true engine_default_semantics_assumed=false no_game_execution=true\n";
}

'''

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", required=True)
    parser.add_argument("--source", type=Path, default=ROOT / "src/client/component/zombies_coop.cpp")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    source_path = args.source.resolve(strict=True)
    text = source_path.read_text(encoding="utf-8")
    source_sha = hashlib.sha256(source_path.read_bytes()).hexdigest()
    post = function_text(text, r"void\s+post_unpack\s*\(\s*\)\s*override\s*\{")
    assert '#include "scheduler.hpp"' in text
    assert post.count("scheduler::once(") == 1
    assert post.count("register_dvar_string(") == 1
    assert post.count("scheduler::pipeline::main") == 1
    assert 'if (!game::Dvar_FindVar("zm_coopEndingMap"))' in post
    integers = re.findall(r'\(void\)game::register_dvar_int\(\s*"([^"\r\n]+)"', post)
    assert sorted(integers) == sorted(["zm_coopPlayerLimit", "zm_coopHeartbeat", "zm_coopStartupEpoch", "zm_coopReadyEpoch", "zm_coopAppliedLimit", "zm_coopEndingRevision", "zm_coopEndingEpoch"])
    begin = function_text(text, r"const\s+char\s*\*\s*begin_startup\s*\(\s*\)\s*\{")
    assert begin.index('"zm_coopEndingRevision", "0"') < begin.index('"zm_coopEndingMap", ""') < begin.index('"zm_coopEndingEpoch", "0"') < begin.index("admission_state.arm")
    output = args.output.resolve() if args.output else Path(tempfile.mkdtemp(prefix="boiii-ending-dvar-"))
    if args.output:
        output.mkdir(parents=True, exist_ok=False)
    ezz_api = "game::dvarFlags_e::ARCHIVE" in post
    generated = output / "deferred_actual.cpp"
    generated.write_text(("#define EZZ_API 1\n" if ezz_api else "") + PRELUDE.replace("PROPOSED_BODY", post) + TESTS, encoding="utf-8")
    binary = output / "deferred_actual.exe"
    command = [args.compiler, "/nologo", "/std:c++20", "/EHsc", "/W4", "/WX", "/Fe:" + str(binary), "/Fo:" + str(binary.with_suffix(".obj")), str(generated)]
    compile_result = subprocess.run(command, cwd=output, capture_output=True, text=True, timeout=90)
    (output / "compile.log").write_text(compile_result.stdout + compile_result.stderr, encoding="utf-8")
    assert compile_result.returncode == 0, output / "compile.log"
    run = subprocess.run([str(binary)], cwd=output, capture_output=True, text=True, timeout=15)
    (output / "stdout.log").write_text(run.stdout, encoding="utf-8")
    (output / "stderr.log").write_text(run.stderr, encoding="utf-8")
    assert run.returncode == 0 and "checks=38 " in run.stdout, (output, run.returncode, run.stdout, run.stderr)
    assert hashlib.sha256(source_path.read_bytes()).hexdigest() == source_sha, "source changed during test"
    report = {"passed": True, "checks": 38, "source_sha256": source_sha, "test_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(), "generated_sha256": hashlib.sha256(generated.read_bytes()).hexdigest(), "command": command, "ezz_api_shape": ezz_api, "stdout": run.stdout, "scope": "Actual post_unpack/deferred callback under explicit main-scheduler and pointer/dependent-dvar stand-ins, including late existing receipts and preserving/destructive registration-default models. No engine execution or native atomicity proof."}
    (output / "RESULTS.private.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"passed": True, "checks": 38, "proof": str(output / "RESULTS.private.json")}))


if __name__ == "__main__":
    main()
