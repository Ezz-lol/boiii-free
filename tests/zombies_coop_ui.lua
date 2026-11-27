local root = assert(arg[1], "pass the checkout directory")
package.preload.zombies_coop_settings = function()
  return dofile(root .. "/data/ui_scripts/party/zombies_coop_settings.lua")
end
local function fixture(limit, mode, inGame, serverLimit, isHost)
  BoiiiZombiesCoopPolicy = nil
  Enum = {
    eModes = { MODE_ZOMBIES = 1 },
    LobbyType = { LOBBY_TYPE_GAME = 7, LOBBY_TYPE_PRIVATE = 8 },
    LobbyMainMode = { LOBBY_MAINMODE_ZM = 1 },
    LobbyMode = { LOBBY_MODE_CUSTOM = 3, LOBBY_MODE_PUBLIC = 4, LOBBY_MODE_THEATER = 5 },
  }
  Engine = {
    DvarInt = function()
      return limit
    end,
    CurrentSessionMode = function()
      return mode
    end,
    IsInGame = function()
      return inGame
    end,
    IsLobbyActive = function()
      return serverLimit ~= nil
    end,
    IsLobbyHost = function()
      return isHost == true
    end,
    GetLobbyMaxClients = function()
      return serverLimit
    end,
  }
  LobbyData = {
    UITargets = {
      UI_ZMLOBBYONLINECUSTOMGAME = { maxClients = 4, mainMode = 1, lobbyType = 7, lobbyMode = 3 },
      UI_ZMLOBBYLANGAME = { maxClients = 4, mainMode = 1, lobbyType = 7, lobbyMode = 3 },
      UI_ZMLOBBYONLINEPUBLICGAME = { maxClients = 4, mainMode = 1, lobbyType = 7, lobbyMode = 4 },
      UI_ZMTHEATER = { maxClients = 4 },
      UI_ZM_UNKNOWN_MOD = { maxClients = 2 },
      UI_ZMSOLO = { maxClients = 1 },
      UI_ZMMAIN = { maxClients = 0 },
      UI_MPLOBBY = { maxClients = 18 },
      UI_CPLOBBY = { maxClients = 4 },
    },
  }
  return dofile(root .. "/data/ui_scripts/party/zombies_coop.lua")
end

for limit = 1, 8 do
  local policy = fixture(limit, 1, false, nil, true)
  policy.applyTargets()
  assert(LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME.maxClients == limit)
  assert(LobbyData.UITargets.UI_ZMLOBBYLANGAME.maxClients == limit)
  assert(LobbyData.UITargets.UI_ZMLOBBYONLINEPUBLICGAME.maxClients == limit)
  assert(LobbyData.UITargets.UI_ZMTHEATER.maxClients == 4)
  assert(LobbyData.UITargets.UI_ZM_UNKNOWN_MOD.maxClients == 2)
  assert(LobbyData.UITargets.UI_ZMSOLO.maxClients == 1)
  assert(LobbyData.UITargets.UI_ZMMAIN.maxClients == 0)
  assert(LobbyData.UITargets.UI_MPLOBBY.maxClients == 18)
  assert(LobbyData.UITargets.UI_CPLOBBY.maxClients == 4)
  assert(policy == dofile(root .. "/data/ui_scripts/party/zombies_coop.lua"))
end

for _, bad in ipairs({ 0, 9, -1, 1.5, "8", math.huge, -math.huge, 0 / 0, false }) do
  local policy = fixture(bad, 1, false, nil)
  policy.applyTargets()
  assert(LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME.maxClients == 4)
end
assert(fixture(nil, 1, false, nil).configuredLimit() == 4)

-- A frontend owner can change the next lobby cap without reloading any file.
local frontendPolicy = fixture(4, 1, false, 4, true)
frontendPolicy.applyTargets()
local frontendTarget = LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME
assert(frontendTarget.maxClients == 4)
Engine.DvarInt = function()
  return 7
end
assert(frontendPolicy.targetLimit(frontendTarget) == 7)
assert(frontendTarget.maxClients == 7)
Engine.DvarInt = function()
  return 2
end
assert(frontendPolicy.targetLimit(frontendTarget) == 2)
assert(LobbyData.UITargets.UI_ZMLOBBYLANGAME.maxClients == 2)
assert(LobbyData.UITargets.UI_ZMLOBBYONLINEPUBLICGAME.maxClients == 2)

-- Joining a dedicated server is not in-game yet: do not reset its cap to four.
for _, inGame in ipairs({ false, true }) do
  local policy = fixture(4, 1, inGame, 8, false)
  policy.applyTargets()
  assert(policy.targetLimit(LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME) == 8)
  assert(policy.gameLimit({}, 4) == 8)
  Engine.IsLobbyActive = function()
    return false
  end
  Engine.IsInGame = function()
    return false
  end
  policy.applyTargets()
  assert(LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME.maxClients == 4)
end
local policy = fixture(8, 1, false, 4, true)
policy.applyTargets()
assert(LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME.maxClients == 8) -- next host config wins
policy = fixture(4, 1, true, 8, true)
assert(policy.gameLimit({}, 4) == 8) -- an active host's cap stays stable
policy = fixture(8, 1, true, 1, true)
assert(policy.gameLimit({}, 8) == 1) -- live solo remains solo
policy = fixture(8, 0, true, 18)
assert(policy.gameLimit({}, 18) == 18)
policy = fixture(4, 1, true, 18)
assert(policy.gameLimit({ maxClients = 4 }, 4) == 18)

policy = fixture(8, 1, false, nil)
policy.applyTargets()
local target = LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME
for _, foreign in ipairs({ 18, 1 }) do
  target.maxClients = foreign
  policy.applyTargets()
  assert(target.maxClients == foreign)
end
assert(policy.targetLimit(nil) == 0)
assert(policy.targetLimit({ maxClients = "4" }) == 0)
assert(policy.targetLimit({ maxClients = 0 / 0 }) == 0)
assert(policy.targetLimit({ maxClients = math.huge }) == 0)
BoiiiZombiesCoopPolicy = "foreign value"
assert(type(dofile(root .. "/data/ui_scripts/party/zombies_coop.lua")) == "table")

-- Exercise the actual party entrypoint during the loading/frontend phase.
policy = fixture(4, 1, false, 8, false)
local writes = {}
Engine.SetDvar = function(name, value)
  writes[name] = value
end
Engine.GetLobbyUIScreen = function()
  return 402
end
Engine.GetCurrentMap = function()
  return "core_frontend"
end
LobbyData.UITargetFromId = function()
  return LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME
end
DataSources = {}
package.loaded.zombies_coop = nil
package.preload.zombies_coop = function()
  return policy
end
dofile(root .. "/data/ui_scripts/party/__init__.lua")
assert(writes.party_maxplayers == 8)
LobbyData.UITargetFromId = function()
  return nil
end
writes = {}
dofile(root .. "/data/ui_scripts/party/__init__.lua")
assert(writes.party_maxplayers == 8) -- the active server remains authoritative
Engine.IsLobbyActive = function()
  return false
end
writes = {}
dofile(root .. "/data/ui_scripts/party/__init__.lua")
assert(writes.party_maxplayers == nil and writes.com_maxclients == nil)

local createdMax, selectedTarget, selectedNetwork
local function selectorFixture(mode, limit, originalCap, networkMode, fallbackMode)
  Enum.LobbyNetworkMode = { LOBBY_NETWORKMODE_LIVE = 1, LOBBY_NETWORKMODE_LAN = 2 }
  LobbyData.UITargets.UI_CPLOBBYONLINECUSTOMGAME = { maxClients = 4, mainMode = 0 }
  LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME =
    { maxClients = originalCap or 4, mainMode = 1, id = 402, networkMode = 1 }
  LobbyData.UITargets.UI_ZMLOBBYLANGAME = { maxClients = originalCap or 4, mainMode = 1, id = 410, networkMode = 2 }
  local originalTarget = (networkMode or fallbackMode) == 2 and LobbyData.UITargets.UI_ZMLOBBYLANGAME
    or LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME
  Dvar = { sv_playlist = {
    get = function()
      return 1
    end,
  } }
  Engine.DvarString = function()
    return mode
  end
  Engine.DvarInt = function()
    return limit
  end
  Engine.SetPlaylistID = function() end
  Engine.SetDvar = function() end
  Engine.GetLobbyNetworkMode = function()
    return fallbackMode or 1
  end
  local function action()
    return {}
  end
  Lobby = {
    Actions = {
      ExecuteScript = action,
      SetNetworkMode = function(controller, value)
        selectedNetwork = value
        return {}
      end,
      LobbySettings = function(controller, target)
        selectedTarget = target
        return {}
      end,
      UpdateUI = action,
      AdvertiseLobby = action,
      LobbyHostStart = function(controller, mainMode, lobbyType, lobbyMode, maxClients)
        createdMax = maxClients
        return {}
      end,
    },
    Process = { AddActions = function() end },
    Interrupt = { NONE = 0 },
  }
  dofile(root .. "/data/lobby_scripts/server_lobby_selector/__init__.lua")
  Lobby.Process.CreateDedicatedModsLobby(0, { maxClients = 18, mainMode = 0, networkMode = networkMode })
  assert(originalTarget.maxClients == (originalCap or 4))
  if mode == "zm" then
    assert(selectedTarget ~= originalTarget and selectedTarget.id == originalTarget.id)
  end
end
for limit = 1, 8 do
  selectorFixture("zm", limit)
  assert(createdMax == limit)
end
selectorFixture("zm", 9)
assert(createdMax == 4)
selectorFixture("cp", 8)
assert(createdMax == 4)
selectorFixture("zm", 8, 1)
assert(createdMax == 1)
selectorFixture("zm", 8, 18)
assert(createdMax == 18)
for _, networkMode in ipairs({ 1, 2 }) do
  selectorFixture("zm", 8, 4, networkMode)
  assert(createdMax == 8 and selectedNetwork == networkMode)
  selectorFixture("zm", 8, 4, nil, networkMode)
  assert(createdMax == 8 and selectedNetwork == networkMode)
end
selectorFixture("cp", 8, 4, 2)
assert(selectedNetwork == 1) -- retain the existing non-Zombies dedicated policy
selectorFixture("zm", 8, 4, 99)
assert(selectedNetwork == 1 and selectedTarget.id == 402)
print("PASS: Ezz custom/LAN limits, loading clients, solo/public/MP isolation, target ownership and dedicated selector")

-- Source-backed staging/public/launch and listing paths.
dofile(root .. "/tests/zombies_coop_lobby_paths.lua")
dofile(root .. "/tests/zombies_coop_ui_ownership.lua")
