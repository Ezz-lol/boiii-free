"""Compile actual connected-player policy and source seams with native stand-ins.

Requires Python 3.9+ and a configured C++20 compiler. Select it with --compiler
or CXX. Generated sources, compiler output and results are retained in a new
output directory. This runs no game, engine hook, download or remote operation.
"""
from pathlib import Path
import argparse
import datetime
import hashlib
import json
import os
import subprocess
import sys

sys.dont_write_bytecode = True
from script_frame_diagnostics import between, function

ROOT = Path(__file__).resolve().parents[1]
PREFIX = r'''
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <optional>
#include <type_traits>
#include "zombies/connected_players.hpp"
namespace policy = zombies::coop::connected_players_policy;
namespace snapshot = zombies::coop::builtin_snapshot;
unsigned checks = 0;
#define CHECK(value) do { ++checks; if (!(value)) { std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#value); std::abort(); } } while (false)
policy::context good() { return {true,true,true,true,false,true,8}; }
'''

POLICY_CASES = r'''
snapshot::snapshot captured(std::uintptr_t base) {
  snapshot::snapshot result{};
  result.valid = true;
  result.image_base = base;
  result.image_size = 0xA000000;
  result.checksum = snapshot::dedicated_checksum;
  result.entry_address = base + snapshot::common_table - snapshot::preferred_base
      + snapshot::connected_index * sizeof(snapshot::builtin_entry);
  result.action_address = base + policy::action_rva;
  result.code_size = static_cast<std::uint32_t>(policy::pristine_body.size());
  result.expected.canonical = snapshot::expected_hash;
  result.expected.action = base + snapshot::expected_action - snapshot::preferred_base;
  result.connected.canonical = snapshot::connected_hash;
  result.connected.action = result.action_address;
  std::copy(policy::pristine_body.begin(),policy::pristine_body.end(),result.code.begin());
  return result;
}
int main() {
  static_assert(std::is_trivially_destructible_v<policy::context>);
  struct client { int state; };
  // Every scope bit and capacity boundary, including stock 18-slot modes.
  for (unsigned mask=0; mask<64; ++mask) {
    for (int capacity=-1; capacity<=19; ++capacity) {
      policy::context current{(mask&1)!=0,(mask&2)!=0,(mask&4)!=0,
          (mask&8)!=0,(mask&16)!=0,(mask&32)!=0,capacity};
      const bool expected = mask==47 && capacity>=5 && capacity<=8;
      CHECK(policy::eligible(current)==expected);
      int reads=0;
      const int result=policy::expected_floor(2,current,[&] { ++reads; return 5; });
      CHECK(reads==(expected?1:0));
      CHECK(result==(expected?5:2));
      for (int instance=-1; instance<=2; ++instance) {
        for (unsigned index=0; index<=2; ++index) {
          CHECK(policy::default_value(instance,index,false,current)==
              (expected && instance==0 && index==0));
          CHECK(policy::default_value(instance,index,true,current));
        }
      }
      if (!expected) {
        // Deliberately inaccessible: out-of-scope evaluation must not read it.
        CHECK(policy::native_connected_count(current,reinterpret_cast<const client*>(1),3)==0);
      }
    }
  }
  auto current=good();
  for (int original=-1; original<=20; ++original) {
    for (int counted=-1; counted<=9; ++counted) {
      int calls=0;
      CHECK(policy::expected_floor(original,current,[&] { ++calls;return counted; })==
          ((counted>=0 && counted<=8)?std::max(original,counted):original));
      CHECK(calls==1);
    }
  }
  std::array<client,18> clients{};
  clients[0].state=0;clients[1].state=1;clients[2].state=2;
  clients[3].state=3;clients[4].state=4;clients[5].state=5;
  clients[6].state=5;clients[7].state=5;
  for (std::size_t i=8;i<clients.size();++i) clients[i].state=5;
  CHECK(policy::native_connected_count(current,clients.data(),3)==5);
  CHECK(policy::native_connected_count(current,clients.data(),5)==3);
  CHECK(policy::native_connected_count(current,static_cast<const client*>(nullptr),3)==0);
  // Eight-entry backing object: a read beyond the configured eight is invalid.
  std::array<client,8> exact{};for(auto& entry:exact)entry.state=5;
  CHECK(policy::native_connected_count(current,exact.data(),3)==8);
  for(int capacity=5;capacity<=8;++capacity) {
    current.capacity=capacity;
    CHECK(policy::native_connected_count(current,clients.data(),3)==capacity-3);
  }
  for(const auto base:{snapshot::preferred_base,std::uintptr_t{0x180000000}}) {
    auto original=captured(base);CHECK(policy::verified_body(original));
    original.expected.maximum=1;CHECK(policy::verified_body(original));
    original.connected.maximum=1;CHECK(policy::verified_body(original));
    for(std::size_t i=0;i<policy::pristine_body.size();++i) {
      auto changed=original;changed.code[i]^=1;CHECK(!policy::verified_body(changed));
    }
    auto reject=[&](auto change) { auto changed=original;change(changed);CHECK(!policy::verified_body(changed)); };
    reject([](auto& s){s.valid=false;});
    reject([](auto& s){++s.checksum;});
    reject([](auto& s){s.image_size=0;});
    reject([](auto& s){s.image_size=policy::action_rva+policy::pristine_body.size()-1;});
    reject([](auto& s){++s.entry_address;});
    reject([](auto& s){s.entry_address=s.image_base-1;});
    reject([](auto& s){++s.action_address;});
    reject([](auto& s){s.action_address=s.image_base-1;});
    reject([](auto& s){s.code_size=static_cast<std::uint32_t>(policy::pristine_body.size()-1);});
    reject([](auto& s){s.code_size=257;});
    reject([](auto& s){++s.connected.canonical;});
    reject([](auto& s){++s.connected.action;});
    reject([](auto& s){s.connected.minimum=1;});
    reject([](auto& s){s.connected.maximum=2;});
    reject([](auto& s){s.connected.type=1;});
    reject([](auto& s){++s.expected.canonical;});
    reject([](auto& s){s.expected.minimum=1;});
    reject([](auto& s){s.expected.maximum=2;});
    reject([](auto& s){s.expected.type=1;});
    reject([](auto& s){++s.expected.action;});
    reject([](auto& s){s.image_base=(std::numeric_limits<std::uintptr_t>::max)()-0x1000;});
  }
  std::printf("Actual policy: %u checks PASS (scope matrix, lazy reads, capacities, states, executed-body identity)\n",checks);
}
'''

SHIM_CASES = r'''
policy::context supplied=good();
int context_reads=0,getter_reads=0,last_instance=-9;unsigned last_index=99;
bool last_default=false,throw_from_getter=false;
std::optional<bool> explicit_argument;
namespace zombies::coop {
connected_players_policy::context connected_players_context() noexcept { ++context_reads;return supplied; }
}
namespace game { NAMESPACE_OPEN
using scriptInstance_t=int;
bool Scr_GetBoolOptional(int instance,unsigned index,bool default_value) {
  CHECK(context_reads==getter_reads+1);++getter_reads;
  last_instance=instance;last_index=index;last_default=default_value;
  if(throw_from_getter)throw 7;
  return explicit_argument.value_or(default_value);
}
NAMESPACE_CLOSE }
namespace zombies::coop { ACTUAL_SHIM }
int main() {
  for(unsigned scope=0;scope<2;++scope) {
    supplied=good();supplied.opted_in=scope!=0;
    for(int instance=0;instance<=1;++instance)for(unsigned index=0;index<=1;++index)
      for(unsigned original=0;original<2;++original)for(int argument=-1;argument<=1;++argument) {
        if(argument<0)explicit_argument.reset();else explicit_argument=argument!=0;
        const int before=getter_reads;
        const bool result=zombies::coop::connected_player_default_stub(instance,index,original!=0);
        const bool expected_default=original!=0||(scope && instance==0 && index==0);
        CHECK(getter_reads==before+1 && context_reads==getter_reads);
        CHECK(last_instance==instance && last_index==index && last_default==expected_default);
        CHECK(result==(argument<0?expected_default:argument!=0));
      }
  }
  // Engine validation may exit nonlocally: the wrapper delegates exactly once.
  throw_from_getter=true;
  bool caught=false;try { (void)zombies::coop::connected_player_default_stub(0,0,false); }
  catch(int value) { caught=value==7; }
  CHECK(caught && getter_reads==context_reads);
  std::printf("Actual optional-default shim: %u checks PASS (explicit false/true, one delegate, ordering)\n",checks);
}
'''

EZZ_CASES = r'''
policy::context supplied=good();int context_reads=0,pool_reads=0,added=-1,add_calls=0;
int lobby_count=0,fixture_min_players=0;
namespace zombies::coop {
connected_players_policy::context connected_players_context() noexcept { ++context_reads;return supplied; }
}
namespace game {
enum class eModes { ZOMBIES,CAMPAIGN,MULTIPLAYER };
eModes selected_mode=eModes::ZOMBIES;
namespace lobby { enum class LobbyType { GAME };enum class LobbyClientType { ALL };
int LobbyHost_GetClientCount(LobbyType,LobbyClientType) { return lobby_count; } }
namespace com { eModes Com_SessionMode_GetMode() { return selected_mode; } }
namespace net { enum class clientState_t { FREE, ZOMBIE, RECONNECTING, CONNECTED, CLIENTLOADING, ACTIVE }; }
namespace sv { struct client { net::clientState_t state=net::clientState_t::FREE; };
struct slot { client* value=nullptr;client* operator*() const {++pool_reads;return value;} };slot svs_clients; }
namespace scr { constexpr int SCRIPTINSTANCE_SERVER=0;
void Scr_AddInt(int instance,int value) { CHECK(instance==0);++add_calls;added=value; } }
}
struct min_setting { int get_int() const { return fixture_min_players; } } lobby_min_players;
ACTUAL_EXPECTED
int main() {
  std::array<game::sv::client,18> clients{};
  for(auto& entry:clients)entry.state=game::net::clientState_t::ACTIVE;
  game::sv::svs_clients.value=clients.data();
  for(const auto mode:{game::eModes::ZOMBIES,game::eModes::CAMPAIGN,game::eModes::MULTIPLAYER}) {
    game::selected_mode=mode;
    for(unsigned scope=0;scope<2;++scope)for(int lobby=-1;lobby<=10;++lobby)
      for(const auto minimum:{-1,0,1,4,8,12}) {
        lobby_count=lobby;fixture_min_players=minimum;supplied=good();
        supplied.zombies=mode==game::eModes::ZOMBIES;supplied.opted_in=scope!=0;
        const int baseline=(mode!=game::eModes::MULTIPLAYER && minimum>0)?minimum:lobby;
        const bool scoped=mode==game::eModes::ZOMBIES && scope;
        const int expected=std::max(1,scoped?std::max(baseline,8):baseline);
        const int before=pool_reads,adds_before=add_calls;
        scr_get_num_expected_players();
        CHECK(pool_reads-before==(scoped?1:0));CHECK(add_calls==adds_before+1);
        CHECK(added==expected);
      }
  }
  // Native startup counts CONNECTED and loading clients, matching the old CB helper.
  supplied=good();game::selected_mode=game::eModes::ZOMBIES;lobby_count=1;fixture_min_players=0;
  clients[0].state=game::net::clientState_t::FREE;
  clients[1].state=game::net::clientState_t::ZOMBIE;
  clients[2].state=game::net::clientState_t::RECONNECTING;
  clients[3].state=game::net::clientState_t::CONNECTED;
  clients[4].state=game::net::clientState_t::CLIENTLOADING;
  scr_get_num_expected_players();CHECK(added==5);
  supplied=good();supplied.capacity=4;game::sv::svs_clients.value=reinterpret_cast<game::sv::client*>(1);
  const int before=pool_reads;scr_get_num_expected_players();CHECK(pool_reads==before && added==1);
  std::printf("Actual Ezz expected-player function: %u checks PASS (baseline modes/minimums, lazy native floor)\n",checks);
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default=os.environ.get('CXX', 'cl'))
    parser.add_argument('--output-dir', type=Path, default=ROOT / 'test-output')
    args = parser.parse_args()
    source_path = ROOT / 'src/client/component/zombies_connected_players.cpp'
    source = source_path.read_text(encoding='utf-8')
    shim = between(source, '// BEGIN CONNECTED PLAYER DEFAULT SHIM',
                   '// END CONNECTED PLAYER DEFAULT SHIM')
    family = 'ezz' if 'game::scr::scriptInstance_t instance' in shim else 'cb'
    sources = [source_path, ROOT / 'src/client/component/zombies/connected_players.hpp',
               ROOT / 'src/client/component/zombies/builtin_snapshot.hpp']
    forbidden = ('Scr_GetNumParam', 'Scr_GetInt', 'Scr_GetBool(', 'scrVmPub', 'Scr_Add',
                 'std::lock', 'unique_lock', 'lock_guard')
    if any(token in shim for token in forbidden) or shim.count('Scr_GetBoolOptional(') != 1:
        raise ValueError('Optional-default shim must delegate once without VM stack/lock work')
    if not (source.index('if (!game::is_server())') < source.index('builtin_snapshot::capture(')
            < source.index('if (!connected_players_policy::verified_body(')
            < source.index('utils::hook::call(')
            < source.index('connected_hook_installed.store(true')):
        raise ValueError('Native installation must retain dedicated/capture/identity guards')
    capture_source = sources[2].read_text(encoding='utf-8')
    if 'result.action_address = static_cast<std::uintptr_t>(result.connected.action);' not in capture_source:
        raise ValueError('Capture no longer binds action address to the checked table entry')
    units = [('policy', PREFIX + POLICY_CASES),
             ('actual_optional_shim', PREFIX + SHIM_CASES.replace('NAMESPACE_OPEN', 'namespace scr {' if family == 'ezz' else '')
              .replace('NAMESPACE_CLOSE', '}' if family == 'ezz' else '').replace('ACTUAL_SHIM', shim))]
    if family == 'ezz':
        path = ROOT / 'src/client/component/patches.cpp'
        patches = path.read_text(encoding='utf-8')
        floor = between(patches, '// BEGIN EXPANDED ZOMBIES EXPECTED FLOOR',
                        '// END EXPANDED ZOMBIES EXPECTED FLOOR')
        actual = function(patches, 'void scr_get_num_expected_players()')
        if floor not in actual or floor.index('expected_floor(') > floor.index('*game::sv::svs_clients'):
            raise ValueError('Expected-player native pool access must stay inside the lazy callback')
        sources.append(path)
        units.append(('actual_expected_players', PREFIX + EZZ_CASES.replace('ACTUAL_EXPECTED', actual)))
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    out = args.output_dir.resolve() / (family + '-connected-players-' + stamp)
    out.mkdir(parents=True, exist_ok=False)
    records = []
    report = {'family': family, 'passed': False,
              'scope': 'Actual source with engine/native stand-ins; not engine runtime proof.',
              'compiler': args.compiler, 'sources': {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in sources},
              'fixtures': records, 'log': str(out / 'checks.log')}
    msvc = Path(args.compiler).name.lower() in ('cl', 'cl.exe', 'clang-cl', 'clang-cl.exe')
    try:
        with (out / 'checks.log').open('x', encoding='utf-8') as log:
            for stem, generated in units:
                cpp = out / (stem + '.cpp')
                cpp.write_text(generated, encoding='utf-8')
                exe = out / (stem + ('.exe' if os.name == 'nt' else ''))
                include = ROOT / 'src/client/component'
                if msvc:
                    command = [args.compiler, '/nologo', '/std:c++20', '/EHsc', '/MT', '/W4', '/WX',
                               '/I' + str(include), '/Fe:' + str(exe), '/Fo:' + str(out / (stem + '.obj')), str(cpp)]
                else:
                    command = [args.compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror',
                               '-I' + str(include), str(cpp), '-o', str(exe)]
                log.write(json.dumps({'compile': command}) + '\n'); log.flush()
                subprocess.run(command, cwd=out, stdout=log, stderr=log, check=True)
                log.write(json.dumps({'run': [str(exe)]}) + '\n'); log.flush()
                subprocess.run([str(exe)], cwd=out, stdout=log, stderr=log, check=True)
                records.append({'fixture': stem, 'passed': True,
                                'generated_source_sha256': hashlib.sha256(cpp.read_bytes()).hexdigest()})
        report['passed'] = True
    except (subprocess.CalledProcessError, OSError) as exc:
        report['error'] = str(exc)
        raise
    finally:
        (out / 'RESULTS.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
        print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
