# Asset Override

Assets provided by Treyarch, mod, or custom maps can be overriden.

At this time, BOIII supports overriding `Rawfile` and `ScriptParseTree` (GSC or
CSC script) assets.

GSC and CSC script overrides are handled uniquely, and as such are
[documented separately](gsc-scripting.md#where-scripts-can-live).

## Filesystem Scheme

Asset overrides must be placed in either of the following trees.

- %LOCALAPPDATA/boiii/data
- <game installation directory>/boiii

The override's path relative to the tree must match the name of the asset in the
engine.

### Mod-specific Override

Mod-specific asset overrides are supported. These assets must be placed in a
directory with name matching the given mod's publisher ID, which must be located
at the top level of the given BOIII data tree. The override's path relative to
the directory with name matching the given mod's publisher ID must match the
name of the asset in the engine.

Mod-specific asset overrides will only be loaded if the given mod is loaded, and
will not be loaded otherwise.

### Map-specific Override

Map-specific asset overrides are supported. These assets must be placed in a
directory with name matching the given map's internal name, which must be
located at the top level of the given BOIII data tree. The override's path
relative to the directory with name matching the given map's internal name must
match the name of the asset in the engine.

Map-specific asset overrides will only be loaded if the given map is loaded, and
will not be loaded otherwise.

## Examples

- To override the asset with name "lua/lobby/ffotd.lua", you would place the
  override - compiled lua bytecode - in
  "<game installation>/boiii/lua/lobby/ffotd.lua"
- To override the asset with name "default_bindings.cfg" in the mod with
  publisher ID 2631943123, you would place the override in
  "<game installation>/boiii/2631943123/default_bindings.cfg"
- To override the asset with name "vision/zm_coast.vision" in the map with
  internal name "zm_coast", you would place the override in
  "<game installation>/boiii/zm_coast/vision/zm_coast.vision".
