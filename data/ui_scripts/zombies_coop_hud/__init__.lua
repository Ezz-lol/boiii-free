-- SPDX-License-Identifier: GPL-3.0-only
-- Dynamic HUD guards implemented against stock BO3 Lua APIs.
-- Extra-slot colours were selected independently for this module.
-- Compatibility references: docs/zombies-coop-more-players-review.md.
-- No lobby limits, dvars or revive objectives are changed.
local previous = rawget(_G, "BoiiiZombiesCoopHud")
if type(previous) == "table" and previous.version == 1 and type(previous.install) == "function" then
  previous.install()
  return previous
end

local hud = { version = 1 }
BoiiiZombiesCoopHud = hud
local scoreRows = require("rows")
local reviveModels = require("revive")
local unpackValues = unpack
local palette = {
  [4] = { 1.00, 0.20, 0.20 },
  [5] = { 0.65, 0.30, 1.00 },
  [6] = { 0.05, 0.85, 0.70 },
  [7] = { 1.00, 0.50, 0.05 },
}

local function pack(...)
  return { n = select("#", ...), ... }
end

local function finite(value)
  return type(value) == "number" and value == value and value > -math.huge and value < math.huge
end

local function readCapacity()
  if Engine.CurrentSessionMode() ~= Enum.eModes.MODE_ZOMBIES then
    return nil
  end
  local map = Engine.GetCurrentMap()
  if type(map) ~= "string" or map == "" or map == "core_frontend"
    or Engine.IsLobbyActive(Enum.LobbyType.LOBBY_TYPE_GAME) ~= true then
    return nil
  end
  return Engine.GetLobbyMaxClients(Enum.LobbyType.LOBBY_TYPE_GAME)
end

local function capacity()
  if type(Engine) ~= "table" or type(Enum) ~= "table" or type(CoD) ~= "table"
    or CoD.isZombie ~= true or CoD.isFrontend == true
    or type(Enum.eModes) ~= "table" or type(Enum.LobbyType) ~= "table"
    or Enum.eModes.MODE_ZOMBIES == nil or Enum.LobbyType.LOBBY_TYPE_GAME == nil
    or type(Engine.CurrentSessionMode) ~= "function" or type(Engine.GetCurrentMap) ~= "function"
    or type(Engine.IsLobbyActive) ~= "function" or type(Engine.GetLobbyMaxClients) ~= "function" then
    return nil
  end
  local ok, limit = pcall(readCapacity)
  if ok and finite(limit) and limit == math.floor(limit) and limit >= 5 and limit <= 8 then
    return limit
  end
end

local function wrapColour(name, glow)
  local original = rawget(_G, name)
  local owned = hud[name]
  if type(original) ~= "function" or original == owned then
    return
  end
  local wrapper = function(...)
    hud.install()
    local slot = select(1, ...)
    if not finite(slot) or slot ~= math.floor(slot) or slot < 4 or slot > 7 then
      return original(...)
    end
    local limit = capacity()
    if not limit or slot >= limit then
      return original(...)
    end
    -- Respect an existing mod's finite RGB and all additional return values.
    local result = pack(pcall(original, ...))
    if result[1] and finite(result[2]) and finite(result[3]) and finite(result[4]) then
      return unpackValues(result, 2, result.n)
    end
    local colour = palette[slot]
    local scale = glow and 0.75 or 1
    return colour[1] * scale, colour[2] * scale, colour[3] * scale
  end
  hud[name] = wrapper
  _G[name] = wrapper
end

local function wrapScoreboard()
  local class = type(CoD) == "table" and CoD.ScoreboardWidgetCP or nil
  local original = type(class) == "table" and class.new or nil
  if type(original) ~= "function" or original == hud.scoreboardWrapper then
    return
  end
  local wrapper = function(...)
    hud.install()
    local result = pack(original(...))
    local limit = capacity()
    if limit and result[1] ~= nil then
      pcall(function()
        local scores = result[1].ScoreboardFactionScoresListCP0
        local team = scores and scores.Team1
        if team and type(team.setVerticalCount) == "function" then
          team:setVerticalCount(limit)
        end
      end)
    end
    return unpackValues(result, 1, result.n)
  end
  hud.scoreboardWrapper = wrapper
  class.new = wrapper
end

local function observeMapQueries()
  local original = type(Engine) == "table" and Engine.GetCurrentMap or nil
  if type(original) ~= "function" or original == hud.mapObserver then
    return
  end
  -- Do not wrap require: both loaders inspect its immediate caller to resolve
  -- relative modules. This read-only getter lets late HUD definitions be found
  -- without changing that caller or creating a polling timer.
  local observer = function(...)
    hud.install()
    return original(...)
  end
  hud.mapObserver = observer
  Engine.GetCurrentMap = observer
end

local function installAll()
  wrapColour("ZombieClientScoreboardColor", false)
  wrapColour("ZombieClientScoreboardGlowColor", true)
  wrapScoreboard()
  if type(scoreRows) == "table" and type(scoreRows.install) == "function" then scoreRows.install(capacity) end
  if type(reviveModels) == "table" and type(reviveModels.install) == "function" then reviveModels.install(capacity) end
  observeMapQueries()
end

function hud.install()
  if hud.installing then
    return
  end
  hud.installing = true
  local ok = pcall(installAll)
  hud.installing = false
  return ok
end

hud.install()
return hud
