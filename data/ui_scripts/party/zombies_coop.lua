-- Stock GAME/ZM online staging, custom, public and LAN targets share this policy.
local policy = rawget(_G, "BoiiiZombiesCoopPolicy")
if type(policy) == "table" and policy.version == 3 then
  return policy
end

policy = { version = 3 }
local originals = setmetatable({}, { __mode = "k" })
local applied = setmetatable({}, { __mode = "k" })
local previousHost, observedRemoteLimit, inheritedLimit
local targets = {
  UI_ZMLOBBYONLINE = "LOBBY_MODE_CUSTOM",
  UI_ZMLOBBYONLINECUSTOMGAME = "LOBBY_MODE_CUSTOM",
  UI_ZMLOBBYONLINEPUBLICGAME = "LOBBY_MODE_PUBLIC",
  UI_ZMLOBBYLANGAME = "LOBBY_MODE_CUSTOM",
}

local function valid(value)
  return type(value) == "number" and value == math.floor(value) and value >= 1 and value <= 8
end

function policy.configuredLimit()
  local value = Engine.DvarInt(nil, "zm_coopPlayerLimit")
  return valid(value) and value or 4
end

function policy.isZombies()
  return Engine.CurrentSessionMode() == Enum.eModes.MODE_ZOMBIES
end

local function nativeLimit(value)
  return type(value) == "number" and value == math.floor(value) and value >= 1 and value <= 18
end

local function isTarget(name, target)
  return type(target) == "table"
    and target.mainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_ZM
    and target.lobbyType == Enum.LobbyType.LOBBY_TYPE_GAME
    and Enum.LobbyMode[targets[name]] ~= nil
    and target.lobbyMode == Enum.LobbyMode[targets[name]]
end

local function gameLimit()
  local game = Enum.LobbyType.LOBBY_TYPE_GAME
  if policy.isZombies() and Engine.IsLobbyActive(game) then
    local host = Engine.IsLobbyHost(game)
    local value = Engine.GetLobbyMaxClients(game)
    local recovery = type(Engine.IsLobbyInRecovery) == "function" and Engine.IsLobbyInRecovery(game)
    if not host then
      observedRemoteLimit = nativeLimit(value) and value or nil
      inheritedLimit = nil
    elseif (previousHost == false and value == observedRemoteLimit) or recovery then
      inheritedLimit = nativeLimit(value) and value or nil
    elseif inheritedLimit ~= value then
      inheritedLimit = nil
    end
    previousHost = host
    -- Loading joiners and launch/migration paths keep the actual host cap.
    -- An existing expanded lobby stays fixed even in a fresh frontend VM.
    -- Saved preferences apply only after that lobby is closed/recreated.
    local launching = type(Engine.LobbyHostLaunchTaskInProgress) == "function"
      and Engine.LobbyHostLaunchTaskInProgress()
    local loading = type(Engine.GetCurrentMap) == "function" and Engine.GetCurrentMap() ~= "core_frontend"
    if
      Engine.IsInGame()
      or launching
      or loading
      or not host
      or inheritedLimit
      or (host and valid(value) and value > 4)
    then
      if nativeLimit(value) then
        return value
      end
    end
  else
    previousHost, observedRemoteLimit, inheritedLimit = nil, nil, nil
  end
end

local function targetLimit(target)
  local value = type(target) == "table" and target.maxClients or nil
  if type(value) == "number" and value >= 0 and value < math.huge and value == math.floor(value) then
    return value
  end
  return 0
end

function policy.applyTargets()
  local limit = gameLimit() or policy.configuredLimit()
  for name in pairs(targets) do
    local target = LobbyData.UITargets[name]
    if isTarget(name, target) then
      -- Forget our baseline if another mod or the engine changed the
      -- table since our last write. Never restore over an outside cap.
      if applied[target] ~= nil and applied[target] ~= target.maxClients then
        originals[target] = nil
      end
      local original = originals[target] or target.maxClients
      if type(original) == "number" and original >= 2 and original <= 4 then
        originals[target] = original
        target.maxClients = limit
        applied[target] = limit
      else
        applied[target] = nil
      end
    end
  end
end

local function listingInfo()
  if type(game) ~= "table" or type(game.cooplobbyinfo) ~= "function" then
    return nil
  end
  local ok, info = pcall(game.cooplobbyinfo)
  if ok and type(info) == "table" and info.enabled == true and info.canAdvertise == true then
    return info
  end
end

local function buttonWrapperInstalled(menus, expected)
  if type(expected) ~= "function" then
    return false
  end
  local current = menus.AddButtonsForTarget
  local sources = menus.BoiiiCoopButtonSources
  -- Accept exact identity through our own outer wrappers only. Unknown/stock
  -- replacements break the chain and must be wrapped afresh.
  for _ = 1, 8 do
    if current == expected then
      return true
    end
    if type(current) ~= "function" or type(sources) ~= "table" then
      return false
    end
    current = sources[current]
  end
  return false
end

local function recordButtonWrapper(menus, wrapper, original)
  if type(menus.BoiiiCoopButtonSources) ~= "table" then
    menus.BoiiiCoopButtonSources = setmetatable({}, { __mode = "k" })
  end
  menus.BoiiiCoopButtonSources[wrapper] = original
end

function policy.installSoloLaunchGuard()
  local base = type(CoD) == "table" and CoD.LobbyBase or nil
  if type(base) ~= "table" or type(base.LaunchGame) ~= "function" or base.LaunchGame == base.BoiiiCoopSoloLaunch then
    return
  end
  local original = base.LaunchGame
  local function launchGame(menu, controller, lobbyType, ...)
    local target = LobbyData.UITargets.UI_ZMLOBBYONLINE
    local game = Enum.LobbyType.LOBBY_TYPE_GAME
    if
      lobbyType == game
      and isTarget("UI_ZMLOBBYONLINE", target)
      and policy.isZombies()
      and Engine.GetLobbyUIScreen() == target.id
      and Engine.GetPlaylistID() == 150
      and Engine.IsLobbyHost(game)
      and not Engine.IsInGame()
      and Engine.GetCurrentMap() == "core_frontend"
    then
      local capacity = Engine.GetLobbyMaxClients(game)
      if valid(capacity) and capacity > 4 then
        Engine.SetLobbyMaxClients(game, 4)
        Engine.SetDvar("party_maxplayers", 4)
        Engine.SetDvar("com_maxclients", 4)
      end
    end
    return original(menu, controller, lobbyType, ...)
  end
  base.BoiiiCoopSoloLaunch = launchGame
  base.LaunchGame = launchGame
end

function policy.installListingButton()
  local menus = type(CoD) == "table" and CoD.LobbyMenus or nil
  if
    type(menus) ~= "table"
    or type(menus.AddButtonsForTarget) ~= "function"
    or buttonWrapperInstalled(menus, menus.BoiiiCoopListingWrapper)
  then
    return
  end
  local original = menus.AddButtonsForTarget
  local function pack(...)
    return { n = select("#", ...), ... }
  end
  local function addButtons(controller, id, ...)
    local result = pack(original(controller, id, ...))
    local target = LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME
    local info = listingInfo()
    if
      type(result[1]) == "table"
      and isTarget("UI_ZMLOBBYONLINECUSTOMGAME", target)
      and id == target.id
      and policy.isZombies()
      and Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_GAME)
      and info
    then
      local found = false
      for _, button in ipairs(result[1]) do
        if button.customId == "btnCoopListCustomLobby" then
          found = true
        end
      end
      if not found then
        table.insert(result[1], {
          optionDisplay = info.advertise and "UNLIST CUSTOM LOBBY" or "LIST CUSTOM LOBBY",
          customId = "btnCoopListCustomLobby",
          isLargeButton = false,
          isLastButtonInGroup = false,
          disabled = false,
          action = function()
            local current = listingInfo()
            if
              not current
              or not policy.isZombies()
              or not Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_GAME)
              or Engine.GetLobbyUIScreen() ~= target.id
            then
              return
            end
            Engine.SetDvar("zm_coopAdvertise", current.advertise and 0 or 1)
            if CoD.OverlayUtility and type(CoD.OverlayUtility.ShowToast) == "function" then
              CoD.OverlayUtility.ShowToast(
                "Custom lobby",
                current.advertise and "Removed from the browser."
                  or "Listed while your lobby is open. This does not change lobby privacy.",
                "t7_icon_connect_overlays"
              )
            end
          end,
        })
      end
    end
    return unpack(result, 1, result.n)
  end
  recordButtonWrapper(menus, addButtons, original)
  menus.BoiiiCoopListingWrapper = addButtons
  menus.AddButtonsForTarget = addButtons
end

function policy.targetLimit(target)
  -- Refresh on navigation/profile queries as well as module load: an owner
  -- may change the next host limit while remaining in the frontend VM.
  policy.applyTargets()
  if type(target) == "table" and target.mainMode ~= nil and target.mainMode ~= Enum.LobbyMainMode.LOBBY_MAINMODE_ZM then
    return targetLimit(target)
  end
  return gameLimit() or targetLimit(target)
end

function policy.gameLimit(target, stockLimit)
  if policy.isZombies() then
    return gameLimit() or targetLimit(target)
  end
  return stockLimit
end

BoiiiZombiesCoopPolicy = policy
return policy
