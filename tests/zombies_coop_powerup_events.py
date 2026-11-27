"""Compile the actual public powerup metadata header against synthetic objects.

Python3.9+, configured C++20 compiler, no game/retail binary/network needed.
Example: python tests/zombies_coop_powerup_events.py --compiler clang-cl
The unit-only fingerprint callback is deliberately non-cryptographic; production
adapters must supply real SHA-256. This test does not activate a retail contract.
"""
from pathlib import Path
import argparse
import datetime
import hashlib
import json
import os
import re
import shlex
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default=os.environ.get('CXX', 'c++'))
    parser.add_argument('--compiler-arg', action='append', default=[])
    parser.add_argument('--header', type=Path, help='Optional proposed actual powerup_events.hpp; sibling compiled_includes.hpp is required')
    parser.add_argument('--output-dir', type=Path)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    header = (args.header or root / 'src/client/component/zombies/powerup_events.hpp').resolve(strict=True)
    if header.name != 'powerup_events.hpp' or not header.with_name('compiled_includes.hpp').is_file():
        parser.error('The actual header and its sibling compiled_includes.hpp are required')
    source = Path(__file__).with_suffix('.cpp')
    if not source.is_file():
        parser.error('The sibling zombies_coop_powerup_events.cpp test source is required')
    output = args.output_dir or Path(tempfile.mkdtemp(prefix='powerup-events-'))
    output.mkdir(parents=True, exist_ok=True)
    raw_compiler = args.compiler
    if Path(raw_compiler).is_file():
        command = [raw_compiler]
    else:
        command = shlex.split(raw_compiler, posix=os.name != 'nt')
        command = [part[1:-1] if len(part) > 1 and part[0] == part[-1] and part[0] in ('"', "'") else part for part in command]
    msvc = Path(command[0]).name.lower() in ('cl', 'cl.exe', 'clang-cl', 'clang-cl.exe')
    executable = output / ('powerup_events.exe' if os.name == 'nt' else 'powerup_events')
    command += args.compiler_arg
    if msvc:
        command += ['/nologo', '/std:c++20', '/EHsc', '/W4', '/WX', '/UNDEBUG', '/DNOMINMAX',
                    '/I' + str(header.parent), '/Fe:' + str(executable),
                    '/Fo:' + str(output / 'powerup_events.obj'), str(source)]
    else:
        command += ['-std=c++20', '-Wall', '-Wextra', '-Werror', '-UNDEBUG', '-I', str(header.parent),
                    str(source), '-o', str(executable)]
    compiled = subprocess.run(command, cwd=output, capture_output=True, text=True, timeout=120)
    (output / 'compile.log').write_text(compiled.stdout + compiled.stderr, encoding='utf-8')
    if compiled.returncode:
        raise SystemExit(f'Compile failed; see {output / "compile.log"}')
    ran = subprocess.run([str(executable)], cwd=output, capture_output=True, text=True, timeout=120)
    (output / 'stdout.log').write_text(ran.stdout, encoding='utf-8')
    (output / 'stderr.log').write_text(ran.stderr, encoding='utf-8')
    if ran.returncode:
        raise SystemExit(f'Regression failed; see {output}')
    match = re.fullmatch(r'checks=(\d+) allocation_cuts=(\d+) synthetic_only=true retail_contract=false production=false\s*', ran.stdout)
    if not match or int(match[1]) < 1400000 or int(match[2]) < 1:
        raise SystemExit('Missing expected bounded synthetic/exception coverage receipt')
    sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
    report = dict(passed=True, checks=int(match[1]), allocation_cuts=int(match[2]),
                  actual_header_sha256=sha(header), compiled_includes_sha256=sha(header.with_name('compiled_includes.hpp')),
                  test_cpp_sha256=sha(source), test_runner_sha256=sha(Path(__file__)),
                  compiler_command=command, fingerprint='Non-cryptographic four-lane FNV fixture callback only; production uses real SHA-256',
                  retail_binary_used=False, retail_contract_activated=False, game_invoked=False, network_used=False,
                  production_edited=False, created_utc=datetime.datetime.now(datetime.timezone.utc).isoformat())
    proof = output / 'RESULTS.json'
    proof.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(dict(proof=str(proof), passed=True, checks=report['checks'], allocation_cuts=report['allocation_cuts'])))


if __name__ == '__main__':
    main()
