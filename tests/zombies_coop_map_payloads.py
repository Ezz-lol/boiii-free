"""Validate authored source/object parity and the fixed server-only map bundle.

No game files, engine process, compiler download, or network access required.
This checks packaging and metadata; it is not gameplay or linked-body proof.
"""
from pathlib import Path
import hashlib
import json
import re
import struct
import sys


def symbol(value):
    if re.fullmatch(r"(?:function|namespace)_[0-9a-fA-F]{1,8}", value):
        return int(value.split("_", 1)[1], 16)
    result = 0x4B9ACE2F
    for byte in value.lower().encode() + b"\0":
        result = ((result ^ byte) * 0x1000193) & 0xFFFFFFFF
    return result


def verify(root):
    folder = root / "scripts/zombies_coop/map_mechanics"
    manifest = json.loads((folder / "payloads.json").read_text())
    assert manifest["vm"] == "bo3-pc-1c"
    assert manifest["wire_schema_revision"] == 4
    assert manifest["wire_schema_changed"] is False
    assert manifest["server_only"] is True
    assert (manifest["capacity_min"], manifest["capacity_max"]) == (5, 8)
    header = root / manifest["generated_header"]
    assert hashlib.sha256(header.read_bytes()).hexdigest() == manifest["generated_header_sha256"]
    text = header.read_text()
    assert len(manifest["entries"]) == 8
    logical_names = set()
    exported = {}
    imports = []
    for index, row in enumerate(manifest["entries"]):
        assert row["index"] == index
        source_path = root / row["source"]
        assert source_path.resolve().is_relative_to(folder.resolve())
        source_bytes = source_path.read_bytes()
        assert hashlib.sha256(source_bytes).hexdigest() == row["source_sha256"]
        source = source_bytes.decode()
        match = re.search(r"> payload_" + str(index) + r"\{\n(.*?)\n\};", text, re.S)
        assert match, index
        data = bytes(int(value, 16) for value in re.findall(r"0x([0-9a-f]{2})", match[1]))
        assert len(data) == row["compiled_bytes"]
        assert hashlib.sha256(data).hexdigest() == row["compiled_sha256"]
        assert data[:8] == bytes.fromhex("804753430d0a001c")
        u32 = lambda offset: struct.unpack_from("<I", data, offset)[0]
        u16 = lambda offset: struct.unpack_from("<H", data, offset)[0]
        code, size = u32(0x14), u32(0x30)
        assert 0x48 <= code < code + size <= len(data)
        assert (code, size) == (row["code_offset"], row["code_bytes"])
        name_offset = u32(0x34)
        assert 0x48 <= name_offset < len(data)
        name = data[name_offset:].split(b"\0", 1)[0].decode()
        assert name == row["logical_name"] and name not in logical_names
        assert name.startswith("custom_scripts/zm/eroded_coop_") and name.endswith(".gsc")
        logical_names.add(name)
        namespace = re.search(r"^#namespace\s+([^;]+);", source, re.M)[1]
        assert namespace == row["namespace"]
        actual = {}
        for entry in range(u16(0x3A)):
            offset = u32(0x20) + entry * 20
            checksum, address, function, space, argc, flags = struct.unpack_from("<IIIIBB", data, offset)
            assert code <= address < code + size
            assert space == symbol(namespace) and function not in actual
            assert not (flags & 2), "map helpers must never autoexecute"
            actual[function] = {"params": argc, "flags": flags, "checksum": checksum}
            exported[(space, function)] = argc
        assert {f"{key:08x}": value for key, value in actual.items()} == row["exports"]
        functions = re.findall(r"^function\s+([A-Za-z0-9_]+)\s*\(([^)]*)\)", source, re.M)
        assert len(functions) == row["export_count"] == len(actual)
        for function, args in functions:
            argc = len(args.split(",")) if args.strip() else 0
            assert actual[symbol(function)]["params"] == argc
        # Imported script calls are cross-checked only when their namespace is
        # one of our explicitly named helpers. Flag0x20 permits the compiler
        # to resolve an unqualified name through builtins in the caller namespace;
        # engine/stock imports are validated by the game.
        for entry in range(u16(0x3C)):
            offset = u32(0x24)
            for _ in range(entry):
                offset += 12 + struct.unpack_from("<H", data, offset + 8)[0] * 4
            function, space, refs, argc, flags = struct.unpack_from("<IIHBB", data, offset)
            imports.append((space, function, argc, refs, flags, name))
    spaces = {space for space, _ in exported}
    for space, function, argc, refs, flags, caller in imports:
        if space in spaces and not (flags & 0x20):
            assert (space, function) in exported, f"missing transitive helper export: {caller} {space:08x}::{function:08x} argc={argc} flags={flags:02x}"
            # Flags 0x01/0x21 refer to a function rather than invoke it.
            assert (flags & 0x0F) == 1 or exported[(space, function)] == argc
    # Every retargeted import must exist in the independently compiled helpers.
    contract_header = (root / "src/client/component/zombies/map_mechanics_contracts.hpp").read_text()
    atom = r"(0x[0-9a-f]+|[0-9]+)"
    key = atom + r",\s*" + atom + r",\s*" + atom + r",\s*" + atom
    destinations = re.findall(r"c\.bindings\.push_back\(\{\{[^}]+\},\s*\{" + key + r"\}", contract_header)
    destinations += re.findall(r"rule\.destination\s*=\s*\{" + key + r"\}", contract_header)
    assert len(destinations) == 17
    for function, space, argc, flags in destinations:
        target = (int(space, 0), int(function, 0))
        assert target in exported, f"retargeted destination is absent: {target}"
        assert (int(flags, 0) & 15) == 1 or exported[target] == int(argc, 0)
    contracts = json.loads((folder / "contracts.json").read_text())
    assert contracts["wire_schema_changed"] is False
    assert contracts["linked_body_authenticated"] is False
    rows = contracts["stock_objects"]
    assert len(rows) == 9 and len({row["path"] for row in rows}) == 9
    assert sum(row["selected_references"] for row in rows) == 22
    assert sum(row["add_core_fields_include"] for row in rows) == 1
    assert next(row for row in rows if row["add_core_fields_include"])["profile"] == "stalingrad_main"
    for row in rows:
        assert row["helpers"] & ~255 == 0 and row["helpers"] != 0
        assert re.fullmatch("[0-9a-f]{64}", row["raw_sha256"])
        assert row["path"].startswith("scripts/zm/") and row["path"].endswith(".gsc")
    generic = next(row for row in rows if row["profile"] == "bgb_global")
    assert generic["map"] == "" and generic["required_for_map"] is False and generic["helpers"] == 64
    print("PASS eight source/object payloads, helper imports, nine contracts and 22 exact references; metadata only")


if __name__ == "__main__":
    verify(Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[1])
