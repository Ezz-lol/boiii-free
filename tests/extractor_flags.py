"""Compile actual flags.cpp and bundled argparse, then test each parse in a fresh process.

Run with a configured C++ compiler environment, for example:
  python tests/extractor_flags.py --compiler clang++
  python tests/extractor_flags.py --compiler cl.exe --compiler-arg /EHsc
No game, network, Windows API, or installed build-system dependency is used.
"""
from pathlib import Path
import argparse
import datetime
import hashlib
import json
import os
import shlex
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--source', type=Path)
    parser.add_argument('--compiler', default=os.environ.get('CXX', 'c++'))
    parser.add_argument('--compiler-arg', action='append', default=[])
    parser.add_argument('--output-dir', type=Path)
    args = parser.parse_args()
    repo = args.repo.resolve()
    source = (args.source or repo / 'src/common/utils/flags.cpp').resolve()
    header = repo / 'deps/argparse/include/argparse/argparse.hpp'
    if not source.is_file() or not header.is_file():
        raise SystemExit('Expected flags.cpp and the repository bundled argparse header')
    output = args.output_dir or Path(tempfile.mkdtemp(prefix='extractor-flags-'))
    output.mkdir(parents=True, exist_ok=True)
    driver = output / 'actual_flags.cpp'
    driver.write_text(r'''#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "SOURCE"
int main(int argc,char**argv){
  const int result=utils::flags::parse_flags(argc,argv);
  if(result){std::cout<<"RESULT\t"<<result<<"\n";return 0;}
  auto pattern=utils::flags::get<std::string>("extract-assets");
  std::cout<<"RESULT\t0\t"<<utils::flags::has_flag("extract-assets")<<"\t"<<std::quoted(pattern.value_or(""))<<"\t"<<utils::flags::program.get<bool>("-nosnd")<<"\t"<<std::quoted(utils::flags::program.get<std::string>("-output"))<<"\n";
  const std::vector<std::string> raw(argv,argv+argc);
  const auto normalized=utils::flags::normalize_extract_assets_arguments(raw,utils::flags::program);
  for(size_t i=1;i<normalized.arguments.size();++i)std::cout<<"ARG\t"<<std::quoted(normalized.arguments[i])<<"\n";
}
'''.replace('SOURCE', source.as_posix()), encoding='utf-8')
    compiler = shlex.split(args.compiler, posix=os.name != 'nt')
    is_msvc = Path(compiler[0]).name.lower() in ('cl', 'cl.exe', 'clang-cl', 'clang-cl.exe')
    executable = output / ('actual_flags.exe' if os.name == 'nt' else 'actual_flags')
    command = compiler + args.compiler_arg
    if is_msvc:
        command += ['/nologo', '/std:c++20', '/EHsc', '/W4', '/WX', '/DNDEBUG', '/DNOMINMAX', '/external:W0',
                    '/external:I' + str(header.parents[1]), '/Fe:' + str(executable),
                    '/Fo:' + str(output / 'actual_flags.obj'), str(driver)]
    else:
        command += ['-std=c++20', '-Wall', '-Wextra', '-Werror', '-DNDEBUG', '-I', str(header.parents[1]),
                    str(driver), '-o', str(executable)]
    compiled = subprocess.run(command, cwd=output, capture_output=True, text=True, timeout=120)
    (output / 'compile.log').write_text(compiled.stdout + compiled.stderr, encoding='utf-8')
    if compiled.returncode:
        raise SystemExit(f'Compilation failed; see {output / "compile.log"}')
    checks = 0
    cases = []

    def test(label, arguments, *, used=False, pattern='', error=False, nosnd=False,
             output_name='assets', kept=(), absent=()):
        nonlocal checks
        result = subprocess.run([str(executable), *arguments], cwd=output,
                                capture_output=True, text=True, timeout=10)
        if result.returncode:
            raise AssertionError((label, result.returncode, result.stdout, result.stderr))
        lines = result.stdout.splitlines()
        row = next(line.split('\t') for line in lines if line.startswith('RESULT\t'))
        actual_error = row[1] != '0'
        assert actual_error == error, (label, row, result.stderr)
        checks += 1
        normalized = [json.loads(line.split('\t', 1)[1]) for line in lines if line.startswith('ARG\t')]
        if not error:
            actual = (row[2] == '1', json.loads(row[3]), row[4] == '1', json.loads(row[5]))
            assert actual == (used, pattern, nosnd, output_name), (label, actual)
            checks += 4
            for item in kept:
                assert item in normalized, (label, item, normalized)
                checks += 1
            for item in absent:
                assert item not in normalized, (label, item, normalized)
                checks += 1
        cases.append(dict(label=label, arguments=arguments, error=actual_error,
                          normalized=normalized, result=row))

    exact = '^scripts/zm/_zm_powerups[.]gsc$'
    for alias in ('-e', '-extract-assets', '--extract-assets'):
        test(alias + ' two tokens', [alias, exact], used=True, pattern=exact, absent=(exact,))
        test(alias + ' assignment', [alias + '=' + exact], used=True, pattern=exact)
        test(alias + ' bare', [alias], used=True, pattern='^.*$')
        test(alias + ' known flag boundary', [alias, '-nosnd'], used=True, pattern='^.*$', nosnd=True)
        test(alias + ' known long assignment boundary', [alias, '--output=out'], used=True, pattern='^.*$', output_name='out')
        test(alias + ' known option literal', [alias + '=-nosnd'], used=True, pattern='-nosnd', nosnd=False)
        test(alias + ' unknown dash literal', [alias, '-unknown.*'], used=True, pattern='-unknown.*')
        test(alias + ' dash assignment', [alias + '=-unknown.*'], used=True, pattern='-unknown.*')
        test(alias + ' equals in regex', [alias + '=^a=b=c$'], used=True, pattern='^a=b=c$')
        test(alias + ' empty assignment', [alias + '='], error=True)
        test(alias + ' empty token', [alias, ''], error=True)
        test(alias + ' invalid assignment', [alias + '=['], error=True)
        test(alias + ' invalid token', [alias, '['], error=True)
        test(alias + ' explicit then options', [alias, exact, '-nosnd', '--output', 'out'], used=True, pattern=exact, nosnd=True, output_name='out')
        test(alias + ' engine arguments', ['+set', 'foo', 'bar', alias, exact, '+connect_lobby', '42', '-nosnd'], used=True, pattern=exact, nosnd=True, absent=('+set', 'foo', 'bar', '+connect_lobby', '42'))
        test(alias + ' unknown before', ['-unknown', alias, exact], used=True, pattern=exact, kept=('-unknown',))
        test(alias + ' unknown after explicit', [alias, exact, '-unknown'], used=True, pattern=exact, kept=('-unknown',))
        test(alias + ' unsupported other assignment is explicit', [alias, '-o=out'], used=True, pattern='-o=out')
        test(alias + ' positional after bare', [alias, '--', '-e=ignored'], used=True, pattern='^.*$', kept=('--', '-e=ignored'))
        for literal in ('nosnd', 'set', 'connect_lobby', 'showconsole'):
            test(alias + ' engine-name literal ' + literal, [alias, literal], used=True, pattern=literal, absent=(literal,))
        for repeated in ('-e', '-extract-assets=^two$', '--extract-assets=^two$'):
            test(alias + ' duplicate ' + repeated, [alias, repeated], error=True)
    test('absent', [])
    test('absent normal options', ['-nosnd', '--output=out'], nosnd=True, output_name='out')
    test('unknown preserved', ['-unknown', 'value'], kept=('-unknown', 'value'))
    test('forced positional', ['--', '-e=ignored', '+set', 'a', 'b'], kept=('--', '-e=ignored', '+set', 'a', 'b'))
    test('engine value is not extraction', ['+set', 'foo', '-e', '-nosnd'], nosnd=True, absent=('+set', 'foo', '-e'))
    test('engine lobby value is not extraction', ['+connect_lobby', '-e', '-nosnd'], nosnd=True, absent=('+connect_lobby', '-e'))
    test('literal regex plus option', ['-e', 'nosnd', '-nosnd'], used=True, pattern='nosnd', nosnd=True)
    test('bare before engine command', ['-e', '+set', 'foo', 'bar'], used=True, pattern='^.*$')
    digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
    report = dict(passed=True, checks=checks, cases=cases, source_sha256=digest(source),
                  flags_header_sha256=digest(source.with_name('flags.hpp')),
                  argparse_header_sha256=digest(header), actual_whole_source=True,
                  game_invoked=False, network_used=False, production_edited=False,
                  compiler_command=command, created_utc=datetime.datetime.now(datetime.timezone.utc).isoformat())
    proof = output / 'RESULTS.json'
    proof.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(dict(proof=str(proof), checks=checks, cases=len(cases), passed=True)))


if __name__ == '__main__':
    main()
