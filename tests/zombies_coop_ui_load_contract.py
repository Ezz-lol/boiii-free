"""Source contract checks for the early HKS wrapper seam; not a game VM test."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
ui = root / "src/client/component/ui_scripting.cpp"
if not ui.exists():
    ui = root / "src/client/component/lua/ui_scripting.cpp"
source = ui.read_text()
begin = source.index("hks_load_stub(")
end = source.index("lua_cod_getrawfile_stub(", begin)
load = source[begin:end]
assert load.count("wrap_score_model_chunk(state, chunk_name, result)") == 2
assert "load_buffer(state, globals.raw_script_name" in load
buffer_begin = source.index("int load_buffer(")
buffer_end = source.index("void load_script(", buffer_begin)
assert "*primary_luaVM" not in source[buffer_begin:buffer_end]
assert "*game::hks::lua_state" not in source[buffer_begin:buffer_end]

native = (root / "src/client/component/zombies_ui_schema.cpp").read_text()
assert "state != *" in native and "score_model_chunk(chunk_name(name))" in native
assert "slots < 5 || slots > 8" in native
assert "top - state->m_apistack.bottom - 1" in native
assert "protected_call(state, 0, 1, 0)" in native
assert "push_value(state, -2)" in native
assert "protected_call(state, 1, 1, 0)" in native
assert "state->m_apistack.bottom[original] = state->m_apistack.top[-1]" in native
assert "m_bytecodeSharingMode = previous_mode" in native
# The helper must never replace a VM global or execute the stock chunk directly.
assert not re.search(r"\*(?:hks::lua_state|hks::primary_luaVM)\s*=", native)
assert "vm_call_internal" not in native
print("early HKS UI load source contract: PASS (runtime client unverified)")
