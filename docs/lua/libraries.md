# Lua Libraries

The following Lua libraries are added to the Lua VM.

## T7Recharged Compatibility

The following Lua libraries are added to the Lua VM to allow compatibility with
mods that expect a Lua API matching that provided by T7Recharged.

Where a function is specified to be "stubbed", this means that the function
either does not return a value and immediately returns, or immediately returns
the default value for its return type. The function is essentially a no-op.

### FileIO

The `FileIO` library is added with the following functions.

- `ClipboardGet()`
- `ClipboardSet(contents)`
- `Copy(srcPath, destPath)`
- `CopyDirectory(srcPath, destPath)`
- `CreateDirectory(path)`
- `DirectoryExists(path)`
- `FileExists(path)`
- `FileSize(path)`
- `HardLink(srcPath, destPath)`
- `ListFiles(path)`
- `Move(srcPath, destPath)`
- `ReadFile(path)`
- `T7PatchLoaded()` - always returns `true`
- `Wine()`
- `WriteFile(path, contents, append = false)`

### Console

The `Console` library is added with the following functions.

- `Print(message)`
- `PrintFile(file, message)`
- `PrintInfo(message)`
- `PrintError(message)`
- `PrintWarning(message)`
- `ShowExternalConsole()`

### Axios

The `Axios` library is added with the following functions.

- `Get(uri)`
- `GetEncrypted(uri)` - alias for `Get`
- `GetRetrieve(uri)` - alias for `Get`
- `GetUpdate(uri)` - alias for `Get`
- `GetUpdateEncrypted(uri)` - alias for `Get`
- `Ping(uri)` - stubbed
- `Post(uri, body)`
- `PostEncrypted(uri, body)` - alias for `Post`

### MainLUA

The `MainLUA` library is added with the following functions.

- `GetHardwareCPU()` - always returns "generic"
- `GetHardwareID()` - always returns "generic"
- `GetTime()`
- `GetXuid()`
- `LuiReload()` - stubbed
- `ModFolder()`
- `ModPublisherID()`
- `ModuleLoaded(dll)` - always returns `true`
- `RemoveUIError()` - stubbed
- `RestartHUD()` - stubbed
- `Revision()`
- `StartTimer()`
- `StopTimer()`
- `UnloadMod()` - stubbed
- `Version()`

### JSON

The `JSON` library is added with the following functions.

- `ReadJson(file, top_level_key, sub_tree_key, type, default_value)`

  Note: the usage of `top_level_key` and `sub_tree_key` as arguments is a poorly
  designed API, in my opinion. This function signature is provided for
  compatibility with T7Recharged, and the next is provided as a more standard
  API for JSON key specification.

- `ReadJson(file, key, type, default_value)`

  Nested tree keys can be specified with standard JSON path syntax, e.g.
  `"/key1/key2/key3"`.

- `WriteJson(file, top_level_key, sub_tree_key, type, value)`
- `WriteJson(file, key, type, value)`

### DiscordRPC

The `DiscordRPC` library is added with the following stubbed functions.

- `ClearPresence()`
- `Enable(id)`
- `GetJoinSecret()`
- `IsAvailable()`
- `OpenInvitePlayers()`
- `Shutdown()`

### NET

The `NET` library is added with the following stubbed functions.

- `DisableDemonware()`
- `DisableSteam()`
- `EnableDemonware()`
- `EnableSteam()`
- `IsDemonwareDisabled()`
- `IsSteamDisabled()`

### HotReload

The `HotReload` library is added with the following stubbed functions.

- `Start()`

### UIErrorHash

The `UIErrorHash` library is added with the following stubbed functions.

- `Remove()`

### Video

The `Video` library is added with the following stubbed functions.

- `HookVideoPath()`
