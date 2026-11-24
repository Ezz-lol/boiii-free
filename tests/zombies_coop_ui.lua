local root = assert(arg[1], "pass the checkout directory")
local function fixture(limit, mode, inGame, serverLimit)
  BoiiiZombiesCoopPolicy = nil
  Enum =
    { eModes = { MODE_ZOMBIES = 1 }, LobbyType = { LOBBY_TYPE_GAME = 1 }, LobbyMainMode = { LOBBY_MAINMODE_ZM = 1 } }
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
    GetLobbyMaxClients = function()
      return serverLimit
    end,
  }
  LobbyData = {
    UITargets = {
      UI_ZMLOBBYONLINECUSTOMGAME = { maxClients = 4 },
      UI_ZMSOLO = { maxClients = 1 },
      UI_ZMMAIN = { maxClients = 0 },
      UI_MPLOBBY = { maxClients = 18 },
      UI_CPLOBBY = { maxClients = 4 },
    },
  }
  return dofile(root .. "/data/ui_scripts/party/zombies_coop.lua")
end

for limit = 1, 8 do
  local policy = fixture(limit, 1, false, nil)
  policy.applyTargets()
  assert(LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME.maxClients == limit)
  assert(LobbyData.UITargets.UI_ZMSOLO.maxClients == 1)
  assert(LobbyData.UITargets.UI_ZMMAIN.maxClients == 0)
  assert(LobbyData.UITargets.UI_MPLOBBY.maxClients == 18)
  assert(LobbyData.UITargets.UI_CPLOBBY.maxClients == 4)
  assert(policy == dofile(root .. "/data/ui_scripts/party/zombies_coop.lua"))
end

for _, bad in ipairs({ 0, 9, -1, 1.5, "8", math.huge }) do
  local policy = fixture(bad, 1, false, nil)
  policy.applyTargets()
  assert(LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME.maxClients == 4)
end

local policy = fixture(4, 1, true, 8)
policy.applyTargets()
assert(policy.targetLimit(LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME) == 8)
assert(policy.gameLimit({}, 4) == 8) -- local default must not shrink the joined server
Engine.IsInGame = function()
  return false
end
policy.applyTargets()
assert(LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME.maxClients == 4)
Engine.DvarInt = function()
  return 7
end
policy.applyTargets()
assert(LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME.maxClients == 7)
policy = fixture(8, 0, true, 8)
assert(policy.gameLimit({}, 18) == 18) -- MP admission stays native
policy = fixture(4, 1, true, 18)
assert(policy.gameLimit({ maxClients = 4 }, 4) == 4) -- reject unsupported expanded value

local createdMax
local function selectorFixture(mode, limit)
  Enum.LobbyNetworkMode = { LOBBY_NETWORKMODE_LIVE = 1 }
  LobbyData.UITargets.UI_CPLOBBYONLINECUSTOMGAME = { maxClients = 4, mainMode = 0 }
  LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME.mainMode = 1
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
  local function action()
    return {}
  end
  Lobby = {
    Actions = {
      ExecuteScript = action,
      SetNetworkMode = action,
      LobbySettings = action,
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
  Lobby.Process.CreateDedicatedModsLobby(0, { maxClients = 18, mainMode = 0 })
end
for limit = 1, 8 do
  selectorFixture("zm", limit)
  assert(createdMax == limit)
end
selectorFixture("zm", 9)
assert(createdMax == 4)
selectorFixture("cp", 8)
assert(createdMax == 4)
print("PASS: Ezz Lua 5.1 limits 1-8, solo/MP isolation, joined-server capacity and dedicated selector")
