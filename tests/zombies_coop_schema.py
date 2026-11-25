"""Validate the paired BO3 1C schema assets and their explicit source contract.

This checks payload linking/isolation and frozen contracts. The test-only GSC/CSC
system fixtures supply runtime initialization evidence; this is not a game VM.
"""
import hashlib
import json
from pathlib import Path
import struct
import sys

root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[1]
base = root / "scripts/zombies_coop"
manifest = json.loads((base / "schema-v1.json").read_text(encoding="utf-8"))
assert (manifest["revision"], manifest["stock_slots"], manifest["schema_slots"], manifest["slot_range"]) == (1, 4, 8, [4, 7])
canonical = json.dumps(manifest["groups"], sort_keys=True, separators=(",", ":")).encode()
contract = hashlib.sha256(canonical).hexdigest()
assert manifest["contract_sha256"] == contract
by_name = {row["name"]: row for row in manifest["groups"]}
assert len(by_name) == 7 and len({row["id"] for row in by_name.values()}) == 7
assert by_name["revive"]["version"] == 1 and by_name["revive"]["bits"] == 5
assert by_name["revive"]["callback"] == "zm_laststand::update_bleedout_timer"
assert by_name["score"]["suffixes"] == {
    ".score_cf_damage": 3, ".score_cf_death_normal": 2, ".score_cf_death_torso": 2,
    ".score_cf_death_neck": 2, ".score_cf_death_head": 2, ".score_cf_death_melee": 2,
}
for name, version, bits in (("castle_inventory", 1, 1), ("stalingrad_wearable", 12000, 2),
                            ("genesis_wearable", 15000, 4), ("moon_wearable", 21000, 1)):
    row = by_name[name]
    assert (row["version"], row["bits"], row["callback"]) == (version, bits, "zm_utility::setsharedinventoryuimodels")
assert by_name["island_slot_state"]["suffixes"] == {"_1": 3, "_2": 3, "_3": 3}
assert by_name["island_slot_state"]["additional"]["bits"] == 1


def fnv(value):
    result = 0x4B9ACE2F
    for char in value.lower().encode() + b"\0":
        result = ((result ^ char) * 0x1000193) & 0xFFFFFFFF
    return result


def inspect(path, extension):
    data = path.read_bytes()
    assert len(data) >= 0x48 and data[:8] == bytes.fromhex("804753430d0a001c")
    u16 = lambda offset: struct.unpack_from("<H", data, offset)[0]
    u32 = lambda offset: struct.unpack_from("<I", data, offset)[0]
    code, size = u32(0x14), u32(0x30)
    assert 0x48 <= code <= len(data) and size <= len(data) - code
    name_offset = u32(0x34)
    assert 0x48 <= name_offset < len(data)
    name = data[name_offset:data.index(b"\0", name_offset)].decode()
    assert name == "scripts/zm/eroded_coop_fields." + extension, name
    includes = []
    for index in range(data[0x44]):
        offset = u32(u32(0x0C) + 4 * index)
        assert 0x48 <= offset < len(data)
        includes.append(data[offset:data.index(b"\0", offset)].decode().replace("\\", "/"))
    assert "scripts/shared/system_shared" in includes
    assert "scripts/shared/clientfield_shared" in includes
    assert "scripts/zm/_zm_laststand" in includes
    assert not any("zm_zod" in name or "zm_tomb" in name for name in includes)
    assert ("scripts/zm/_zm_utility" in includes) == (extension == "csc")
    imports = set()
    offset = u32(0x24)
    for _ in range(u16(0x3C)):
        name, namespace, refs, params, flags = struct.unpack_from("<IIHBB", data, offset)
        imports.add((name, namespace, params))
        offset += 12 + 4 * refs
        assert offset <= len(data)
    assert (fnv("register"), fnv("system"), 4) in imports
    assert (fnv("register"), fnv("clientfield"), 8 if extension == "csc" else 5) in imports
    if extension == "csc":
        assert any(n == fnv("update_bleedout_timer") and ns == fnv("zm_laststand") for n, ns, _ in imports)
        assert any(n == fnv("setsharedinventoryuimodels") and ns == fnv("zm_utility") for n, ns, _ in imports)
    exports = []
    for index in range(u16(0x3A)):
        _, address, name, namespace, params, flags = struct.unpack_from("<IIIIBB", data, u32(0x20) + 20 * index)
        assert code <= address < code + size
        exports.append((name, namespace, params, flags))
    assert any(n == fnv("__init__system__") and ns == fnv("eroded_coop_fields") and flags & 2 for n, ns, _, flags in exports)
    return data


for extension, side in (("gsc", "Server"), ("csc", "Client")):
    source = (base / ("eroded_coop_fields." + extension)).read_text(encoding="utf-8")
    assert contract in source and 'getdvarint("zm_coopSchemaSlots")' in source
    assert 'system::register("eroded_coop_fields", &register_schema, undefined, "zm_laststand")' in source
    assert 'for (slot = 4; slot < 8; slot++)' in source
    assert 'zm_coopSchema' + side + 'Revision' in source
    assert 'zm_coopSchema' + side + 'Mask' in source and 'zm_coopSchema' + side + 'Contract' in source
    assert 'zm_coopPlayerLimit' not in source and 'getplayers' not in source.lower()
    assert 'coop_schema_capacity(' not in source and 'coop_schema_report(' not in source
    assert not (root / "data/custom_scripts/zm" / ("eroded_coop_fields." + extension)).exists()
    inspect(root / "data/coop_scripts" / ("eroded_coop_fields." + extension), extension)
    fixture = (root / "tests/gsc" / ("eroded_coop_schema_selftest." + extension)).read_text(encoding="utf-8")
    assert contract in fixture and '"eroded_coop_fields"' in fixture
    for official_map in ("zm_zod", "zm_factory", "zm_castle", "zm_island", "zm_stalingrad", "zm_genesis", "zm_prototype",
                         "zm_asylum", "zm_sumpf", "zm_theater", "zm_cosmodrome", "zm_temple", "zm_moon", "zm_tomb"):
        assert 'schema_family_mask("' + official_map + '")' in fixture
print("PASS: paired BO3 1C asset names/imports/autoexec, fixed schema manifest and stock-session isolation")
