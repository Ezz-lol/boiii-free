"""Structural loader checks; --lua also evaluates the readiness suffix in Lua 5.1.

Neither mode executes the game's HKS VM.
"""
import ast
import pathlib
import re
import sys

root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".")
native = (root / "src/client/component/zombies_lobby_vm.cpp").read_text()
companion = (root / "data/lobby_scripts/zombies_coop/__init__.lua").read_text()

assert 'module_path = "lobby_scripts/zombies_coop/__init__.lua"' in native
assert "!game::is_client()" in native and 'has_flag("experimental-zombies-coop")' in native
assert re.search(r"state == \*(?:primary_luaVM|game::hks::lua_state)", native)
assert not re.search(r"\*(?:primary_luaVM|game::hks::lua_state)\s*=", native)
assert "ui_scripting::" not in native  # no primary-VM refs, functions or GC objects
assert "state->m_apistack.top - state->m_apistack.bottom" in native
assert "state->m_apistack.top = state->m_apistack.bottom + top" in native
assert "state->m_global->m_bytecodeSharingMode = mode" in native
assert re.search(r"(?:lobby_vm_pcall|hksi_lua_pcall)\(state, 0, 1, 0\)", native)
stub = native.split("void lobby_vm_init_stub", 1)[1].split("} // namespace", 1)[0]
assert stub.index("client_hooks_ready.store(false") < stub.index("lobby_vm_init_hook.invoke<void>()") < stub.index("load_client_lobby_hooks();")
for diagnostic in ("invalid_vm", "read_failed", "compile_failed", "execute_failed", "installed", "incomplete", "load_failed"):
    assert diagnostic in native, diagnostic
assert "client_hooks_ready.store(false" in native and "client_hooks_ready.store(installed" in native
assert "source.size() > 65536" in native
assert "utils::io::read_file" not in native
assert "std::string bytes(65537, '\\0')" in native
assert not re.search(r"std::array\s*<\s*char\s*,\s*65537\s*>", native)
read_at = native.index("input.read(bytes.data(), static_cast<std::streamsize>(bytes.size()))")
count_at = native.index("input.gcount()", read_at)
assign_at = native.index("source.assign(bytes.data(), static_cast<std::size_t>(count))", count_at)
read_guard = native[count_at:assign_at]
for check in ("input.bad()", "!input.eof() && input.fail()", "count <= 0", "count > 65536"):
    assert check in read_guard, check
assert re.search(r"event=read_failed[^}]*\breturn;", read_guard)
assert "path = candidate" not in native[read_at:assign_at]
assert not re.search(r"\b(?:require|dofile|loadfile)\s*\(", companion)

# Check the exact Lua result expression compiled by the native loader, rather
# than accepting a previous-version HostStart marker as evidence of readiness.
chunk = re.search(r"const std::string code\s*=\s*source\s*\+([\s\S]*?);", native).group(1)
suffix = "".join(ast.literal_eval(s) for s in re.findall(r'"(?:\\.|[^"\\])*"', chunk))
assert "LobbyVM.ZombiesCoopModuleVersion==4" in suffix
assert "type(Lobby.Actions.LobbyHostStart)=='function'" in suffix
for callback, marker in (("LobbyHostStart", "ZombiesCoopHostStart"), ("LaunchGame", "ZombiesCoopLaunchGame"), ("CanLobbyCanMigrate", "ZombiesCoopCanMigrate")):
    assert callback + "==" in suffix and marker in suffix
print("PASS: structural client-only LobbyVM routing, fixed companion, protected call, stack restoration and strict version readiness")

if "--lua" in sys.argv[2:]:
    # The workstation test runner supplies Lupa here; a CI runner can install
    # Lupa normally. This remains a Lua stand-in, never game-HKS validation.
    libraries = root.resolve().parent / "tools/python-libs"
    if libraries.is_dir():
        sys.path.insert(0, str(libraries))
    from lupa.lua51 import LuaRuntime

    fixture = """
local host, launch, migrate = function() end, function() end, function() end
Lobby = {Actions={LobbyHostStart=host, ZombiesCoopHostStart=host}}
LobbyVM = {ZombiesCoopModuleVersion=4, LaunchGame=launch, ZombiesCoopLaunchGame=launch,
    CanLobbyCanMigrate=migrate, ZombiesCoopCanMigrate=migrate}
"""
    lua = LuaRuntime()
    assert lua.execute(fixture + suffix) is True
    for mutation in (
        "Lobby.Actions.LobbyHostStart=nil; Lobby.Actions.ZombiesCoopHostStart=nil;",
        "Lobby.Actions.LobbyHostStart=function() end;",
        "Lobby.Actions=nil;",
        "Lobby=nil;",
        "LobbyVM.ZombiesCoopModuleVersion=2;",
        "LobbyVM.ZombiesCoopModuleVersion=3;",
        "LobbyVM.LaunchGame=nil; LobbyVM.ZombiesCoopLaunchGame=nil;",
        "LobbyVM.CanLobbyCanMigrate=nil; LobbyVM.ZombiesCoopCanMigrate=nil;",
    ):
        assert lua.execute(fixture + mutation + suffix) is False, mutation
    print("PASS: exact native readiness suffix rejects missing callbacks, nil marker pairs, changed ownership and old versions in Lua 5.1")
