-- SPDX-License-Identifier: GPL-3.0-only
-- Prepare the existing CSC countdown's model path, not native world-space widgets.
-- The stock HUD owns setupBleedOutWidget and its until-false continuation loop.
local previous = rawget(_G, "BoiiiZombiesCoopReviveModels")
if type(previous) == "table" and previous.version == 1 and type(previous.install) == "function" then
  return previous
end

local revive = { version = 1, nativeBindingVerified = false }
BoiiiZombiesCoopReviveModels = revive
local unpackValues = unpack

local function pack(...)
  return { n = select("#", ...), ... }
end

local function prepare(root)
  if root == nil or type(revive.capacity) ~= "function" then
    return
  end
  local limit = revive.capacity()
  if type(limit) ~= "number" or limit ~= limit or limit ~= math.floor(limit) or limit < 5 or limit > 8 then
    return
  end
  if
    type(Engine) ~= "table"
    or type(Engine.GetModel) ~= "function"
    or type(Engine.CreateModel) ~= "function"
    or type(Engine.GetModelValue) ~= "function"
    or type(Engine.SetModelValue) ~= "function"
  then
    return
  end
  -- Registration uses one fixed schema for capacities five through eight.
  -- Never fill, reset or replace an existing slot model: it belongs to its
  -- original creator, including the stock HUD or another mod.
  for slot = 4, 7 do
    local name = "bleedOutModel" .. slot
    if Engine.GetModel(root, name) == nil then
      local model = Engine.CreateModel(root, name)
      if model ~= nil and Engine.GetModel(model, "bleedOutPercent") == nil then
        local timer = Engine.CreateModel(model, "bleedOutPercent")
        if timer ~= nil and Engine.GetModelValue(timer) == nil then
          Engine.SetModelValue(timer, 0)
        end
      end
    end
  end
end

function revive.install(capacityFunction)
  if type(capacityFunction) ~= "function" then
    return false
  end
  revive.capacity = capacityFunction
  local source = type(DataSources) == "table" and DataSources.WorldSpaceIndicators or nil
  local original = type(source) == "table" and source.getModel or nil
  if type(original) ~= "function" then
    return false
  end
  if original == revive.wrapper then
    return true
  end
  local wrapper = function(...)
    -- Preserve the original arguments, return arity and errors exactly.
    local result = pack(original(...))
    if not revive.preparing then
      revive.preparing = true
      -- Optional model preparation must not break a stock/custom datasource.
      pcall(prepare, result[1])
      revive.preparing = false
    end
    return unpackValues(result, 1, result.n)
  end
  revive.wrapper = wrapper
  source.getModel = wrapper
  return true
end

return revive
