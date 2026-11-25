-- Executes the candidate extension in Lua 5.1. Optional arg[2] is an external
-- pinned retail datasource reference; retail source is not copied into this repo.
local root = assert(arg[1])
local checks = 0
local function check(value, message) checks = checks + 1; assert(value, message) end
local function pack(...) return { n = select("#", ...), ... } end
local function fixture(capacity, localClient)
  local w = { cap = capacity, localClient = localClient or 0, players = {}, writes = 0 }
  local env = setmetatable({}, { __index = _G }); env._G = env
  local function model() return { children = {}, subscriptions = {} } end
  w.root = model()
  local function get(parent, path, create)
    if not parent then return nil end
    for key in tostring(path):gmatch("[^.]+") do
      if not parent.children[key] and create then parent.children[key] = model() end
      parent = parent.children[key]
      if not parent then return nil end
    end
    return parent
  end
  local function element()
    local e = { children = {}, subscriptions = {}, events = {}, alpha = 1, closes = 0 }
    function e:addElement(child) self.children[#self.children + 1] = child; child.parent = self end
    function e:setLeftRight(...) self.leftRight = pack(...) end
    function e:setTopBottom(...) self.topBottom = pack(...) end
    function e:setAlpha(value) self.alpha = value end
    function e:setScale(value) self.scale = value end
    function e:setModel(value, controller) self.model = value; self.controller = controller end
    function e:setState(value) self.state = value end
    function e:playClip(value) self.clip = value end
    function e:registerEventHandler(name, fn) self.events[name] = fn end
    function e:subscribeToModel(m, fn, immediate)
      local sub = { owner = self, fn = fn, model = m }
      m.subscriptions[#m.subscriptions + 1] = sub
      self.subscriptions[#self.subscriptions + 1] = sub
      if immediate ~= false then fn(m) end
      return sub
    end
    function e:removeSubscription(sub) sub.dead = true end
    function e:close()
      self.closes = self.closes + 1
      self.childCountAtClose = #self.children
      for _, sub in ipairs(self.subscriptions) do sub.dead = true end
      if self.parent then
        for i, child in ipairs(self.parent.children) do
          if child == self then table.remove(self.parent.children, i); break end
        end
        self.parent = nil
      end
    end
    return e
  end
  get(w.root, "playerConnected", true)
  get(w.root, "playerDisconnected", true)
  env.Engine = {
    GetModelForController = function(c) check(c == 0, "correct local controller"); return w.root end,
    CreateModel = function(parent, path) return get(parent, path, true) end,
    GetModel = function(parent, path) return get(parent, path, false) end,
    GetModelValue = function(m) return m and m.value end,
    SetModelValue = function(m, value)
      m.value = value
      local snapshot = {}; for _, sub in ipairs(m.subscriptions) do snapshot[#snapshot + 1] = sub end
      for _, sub in ipairs(snapshot) do if not sub.dead then sub.fn(m) end end
    end,
    GetClientNum = function() return w.localClient end,
    GetPlayerListData = function(c, slot)
      check(slot >= 0 and slot < math.max(capacity, 8), "bounded actual native slot lookup")
      return w.players[slot] or { clientNum = slot, playerConnected = 0 }
    end,
    UnsubscribeAndFreeModel = function() end,
    SetDvar = function() w.writes = w.writes + 1 end,
    SetLobbyMaxClients = function() w.writes = w.writes + 1 end,
  }
  env.LUI = { UIElement = { new = element } }
  env.CoD = { Zombie = {}, ZMScr_ListingSm = { new = function() return element() end } }
  env.CoD.ScoreboardUtility = { MinRowsToShowOnEachTeam = 4 }
  env.Engine.GetCurrentTeamCount = function() return 1 end
  env.Engine.IsZombiesGame = function() return true end
  env.CoD.ZMScr_PlusPointsContainer = { new = function()
    local popup = element(); local points = element()
    local function label() return { setText = function(self, text) self.text = text end } end
    points.Label1 = label(); points.Label2 = label(); popup.ZMScrPlusPoints = points
    return popup
  end }
  env.CoD.ZMScr = { new = function(...)
    w.args = pack(...)
    if w.constructorError then error("original failure") end
    if w.foreign then return false, nil, "foreign" end
    local widget = element()
    widget.ListingUser = element(); widget.Listing2 = element()
    widget.Listing3 = element(); widget.Listing4 = element()
    if w.foreignExtra then widget.Listing5 = element() end
    widget.Listing4.alpha = 0
    return widget, nil, "tail"
  end }
  env.DataSources = {}
  env.Dvar = { com_maxclients = { get = function() return w.cap or 4 end } }
  env.IsDemoPlaying = function() return false end
  env.DebugPrint = function() end
  env.require = function() w.preloads = (w.preloads or 0) + 1 end
  for slot = 0, 7 do
    w.players[slot] = { clientNum = slot, playerConnected = slot < capacity and 1 or 0,
      playerScoreShown = 1, score = 1000 + slot, zombiePlayerIcon = "portrait" .. slot }
  end
  local source = assert(loadfile(root .. "/data/ui_scripts/zombies_coop_hud/rows.lua"))
  setfenv(source, env); w.extension = source()
  w.extension.install(function() return w.cap and w.cap >= 5 and w.cap <= 8 and w.cap or nil end)
  w.env = env
  function w.set(path, value) env.Engine.SetModelValue(get(w.root, path, true), value) end
  function w.read(path) return env.Engine.GetModelValue(get(w.root, path, false)) end
  function w.prepare()
    if env.DataSources.PlayerListZM then
      w.list = w.list or element()
      function w.list:updateDataSource() env.DataSources.PlayerListZM.prepare(0, self) end
      env.DataSources.PlayerListZM.prepare(0, w.list)
      return
    end
    local connected = {}
    for slot = 0, 7 do
      if w.players[slot].playerConnected == 1 then
        if slot == w.localClient then table.insert(connected, 1, slot) else connected[#connected + 1] = slot end
      end
    end
    for index, slot in ipairs(connected) do
      local row = "PlayerList." .. (index - 1) .. "."
      w.set(row .. "clientNum", slot); w.set(row .. "playerConnected", 1)
      w.set(row .. "playerScoreShown", 1); w.set(row .. "playerScore", w.players[slot].score)
    end
  end
  if arg[2] then
    local file = assert(io.open(arg[2], "rb")); local text = file:read("*a"); file:close()
    local first = assert(text:find("DataSources.PlayerListZM = {", 1, true))
    local last = assert(text:find("DataSources.RaidsPurchases = {", first, true))
    local stock = assert(loadstring(text:sub(first, last - 1), "@pinned-retail-PlayerListZM"))
    setfenv(stock, env); stock()
    first = assert(text:find("DataSources.ScoreboardTeam1List = {", 1, true))
    last = assert(text:find("DataSources.ScoreboardTeam2List = {", first, true))
    stock = assert(loadstring(text:sub(first, last - 1), "@pinned-retail-ScoreboardTeam1List"))
    setfenv(stock, env); stock()
  end
  w.prepare()
  return w
end

for cap = 5, 8 do
  local w = fixture(cap, 7)
  local before = w.env.CoD.ZMScr.new
  w.extension.install(function() return w.cap end)
  check(w.env.CoD.ZMScr.new == before, "repeated install does not chain wrappers")
  local out = pack(w.env.CoD.ZMScr.new("menu", 0, nil, "argument"))
  check(out.n == 3 and out[2] == nil and out[3] == "tail", "all original constructor returns retained")
  check(w.args.n == 4 and w.args[3] == nil and w.args[4] == "argument", "all original arguments retained")
  local widget = out[1]; local own = widget.boiiiCoopRows
  check(#own.items == cap - 4, "only configured extra display positions constructed")
  check(widget.Listing4.alpha == 0, "stock hidden alpha not overwritten")
  for index, item in ipairs(own.items) do
    check(item.pane.parent == widget.Listing4, "new rows inherit stock HUD start/hide alpha")
    check(item.visible and item.pane.alpha == 1, "connected extra display row visible")
    check(item.listing.model == w.env.Engine.GetModel(w.root, "PlayerList." .. (index + 3)), "retail display model bound")
    check(item.pane.topBottom[3] == -26 * index, "bounded upwards row spacing")
  end
  local item = own.items[1]
  local path = "PlayerList.4.playerScore"
  local initialScore = w.read(path)
  check(item.popupCount == 0, "initial score establishes baseline without false popup")
  w.set(path, initialScore + 10); check(item.popupCount == 1, "first actual positive score delta creates stock popup")
  local popup = next(item.popups)
  check(popup.ZMScrPlusPoints.Label1.text == "+10", "actual score value rendered")
  popup.events.clip_over(popup)
  check(item.popupCount == 0 and popup.closes == 1, "animation completion releases popup")
  for i = 1, 20 do w.set(path, initialScore + 10 + i * 10) end
  check(item.popupCount == 8, "burst of score events bounded to eight live effects per row")
  w.set("PlayerList.4.clientNum", 99)
  check(not item.visible and item.popupCount == 0, "invalid native identity hidden and effects released")
  w.set("PlayerList.4.clientNum", 4)
  w.set(path, 9000); check(item.popupCount == 0, "identity reuse never animates previous owner's score")
  w.set(path, 8500); check(item.popupCount == 1, "expenses render actual negative delta")
  popup = next(item.popups)
  check(popup.ZMScrPlusPoints.Label1.text == "-500" and popup.ZMScrPlusPoints.clip == "NegativeScore", "negative score stock clip")
  for _, hidden in ipairs({ false, 0, "nil" }) do
    local shown = hidden
    if hidden == "nil" then shown = nil end
    w.set("PlayerList.4.playerScoreShown", shown)
    check(not item.visible and item.popupCount == 0, "false/zero/nil visibility closes effects and hides row")
    w.set(path, 9000); w.set(path, 9010)
    check(item.popupCount == 0, "hidden false/zero/nil score updates suppress popups")
    w.set("PlayerList.4.playerScoreShown", 1)
  end
  widget:close("close-argument")
  check(own.closed and item.popupCount == 0 and item.pane.closes == 1 and item.listing.closes == 1, "HUD close releases all owned rows and effects")
  check(item.pane.childCountAtClose == 0 and item.pane.parent == nil and item.listing.parent == nil, "children detach before parent close; no second recursive child close")
  w.set(path, 8510); check(item.popupCount == 0, "post-close model changes cannot resurrect effects")
  check(w.writes == 0, "no native capacity, dvar or pool mutations")
end

do
  local w = fixture(8, 7); local widget = w.env.CoD.ZMScr.new("menu", 0)
  -- Seven is local: display4 means native slot3, not native slot4.
  if arg[2] then check(w.read("PlayerList.4.clientNum") == 3, "actual retail datasource publishes all eight rows and local-first stable identities") end
  if arg[2] then
    local scoreboard = w.env.Engine.CreateModel(w.root, "scoreboard.team1")
    local list = { scoreboardInfoModel = scoreboard }
    w.set("scoreboard.team1.count", 8)
    check(w.env.DataSources.ScoreboardTeam1List.getCount(list) == 8, "actual retail scoreboard datasource accepts engine count eight")
    for index = 1, 8 do
      local m = w.env.Engine.CreateModel(scoreboard, tostring(index - 1))
      check(w.env.DataSources.ScoreboardTeam1List.getItem(0, list, index) == m, "actual retail scoreboard datasource exposes every indexed row")
    end
    w.set("scoreboard.team1.count", 4)
    check(w.env.DataSources.ScoreboardTeam1List.getCount(list) == 4, "actual scoreboard count contracts after disconnect")
  end
  for slot = 0, 7 do w.players[slot].playerConnected = (slot == 0 or slot >= 5) and 1 or 0 end
  w.prepare(); w.set("playerDisconnected", 4)
  for _, item in ipairs(widget.boiiiCoopRows.items) do
    check(not item.visible, "8-to-4 with holes hides stale compacted trailing rows")
  end
  w.players[4].playerConnected = 1; w.players[4].score = 1234
  w.prepare(); w.set("playerConnected", 4)
  local item = widget.boiiiCoopRows.items[1]
  check(item.visible and item.slot == 6, "rejoin repopulates display4 with actual native identity")
  for slot = 0, 7 do w.players[slot].playerConnected = 1 end
  w.prepare(); w.set("playerConnected", 3)
  for _, row in ipairs(widget.boiiiCoopRows.items) do check(row.visible, "4-to-8 restores all connected display positions") end
  w.cap = 4; w.set("playerDisconnected", 3)
  for _, row in ipairs(widget.boiiiCoopRows.items) do check(not row.visible, "new stock session hides retained extra widgets") end
end

for _, cap in ipairs({ 1, 4, 9, 18 }) do
  local w = fixture(cap, 0)
  check(w.env.CoD.ZMScr.new("menu", 0).boiiiCoopRows == nil, "stock/unsupported capacity does not extend HUD")
end
do
  local w = fixture(8, 0); w.foreign = true
  local out = pack(w.env.CoD.ZMScr.new("menu", 0))
  check(out.n == 3 and out[1] == false and out[3] == "foreign", "foreign non-widget constructor result preserved")
  w.foreign = false; w.constructorError = true
  check(not pcall(w.env.CoD.ZMScr.new, "menu", 0), "original constructor failures preserved")
end
do
  local w = fixture(8, 0); w.foreignExtra = true
  check(w.env.CoD.ZMScr.new("menu", 0).boiiiCoopRows == nil, "recognizable existing expanded HUD is not duplicated")
  w.foreignExtra = false
  w.env.CoD.ZMScr_ListingSm.new = function() error("custom small-row failure") end
  local widget = w.env.CoD.ZMScr.new("menu", 0)
  check(widget.boiiiCoopRows.closed, "optional extension failure cleans partial rows without breaking original HUD")
  check(widget.boiiiCoopRows.items[1].pane.closes == 1, "partially constructed pane is released")
  widget:close()
  check(widget.boiiiCoopRows.items[1].pane.closes == 1, "later stock close does not close failed pane twice")
end
print("PASS: " .. checks .. " extra score-list checks; constructor forwarding, real display models, 8->4->8 holes, native identity reuse, visibility, bounded score effects and teardown" .. (arg[2] and "; actual pinned retail PlayerListZM executed" or "; model stand-ins only"))
