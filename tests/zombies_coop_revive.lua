-- Lua 5.1 stand-ins exercise the candidate and the pinned stock datasource.
-- This does not execute CSC, setupBleedOutWidget or the native client engine.
local root = assert(arg[1], "pass the checkout directory")
local reference = (arg and arg[2]) or root .. "/../research/stock-reference/hud-20261126/ui/uieditor/datasources.lua"
local input = assert(io.open(reference, "rb"))
local source = input:read("*a")
input:close()
local datasource = assert(
  source:match("(DataSources%.WorldSpaceIndicators%s*=%s*{.-})%s*DataSources%.MessageDialog"),
  "pinned datasource missing"
)
local checks = 0
local function check(value, message)
  checks = checks + 1
  assert(value, message)
end
local function pack(...)
  return { n = select("#", ...), ... }
end
local function world(late)
  local w = { limit = 8, roots = {}, creates = 0, sets = 0 }
  local env = setmetatable({}, { __index = _G })
  env._G = env
  env.BoiiiZombiesCoopReviveModels = nil
  env.DataSources = {}
  env.Engine = {
    GetModelForController = function(controller)
      w.roots[controller] = w.roots[controller] or { children = {} }
      return w.roots[controller]
    end,
    GetModel = function(parent, name)
      return parent.children[name]
    end,
    CreateModel = function(parent, name)
      if parent.children[name] then
        return parent.children[name]
      end
      if w.createError then
        error("optional preparation failed")
      end
      w.creates = w.creates + 1
      local model = { children = {}, name = name }
      parent.children[name] = model
      if w.concurrentValue and name == "bleedOutPercent" then
        model.value = w.concurrentValue
      end
      return model
    end,
    GetModelValue = function(model)
      return model.value
    end,
    SetModelValue = function(model, value)
      model.value = value
      w.sets = w.sets + 1
    end,
  }
  function w.define()
    local chunk = assert(loadstring(datasource))
    setfenv(chunk, env)
    chunk()
  end
  function w.load()
    local chunk = assert(loadfile(root .. "/data/ui_scripts/zombies_coop_hud/revive.lua"))
    setfenv(chunk, env)
    return chunk()
  end
  if not late then
    w.define()
  end
  w.env = env
  w.module = w.load()
  w.capacity = function()
    return w.limit
  end
  w.module.install(w.capacity)
  return w
end

for limit = 5, 8 do
  local w = world()
  w.limit = limit
  local model = w.env.DataSources.WorldSpaceIndicators.getModel(0)
  for slot = 4, 7 do
    check(model.children["bleedOutModel" .. slot].children.bleedOutPercent.value == 0, "fixed early timer schema")
  end
  check(model.children.bleedOutModel0 == nil and model.children.bleedOutModel8 == nil, "stock/foreign slots untouched")
  check(w.sets == 4 and w.module.nativeBindingVerified == false, "only four timer defaults, no native readiness claim")
  local timer = model.children.bleedOutModel4.children.bleedOutPercent
  timer.value = 0.73
  local creates = w.creates
  w.env.DataSources.WorldSpaceIndicators.getModel(0)
  check(timer.value == 0.73 and w.sets == 4 and w.creates == creates, "CSC countdown survives repeated reads")
  local wrapper = w.env.DataSources.WorldSpaceIndicators.getModel
  w.module.install(w.capacity)
  check(
    w.load() == w.module and wrapper == w.env.DataSources.WorldSpaceIndicators.getModel,
    "reload and install idempotent"
  )
  local second = w.env.DataSources.WorldSpaceIndicators.getModel(1)
  check(second ~= model and second.children.bleedOutModel7.children.bleedOutPercent.value == 0, "controllers isolated")
end

for _, limit in ipairs({ 0, 1, 4, 9, 18, 4.5, "8", false, math.huge, -math.huge, 0 / 0 }) do
  local w = world()
  w.limit = limit
  local model = w.env.DataSources.WorldSpaceIndicators.getModel(0)
  check(next(model.children) == nil and w.sets == 0, "default/invalid capacity leaves stock datasource unchanged")
end
do
  local w = world()
  w.limit = nil
  check(next(w.env.DataSources.WorldSpaceIndicators.getModel(0).children) == nil, "unknown capacity left stock")
  check(w.module.install(nil) == false, "invalid installer input rejected")
end
do
  local w = world()
  local e = w.env.Engine
  local parent = e.CreateModel(e.GetModelForController(0), "WorldSpaceIndicators")
  local foreign = e.CreateModel(parent, "bleedOutModel4")
  foreign.marker = "foreign"
  local custom = e.CreateModel(parent, "bleedOutModel5")
  local timer = e.CreateModel(custom, "bleedOutPercent")
  timer.value = 0.2
  w.env.DataSources.WorldSpaceIndicators.getModel(0)
  check(
    parent.children.bleedOutModel4 == foreign and next(foreign.children) == nil and foreign.marker == "foreign",
    "existing partial foreign slot preserved"
  )
  check(timer.value == 0.2 and w.sets == 2, "existing populated foreign timer preserved")
  w.limit = 4
  w.env.DataSources.WorldSpaceIndicators.getModel(0)
  check(timer.value == 0.2 and parent.children.bleedOutModel7 ~= nil, "no freeing referenced model on session change")
end
do
  local w = world()
  w.concurrentValue = 0.9
  local model = w.env.DataSources.WorldSpaceIndicators.getModel(0)
  check(
    model.children.bleedOutModel4.children.bleedOutPercent.value == 0.9 and w.sets == 0,
    "value initialized during creation preserved"
  )
end
do
  local w = world(true)
  check(w.module.install(w.capacity) == false, "late datasource tolerated")
  w.define()
  check(w.module.install(w.capacity) == true, "late datasource discovered")
  local called
  w.env.DataSources.WorldSpaceIndicators.getModel = function(...)
    called = pack(...)
    return nil, "foreign", nil
  end
  w.module.install(w.capacity)
  local result = pack(w.env.DataSources.WorldSpaceIndicators.getModel(9, nil, "tail"))
  check(called.n == 3 and called[1] == 9 and called[2] == nil and called[3] == "tail", "custom args preserved")
  check(
    result.n == 3 and result[1] == nil and result[2] == "foreign" and result[3] == nil,
    "custom returns and trailing nil preserved"
  )
  w.env.DataSources.WorldSpaceIndicators.getModel = function()
    error("original error")
  end
  w.module.install(w.capacity)
  local ok, message = pcall(w.env.DataSources.WorldSpaceIndicators.getModel)
  check(not ok and message:find("original error", 1, true), "original error propagated")
end
do
  local w = world()
  local model = w.env.Engine.CreateModel(w.env.Engine.GetModelForController(0), "WorldSpaceIndicators")
  w.createError = true
  check(
    w.env.DataSources.WorldSpaceIndicators.getModel(0) == model,
    "optional preparation error leaves original result"
  )
  check(w.module.preparing == false, "failed preparation releases reentrancy guard")
end
do
  local w = world()
  local calls = 0
  w.module.install(function()
    calls = calls + 1
    w.env.DataSources.WorldSpaceIndicators.getModel(0)
    return 8
  end)
  local model = w.env.DataSources.WorldSpaceIndicators.getModel(0)
  check(calls == 1 and model.children.bleedOutModel7 ~= nil, "capacity callback recursive model lookup bounded")
end
do
  local w = world()
  w.module.install(function()
    error("capacity unavailable")
  end)
  local model = w.env.DataSources.WorldSpaceIndicators.getModel(0)
  check(
    next(model.children) == nil and w.module.preparing == false,
    "unavailable capacity leaves stock result and releases guard"
  )
  w.module.install(w.capacity)
  w.env.Engine.GetModelValue = nil
  check(
    w.env.DataSources.WorldSpaceIndicators.getModel(0) == model and next(model.children) == nil,
    "missing optional model API leaves stock result"
  )
end

local moduleFile = assert(io.open(root .. "/data/ui_scripts/zombies_coop_hud/revive.lua", "rb"))
local moduleSource = moduleFile:read("*a")
moduleFile:close()
check(
  not moduleSource:find(":setupBleedOutWidget%s*%(") and not moduleSource:find("ZM_Revive%.new%s*%("),
  "no unverified native widget calls"
)
check(
  not moduleSource:find("Engine%.FreeModel%s*%(") and not moduleSource:find("Engine%.SetDvar%s*%("),
  "no referenced model deletion or capacity writes"
)

print(
  "PASS: pinned WorldSpaceIndicators datasource, owned early timer models, foreign ownership/forwarding/reload/error guards ("
    .. checks
    .. " checks); native widgets untested"
)
