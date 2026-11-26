"""Check revision-1C callback imports/exports; this is not runtime validation."""
from pathlib import Path
import argparse
import hashlib
import struct


def retail_hash(text):
    value = 0x4B9ACE2F
    for byte in text.lower().encode("ascii") + b"\0":
        value = ((value ^ byte) * 0x1000193) & 0xFFFFFFFF
    return value


def inspect(path):
    data = path.read_bytes()
    assert len(data) >= 0x48 and data[:8] == bytes.fromhex("804753430d0a001c")
    u16 = lambda at: struct.unpack_from("<H", data, at)[0]
    u32 = lambda at: struct.unpack_from("<I", data, at)[0]
    includes = []
    for index in range(data[0x44]):
        at = u32(u32(0x0C) + index * 4)
        assert 0x48 <= at < len(data)
        includes.append(data[at:data.index(b"\0", at)].decode("ascii").replace("\\", "/"))
    imports = set()
    at = u32(0x24)
    for _ in range(u16(0x3C)):
        name, namespace, refs, params, flags = struct.unpack_from("<IIHBB", data, at)
        imports.add((name, namespace, params, flags & 15))
        at += 12 + refs * 4
        assert at <= len(data)
    exports = {u32(u32(0x20) + index * 20 + 8) for index in range(u16(0x3A))}
    return includes, imports, exports


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiled", type=Path, required=True, help="ACTS common object; shipped CB payload or a fresh Ezz source compile")
    parser.add_argument("--fixture", type=Path, required=True, help="Freshly compiled isolated common self-test object")
    args = parser.parse_args()
    includes, imports, exports = inspect(args.compiled)
    assert "scripts/zm/_zm_behavior" in includes or "scripts/zm/_zm_behavior.gsc" in includes
    assert "scripts/zm/gametypes/_zm_gametype" in includes or "scripts/zm/gametypes/_zm_gametype.gsc" in includes
    assert "scripts/zm/gametypes/_globallogic" in includes or "scripts/zm/gametypes/_globallogic.gsc" in includes
    assert not any("zm_zod" in item for item in includes), includes
    for name, namespace in (("onendgame", "zm_gametype"), ("notetrackboardmelee", "zm_behavior"), ("blank", "globallogic")):
        assert (retail_hash(name), retail_hash(namespace), 0, 1) in imports, (name, namespace)
    # New global calls are stock APIs: native BG builtin table lists GetArrayKeys,
    # IsEntity and IsPlayer. No struct/vector type helper is assumed or imported.
    for name in ("getarraykeys", "isentity", "isplayer"):
        assert (retail_hash(name), retail_hash("eroded_coop"), 1, 2) in imports, name
    for name in ("isstruct", "isvector", "gettype"):
        assert not any(item[0] == retail_hash(name) for item in imports), name
    for name in ("disconnect_endgame_pending", "disconnect_player_list", "disconnect_rosters_empty", "endgame_guard_owned", "melee_guard_owned",
                 "disconnect_guards_owned", "install_disconnect_guards", "guarded_endgame", "guarded_window_melee"):
        assert retail_hash(name) in exports, name
    fixture_includes, fixture_imports, fixture_exports = inspect(args.fixture)
    assert any(item in ("custom_scripts/zm/eroded_coop", "custom_scripts/zm/eroded_coop.gsc") for item in fixture_includes)
    assert retail_hash("test_disconnect_guards") in fixture_exports
    for name in ("disconnect_player_list", "disconnect_rosters_empty", "endgame_guard_owned", "melee_guard_owned"):
        assert any(item[:2] == (retail_hash(name), retail_hash("eroded_coop")) for item in fixture_imports), name
    for path in (args.compiled, args.fixture):
        print(str(path) + " sha256=" + hashlib.sha256(path.read_bytes()).hexdigest())
    print("PASS: exact retail callback imports, bounded-list builtin imports and helper fixture exports; engine execution remains separate")


if __name__ == "__main__":
    main()
