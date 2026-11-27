local root = assert(arg[1], "pass the checkout directory")
local function dedicatedFixture()
  local state = {
    dedicated = false,
    mods = false,
    active = false,
    inGame = true,
    map = "",
    launching = false,
    recovery = false,
    limit = 8,
  }
  local writes, seen = {}, {}
  Enum = {
    LobbyType = { LOBBY_TYPE_GAME = 7, LOBBY_TYPE_PRIVATE = 8 },
    LobbyMainMode = { LOBBY_MAINMODE_ZM = 1 },
    LobbyMode = { LOBBY_MODE_CUSTOM = 3, LOBBY_MODE_PUBLIC = 4 },
  }
  Engine = {
    DvarInt = function(_, key)
      assert(key == "zm_coopPlayerLimit")
      return state.limit
    end,
    IsDedicatedServer = function()
      return state.dedicated
    end,
    IsDedicatedServerMods = function()
      return state.mods
    end,
    IsLobbyActive = function(kind)
      assert(kind == 7)
      return state.active
    end,
    IsInGame = function()
      return state.inGame
    end,
    GetCurrentMap = function()
      return state.map
    end,
    LobbyHostLaunchTaskInProgress = function()
      return state.launching
    end,
    IsLobbyInRecovery = function(kind)
      assert(kind == 7)
      return state.recovery
    end,
    SetDvar = function(name, value)
      writes[name] = value
    end,
  }
  Lobby = {
    Actions = {
      LobbyHostStart = function(...)
        seen.args, seen.count = { ... }, select("#", ...)
        return "host action", false, nil, "tail"
      end,
    },
  }
  LobbyVM = { LaunchGame = function() end, CanLobbyCanMigrate = function() end }
  dofile(root .. "/data/lobby_scripts/zombies_coop/__init__.lua")
  return state, writes, seen
end

-- Both stock dedicated entrypoints may create a GAME lobby before a client
-- frontend exists. Native IsInGame is not a frontend detector for this path.
for _, flags in ipairs({ { true, false }, { false, true }, { true, true } }) do
  for limit = 1, 8 do
    for _, capacity in ipairs({ 2, 3, 4 }) do
      for _, mode in ipairs({ 3, 4 }) do
        local state, writes, seen = dedicatedFixture()
        state.dedicated, state.mods, state.limit = flags[1], flags[2], limit
        state.inGame, state.map = true, "zm_zod"
        assert(select("#", Lobby.Actions.LobbyHostStart(0, 1, 7, mode, capacity, "extra", nil)) == 4)
        assert(seen.args[5] == limit and seen.args[6] == "extra" and seen.count == 7)
        assert(writes.com_maxclients == limit and writes.party_maxplayers == limit)
      end
    end
  end
end

-- An existing/migrated dedicated host owns its allocation, even with frontend
-- flags. Launch/recovery and PRIVATE lobby actions do not allocate a new GAME.
for _, flags in ipairs({ { true, false }, { false, true } }) do
  for _, guard in ipairs({ "active", "launching", "recovery", "private" }) do
    for _, capacity in ipairs({ 1, 2, 4, 5, 8, 18 }) do
      local state, writes, seen = dedicatedFixture()
      state.dedicated, state.mods = flags[1], flags[2]
      state.inGame, state.map = false, "core_frontend"
      local kind = 7
      if guard == "private" then
        kind = 8
      else
        state[guard] = true
      end
      Lobby.Actions.LobbyHostStart(0, 1, kind, 3, capacity)
      assert(seen.args[5] == capacity and next(writes) == nil)
    end
  end
end
for _, capacity in ipairs({ 1, 5, 6, 7, 8, 9, 18, "4" }) do
  local state, writes, seen = dedicatedFixture()
  state.dedicated, state.limit = true, 4
  Lobby.Actions.LobbyHostStart(0, 1, 7, 3, capacity)
  assert(seen.args[5] == capacity and next(writes) == nil)
end
-- Explicit expanded creation preserves the exact requested native capacity;
-- saved 1..4 preferences leave the unrelated action and dvars stock.
for _, flags in ipairs({ { true, false }, { false, true } }) do
  for limit = 1, 8 do
    for capacity = 5, 8 do
      local state, writes, seen = dedicatedFixture()
      state.dedicated, state.mods, state.limit = flags[1], flags[2], limit
      Lobby.Actions.LobbyHostStart(0, 1, 7, 3, capacity, "extra", nil)
      assert(seen.args[5] == capacity and seen.count == 7)
      if limit > 4 then
        assert(writes.com_maxclients == capacity and writes.party_maxplayers == capacity)
      else
        assert(next(writes) == nil)
      end
    end
  end
end
-- Launched clients cannot use the dedicated exception. Non-boolean getter
-- results are not evidence that a client is a dedicated host.
for _, flags in ipairs({ { false, false }, { 1, false }, { false, 1 } }) do
  local state, writes, seen = dedicatedFixture()
  state.dedicated, state.mods = flags[1], flags[2]
  Lobby.Actions.LobbyHostStart(0, 1, 7, 3, 4)
  assert(seen.args[5] == 4 and next(writes) == nil)
end
local state, writes, seen = dedicatedFixture()
state.dedicated, state.inGame, state.map = true, false, ""
Lobby.Actions.LobbyHostStart(0, 1, 7, 3, 4)
assert(seen.args[5] == 8 and writes.com_maxclients == 8)
-- A retained version-three extension marker cannot suppress the new guard.
local previous = Lobby.Actions.LobbyHostStart
Lobby.Actions.ZombiesCoopHostVersion = 3
dofile(root .. "/data/lobby_scripts/zombies_coop/__init__.lua")
assert(Lobby.Actions.LobbyHostStart ~= previous and Lobby.Actions.ZombiesCoopHostVersion == 4)
assert(LobbyVM.ZombiesCoopModuleVersion == 4)
local current = Lobby.Actions.LobbyHostStart
dofile(root .. "/data/lobby_scripts/zombies_coop/__init__.lua")
assert(Lobby.Actions.LobbyHostStart == current)
print("PASS: dedicated cold GAME creation, both native getters, allocation ownership and launch/recovery/client guards")
