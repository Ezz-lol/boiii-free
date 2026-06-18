# GSC scripting additions

This compiler is intentionally more relaxed than stock BO3. It accepts a hybrid
of BO2-style and BO3-style syntax, so older scripts are easier to port and you
can mix styles when it helps.

## Quick links

- [Where scripts can live](#where-scripts-can-live)
- [Compiler and loader notes](#compiler-and-loader-notes)
- [Hooks](#hooks)
- [Commands and output](#commands-and-output)
- [File and data helpers](#file-and-data-helpers)
- [Player helpers](#player-helpers)
- [HUD text](#hud-text)
- [Small helpers](#small-helpers)

## Where scripts can live

The loader checks a few places, depending on what kind of script you are adding.

```text
%LOCALAPPDATA%/boiii/data/
├── scripts/
│   └── ... stock script overrides such as scripts/zm/... or scripts/mp/...
├── custom_scripts/
│   ├── shared/
│   ├── core/
│   ├── codescripts/
│   ├── <mode-specific folder>
│   ├── <mapname>/
│   └── your_script.gsc
└── other files used by your scripts

<game folder>/boiii/
├── scripts/
│   └── ... same override idea as above
└── custom_scripts/
    └── ... same custom script layout as above
```

Small notes:

- `scripts/` is for overriding stock scripts the game already uses.
- `custom_scripts/` is for your own extra scripts that BOIII compiles and loads
  for you.
- `custom_scripts/shared`, `custom_scripts/core`, `custom_scripts/codescripts`,
  the active mode folder, and the active map folder are scanned recursively.
- The top level `custom_scripts/` folder is also loaded, but only at that folder
  level.
- For imports and inserted files, the compiler first checks paths near the
  current source file, then `%LOCALAPPDATA%/boiii/data`,
  `%LOCALAPPDATA%/boiii/data/scripts`, `<game folder>/boiii`, and
  `<game folder>/boiii/scripts`.

## Compiler and loader notes

- The `function` keyword is optional for normal function definitions.
- `#using` and `#include` are still accepted, but you do not need to import a
  file just to call one of its functions by full path.
- Function references accept both `::func` and `&func`, plus namespaced forms
  like `&namespace::func`.
- Direct path calls like `scripts\zm\_zm_score::get_points_multiplier(self)` are
  accepted and the compiler can resolve the path namespace for you.
- Raw hash-style names are recognized when a namespace or function name is
  hashed. Supported prefixes are `hash_`, `id_`, `function_`, `var_`, and
  `namespace_`.
- RawFile-backed custom scripts are accepted by the loader too, which helps
  custom scripts resolve cleanly instead of depending only on stock
  `scriptparsetree` lookups.

Example:

```gsc
main()
{
    multiplier = scripts\zm\_zm_score::get_points_multiplier(self);
    callback_a = ::on_test;
    callback_b = &on_test;
    callback_c = &scripts\zm\_zm_score::player_add_points;
}

function on_test()
{
    println("callback hit");
}
```

Raw hash-name example:

```gsc
replacefunc(namespace_12345678::function_9ABCDEF0, ::my_hook);
```

# Hooks

## replacefunc

`replacefunc` uses 2 parameters:

```gsc
replacefunc(scripts\zm\_zm_score::player_add_points, ::hooked_player_add_points);

function hooked_player_add_points(event, mod, hit_location, is_dog, zombie_team, damage_weapon)
{
    println("[test] hooked player_add_points");
}
```

Notes:

- First argument is the target function reference.
- Second argument is the replacement function reference.
- The replacement should use the same parameter list as the original.
- `replacefunc` is registered at runtime, so call it from `main()` or another
  startup path before the target starts getting used.
- `clearreplacefuncs()` clears all active replacements.

## detour

`detour` is the compile/load-time form. You write the replacement as the
function body itself.

Simple form:

```gsc
detour scripts\zm\_zm_score::player_add_points(event, mod, hit_location, is_dog, zombie_team, damage_weapon)
{
    println("[test] detour hit");
}
```

Explicit namespace plus script path form:

```gsc
detour globallogic_score<scripts\mp\gametypes\_globallogic_score.gsc>::givePlayerScore(event, player, victim, descValue, weapon)
{
    println("[test] givePlayerScore detour hit");
}
```

Use the second form when you want a custom namespace name or when the file path
matters.

Notes:

- `detour` is registered while scripts load, so it is ready before normal
  runtime code starts calling the target.
- The detour body fully replaces the original export.
- Just like `replacefunc`, keep the same parameter list as the original
  function.
- `detour` and `replacefunc` both end up using the same redirect backend once
  they are active. The main difference is when they get registered.

# Commands and output

## addcommand

There are 2 ways to use `addcommand`.

Legacy queue form:

```gsc
addcommand("testcmd");

autoexec function command_loop()
{
    for (;;)
    {
        cmd = getcommand("testcmd");
        if (isdefined(cmd) && cmd != "")
        {
            println("raw command: " + cmd);
        }

        wait 0.05;
    }
}
```

Callback form:

```gsc
addcommand("testcmd", ::cmd_testcmd);

function cmd_testcmd(args)
{
    if (args.size < 1)
    {
        println("usage: testcmd <value>");
        return;
    }

    println("first arg = " + args[0]);
}
```

Notes:

- In the 2-parameter form, the callback gets only the arguments after the
  command name.
- `testcmd hello world` becomes `args[0] = "hello"` and `args[1] = "world"`.
- The compiler generates the small polling loop for the callback form for you.
- `getcommand("name")` still works if you want the raw queued string yourself.

## executecommand

Use `executecommand` when you want GSC to push a console command into the
command buffer.

```gsc
executecommand("status");
executecommand("fast_restart");
```

## say and tell

Use `say` to send a chat message to everyone:

```gsc
say("Server message");
```

Use `tell` to send a chat message to one player:

```gsc
tell(0, "Private message");
player tell("Private message");
```

`tell` supports both the normal function form and the entity method form.

## print, println and printf

These write to the console/log. They do not send chat messages.

```gsc
print("loading...");
println("done");
printf("score = %d", self.score);
```

# File and data helpers

All file helpers use paths relative to `boiii/scriptdata`.

## Text file helpers

Available helpers:

- `writefile(path, data[, append])`
- `appendfile(path, data)`
- `readfile(path)`
- `fileexists(path)`
- `removefile(path)`
- `removedirectory(path)` / `rmdir(path)`
- `rm(path[, recurse])`
- `filesize(path)`
- `createdirectory(path)` / `mkdir(path)`
- `directoryexists(path)`
- `listfiles(path)`
- `ls(path[, recurse[, include_directories]])`

Example:

```gsc
mkdir("logs");
writefile("logs/test.txt", "hello\n");
appendfile("logs/test.txt", "world\n");

content = readfile("logs/test.txt");
println(content);

entries = ls("logs", false, false);
```

Notes:

- `listfiles` returns a comma-separated string.
- `ls` returns an array, which is usually easier to work with in scripts.

## JSON helpers

Available helpers:

- `jsonvalid(json_string)`
- `jsonparse(json_string, key)`
- `jsonset(json_string, key, value_string)`
- `jsondump(path, json_string)`

Example:

```gsc
json = "{}";
json = jsonset(json, "mode", "\"zm\"");
json = jsonset(json, "round", "25");

if (jsonvalid(json))
{
    println("mode = " + jsonparse(json, "mode"));
    jsondump("logs/state.json", json);
}
```

Notes:

- `jsonparse` reads one key from a JSON object and returns the value as a
  string.
- `jsonset` tries to parse the value as JSON first. If that fails, it stores it
  as a string.

## Int64 helpers

Available helpers:

- `int64_op(a, op, b)`
- `int64_isint(value)`
- `int64_toint(value)`
- `int64_min(a, b)`
- `int64_max(a, b)`
- `int64_abs(value)`
- `int64_clamp(value, min, max)`
- `int64_tostring(value)`

Example:

```gsc
big_value = int64_op("5000000000", "+", "25");
println(big_value);

if (!int64_isint(big_value))
{
    println("value is larger than a normal GSC int");
}
```

Notes:

- Int64 helpers return strings for the non-boolean results.
- Use `int64_toint` only when the value is in normal 32-bit range.

# Player helpers

## setname and resetname

These can be called either by client number or from the player entity itself.

```gsc
setname(0, "PlayerOne");
resetname(0);

player setname("PlayerOne");
player resetname();
```

The override is synced to clients and the player gets updated after the change.

## settag and resettag

These work the same way as the name helpers.

```gsc
settag(0, "DEV");
resettag(0);

player settag("DEV");
player resettag();
```

The clan tag override is synced to clients too.

## setclientdvar

This sends a dvar command to one client.

```gsc
setclientdvar(0, "cg_fov 120");
player setclientdvar("cg_fov 120");
```

Notes:

- This is client-side. It does not change the server's own dvar value.
- Sent dvars are tracked, and BOIII resets them when the script system shuts
  down so the client is not left with stale overrides from an old match.

# HUD text

The server-side HUD text path was patched so repeated text updates do not hit
the old configstring overflow crash as easily.

You still use the normal HUD `setText` call. The fix is under the hood.

Example:

```gsc
function hud_text_demo()
{
    elem = newhudelem();
    elem.x = 320;
    elem.y = 120;
    elem.fontscale = 1.5;
    elem setText("Round " + level.round_number);
}
```

# Small helpers

## getfunction

`getfunction(script, function)` returns whether the export can be found.

```gsc
if (getfunction("scripts/zm/_zm_score", "player_add_points"))
{
    println("player_add_points exists");
}
```

This is handy when you want to guard optional hooks.

## conststring

`conststring(hash)` pushes a const string from a scr string hash.

```gsc
hash = 0x12345678;
value = conststring(hash);
```

## Method-style custom calls

The project also supports the cleaner entity-style form for a few helpers:

```gsc
player tell("hello");
player setname("newname");
player settag("dev");
player resetname();
player resettag();
player setclientdvar("cg_fov 120");
```

That is just a cleaner script-side syntax for the same builtins.
