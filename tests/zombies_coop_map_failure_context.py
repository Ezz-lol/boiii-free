"""Compile the complete actual map asset component under native stand-ins.

All source objects are synthetic. This checks diagnostic attribution and stale
publication guards, not native/game execution or linked-body authentication.
"""
from pathlib import Path
import argparse, ast, hashlib, json, os, re, shlex, subprocess


TESTS = r'''
unsigned checks{};
static void check(bool value) { ++checks; assert(value); }
static std::string text(const auto &value) { return std::string(value.data()); }

int main() {
  using namespace zombies::coop;
  using namespace zombies::coop::schema_assets;
  const auto reset = [] {
    mock::after_hash = {}; game::read_hook = {};
    mock::failed = false; mock::failures = 0; mock::capacity = 8;
    mock::actual_map = "zm_stalingrad"; mock::frozen_map = "zm_stalingrad";
    game::mode = game::eModes::ZOMBIES;
    begin_vm_generation(false);
    ++mock::epoch;
    check(common_helper::test_context.publish(mock::epoch.load(), 8, "zm_stalingrad"));
    check(!current_map_failure().available);
  };
  const auto asset = [](const char *name, std::vector<std::uint8_t> &bytes) {
    return script_asset{name, static_cast<decltype(script_asset::len)>(bytes.size()),
        reinterpret_cast<decltype(script_asset::buffer)>(bytes.data())};
  };
  const auto bytes_for = [](unsigned row, bool valid_magic) {
    const auto contract = map_mechanics::descriptors[row].factory();
    std::vector<std::uint8_t> bytes(contract.object_size, 0);
    if (valid_magic) std::copy(t7::object_magic.begin(), t7::object_magic.end(), bytes.begin());
    return bytes;
  };
  const auto error_for = [](const map_diagnostics::failure_snapshot &snapshot) {
    std::array<char, 1024> output{};
    map_diagnostics::format_failure_error("paired schema failed", snapshot, output);
    check(output.back() == 0);
    return std::string(output.data());
  };
  const auto check_error_path = [&] {
    const auto calls = mock::error_calls;
    mock::startup_error = "paired schema failed";
    check_startup();
    check(mock::error_calls == calls + 1 && mock::error_code == 42);
    check(mock::error_message == error_for(current_map_failure()));
    mock::startup_error = nullptr;
    check_startup();
    check(mock::error_calls == calls + 1);
  };

  reset();
  check(error_for(current_map_failure()) == "paired schema failed [map31_failure=unavailable]");
  check_error_path();
  auto bad_header = bytes_for(6, false);
  auto main = asset("scripts/zm/zm_stalingrad.gsc", bad_header);
  const auto main_lease = assets.capture_generation(main.name);
  check(transform_map_stock(main.name, &main) == &main);
  auto first = current_map_failure();
  check(mock::failed && mock::failures == 1);
  check(first.available && first.stage == map_diagnostics::failure_stage::stock_transform);
  check(first.row == 6 && first.helper == -1);
  check(first.epoch == mock::epoch && first.capacity == 8 && first.generation == main_lease.generation);
  check(text(first.reason) == "raw_contract_declined" && text(first.detail) == "raw_header_or_size");
  check(first.source.bytes == bad_header.size() && !first.source.sha256_computed);
  check(std::all_of(first.source.sha256.begin(), first.source.sha256.end(), [](auto byte) { return byte == 0; }));
  const auto first_error = error_for(first);
  check(first_error.find("map31_failure=stock_transform row=6 helper=-1") != std::string::npos);
  check(first_error.find("reason=raw_contract_declined detail=raw_header_or_size") != std::string::npos);
  check(first_error.find("sha256=unavailable") != std::string::npos);
  check(first_error.find("epoch=" + std::to_string(first.epoch)) != std::string::npos);
  check(first_error.find("generation=" + std::to_string(first.generation)) != std::string::npos);
  check_error_path();
  // Failure admission is still closed; a later route cannot overwrite the first receipt.
  auto later_bytes = bytes_for(4, false);
  auto later = asset("scripts/zm/zm_stalingrad_dragon.gsc", later_bytes);
  check(transform_map_stock(later.name, &later) == &later);
  check(current_map_failure().row == 6 && mock::failures == 1);
  const auto old_identity = capture_map_backing_identity(main.name);
  check(old_identity.has_value());
  map_publish_failure(*old_identity, "synthetic later publication",
      map_diagnostics::failure_stage::stock_exception, 4, -1, "later_failure");
  check(current_map_failure().row == 6 && text(current_map_failure().detail) == "raw_header_or_size");
  check(mock::failures == 1);
  common_helper::invalidate_native_session();
  check(!current_map_failure().available);

  reset();
  map_publish_failure(*old_identity, "synthetic stale publication",
      map_diagnostics::failure_stage::stock_exception, 6, -1, "stale_failure");
  check(!mock::failed && mock::failures == 0 && !current_map_failure().available);
  auto unknown_bytes = bytes_for(6, true);
  auto unknown = asset("scripts/zm/not_a_fixed_module.gsc", unknown_bytes);
  check(transform(unknown.name, &unknown) == &unknown);
  check(!mock::failed && !current_map_failure().available);
  check(!handles_map_stock(unknown.name));
  check(lookup_map_helper("custom_scripts/zm/not_a_fixed_helper.gsc") == nullptr);
  check(!mock::failed && !current_map_failure().available);

  auto unknown_digest = asset("scripts/zm/zm_stalingrad.gsc", unknown_bytes);
  check(transform_map_stock(unknown_digest.name, &unknown_digest) == &unknown_digest);
  const auto observed = current_map_failure();
  check(observed.available && observed.row == 6 && observed.helper == -1);
  check(text(observed.reason) == "raw_contract_declined" && text(observed.detail) == "unknown_asset_sha256");
  check(observed.source.bytes == unknown_bytes.size() && observed.source.sha256_computed);
  check(observed.source.sha256 == EXPECTED_MAIN_DIGEST);
  const auto digest_error = error_for(observed);
  check(digest_error.find("sha256=EXPECTED_MAIN_HEX") != std::string::npos);
  check_error_path();

  // A foreign failure latch has no attributed map receipt.
  reset();
  schema_asset_failure("synthetic non-map failure");
  check(mock::failed && !current_map_failure().available);
  check(error_for(current_map_failure()).find("map31_failure=unavailable") != std::string::npos);

  // Retiring the server generation during the real hash callback rejects the old worker.
  reset();
  unsigned hash_calls = 0;
  mock::after_hash = [&] {
    if (++hash_calls == 1) {
      begin_vm_generation(false); ++mock::epoch;
      check(common_helper::test_context.publish(mock::epoch.load(), 8, "zm_stalingrad"));
    }
  };
  check(transform_map_stock(unknown_digest.name, &unknown_digest) == &unknown_digest);
  mock::after_hash = {};
  check(hash_calls == 1 && !mock::failed && mock::failures == 0 && !current_map_failure().available);

  // A successor native ticket also rejects stale work even without a store-generation change.
  reset();
  hash_calls = 0;
  mock::after_hash = [&] {
    if (++hash_calls == 1) {
      ++mock::epoch;
      check(common_helper::test_context.publish(mock::epoch.load(), 8, "zm_stalingrad"));
    }
  };
  check(transform_map_stock(unknown_digest.name, &unknown_digest) == &unknown_digest);
  mock::after_hash = {};
  check(hash_calls == 1 && !mock::failed && mock::failures == 0 && !current_map_failure().available);

  // Terminal unpinned-helper failure is last: its real emergency path exhausts the store.
  reset();
  check(lookup_map_helper(t7::map_companion_names[5].data()) == nullptr);
  const auto helper = current_map_failure();
  check(mock::failed && mock::failures == 1 && helper.available);
  check(helper.stage == map_diagnostics::failure_stage::helper_lookup);
  check(helper.row == -1 && helper.helper == 5);
  check(text(helper.reason) == "helper_retention_declined");
  check(text(helper.detail) == "Map companion has not been pinned.");
  check(helper.source.bytes == map_mechanics::compiled_payloads[5].bytes.size() && !helper.source.sha256_computed);
  check(error_for(helper).find("map31_failure=helper_lookup row=-1 helper=5") != std::string::npos);
  check_error_path();
  common_helper::invalidate_native_session();
  check(!current_map_failure().available);
  std::cout << "checks=" << checks
            << " actual_component_bodies=true synthetic_sources=true stale_publication_rejected=true"
               " error_formatter=true native_engine=false linked_body_authenticated=false\n";
}
'''


def actual_function(source, marker):
    start = source.index(marker)
    assert source.find(marker, start + 1) < 0, marker
    position = source.index('{', start)
    depth, cursor = 1, position + 1
    while cursor < len(source):
        if source.startswith('//', cursor):
            cursor = source.index('\n', cursor)
            continue
        if source.startswith('/*', cursor):
            cursor = source.index('*/', cursor) + 2
            continue
        if source[cursor] in ('"', "'"):
            quote = source[cursor]
            cursor += 1
            while cursor < len(source):
                if source[cursor] == '\\':
                    cursor += 2
                    continue
                if source[cursor] == quote:
                    cursor += 1
                    break
                cursor += 1
            continue
        if source[cursor] == '{':
            depth += 1
        elif source[cursor] == '}':
            depth -= 1
            if not depth:
                return source[start:cursor + 1]
        cursor += 1
    raise ValueError('Unclosed actual function: ' + marker)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--compiler', required=True)
    parser.add_argument('--compiler-arg', action='append', default=[])
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    repo = args.repo.resolve()
    source = repo / 'src/client/component/zombies_schema_assets.cpp'
    fixture = repo / 'tests/zombies_coop_castle_context.py'
    public = repo / 'src/client/component/zombies/schema_assets.hpp'
    diagnostic = repo / 'src/client/component/zombies/map_failure_diagnostic.hpp'
    coop_source = repo / 'src/client/component/zombies_coop.cpp'
    nodes = ast.parse(fixture.read_text()).body
    prelude = next(ast.literal_eval(node.value) for node in nodes
        if isinstance(node, ast.Assign) and isinstance(node.value, ast.Constant)
        and any(isinstance(target, ast.Name) and target.id == 'prelude' for target in node.targets))
    prelude = prelude.replace('namespace mock {\n', 'namespace mock {\nstd::atomic_int epoch{1};\n', 1)
    provider = '''namespace common_helper {
native_publication test_context;
native_ticket capture_native_session() noexcept {return test_context.capture();}
void invalidate_native_session() noexcept {test_context.invalidate();}
bool native_session_current(const native_ticket& saved) noexcept {
  return test_context.provenance_current(saved,mock::epoch.load(),mock::capacity.load(),mock::failed.load());
}
bool native_session_owned(const native_ticket& saved) noexcept {
  return test_context.owned(saved,mock::epoch.load(),mock::capacity.load());
}
bool native_admission_current() noexcept {return native_session_current(capture_native_session());}
bool revoke_native_session(const native_ticket& saved) noexcept {
  return saved && saved->epoch==mock::epoch.load() && saved->capacity==mock::capacity.load() && test_context.revoke(saved);
}
}'''
    prelude, count = re.subn(r'namespace common_helper \{\n.*?\n\}', lambda _: provider, prelude, count=1, flags=re.S)
    assert count == 1
    old_epoch = 'int active_startup_epoch() noexcept{return 1;}'
    assert prelude.count(old_epoch) == 1
    prelude = prelude.replace(old_epoch, 'int active_startup_epoch() noexcept{return mock::epoch.load();}')
    prelude = prelude.replace('#include "HEADER_ROOT/zombies/map_runtime_bridge.hpp"',
        '#include "HEADER_ROOT/zombies/map_failure_diagnostic.hpp"\n#include "HEADER_ROOT/zombies/map_runtime_bridge.hpp"')
    interface = public.read_text()
    start = interface.index('using common_lifecycle')
    end = interface.index('bool common_required()', start)
    prelude = prelude.replace('COMMON_ALIASES', interface[start:end])
    start = interface.index('// Retained map module/helper code dependencies')
    end = interface.index('// Pin the exact bytes', start)
    prelude = prelude.replace('MAP_DECLARATIONS', interface[start:end])
    prelude = prelude.replace('HEADER_ROOT', (repo / 'src/client/component').as_posix())
    prelude = prelude.replace('LENGTH_TYPE', 'int')
    prelude = prelude.replace('// This legacy Castle fixture intentionally has no common-source session.\n'
        '// Positive provider/pair behavior is covered by common_context/common_sources.',
        '// This fixture uses the real atomic native-publication provider with controlled epoch/capacity.')
    original = source.read_text()
    body = '\n'.join(line for line in original.splitlines() if not line.lstrip().startswith('#include'))
    assert 'current_map_failure()' in body, 'Actual diagnostic implementation is required'
    ezz = '#define BOIII_COOP_EZZ' in original
    error_sink = '''
namespace mock {
const char *startup_error{};
unsigned error_calls{};
int error_code{-1};
std::string error_message;
}
namespace game {
ERROR_DROP_SINK
}
namespace zombies::coop {
const char *poll_startup() { return mock::startup_error; }
ACTUAL_ERROR_PATH
}
'''
    if ezz:
        sink = '''enum class errorParm { DROP=42 };
namespace com { void Com_Error(errorParm code,const char *format,const char *message) {
  assert(std::string_view(format)=="%s");++mock::error_calls;
  mock::error_code=static_cast<int>(code);mock::error_message=message;
} }'''
    else:
        sink = '''inline constexpr int ERR_DROP=42;
void Com_Error(int code,const char *format,const char *message) {
  assert(std::string_view(format)=="%s");++mock::error_calls;
  mock::error_code=code;mock::error_message=message;
}'''
    error_sink = error_sink.replace('ERROR_DROP_SINK', sink).replace('ACTUAL_ERROR_PATH', actual_function(coop_source.read_text(), 'void check_startup()'))
    # This deterministic synthetic object passes only the size/magic gate, then fails SHA-256.
    contract = repo / 'src/client/component/zombies/map_mechanics_contracts.hpp'
    fixed_main = contract.read_text().split('namespace zombies::coop::map_import_binding::fixed_stalingrad_main {', 1)[1]
    object_size = int(re.search(r'c\.object_size\s*=\s*(\d+)', fixed_main).group(1))
    synthetic = bytes((0x80, 0x47, 0x53, 0x43, 0x0d, 0x0a, 0, 0x1c)) + bytes(object_size - 8)
    expected = hashlib.sha256(synthetic).digest()
    tests = TESTS.replace('EXPECTED_MAIN_DIGEST', 'std::array<std::uint8_t,32>{' + ','.join(map(str, expected)) + '}')
    tests = tests.replace('EXPECTED_MAIN_HEX', expected.hex())
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    generated = out / 'actual_map_failure_context.cpp'
    generated.write_text(prelude + '\n' + body + '\n' + error_sink + '\n' + tests)
    binary = out / 'actual_map_failure_context.exe'
    compiler = [args.compiler] if Path(args.compiler).is_file() else shlex.split(args.compiler, posix=os.name != 'nt')
    command = compiler + args.compiler_arg + ['/nologo', '/std:c++20', '/EHsc', '/W4', '/WX',
        '/Fe:' + str(binary), '/Fo:' + str(binary.with_suffix('.obj')), str(generated), 'bcrypt.lib']
    compiled = subprocess.run(command, cwd=out, capture_output=True, text=True, timeout=120)
    (out / 'compile.log').write_text(compiled.stdout + compiled.stderr)
    assert compiled.returncode == 0, out / 'compile.log'
    run = subprocess.run([str(binary)], cwd=out, capture_output=True, text=True, timeout=30)
    (out / 'stdout.log').write_text(run.stdout)
    (out / 'stderr.log').write_text(run.stderr)
    assert run.returncode == 0, (out, run.stderr)
    sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
    proof = dict(passed=True, stdout=run.stdout, command=command,
        actual_source_sha256=sha(source), public_interface_sha256=sha(public),
        actual_startup_source_sha256=sha(coop_source),
        actual_diagnostic_header_sha256=sha(diagnostic), generated_sha256=sha(generated),
        fixture_prelude_sha256=sha(fixture), test_sha256=sha(Path(__file__)),
        scope='Complete actual asset-component body, actual check_startup error wrapper, and real diagnostic formatter under native state/hash/poll/drop stand-ins. Synthetic source objects only; no engine longjmp, runtime/canary or linked-body authentication proof.')
    (out / 'RESULTS.private.json').write_text(json.dumps(proof, indent=2) + '\n')
    print(json.dumps(dict(proof=str(out / 'RESULTS.private.json'), stdout=run.stdout)))


if __name__ == '__main__':
    main()
