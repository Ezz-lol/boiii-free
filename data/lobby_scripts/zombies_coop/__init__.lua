-- Installed once per Lobby VM. These are host actions, not a matchmaking queue.
local function bounded(value, max)
  return type(value) == "number" and value == math.floor(value) and value >= 1 and value <= max
end

local function configuredLimit()
  local limit = Engine.DvarInt(nil, "zm_coopPlayerLimit")
  return bounded(limit, 8) and limit or 4
end

local function launched()
  return Engine.IsInGame()
    or (type(Engine.LobbyHostLaunchTaskInProgress) == "function" and Engine.LobbyHostLaunchTaskInProgress())
    or Engine.GetCurrentMap() ~= "core_frontend"
end

local actions = type(Lobby) == "table" and Lobby.Actions or nil
if
  type(actions) == "table"
  and type(actions.LobbyHostStart) == "function"
  and (actions.LobbyHostStart ~= actions.ZombiesCoopHostStart or actions.ZombiesCoopHostVersion ~= 3)
then
  local original = actions.LobbyHostStart
  local function hostStart(...)
    local controller, mainMode, lobbyType, lobbyMode, maxClients = ...
    local custom = Enum.LobbyMode and Enum.LobbyMode.LOBBY_MODE_CUSTOM
    local public = Enum.LobbyMode and Enum.LobbyMode.LOBBY_MODE_PUBLIC
    local game = Enum.LobbyType.LOBBY_TYPE_GAME
    local private = Enum.LobbyType.LOBBY_TYPE_PRIVATE
    if
      mainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_ZM
      and (lobbyType == game or lobbyType == private)
      and ((custom ~= nil and lobbyMode == custom) or (public ~= nil and lobbyMode == public))
      and bounded(maxClients, 4)
      and maxClients >= 2
      and not launched()
    then
      local limit = configuredLimit()
      Engine.SetDvar("party_maxplayers", limit)
      if lobbyType == game then
        Engine.SetDvar("com_maxclients", limit)
      end
      return original(controller, mainMode, lobbyType, lobbyMode, limit, select(6, ...))
    end
    -- Explicit Solo, migrated host limits, other modes and foreign >8 caps retain ownership.
    return original(...)
  end
  actions.ZombiesCoopHostStart = hostStart
  actions.ZombiesCoopHostVersion = 3
  actions.LobbyHostStart = hostStart
end

if type(LobbyVM) ~= "table" then
  return
end

-- Stock online screen 400 is also the Solo entrypoint. Playlist 150 selects
-- Solo and launches its map directly, without creating a new one-slot lobby.
if type(LobbyVM.LaunchGame) == "function" and LobbyVM.LaunchGame ~= LobbyVM.ZombiesCoopLaunchGame then
  local original = LobbyVM.LaunchGame
  local function launchGame(params, ...)
    local target = LobbyData.UITargets.UI_ZMLOBBYONLINE
    local game = Enum.LobbyType.LOBBY_TYPE_GAME
    if
      type(params) == "table"
      and params.lobbyType == game
      and type(target) == "table"
      and target.mainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_ZM
      and target.lobbyType == game
      and Engine.CurrentSessionMode() == Enum.eModes.MODE_ZOMBIES
      and Engine.GetLobbyUIScreen() == target.id
      and Engine.GetPlaylistID() == 150
      and Engine.IsLobbyHost(game)
      and not launched()
    then
      local capacity = Engine.GetLobbyMaxClients(game)
      if bounded(capacity, 8) and capacity > 4 then
        Engine.SetLobbyMaxClients(game, 4)
        Engine.SetDvar("party_maxplayers", 4)
        Engine.SetDvar("com_maxclients", 4)
      end
    end
    return original(params, ...)
  end
  LobbyVM.ZombiesCoopLaunchGame = launchGame
  LobbyVM.LaunchGame = launchGame
end

-- Active expanded controller state is not transferable. Preserve the stock
-- custom-game refusal and stop public-game migration above four as well.
if type(LobbyVM.CanLobbyCanMigrate) == "function" and LobbyVM.CanLobbyCanMigrate ~= LobbyVM.ZombiesCoopCanMigrate then
  local original = LobbyVM.CanLobbyCanMigrate
  local function canMigrate(params, ...)
    local game = Enum.LobbyType.LOBBY_TYPE_GAME
    if
      type(params) == "table"
      and params.lobbyType == game
      and Engine.CurrentSessionMode() == Enum.eModes.MODE_ZOMBIES
      and Engine.IsLobbyActive(game)
      and launched()
    then
      local capacity = Engine.GetLobbyMaxClients(game)
      if bounded(capacity, 8) and capacity > 4 then
        return false
      end
    end
    return original(params, ...)
  end
  LobbyVM.ZombiesCoopCanMigrate = canMigrate
  LobbyVM.CanLobbyCanMigrate = canMigrate
end

LobbyVM.ZombiesCoopModuleVersion = type(actions) == "table"
    and actions.ZombiesCoopHostVersion == 3
    and type(actions.LobbyHostStart) == "function"
    and actions.LobbyHostStart == actions.ZombiesCoopHostStart
    and type(LobbyVM.LaunchGame) == "function"
    and LobbyVM.LaunchGame == LobbyVM.ZombiesCoopLaunchGame
    and type(LobbyVM.CanLobbyCanMigrate) == "function"
    and LobbyVM.CanLobbyCanMigrate == LobbyVM.ZombiesCoopCanMigrate
    and 3
  or nil
