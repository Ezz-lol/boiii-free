"""Source checks for cached-only crash collection; not an injected engine crash."""
from pathlib import Path
import re
import sys
root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[1]
source = (root / "src/client/component/zombies_diagnostics.cpp").read_text(encoding="utf-8")
crash = source[source.index("std::string crash_diagnostics()"):source.index("struct diagnostics_component")]
assert not re.search(r"game::|get_dvar|Dvar_|mutex|lock_guard|scoped_lock|schema_server_ready|frozen_schema", crash)
assert "cached_state.copy" in crash and "events.copy" in crash
assert "catch (...)" in crash and 'return "{}"' in crash
record = source[source.index("void diagnostic_event("):source.index("std::string crash_diagnostics()")]
assert "events.record" in record and "GetTickCount64" in record
assert not re.search(r"printf|game::|get_dvar|mutex|lock_guard|std::string\s", record)
assert 'has_flag("coop-debug")' in source
assert re.search(r"scheduler::loop\(capture_snapshot, scheduler::(?:pipeline::)?main", source)
arrays = source[source.index("constexpr const char *integers") if "constexpr const char *integers" in source else source.index("constexpr const char* integers"):source.index("std::string snapshot =")]
allowed = set(re.findall(r'"([A-Za-z0-9_]+)"', arrays))
required = {"mapname", "g_gametype", "sv_running", "com_maxclients", "zm_coopPlayers", "zm_coopStartupEpoch",
            "zm_coopSchemaServerRevision", "zm_coopSchemaClientRevision", "zm_coopRoundStatus",
            "zm_coopRoundTarget", "zm_coopRoundConsumed", "zm_coopRoundPending", "zm_coopLastEvent", "zm_coopIslandTrialsStatus"}
assert required <= allowed
assert all(name in {"mapname", "g_gametype", "sv_running", "com_maxclients"} or name.startswith("zm_coop") for name in allowed)
assert not any(re.search(r"password|^rcon|secret|token|xuid|userinfo|commandline|endpoint", name, re.I) for name in allowed)
ring = (root / "src/client/component/zombies/diagnostic_ring.hpp").read_text(encoding="utf-8")
assert "event_limit = 64" in ring and "phase_limit = 32" in ring
assert "is_always_lock_free" in ring and "compare_exchange_strong" in ring
assert not re.search(r"std::mutex|lock_guard|scoped_lock|\.wait\(|sleep\(|while\s*\(", ring)
collector = (root / "src/client/component/exception.cpp").read_text(encoding="utf-8")
writer = collector[collector.index("void write_minidump("):]
assert writer.index("write_file(coop_name, coop_info)") < writer.index("generate_crash_info(exceptioninfo)")
assert writer.index("write_file(coop_name, coop_info)") < writer.index("create_minidump(exceptioninfo)")
assert 'zip_file.add("zombies-coop.json", coop_info)' in writer
assert 'line(zombies::coop::crash_diagnostics())' in collector
print("PASS cached-only crash path, explicit diagnostic allowlist, bounded lock-free records and early report sidecar")
