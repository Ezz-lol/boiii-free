-- This module only configures lobby capacity; map/gameplay support is separate.
local policy = rawget(_G, "BoiiiZombiesCoopPolicy")
if policy then
  return policy
end

policy = {}
local originals = setmetatable({}, { __mode = "k" })

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
  if policy.isZombies() and Engine.IsInGame() and Engine.IsLobbyActive(Enum.LobbyType.LOBBY_TYPE_GAME) then
    local value = Engine.GetLobbyMaxClients(Enum.LobbyType.LOBBY_TYPE_GAME)
    if valid(value) then
      return value
    end
  end
end

function policy.applyTargets()
  local limit = gameLimit() or policy.configuredLimit()
  for name, target in pairs(LobbyData.UITargets) do
    if type(name) == "string" and name:match("^UI_ZM") and type(target) == "table" then
      local original = originals[target] or target.maxClients
      if type(original) == "number" and original >= 2 and original <= 4 then
        originals[target] = original
        target.maxClients = limit
      end
    end
  end
end

function policy.targetLimit(target)
  if policy.isZombies() then
    local live = gameLimit()
    if live then
      return live
    end
  end
  return target.maxClients
end

function policy.gameLimit(target, stockLimit)
  if policy.isZombies() then
    return gameLimit() or policy.targetLimit(target)
  end
  return stockLimit
end

BoiiiZombiesCoopPolicy = policy
return policy
