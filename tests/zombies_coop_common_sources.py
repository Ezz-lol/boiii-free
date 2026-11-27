"""Portable source-pair/cache lifecycle fixture; no game or private data required.

The default checks tracked common artifacts against the production pins before
using a documented synthetic metadata/hash fixture. Optional actual
common artifacts are independently SHA-256 checked before running the same
control-flow cases. Neither path proves engine linking or client compatibility.
"""
from pathlib import Path
import argparse,hashlib,json,os,re,shlex,subprocess,tempfile

def verify_tracked_common(repo,header):
    """Catch stale production pins before synthetic hash stand-ins can mask them."""
    text=header.read_text()
    digest=lambda path:hashlib.sha256(path.read_bytes()).hexdigest()
    def pin(name):
        values=re.findall(r'\b'+name+r'\s*=\s*hex_digest\(\s*"([0-9a-f]{64})"\s*\)',text)
        if len(values)!=1:raise ValueError('Missing/duplicate production common pin: '+name)
        return values[0]
    readable=repo/'scripts/zombies_coop/eroded_coop.gsc'
    packaged_cb=readable.is_file()
    if not packaged_cb:readable=repo/'data/custom_scripts/zm/eroded_coop.gsc'
    if not readable.is_file()or readable.stat().st_size>1024*1024:
        raise ValueError('Missing/oversized tracked common source')
    readable_bytes=readable.read_bytes()
    # CB packages compiled data; match its existing utf8-lf source manifest.
    # Ezz binds raw readable/cleaned data at runtime, so its check stays exact.
    normalized=readable_bytes.replace(b'\r\n',b'\n')if packaged_cb else readable_bytes
    readable_digest=hashlib.sha256(normalized).hexdigest()
    if readable_digest!=pin('source_pin'):
        raise ValueError('Tracked common source differs from production source_pin')
    records={'readable':{'path':readable.relative_to(repo).as_posix(),'sha256':digest(readable),
        'canonical_sha256':readable_digest,'normalization':'utf8-crlf-to-lf'if packaged_cb else'raw'}}
    if packaged_cb:
        compiled=repo/'data/custom_scripts/zm/eroded_coop.gsc'
        if not compiled.is_file()or compiled.stat().st_size>1024*1024 or digest(compiled)!=pin('cb_compiled_pin'):
            raise ValueError('Packaged CB common differs from production cb_compiled_pin')
        manifest=repo/'tests/zombies_coop_payload.json'
        metadata=json.loads(manifest.read_text())
        if metadata['source_normalization']!='utf8-lf':raise ValueError('Unknown CB common source normalization')
        rows=[row for row in metadata['entries']
              if row['source']=='scripts/zombies_coop/eroded_coop.gsc' and row['payload']=='data/custom_scripts/zm/eroded_coop.gsc']
        if len(rows)!=1 or rows[0]['source_sha256']!=readable_digest or rows[0]['payload_sha256']!=digest(compiled) or rows[0]['payload_bytes']!=compiled.stat().st_size:
            raise ValueError('Tracked CB common manifest differs from production source/object pins')
        records['compiled']={'path':compiled.relative_to(repo).as_posix(),'sha256':digest(compiled)}
        records['manifest']={'path':manifest.relative_to(repo).as_posix(),'sha256':digest(manifest)}
    return records

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,default=Path(__file__).resolve().parents[1])
    parser.add_argument('--compiler',default=os.environ.get('CXX','c++'))
    parser.add_argument('--compiler-arg',action='append',default=[])
    parser.add_argument('--output-dir',type=Path)
    parser.add_argument('--compiled-common',type=Path)
    parser.add_argument('--common-source',type=Path)
    args=parser.parse_args();repo=args.repo.resolve()
    header=repo/'src/client/component/zombies/common_helper_source_pair.hpp'
    source=repo/'tests/zombies_coop_common_sources.cpp'
    if bool(args.compiled_common)!=bool(args.common_source):parser.error('Supply both optional actual common artifacts')
    digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    tracked=verify_tracked_common(repo,header)
    actual=[]
    if args.compiled_common:
        text=header.read_text()
        for path,name in ((args.compiled_common,'cb_compiled_pin'),(args.common_source,'source_pin')):
            if not path.is_file()or path.stat().st_size>1024*1024:raise ValueError('Missing/oversized optional actual artifact')
            pin=re.search(r'\b'+name+r'\s*=\s*hex_digest\(\s*"([0-9a-f]{64})"\s*\)',text)
            if not pin or digest(path)!=pin[1]:raise ValueError('Optional artifact differs from compiled source pin: '+name)
            actual.append(str(path.resolve()))
    output=args.output_dir
    if output:output.mkdir(parents=True,exist_ok=False)
    else:output=Path(tempfile.mkdtemp(prefix='coop-common-sources-'))
    compiler=[args.compiler]if Path(args.compiler).is_file()else shlex.split(args.compiler,posix=os.name!='nt');msvc=Path(compiler[0]).name.lower()in('cl','cl.exe','clang-cl','clang-cl.exe')
    exe=output/('common_sources.exe'if os.name=='nt'else'common_sources')
    include=repo/'src/client/component/zombies'
    command=compiler+args.compiler_arg
    if msvc:command+=['/nologo','/std:c++20','/EHsc','/W4','/WX','/DNOMINMAX','/I'+str(include),'/Fe:'+str(exe),'/Fo:'+str(exe.with_suffix('.obj')),str(source)]
    else:command+=['-std=c++20','-Wall','-Wextra','-Werror','-I',str(include),str(source),'-o',str(exe)]
    result=subprocess.run(command,cwd=output,capture_output=True,text=True,timeout=120)
    (output/'compile.log').write_text(result.stdout+result.stderr)
    if result.returncode:raise SystemExit('Compilation failed: '+str(output/'compile.log'))
    result=subprocess.run([str(exe),*actual],cwd=output,capture_output=True,text=True,timeout=30)
    (output/'stdout.log').write_text(result.stdout);(output/'stderr.log').write_text(result.stderr)
    if result.returncode:raise SystemExit('Source-pair fixture failed: '+str(output))
    pins={str(p.relative_to(repo)):digest(p)for p in(source,header,include/'common_helper_native_context.hpp',include/'schema_asset_store.hpp',include/'compiled_includes.hpp')}
    record=dict(passed=True,stdout=result.stdout,pins=pins,tracked_artifacts_sha256_checked=True,tracked_artifacts=tracked,actual_artifacts_sha256_checked=bool(actual),scope='Tracked readable artifacts and CB packaged object/manifest match production pins before synthetic control flow and source/cache/load-return lifetime checks; Ezz compile receipt is simulated. No engine linker/body authentication, game, network or client proof.')
    (output/'RESULTS.json').write_text(json.dumps(record,indent=2)+'\n');print(json.dumps(dict(proof=str(output/'RESULTS.json'),stdout=result.stdout)))

if __name__=='__main__':main()
