"""Compile and run actual script-frame diagnostic code with bounded stand-ins.

Requires Python 3.9+ and a configured C++20 compiler/standard library. For MSVC,
run from an x64 developer shell; --compiler or CXX may select cl/clang-cl or a
C++ compiler supporting -std=c++20. No SDK installation, downloads, repository
source changes, engine execution or cleanup/deletion is performed.

Example: python tests/script_frame_diagnostics.py --compiler clang-cl --output-dir ./test-output
"""
from pathlib import Path
import argparse
import datetime
import hashlib
import json
import os
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]

INCLUDES = r'''
#include <array>
#include <atomic>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cassert>
#include <vector>
#include <string>
#include <format>
#include <cstdio>
#include <thread>
#include <fstream>
#include <iterator>
'''
FORMATTER_CASES = r'''

using scriptInstance_t=int;constexpr int SCRIPTINSTANCE_SERVER=0,SCRIPTINSTANCE_CLIENT=1;
struct script_frame {FRAME};
std::vector<script_frame> supplied;
bool supported_frame_instance(int i){return i==0||i==1;}
int frame_reads=0;
std::vector<script_frame> get_script_frames(int,const uint8_t*){++frame_reads;return supplied;}
FORMATTER
int main(){
 script_frame f;f.next_pc=0x1234;f.fault_pc=0x1233;f.bytecode_position=true;f.file="scripts/zm/_zm.gsc";f.function="namespace_0x123::function_0x456";f.script_offset=0x13bc8;f.script_offset_available=true;f.function_offset=8;f.function_offset_available=true;
 supplied={f};auto lines=get_script_callstack(0,nullptr);assert(lines.size()==1);auto line=lines[0];
 for(auto token:{"next_pc=0x1234","fault_pc=0x1233","script_offset=0x13BC8","function=namespace_0x123::function_0x456","function_offset=0x8","line=unavailable"})assert(line.find(token)!=std::string::npos);
 f.line=9;f.source="return value;";supplied={f};line=get_script_callstack(1,nullptr)[0];assert(line.find("line=9")!=std::string::npos&&line.find("function=")!=std::string::npos);
 f.file.clear();supplied={f};line=get_script_callstack(0,nullptr)[0];assert(line.find("file/line=unavailable")!=std::string::npos&&line.find("next_pc=")!=std::string::npos);
 f={};supplied={f};line=get_script_callstack(0,nullptr)[0];assert(line.find("fault_pc=unavailable")!=std::string::npos);
 auto before=frame_reads;line=get_script_callstack(2,reinterpret_cast<uint8_t*>(0x456))[0];assert(frame_reads==before&&line.find("invalid script instance 2")!=std::string::npos);
 std::puts("Ezz actual frame formatter: missing-line/function/raw/fault/relative/invalid-instance PASS");
}
'''
METADATA_CASES = r'''

using scriptInstance_t=int;constexpr int SCRIPTINSTANCE_SERVER=0,SCRIPTINSTANCE_CLIENT=1;
struct GSC_EXPORT_ITEM {uint32_t checksum,address,name,name_space;uint8_t tail[4];};
struct GSC_OBJ {static constexpr int T7_MAGIC=28;uint64_t magic=28;uint32_t cseg_offset{},cseg_size{},exports_count{},exports_offset{};bool hasMagic(int m)const{return magic==static_cast<uint64_t>(m);}};
struct objFileInfo_t {GSC_OBJ*activeVersion{};};
struct Count {mutable unsigned reads=0;unsigned value=0;bool mutate=false;operator uint32_t()const{++reads;return mutate&&reads>1?501:value;}};
template<typename T>struct Pool{std::array<T,2>instance;};
Pool<Count>counts;auto*gObjFileInfoCount=&counts;Pool<std::array<objFileInfo_t,500>>objects;auto*gObjFileInfo=&objects;
namespace vm {struct End {void*get()const{return reinterpret_cast<void*>(UINTPTR_MAX);}};End g_endPos;}
HELPERS
int main(){
 std::vector<uint8_t>bytes(0x1000);auto*obj=new(bytes.data())GSC_OBJ{};obj->cseg_offset=0x80;obj->cseg_size=0x100;obj->exports_offset=0x200;obj->exports_count=2;
 auto*table=reinterpret_cast<GSC_EXPORT_ITEM*>(bytes.data()+0x200);table[0]={0,0x80,1,2,{}};table[1]={0,0x90,3,2,{}};
 counts.instance[0].value=1;objects.instance[0][0].activeVersion=obj;
 auto p=[&](size_t next){return bytes.data()+next;};
 assert(frame_object(0,p(0x81)));assert(!frame_object(0,p(0x80)));assert(!frame_object(0,p(0x181)));
 auto*f=containing_export(0,p(0x96));assert(f&&f->name==3);assert(!containing_export(0,p(0x80)));
 counts.instance[0].reads=0;counts.instance[0].mutate=true;assert(!frame_object(0,p(0x300)));assert(counts.instance[0].reads==1);counts.instance[0].mutate=false;
 auto before=counts.instance[0].reads;assert(!frame_object(2,p(0x81)));assert(counts.instance[0].reads==before);
 counts.instance[0].value=501;assert(!frame_object(0,p(0x81)));counts.instance[0].value=1;
 obj->exports_count=5000;assert(!containing_export(0,p(0x96)));obj->exports_count=2;
 obj->exports_offset=64*1024*1024-1;assert(!containing_export(0,p(0x96)));obj->exports_offset=0x200;
 table[1].address=0x7f;assert(!containing_export(0,p(0x96)));table[1].address=0x80;assert(!containing_export(0,p(0x96)));
 std::puts("Ezz actual metadata helpers: instance/snapshot-count/code/export bounds/ambiguity PASS");
}
'''



def function(source, signature):
    """Extract the actual definition while ignoring braces in strings/comments."""
    if source.count(signature) != 1:
        raise ValueError('Expected one actual source definition: ' + signature)
    start = source.index(signature)
    opening = source.index('{', start)
    at, depth, quote, comment = opening + 1, 1, None, None
    while at < len(source):
        char = source[at]
        following = source[at + 1:at + 2]
        if comment == 'line':
            if char == '\n':
                comment = None
        elif comment == 'block':
            if char == '*' and following == '/':
                comment = None
                at += 1
        elif quote:
            if char == '\\':
                at += 1
            elif char == quote:
                quote = None
        elif char == '/' and following in ('/', '*'):
            comment = 'line' if following == '/' else 'block'
            at += 1
        elif char in ('"', "'"):
            quote = char
        elif char == '{':
            depth += 1
        elif char == '}':
            depth -= 1
            if depth == 0:
                return source[start:at + 1]
        at += 1
    raise ValueError('Unterminated source definition: ' + signature)


def between(source, begin, end):
    if source.count(begin) != 1 or end not in source.split(begin, 1)[1]:
        raise ValueError('Actual diagnostic source markers changed')
    return source.split(begin, 1)[1].split(end, 1)[0]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default=os.environ.get('CXX', 'cl'),
                        help='Compiler executable, using the current configured build environment')
    parser.add_argument('--output-dir', type=Path,
                        default=Path(tempfile.gettempdir()) / 'boiii-script-frame-tests',
                        help='Parent for a new unique output folder; no existing output is removed')
    args = parser.parse_args()
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    out = args.output_dir.resolve() / ('script-frame-diagnostics-' + stamp)
    out.mkdir(parents=True, exist_ok=False)
    sources = {}
    text = (ROOT / 'src/client/component/script.cpp').read_text(encoding='utf-8')
    sources['src/client/component/script.cpp'] = hashlib.sha256(
        (ROOT / 'src/client/component/script.cpp').read_bytes()).hexdigest()
    header_path = ROOT / 'src/client/component/script.hpp'
    header = header_path.read_text(encoding='utf-8')
    sources['src/client/component/script.hpp'] = hashlib.sha256(header_path.read_bytes()).hexdigest()
    frame = between(header, 'struct script_frame {', '\n};')
    formatter = function(text, 'std::vector<std::string> get_script_callstack(')
    helpers = '\n'.join(function(text, signature) for signature in (
        'bool supported_frame_instance(', 'const objFileInfo_t *frame_object(', 'const GSC_EXPORT_ITEM *containing_export('))
    units = [('ezz_formatter', INCLUDES + FORMATTER_CASES.replace('FRAME', frame).replace('FORMATTER', formatter), []),
             ('ezz_metadata', INCLUDES + METADATA_CASES.replace('HELPERS', helpers), [])]
    compiler_name = Path(args.compiler).name.lower()
    msvc = compiler_name in ('cl', 'cl.exe', 'clang-cl', 'clang-cl.exe')
    records = []
    with (out / 'checks.log').open('x', encoding='utf-8') as log:
        for stem, generated, runtime_args in units:
            source = out / (stem + '.cpp')
            source.write_text(generated, encoding='utf-8')
            exe = out / (stem + ('.exe' if os.name == 'nt' else ''))
            if msvc:
                command = [args.compiler, '/nologo', '/std:c++20', '/EHsc', '/MT', '/W4', '/WX',
                           '/Fe:' + str(exe), '/Fo:' + str(out / (stem + '.obj')), str(source)]
            else:
                command = [args.compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror', '-pthread',
                           str(source), '-o', str(exe)]
            log.write(json.dumps({'compile': command}) + '\n')
            log.flush()
            subprocess.run(command, cwd=out, stdout=log, stderr=log, check=True)
            log.write(json.dumps({'run': [str(exe), *runtime_args]}) + '\n')
            log.flush()
            subprocess.run([str(exe), *runtime_args], cwd=out, stdout=log, stderr=log, check=True)
            records.append({'fixture': stem, 'passed': True,
                            'generated_source_sha256': hashlib.sha256(source.read_bytes()).hexdigest()})
    report = {'passed': True, 'scope': 'Actual production helper/formatter code with explicit native stand-ins; no engine/runtime proof.',
              'source_files': sources, 'compiler': args.compiler, 'fixtures': records,
              'log': str(out / 'checks.log')}
    (out / 'RESULTS.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
