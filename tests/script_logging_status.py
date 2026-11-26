"""Compile actual logging-status helpers with engine/storage/CLI stand-ins.

Run in a configured C++20 environment:
  python tests/script_logging_status.py --compiler clang-cl --output-dir ./test-output
The compiler defaults to CXX or cl. Uses only this checkout and Python's standard
library; no setup/downloads/source changes/deletions or engine execution.
"""
from pathlib import Path
import argparse
import datetime
import hashlib
import json
import os
import re
import subprocess
import sys
import tempfile
sys.dont_write_bytecode = True
from script_frame_diagnostics import between, function

ROOT = Path(__file__).resolve().parents[1]
FAMILY = 'ezz'
KEYS = {'version', 'family', 'role', 'supported', 'enabled', 'binding', 'dvar_bound', 'flag_present'}
INCLUDES = r"""
#include <cassert>
#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <string>
#include <vector>
#include <string_view>
std::vector<std::string> captured;
int observed_printf(const char*fmt,...){
 char buffer[512]{};va_list args;va_start(args,fmt);
 int length=std::vsnprintf(buffer,sizeof(buffer),fmt,args);va_end(args);
 assert(length>=0&&static_cast<size_t>(length)<sizeof(buffer));captured.emplace_back(buffer);return length;
}
void emit_one(){assert(captured.size()==1);std::fputs(captured[0].c_str(),stdout);captured.clear();}
"""


EZZ_DEFINITIONS = r"""
namespace utils::flags {
 bool present=false;unsigned reads=0;
 bool has_flag(const char*name){assert(std::string_view(name)=="log-script-errors");++reads;return present;}
}
namespace game {
 bool server=true,client=false;bool is_server(){return server;}bool is_client(){return client;}
 struct EngineDependentDvarMut {
  bool bound=false,value=false,unreadable=false;
  explicit operator bool()const{return bound;}
  bool get_bool()const{assert(!unreadable);return value;}
 };
 #define alwaysinline inline
 ACTUAL_SETTING
 #undef alwaysinline
 ACTUAL_VARIABLE
 ACTUAL_ENABLED
}
"""
EZZ_CASES = r"""
int main(){
 for(bool client:{false,true}){
  game::server=!client;game::client=client;
  // Unbound settings read CLI; once bound, the current dvar overrides CLI.
  for(int binding=0;binding<3;++binding){
   for(bool cli:{false,true}){
    game::log_script_errors_flag.dvar={binding!=0,binding==2,false};utils::flags::present=cli;
    utils::flags::reads=0;bool actual=script_error_logging_enabled();
    assert(actual==(binding==0?cli:binding==2));assert(utils::flags::reads==(binding==0?1u:0u));
    print_script_logging_status();emit_one();
   }
  }
 }
 // Unsupported role may display CLI but must not read storage/VM state.
 game::server=false;game::client=false;game::log_script_errors_flag.dvar={true,true,true};
 utils::flags::present=false;print_script_logging_status();emit_one();
}
"""

def expected(role, supported, enabled, binding, bound, flag):
    return dict(version=1, family=FAMILY, role=role, supported=supported,
                enabled=enabled, binding=binding, dvar_bound=bound, flag_present=flag)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default=os.environ.get('CXX', 'cl'))
    parser.add_argument('--output-dir', type=Path,
                        default=Path(tempfile.gettempdir()) / 'boiii-script-logging-tests')
    args = parser.parse_args()
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    out = args.output_dir.resolve() / ('script-logging-status-' + stamp)
    out.mkdir(parents=True, exist_ok=False)
    source_path = ROOT / 'src/client/component/script.cpp'
    text = source_path.read_text(encoding='utf-8')
    helper = between(text, '// BEGIN READONLY SCRIPT LOGGING STATUS', '// END READONLY SCRIPT LOGGING STATUS')
    assert helper.count('printf(') == 1, 'One bounded JSON receipt per command'
    assert not re.search(r'\b(?:Scr_|gScr|register_dvar|set_bool|set_int|SetDvar|DB_|GetVM|loaded_scripts)', helper), 'Status helper must not touch VM/database/setters'
    registration = r'command::add\(\s*"coopScriptErrorStatus"\s*,\s*print_script_logging_status\s*\)'
    assert len(re.findall(registration, text)) == 1, 'Register the actual read-only handler once'
    sources = {'src/client/component/script.cpp': hashlib.sha256(source_path.read_bytes()).hexdigest()}
    assert 'script_error_logging_enabled()' in function(text, 'void Scr_Error_LogAll('), 'Actual logger must use the reported helper'
    settings_path = ROOT / 'src/client/game/settings.hpp'
    settings = settings_path.read_text(encoding='utf-8')
    sources['src/client/game/settings.hpp'] = hashlib.sha256(settings_path.read_bytes()).hexdigest()
    setting = function(settings, 'struct flag_setting {') + ';'
    variable = function(settings, 'inline constinit flag_setting log_script_errors_flag{') + ';'
    enabled = function(settings, 'inline bool log_script_errors()')
    definitions = EZZ_DEFINITIONS.replace('ACTUAL_SETTING', setting).replace('ACTUAL_VARIABLE', variable).replace('ACTUAL_ENABLED', enabled)
    cases = EZZ_CASES
    wanted = []
    for role in ('dedicated', 'client'):
        for binding in range(3):
            for cli in (False, True):
                wanted.append(expected(role, True, cli if binding == 0 else binding == 2,
                                       'command_line' if binding == 0 else 'dvar', binding != 0, cli))
    wanted.append(expected('client', False, False, 'unsupported', False, False))
    cpp = out / 'script_logging_status.cpp'
    cpp.write_text(INCLUDES + definitions + '\n#define printf observed_printf\n' + helper + '\n#undef printf\n' + cases,
                   encoding='utf-8')
    exe = out / ('script_logging_status' + ('.exe' if os.name == 'nt' else ''))
    if Path(args.compiler).name.lower() in ('cl', 'cl.exe', 'clang-cl', 'clang-cl.exe'):
        command = [args.compiler, '/nologo', '/std:c++20', '/EHsc', '/MT', '/W4', '/WX',
                   '/Fe:' + str(exe), '/Fo:' + str(out / 'script_logging_status.obj'), str(cpp)]
    else:
        command = [args.compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror', str(cpp), '-o', str(exe)]
    with (out / 'checks.log').open('x', encoding='utf-8') as log:
        log.write(json.dumps({'compile': command}) + '\n'); log.flush()
        subprocess.run(command, cwd=out, stdout=log, stderr=log, check=True)
        result = subprocess.run([str(exe)], cwd=out, stdout=subprocess.PIPE, stderr=log, text=True, check=True)
    (out / 'receipts.log').write_text(result.stdout, encoding='utf-8')
    lines = result.stdout.splitlines()
    assert len(lines) == len(wanted), 'Every state yields exactly one JSON receipt'
    for line, expectation in zip(lines, wanted):
        assert line.startswith('COOP_SCRIPT_LOGGING|') and len(line.encode('utf-8')) < 512
        pairs = json.loads(line.split('|', 1)[1], object_pairs_hook=list)
        assert len(pairs) == len(KEYS) and {key for key, _ in pairs} == KEYS, 'Exact unique JSON keys'
        value = dict(pairs)
        assert type(value['version']) is int and value['version'] == 1
        assert all(type(value[key]) is bool for key in ('supported', 'enabled', 'dvar_bound', 'flag_present'))
        assert value == expectation, (value, expectation)
    report = {'passed': True, 'family': FAMILY, 'receipt_cases': len(lines), 'source_files': sources,
              'scope': 'Actual marked helper/printf and actual settings method with explicit stand-ins; no VM/engine runtime proof.',
              'compiler': args.compiler, 'generated_source_sha256': hashlib.sha256(cpp.read_bytes()).hexdigest(),
              'log': str(out / 'checks.log'), 'receipts': str(out / 'receipts.log')}
    (out / 'RESULTS.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
