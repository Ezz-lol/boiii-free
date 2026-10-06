if type(game) ~= "table" or type(game.receivelootkeys) ~= "function" then
  return
end

local function notifyArgs(model)
  local args = {}
  if CoD.GetScriptNotifyData then
    local ok, data = pcall(CoD.GetScriptNotifyData, model)
    if ok and type(data) == "table" then
      return data
    end
  end
  local count = 1
  pcall(function()
    count = Engine.GetModelValue(Engine.GetModel(model, "numArgs")) or 1
  end)
  for index = 1, count do
    pcall(function()
      table.insert(args, Engine.GetModelValue(Engine.GetModel(model, "arg" .. index)))
    end)
  end
  return args
end

local subscribing = false

local function debugEnabled()
  local enabled = false
  pcall(function()
    enabled = Dvar.cg_loot_debug:get() == true or Dvar.cg_loot_debug:get() == 1
  end)
  return enabled
end

local function log(message)
  if type(game.lootlog) == "function" then
    pcall(game.lootlog, message)
  end
end

local function onScriptNotify(model)
  if subscribing then
    return -- ignore the model's old value when we first subscribe
  end
  local name = Engine.GetModelValue(model)
  if debugEnabled() then
    local args = notifyArgs(model)
    local parts = {}
    for index, value in ipairs(args) do
      parts[index] = tostring(value)
    end
    log("script notify: " .. tostring(name) .. " (" .. type(name) .. ") args: " .. table.concat(parts, ", "))
  end
  local kind, amount
  if name == "boiii_loot_keys" or name == "boiii_loot_drops" then
    kind = (name == "boiii_loot_keys") and 1 or 2
    amount = tonumber(notifyArgs(model)[1])
  elseif name == "close_side_mission_countdown" then
    local args = notifyArgs(model)
    if tonumber(args[1]) ~= 7357 then
      return -- a genuine use of the event, not a reward
    end
    kind = tonumber(args[2])
    amount = tonumber(args[3])
  else
    return
  end

  if not amount or amount <= 0 then
    return
  end
  if kind == 1 then
    game.receivelootkeys(amount)
  elseif kind == 2 then
    game.receivelootdrops(amount)
  end
end

local function subscribe()
  local root = LUI.roots and LUI.roots.UIRoot0
  if not root or root.boiiiLootNotifySubscribed then
    return
  end
  local controllerModel = Engine.GetModelForController(0)
  local notify = Engine.GetModel(controllerModel, "scriptNotify") or Engine.CreateModel(controllerModel, "scriptNotify")

  subscribing = true
  root:subscribeToModel(notify, function(model)
    pcall(onScriptNotify, model)
  end, false)
  subscribing = false
  root.boiiiLootNotifySubscribed = true
  log("listening for server rewards")
end

local ok, err = pcall(subscribe)
if not ok then
  log("could not listen for server rewards: " .. tostring(err))
end
