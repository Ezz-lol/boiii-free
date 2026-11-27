"""Native source guards for safe bot harness commands; no live client is removed."""
from pathlib import Path
import re
import sys
root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[1]
source = (root / "src/client/component/bots.cpp").read_text(encoding="utf-8")
assert "atoi(" not in source
assert 'on_g_init_game(invalidate_spawn_batches)' in source
assert 'on_g_shutdown_game(invalidate_spawn_batches)' in source
assert 'schema_server_ready()' in source and 'frozen_schema_capacity()' in source and 'admission_ready()' in source
assert 'frozen_schema_map() == current.map' in source
roster = source[source.index("std::string roster_map_token"):source.index("bool drop_current_test_client")]
assert "bIsTestClient" in roster and "COOP_BOT_ROSTER|" in roster and "fflush(stdout)" in roster
assert not re.search(r'\.name|xuid|guid|address|userinfo|password|rcon', roster, re.I)
drop = source[source.index("bool drop_current_test_client"):source.index("struct component final")]
assert "policy::may_drop_test_client" in drop and "target->bIsTestClient" in drop
assert "CONNECTED" in drop and "SV_DropClient(target" in drop
assert drop.index("policy::may_drop_test_client") < drop.index("SV_DropClient(target")
assert not re.search(r'execute[^\n]*kick|clientkick|kickclient', drop, re.I)
assert 'command::add("coopKickTestClient"' in source and 'command::add("coopBotRoster"' in source
assert "*generation != requested_epoch" in source
assert re.search(r"requested\s*\?\s*requested->epoch\s*:\s*ending_requested\s*\?\s*ending_requested->identity.epoch\s*:\s*0", source)
assert "read_roster_session()" in source and "read_ending_cleanup_session()" in source
assert "cleanup::may_cleanup_test_client" in source
assert "policy::parse_unsigned(params[1], 17)" in source
assert "policy::parse_unsigned(params[2], UINT64_MAX)" in source
print("PASS bounded native bot roster, generation checks, exact test-client-only native drop and no human kick route")
