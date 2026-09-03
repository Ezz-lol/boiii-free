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
- `CreateDirectory(path)`
- `DirectoryExists(path)`
- `FileExists(path)`
- `FileSize(path)`
- `ReadFile(path)`
- `WriteFile(path, contents)`

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
- `GetRetrieve(uri)` - alias for `Get`
- `GetUpdate(uri)` - alias for `Get`
- `GetUpdateEncrypted(uri)` - alias for `Get`
- `Post(uri, body)`
- `PostEncrypted(uri, body)` - alias for `Post`

### MainLUA

The `MainLUA` library is added with the following functions.

Note: each of these returns either a generic or default value, and is not a true
implementation. These functions were provided only to support mods which require
them - they return a value that is minimally sufficient to achieve this goal.

- `GetHardwareCPU()`
- `GetHardwareID()`
- `ModuleLoaded(dll)`
- `Revision()`
- `UnloadMod()`

### DiscordRPC

The `DiscordRPC` library is added with the following stubbed functions.

- `Shutdown()`
- `IsAvailable()`
- `ClearPresence()`
- `Enable(id)`

### HotReload

The `HotReload` library is added with the following stubbed functions.

- `Start()`

### UIErrorHash

The `UIErrorHash` library is added with the following stubbed functions.

- `Remove()`
