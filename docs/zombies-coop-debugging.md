# Zombies coop debugging

Add `-coop-debug` to the normal client or dedicated launch command to print
`[coop_debug]` state samples and phase events. Expanded gameplay still needs
`-experimental-zombies-coop` and matching data. Debug output does not change
capacity, authentication, clientfield checksums or gameplay rules.

## Script frame positions

`-coop-debug` does not enable script-error frames. Also add `-log-script-errors`
to enable Ezz's existing script-error logger. Each frame retains its raw
`next_pc`, explicit `fault_pc = next_pc - 1`, asset-relative `script_offset`,
bytecode-relative `code_offset`, function name/hash and `function_offset` when
available, including when line information is absent. Null/end positions and
missing metadata are labelled. Invalid script instances never index VM pools;
frame counts are validated against the 64-frame engine array and output is
limited to the current position plus 16 callers.
`fault_pc` is the VM source-lookup byte address and can lie inside an operand;
it is neither a native exception RIP nor a guaranteed opcode start.

Run `coopScriptErrorStatus` through the existing console or authorized RCON to
read the same `game::log_script_errors()` predicate used by the error logger.
One flushed `COOP_SCRIPT_LOGGING|` JSON record reports role, support, enabled
state, whether the setting dvar is bound and whether the launch flag is present.
On the dedicated image, the client-only settings component normally leaves this
binding unbound, so the predicate uses `-log-script-errors`. On the client image,
a bound `boiii_logScriptErrors` setting takes precedence over the flag. Setting
the unrelated `sv_logScriptErrors` does not enable Ezz's logger, and querying an
unbound `boiii_logScriptErrors` cannot verify the dedicated flag predicate.
The command registers or changes no dvar. Confirm its actual enabled receipt
before testing; an empty error list or requested flag alone does not establish
script-clean startup, continuous observation or the absence of all errors.

Object/function metadata uses the existing per-instance object pool and validated
code/export bounds, with no new database lookup or script-asset load. Existing
optional GDB/source-line resolution remains. File, function and source text are
bounded in each output frame. Both known script-instance layouts are supported
by source; actual client runtime validation is still absent. Keep logs private
and match positions to the exact binary/source/PDB. These additional diagnostics
do not repair teardown or establish gameplay compatibility.

The bounded recorder runs without this flag. It retains up to 64 events with a
31-character source-defined phase, monotonic tick time and two integers. A
contended writer drops its event rather than waiting. `events_dropped` records
those losses. Main-thread state is sampled once per second into a bounded atomic
cache. An interrupted publication can leave `cached_state` unavailable; crash
collection does not wait for the interrupted thread.

## Crash reports

Existing crash reports include `zombies-coop.json` and a copy of the cached
information in `info.txt`. CB writes `minidumps/boiii-crash-*.zip` relative to the
working directory. Ezz writes `%LOCALAPPDATA%/boiii/minidumps/ezz-crash-*.zip`
(inside the selected Wine prefix when running under Wine).

A `.coop.json` sidecar is saved before the wider report or native minidump is
built. Text and JSON sidecars remain if report generation fails; a successful ZIP
removes those sidecars. These operations are best effort, including when the
process heap or dump generator is damaged.

The new JSON contains only the role, build/commit, schema contract hash, bounded
phase events and an explicit allowlist: map/mode, server state, capacity, epochs,
actual GSC player count, schema receipts and map/owner/spawn/GobbleGum/round state.
Numeric, boolean and numeric-string dvars are read according to their type;
unexpected types are ignored. It does not collect names, account IDs, network
addresses, passwords, tokens, command-line arguments or script payloads. The
existing `crash.dmp` remains a process dump and should stay private.

The common minidump writer also requests a bounded instruction-memory window.
It queries the fault address's page before entering DbgHelp, then considers only
the immediately preceding and following pages. A page must be fully contained
in a committed region with readable executable protection. Guard, no-access,
execute-only, uncommitted and unavailable pages are skipped. If the fault page
is ineligible, neighbouring pages are not collected. This also works for private
executable allocations that are absent from the normal loaded-module list, such
as the manually mapped engine; it does not depend on decrypting the stock file.

The additional request is at most three system pages: 12 KiB on the tested x64
Windows system with 4 KiB pages. Page sizes above 64 KiB are rejected. No
whole-process-memory flag is added, and the existing minidump flags, exception
context, sidecars, module/thread flags and normal file I/O are preserved. The
per-write callback only yields precomputed ranges; it performs no allocation,
memory query, engine call or game-lock acquisition. A page can still become
unavailable between querying and writing, so inclusion remains best effort.

`tests/fault_memory.cpp` checks selection, page boundaries, gaps, protections,
overflow, callback exhaustion and independent write contexts.
`tests/fault_memory_dump.cpp` links the actual common writer in a standalone
process, allocates a small private executable test region, supplies a synthetic
exception context without raising an exception, and reads the generated dump
back to verify all three pages' exact bytes and absence from the module list.
This validates the local Windows dump mechanism; it is not an engine crash,
Wine canary, real-client or gameplay test. Its private dumps must remain private.

The callback contract follows Microsoft's
[MemoryCallback enumeration](https://learn.microsoft.com/en-us/windows/win32/api/minidumpapiset/ne-minidumpapiset-minidump_callback_type)
and [callback output structure](https://learn.microsoft.com/en-us/windows/win32/api/minidumpapiset/ns-minidumpapiset-minidump_callback_output).
The page filter follows
[VirtualQuery](https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualquery)
and the [memory protection constants](https://learn.microsoft.com/en-us/windows/win32/memory/memory-protection-constants).

A last phase locates activity near a failure. Confirm the cause using the
exception address, registers, stack/script position, checksum errors and the
matching executable/source revision. Events from another thread can appear immediately before an unrelated fault.
The main-thread sample can become much older than one second during a blocked
load. Compare `capture_tick_ms`, `cached_state.sample_tick_ms` and event ticks
in the same monotonic clock domain before treating cached state as current.
Receipt dvars describe execution; native payload pins and clientfield checksum
validation remain separate compatibility checks.

## Safe bot harness commands

Run these through an already authorized local console or RCON connection on a
ready hosted session. Expanded Zombies requires the frozen native capacity and
successful server schema/startup checks. These commands use the engine's native
test-client operations; they do not provide a remote authentication bypass or
establish full combat AI behavior.

- `spawnBot` adds one test client. `spawnBot 1` through `spawnBot 18` requests a
  bounded count. `spawnBot all` fills currently available slots. Occupied humans,
  bots and loading/reconnecting slots consume capacity. The session is checked
  before each allocation; map, mode, generation or capacity changes cancel the
  remainder. An eight-slot listen host therefore has room for seven bots.
- `coopBotRoster` queues a server-thread snapshot and prints one flushed line:
  `COOP_BOT_ROSTER|{"version":1,"generation":12,"map":"zm_zod","capacity":8,"slots":[{"slot":0,"test":true,"state":5}]}`.
  Slots are zero-based and only nonfree native slots are listed. `test` is the
  native `bIsTestClient` flag. No names or account IDs are included. A refusal
  returns a version-one object with `error` and no slot list.
- `coopKickTestClient <slot> <generation>` accepts slots 0-17 and the generation
  from a fresh roster for the current process. It rechecks the same session,
  generation, capacity, connected state and native test-client flag on the server
  thread immediately before the exact native drop. A slot reused by a human is
  refused. It prints `COOP_BOT_KICK|{"slot":0,"generation":12,"dropped":true}`
  or a refusal with `dropped:false`.

Read these markers from the captured console log: queued output may occur after
the RCON reply redirect ends. A successful drop reports the native operation;
wait for native cleanup and actual GSC membership to settle before asserting a
new population. Never infer bot identity from a name or a guessed status line.

## Validation boundary

For a private optimized Release with matching native symbols, generate the
projects with `premake5 --coop-debug-symbols vs2022` before the normal x64 Release
MSBuild step. The default Release symbol policy is unchanged. Keep that exact
`boiii.pdb`, executable and source commit together; a different build's PDB cannot
reliably explain the crash. PDBs and dumps may contain private information and
should not be attached publicly.

The test-only `eroded_coop_schema_selftest.gsc` reports
`zm_coopSchemaServerTestStatus=passed` and
`zm_coopSchemaServerTestFailures=0` after checking the early server schema and
extra laststand/name records. Its CSC counterpart requires a real client VM.
Do not ship either fixture as normal application data.

Dedicated canaries and bots do not validate a joining client's CSC/bootstrap,
UI field registration, score widgets, revive indicators or occupied network
sessions. Those require matching real clients, slots 5-8, down/revive and points
events, disconnect/rejoin, rotations and each map/custom-map adapter under test.


## Bounded connected-player builtin capture

On the supported dedicated-server image only, `-coop-debug` requests one early
read-only capture of the connected-player builtin metadata and exactly 256 code
bytes. `coopDumpConnectedPlayersBuiltin` permits a manual retry if that capture
failed. There are at most two attempts per process and no further capture after
a successful write. Clients report unsupported before any engine-table access.

The private `logs/coop-builtin-PID-tick-attempt.json` file is created without
replacing an existing file and remains smaller than 2 KiB. It includes the image
identity, guarded table identity, action coordinates, code bytes and a failure
reason. Readable committed pages, the allocation/image bounds, known dedicated
checksum and executable section/page protections must all validate; an incomplete
or changed read fails closed. `COOP_BUILTIN_CAPTURE` reports whether the file and
capture succeeded. The early attempt can fail if the table is not yet unpacked;
the explicit retry is for that case.

This diagnostic does not invoke the builtin, change its return value, hook a
function, read the VM stack, produce a process dump or fix the startup/teardown
failure. It records no player names, XUIDs, credentials or other player secrets.
The captured game-code bytes and resulting analysis must remain private.

The table guard accepts zero required arguments and at most one optional argument
for each of the expected-player and connected-player entries. Their actual
minimum, maximum and type metadata are recorded separately. This only permits a
read-only code capture; neither zero-argument nor one-argument calls are made.
The optional argument's behavior must be established from the captured body
before considering any readiness-count change.


## Dedicated test-client readiness

The supported dedicated image's stock connected-player builtin counts ACTIVE
humans by default and includes ACTIVE native test clients only when its optional
boolean is true. With experimental coop enabled, an armed five-through-eight
Zombies session changes only that boolean's default to include test clients.
Explicit `getnumconnectedplayers(false)` and `(true)` keep their stock meanings.
The native ACTIVE filter, pool iteration, return-value writer and human readiness
rules remain in the engine. No client-image hook is installed.

Installation requires the supported image/table identities, exact captured
93-byte executed-function fingerprint and verified optional-getter CALL target; any
mismatch declines the change and records `connected_bots_declined` plus a
`COOP_CONNECTED_BOTS` receipt. The hook affects only that dedicated CALL and
releases all coop state locks before the original optional getter runs. Saved
capacity alone does not enable it: the current native allocation, map lifecycle,
mode and experimental flag must all agree. Ezz also floors its existing expected
player result to native clients at CONNECTED or later within that same session
scope, preserving its existing four-player/MP/CP/minimum-player behavior.

This source-backed correction still requires isolated startup, phase and
teardown verification with the exact resulting build. It does not itself prove
real-client gameplay, texture acknowledgements, HUD behavior or all-map quests.

## Castle selector and current round

`zm_coopCastleVOStatus` starts as `armed_for_known_module` when the paired server
fields are bound. `owned_selector_active` means the reviewed responder helper has
actually run; it is published once per VM, with a short `castle_selector=active`
receipt. A bounded refusal reason accompanies `declined_castle_dialogue_owner`.
The cached private crash snapshot includes this status. It contains no player
names, identifiers or audio alias contents.

`zm_coopRoundNumber` is the current typed stock round sampled by the owned,
epoch-checked heartbeat. Zero means no valid positive round has been observed;
it is not a synthetic round or a gameplay counter. Crash collection continues to
read only the cached snapshot, without entering the script VM.
