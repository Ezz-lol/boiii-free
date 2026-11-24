-- Only custom/LAN Zombies targets belong to the expanded coop policy.
local policy = rawget(_G, "BoiiiZombiesCoopPolicy")
if type(policy) == "table" and policy.version == 2 then
  return policy
end

policy = { version = 2 }
local originals = setmetatable({}, { __mode = "k" })
local applied = setmetatable({}, { __mode = "k" })
local targets = { UI_ZMLOBBYONLINECUSTOMGAME = true, UI_ZMLOBBYLANGAME = true }

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

local function gameLimit()
  local game = Enum.LobbyType.LOBBY_TYPE_GAME
  if policy.isZombies() and Engine.IsLobbyActive(game) then
    -- A joining client is not in-game yet. Preserve its host's capacity
    -- during loading; frontend hosts can still change their next lobby.
    if Engine.IsInGame() or not Engine.IsLobbyHost(game) then
      local value = Engine.GetLobbyMaxClients(game)
      if valid(value) then
        return value
      end
    end
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
  for name, target in pairs(LobbyData.UITargets) do
    if targets[name] and type(target) == "table" then
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

function policy.targetLimit(target)
  -- Refresh on navigation/profile queries as well as module load: an owner
  -- may change the next host limit while remaining in the frontend VM.
  policy.applyTargets()
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
