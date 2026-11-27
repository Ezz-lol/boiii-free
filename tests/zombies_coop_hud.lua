-- Standalone Lua 5.1 checks of the actual candidate module; no game/UI automation.
local root = (arg and arg[1]) or "."
local modulePath = root .. "/data/ui_scripts/zombies_coop_hud/__init__.lua"
local checks = 0
local function check(value, message)
  checks = checks + 1
  assert(value, message)
end
local function pack(...)
  return { n = select("#", ...), ... }
end

local function world(late)
  local w = { cap = 8, mode = 2, map = "zm_zod", active = true, setters = 0, boardSets = 0 }
  local env = setmetatable({}, { __index = _G })
  env._G = env
  env.CoD = { isZombie = true, isFrontend = false }
  env.Enum = { eModes = { MODE_ZOMBIES = 2 }, LobbyType = { LOBBY_TYPE_GAME = 1 } }
  env.Engine = {
    CurrentSessionMode = function()
      return w.mode
    end,
    GetCurrentMap = function(...)
      w.mapArgs = pack(...)
      return w.map, nil, "map-tail"
    end,
    IsLobbyActive = function(kind)
      check(kind == 1, "GAME lobby queried")
      return w.active
    end,
    GetLobbyMaxClients = function(kind)
      check(kind == 1, "GAME capacity queried")
      return w.cap
    end,
    DvarInt = function()
      return 4
    end, -- joiner's local preference is intentionally four
    IsLobbyHost = function()
      return false
    end,
    SetDvar = function()
      w.setters = w.setters + 1
      error("unexpected dvar write")
    end,
    SetLobbyMaxClients = function()
      w.setters = w.setters + 1
      error("unexpected lobby write")
    end,
  }
  env.require = function(...)
    return ...
  end
  w.originalRequire = env.require
  function w.define()
    local function colour(...)
      w.colourArgs = pack(...)
      if w.echo then
        return "forward", nil, "colour-tail"
      end
      if w.custom then
        return unpack(w.custom, 1, w.custom.n)
      end
      local slot = select(1, ...)
      if type(slot) ~= "number" or slot < 0 or slot > 3 then
        error("stock colour unavailable")
      end
      return 0.1, 0.2, 0.3, nil, "stock-tail"
    end
    env.ZombieClientScoreboardColor = colour
    env.ZombieClientScoreboardGlowColor = colour
    env.CoD.ScoreboardWidgetCP = {
      new = function(...)
        w.boardArgs = pack(...)
        if w.boardError then
          error("original constructor error")
        end
        local team = {
          rows = 4,
          setVerticalCount = function(self, count)
            w.boardSets = w.boardSets + 1
            self.rows = count
          end,
        }
        local widget = { ScoreboardFactionScoresListCP0 = { Team1 = team } }
        return widget, nil, "board-tail"
      end,
    }
  end
  if not late then
    w.define()
  end
  function w.load()
    local chunk = assert(loadfile(modulePath))
    setfenv(chunk, env)
    return chunk()
  end
  w.env = env
  w.hud = w.load()
  return w
end

for cap = 5, 8 do
  local w = world()
  w.cap = cap
  local out = pack(w.env.CoD.ScoreboardWidgetCP.new("menu", nil, 9))
  check(out.n == 3 and out[2] == nil and out[3] == "board-tail", "constructor return arity preserved")
  check(
    w.boardArgs.n == 3 and w.boardArgs[1] == "menu" and w.boardArgs[2] == nil and w.boardArgs[3] == 9,
    "constructor args preserved"
  )
  check(out[1].ScoreboardFactionScoresListCP0.Team1.rows == cap and w.boardSets == 1, "dynamic advertised rows")
  for slot = 4, cap - 1 do
    local r, g, b = w.env.ZombieClientScoreboardColor(slot)
    local gr, gg, gb = w.env.ZombieClientScoreboardGlowColor(slot)
    check(
      type(r) == "number" and gr == r * 0.75 and gg == g * 0.75 and gb == b * 0.75,
      "extra slot palette/glow fallback"
    )
  end
  check(w.setters == 0, "joiner's local preference and lobby unchanged")
  check(w.env.require == w.originalRequire, "relative require caller unchanged")
end

do
  local w = world()
  local e = w.env
  local result = pack(e.ZombieClientScoreboardColor(2, nil, "argument"))
  check(result.n == 5 and result[4] == nil and result[5] == "stock-tail", "stock slots retain all returns")
  check(w.colourArgs.n == 3 and w.colourArgs[2] == nil and w.colourArgs[3] == "argument", "stock slots retain varargs")
  w.custom = { n = 5, 2.5, -0.2, 0.7, nil, "foreign-tail" }
  result = pack(e.ZombieClientScoreboardColor(6))
  check(
    result.n == 5 and result[1] == 2.5 and result[2] == -0.2 and result[5] == "foreign-tail",
    "finite foreign RGB preserved with all returns"
  )
  w.custom = { n = 3, 0 / 0, 0.1, 0.2 }
  result = pack(e.ZombieClientScoreboardColor(6))
  check(result[1] == 0.05 and result[2] == 0.85 and result[3] == 0.70, "NaN foreign RGB falls back")
  w.custom = { n = 3, 1, math.huge, 0.2 }
  check(e.ZombieClientScoreboardColor(6) == 0.05, "infinite foreign RGB falls back")
  w.custom = { n = 3, nil, nil, nil }
  check(e.ZombieClientScoreboardColor(6) == 0.05, "undefined foreign RGB falls back")
  w.custom = nil
  w.echo = true
  for _, slot in ipairs({ -1, 4.5, 8, 18, "5", 0 / 0, math.huge }) do
    result = pack(e.ZombieClientScoreboardColor(slot, nil, "pass"))
    check(
      result.n == 3 and result[1] == "forward" and result[3] == "colour-tail" and w.colourArgs.n == 3,
      "invalid slot forwarded unchanged"
    )
  end
  result = pack(e.ZombieClientScoreboardColor())
  check(w.colourArgs.n == 0 and result[1] == "forward", "zero args not converted to one nil")
  result = pack(e.Engine.GetCurrentMap("map-arg", nil, 7))
  check(
    result.n == 3 and result[2] == nil and result[3] == "map-tail" and w.mapArgs.n == 3 and w.mapArgs[3] == 7,
    "map observer preserves args and returns"
  )
  local colour, board, map = e.ZombieClientScoreboardColor, e.CoD.ScoreboardWidgetCP.new, e.Engine.GetCurrentMap
  w.load()
  w.load()
  w.hud.install()
  check(
    e.ZombieClientScoreboardColor == colour and e.CoD.ScoreboardWidgetCP.new == board and e.Engine.GetCurrentMap == map,
    "duplicate/reload idempotent"
  )
end

for _, cap in ipairs({ 0, 1, 2, 3, 4, 9, 18, 4.5, 0 / 0, math.huge, "8", false }) do
  local w = world()
  w.cap = cap
  local widget = w.env.CoD.ScoreboardWidgetCP.new()
  check(
    widget.ScoreboardFactionScoresListCP0.Team1.rows == 4 and w.boardSets == 0,
    "stock/foreign/invalid capacity preserved"
  )
  check(not pcall(w.env.ZombieClientScoreboardColor, 4), "out-of-scope original error propagated")
end

for _, change in ipairs({
  function(w)
    w.mode = 1
  end,
  function(w)
    w.mode = 0
  end,
  function(w)
    w.env.CoD.isZombie = false
  end,
  function(w)
    w.env.CoD.isFrontend = true
  end,
  function(w)
    w.map = "core_frontend"
  end,
  function(w)
    w.map = ""
  end,
  function(w)
    w.map = nil
  end,
  function(w)
    w.active = false
  end,
  function(w)
    w.env.Engine.CurrentSessionMode = nil
  end,
  function(w)
    w.env.Engine.IsLobbyActive = nil
  end,
  function(w)
    w.env.Engine.GetLobbyMaxClients = nil
  end,
  function(w)
    w.env.Engine.GetCurrentMap = nil
  end,
  function(w)
    w.env.Engine.GetLobbyMaxClients = function()
      error("API unavailable")
    end
  end,
  function(w)
    w.env.Engine.CurrentSessionMode = function()
      error("API unavailable")
    end
  end,
  function(w)
    w.env.Engine.GetCurrentMap = function()
      error("API unavailable")
    end
  end,
  function(w)
    w.env.Enum.eModes = nil
  end,
  function(w)
    w.env.Enum.LobbyType = nil
  end,
  function(w)
    w.env.Enum = nil
  end,
  function(w)
    w.env.Engine = nil
  end,
  function(w)
    w.cap = nil
  end,
}) do
  local w = world()
  change(w)
  check(
    w.env.CoD.ScoreboardWidgetCP.new().ScoreboardFactionScoresListCP0.Team1.rows == 4 and w.boardSets == 0,
    "mode/phase/missing API leaves scoreboard stock"
  )
  check(not pcall(w.env.ZombieClientScoreboardColor, 4), "mode/phase/missing API leaves colour behavior stock")
end

do
  local w = world()
  local e = w.env
  check(e.CoD.ScoreboardWidgetCP.new().ScoreboardFactionScoresListCP0.Team1.rows == 8, "initial expanded match")
  w.mode = 0
  e.CoD.isZombie = false
  check(
    e.CoD.ScoreboardWidgetCP.new().ScoreboardFactionScoresListCP0.Team1.rows == 4,
    "shared CP widget restores stock behavior after transition"
  )
  w.mode = 2
  e.CoD.isZombie = true
  w.cap = 5
  check(
    e.CoD.ScoreboardWidgetCP.new().ScoreboardFactionScoresListCP0.Team1.rows == 5,
    "new advertised cap observed per call"
  )
  w.boardError = true
  check(not pcall(e.CoD.ScoreboardWidgetCP.new), "original constructor error propagated")
end

do
  local w = world()
  local e = w.env
  local originalMap = e.Engine.GetCurrentMap
  e.Engine.GetCurrentMap = function()
    error("original map error")
  end
  w.hud.install()
  check(not pcall(e.Engine.GetCurrentMap), "map observer preserves original errors")
  e.Engine.GetCurrentMap = originalMap
  e.CoD.ScoreboardWidgetCP.new = function(...)
    return false, nil, "foreign-widget"
  end
  w.hud.install()
  local out = pack(e.CoD.ScoreboardWidgetCP.new("foreign"))
  check(
    out.n == 3 and out[1] == false and out[2] == nil and out[3] == "foreign-widget",
    "nonstandard widget/returns preserved"
  )
end

do
  local w = world(true)
  local e = w.env
  check(e.ZombieClientScoreboardColor == nil and e.CoD.ScoreboardWidgetCP == nil, "missing late definitions tolerated")
  w.define()
  w.cap = 7
  e.Engine.GetCurrentMap()
  check(
    e.ZombieClientScoreboardColor(6) == 0.05
      and e.CoD.ScoreboardWidgetCP.new().ScoreboardFactionScoresListCP0.Team1.rows == 7,
    "late helpers and widget installed by map query"
  )
  local custom = function(...)
    return 0.4, 0.5, 0.6, nil, "later-foreign"
  end
  e.ZombieClientScoreboardColor = custom
  e.Engine.GetCurrentMap()
  local values = pack(e.ZombieClientScoreboardColor(6))
  check(values.n == 5 and values[1] == 0.4 and values[5] == "later-foreign", "later foreign helper respected")
  w.define()
  e.Engine.GetCurrentMap()
  w.boardSets = 0
  check(
    e.CoD.ScoreboardWidgetCP.new().ScoreboardFactionScoresListCP0.Team1.rows == 7 and w.boardSets == 1,
    "late replacement widget wrapped once"
  )
  check(e.require == w.originalRequire and w.setters == 0, "no module loader/lobby/dvar mutation")
end

print(
  "PASS: bounded HUD colours/scoreboard, dynamic 5-8, stock isolation, forwarding, late load and ownership ("
    .. checks
    .. " checks)"
)
