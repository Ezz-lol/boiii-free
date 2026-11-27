local STOCK_MAX = 4
local MAX_PLAYERS = 18
local REVIVE_OBJECTIVE = 110
local SCORE_LIST_ROWS = 8
local LOBBY_ROWS = 15
local LOBBY_ROWS_WITH_GUMS = 5

local function try(func, ...)
  local ok, result = pcall(func, ...)
  if ok then
    return result
  end
end

if Engine.GetCurrentMap() == "core_frontend" then
  local targets = { "UI_ZMLOBBYONLINE", "UI_ZMLOBBYONLINECUSTOMGAME", "UI_ZMLOBBYLANGAME" }
  for _, name in ipairs(targets) do
    local target = LobbyData.UITargets[name]
    if target and type(target.maxClients) == "number" and target.maxClients < MAX_PLAYERS then
      target.maxClients = MAX_PLAYERS
    end
  end

  local function isRaisedTarget(screen)
    for _, name in ipairs(targets) do
      local target = LobbyData.UITargets[name]
      if target and target.id == screen then
        return true
      end
    end
    return false
  end

  local getLobbyMaxClients = Engine.GetLobbyMaxClients
  local setLobbyMaxClients = Engine.SetLobbyMaxClients
  local raising = false
  local pickedOnScreen = nil

  local function raiseLobby()
    local screen = Engine.GetLobbyUIScreen()
    if raising or not isRaisedTarget(screen) or pickedOnScreen == screen then
      return
    end
    pickedOnScreen = nil
    raising = true
    for _, lobbyType in ipairs({ Enum.LobbyType.LOBBY_TYPE_PRIVATE, Enum.LobbyType.LOBBY_TYPE_GAME }) do
      if Engine.IsLobbyActive(lobbyType) and Engine.IsLobbyHost(lobbyType) then
        local current = getLobbyMaxClients(lobbyType)
        if type(current) == "number" and current > 1 and current < MAX_PLAYERS then
          setLobbyMaxClients(lobbyType, MAX_PLAYERS)
        end
      end
    end
    for _, dvar in ipairs({ "party_maxplayers", "com_maxclients" }) do
      local value = Engine.DvarInt(nil, dvar)
      if value > 1 and value < MAX_PLAYERS then
        Engine.SetDvar(dvar, MAX_PLAYERS)
      end
    end
    raising = false
  end

  Engine.GetLobbyMaxClients = function(...)
    try(raiseLobby)
    return getLobbyMaxClients(...)
  end

  Engine.SetLobbyMaxClients = function(...)
    if not raising then
      pickedOnScreen = Engine.GetLobbyUIScreen()
    end
    return setLobbyMaxClients(...)
  end

  LobbyHas4PlayersOrLess = function()
    local lobbyType = Enum.LobbyType.LOBBY_TYPE_PRIVATE
    if Engine.IsLobbyActive(Enum.LobbyType.LOBBY_TYPE_GAME) then
      lobbyType = Enum.LobbyType.LOBBY_TYPE_GAME
    end
    return Engine.GetLobbyClientCount(lobbyType) <= LOBBY_ROWS_WITH_GUMS
  end

  local prepareLobbyList = DataSources.LobbyList and DataSources.LobbyList.prepare
  if prepareLobbyList then
    DataSources.LobbyList.prepare = function(controller, list, ...)
      if not list.morePlayersCapped then
        list.morePlayersCapped = true
        local setVerticalCount = list.setVerticalCount
        list.setVerticalCount = function(self, count, ...)
          return setVerticalCount(self, math.min(count, LOBBY_ROWS), ...)
        end
      end
      return prepareLobbyList(controller, list, ...)
    end
  end
  return
end

if not CoD.isZombie then
  return
end

local extraColors = {
  [4] = { 0.95, 0.64, 0.53 },
  [5] = { 0.53, 0.95, 0.70 },
  [6] = { 0.75, 0.53, 0.95 },
  [7] = { 0.95, 0.81, 0.53 },
  [8] = { 0.53, 0.95, 0.87 },
  [9] = { 0.93, 0.53, 0.95 },
  [10] = { 0.90, 0.95, 0.53 },
  [11] = { 0.53, 0.84, 0.95 },
  [12] = { 0.95, 0.53, 0.78 },
  [13] = { 0.73, 0.95, 0.53 },
  [14] = { 0.53, 0.67, 0.95 },
  [15] = { 0.95, 0.53, 0.61 },
  [16] = { 0.55, 0.95, 0.53 },
  [17] = { 0.58, 0.53, 0.95 },
}

local function extraColor(clientNum)
  return extraColors[tonumber(clientNum)]
end

try(require, "ui.uieditor.modifyFunctions")
if ZombieClientScoreboardColor and ZombieClientScoreboardGlowColor then
  local stockColor = ZombieClientScoreboardColor
  local stockGlowColor = ZombieClientScoreboardGlowColor
  ZombieClientScoreboardColor = function(clientNum)
    local color = extraColor(clientNum)
    if not color then
      return stockColor(clientNum)
    end
    return color[1], color[2], color[3]
  end
  ZombieClientScoreboardGlowColor = function(clientNum)
    local color = extraColor(clientNum)
    if not color then
      return stockGlowColor(clientNum)
    end
    return color[1] * 0.75, color[2] * 0.75, color[3] * 0.75
  end
end

try(require, "ui.uieditor.widgets.Scoreboard.CP.ScoreboardWidgetCP")
if CoD.ScoreboardWidgetCP then
  local createScoreboard = CoD.ScoreboardWidgetCP.new
  CoD.ScoreboardWidgetCP.new = function(menu, controller)
    local widget = createScoreboard(menu, controller)
    local scores = widget.ScoreboardFactionScoresListCP0
    if scores and scores.Team1 then
      scores.Team1:setVerticalCount(MAX_PLAYERS)
    end
    local inventoryBarMaps =
      { zm_zod = true, zm_castle = true, zm_stalingrad = true, zm_genesis = true, zm_tomb = true }
    local bottom = inventoryBarMaps[Engine.GetCurrentMap()] and 570 or 710
    local countModel = Engine.CreateModel(Engine.GetGlobalModel(), "scoreboard.team1.count")
    local function fit()
      local topAnchor, bottomAnchor, top, bottomOffset = widget:getLocalTopBottom()
      widget.morePlayersTop = widget.morePlayersTop or top
      local height = bottomOffset - top
      local content = 47 + 26 * math.max(Engine.GetModelValue(countModel) or 0, 1)
      local baseTop = widget.morePlayersTop
      if baseTop + content <= bottom then
        widget:setScale(1)
        widget:setTopBottom(topAnchor, bottomAnchor, baseTop, baseTop + height)
        return
      end
      local visibleTop = math.max(185, bottom - content)
      local scale = math.min(1, (bottom - visibleTop) / content)
      local newTop = visibleTop - height / 2 * (1 - scale)
      widget:setScale(scale)
      widget:setTopBottom(topAnchor, bottomAnchor, newTop, newTop + height)
    end
    widget:subscribeToModel(countModel, fit, false)
    widget:subscribeToModel(
      Engine.CreateModel(
        Engine.GetModelForController(controller),
        "UIVisibilityBit." .. Enum.UIVisibilityBit.BIT_SCOREBOARD_OPEN
      ),
      fit,
      false
    )
    return widget
  end
end

local function addScoreRows(widget, menu, controller)
  local playerList = DataSources.ZMPlayerList.getModel(controller)
  local rows = {}
  for place = STOCK_MAX, SCORE_LIST_ROWS - 1 do
    local entry = Engine.CreateModel(playerList, place)
    Engine.SetModelValue(Engine.CreateModel(entry, "zombieInventoryIcon"), "blacktransparent")
    Engine.SetModelValue(Engine.CreateModel(entry, "zombieWearableIcon"), "blacktransparent")
    for name, value in pairs({ zombiePlayerIcon = "blacktransparent", playerScoreShown = 0 }) do
      local model = Engine.CreateModel(entry, name)
      if Engine.GetModelValue(model) == nil then
        Engine.SetModelValue(model, value)
      end
    end
    local row = CoD.ZMScr_ListingSm.new(menu, controller)
    local top = -26 * (place - STOCK_MAX + 1)
    row:setLeftRight(true, false, 16.28, 101.28)
    row:setTopBottom(true, false, top, top + 35)
    row:setAlpha(0)
    row:subscribeToGlobalModel(controller, "ZMPlayerList", tostring(place), function(model)
      row:setModel(model, controller)
    end)
    widget:addElement(row)
    widget["Listing" .. (place + 1)] = row
    table.insert(rows, row)
  end

  local extraList = Engine.CreateModel(Engine.GetModelForController(controller), "MorePlayersList")
  local extraModels = {}
  for place = SCORE_LIST_ROWS, MAX_PLAYERS - 1 do
    local entry = Engine.CreateModel(extraList, place)
    Engine.SetModelValue(Engine.CreateModel(entry, "zombieInventoryIcon"), "blacktransparent")
    Engine.SetModelValue(Engine.CreateModel(entry, "zombieWearableIcon"), "blacktransparent")
    Engine.SetModelValue(Engine.CreateModel(entry, "zombiePlayerIcon"), "blacktransparent")
    Engine.SetModelValue(Engine.CreateModel(entry, "playerScoreShown"), 0)
    Engine.SetModelValue(Engine.CreateModel(entry, "clientNum"), place)
    Engine.SetModelValue(Engine.CreateModel(entry, "playerScore"), 0)
    local row = CoD.ZMScr_ListingSm.new(menu, controller)
    local top = -26 * (place - STOCK_MAX + 1)
    row:setLeftRight(true, false, 16.28, 101.28)
    row:setTopBottom(true, false, top, top + 35)
    row:setAlpha(0)
    row:setModel(entry, controller)
    widget:addElement(row)
    table.insert(rows, row)
    table.insert(extraModels, entry)
  end

  local function modelValue(parent, name)
    local model = parent and Engine.GetModel(parent, name)
    return model and Engine.GetModelValue(model)
  end

  local function portraitsByCharacter(chars)
    local portraits = {}
    for place = 0, SCORE_LIST_ROWS - 1 do
      local entry = Engine.GetModel(playerList, place)
      local clientNum = modelValue(entry, "clientNum")
      local portrait = modelValue(entry, "zombiePlayerIcon")
      local char = clientNum and chars:sub(clientNum + 1, clientNum + 1)
      if char and char ~= "" and char ~= "-" and portrait and portrait ~= "" and portrait ~= "blacktransparent" then
        portraits[char] = portrait
      end
    end
    return portraits
  end

  local function refreshExtraRows()
    local team = Engine.GetModel(Engine.GetGlobalModel(), "scoreboard.team1")
    local count = team and Engine.GetModelValue(Engine.GetModel(team, "count")) or 0
    local players = {}
    for i = 0, count - 1 do
      local item = Engine.GetModel(team, i)
      local clientNum = item and Engine.GetModelValue(Engine.GetModel(item, "clientNum"))
      if clientNum and clientNum >= SCORE_LIST_ROWS then
        local score = tonumber(Engine.GetScoreboardColumnForClient(clientNum, 0)) or 0
        table.insert(players, { clientNum = clientNum, score = score })
      end
    end
    table.sort(players, function(a, b)
      return a.clientNum < b.clientNum
    end)
    local chars = Engine.DvarString(nil, "more_players_chars") or ""
    local portraits = portraitsByCharacter(chars)
    for i, entry in ipairs(extraModels) do
      local player = players[i]
      if player then
        Engine.SetModelValue(Engine.GetModel(entry, "clientNum"), player.clientNum)
        Engine.SetModelValue(Engine.GetModel(entry, "playerScore"), player.score)
        local char = chars:sub(player.clientNum + 1, player.clientNum + 1)
        Engine.SetModelValue(Engine.GetModel(entry, "zombiePlayerIcon"), portraits[char] or "blacktransparent")
      end
      Engine.SetModelValue(Engine.GetModel(entry, "playerScoreShown"), player and 1 or 0)
    end
  end

  widget:registerEventHandler("more_players_scores", function()
    try(refreshExtraRows)
  end)
  widget:addElement(LUI.UITimer.new(1000, { name = "more_players_scores" }, false, widget))

  local clipAlpha = {
    DefaultState = { DefaultClip = 0, HudStart = 1 },
    HudStart = { DefaultClip = 1, DefaultState = 0 },
  }
  for stateName, clips in pairs(clipAlpha) do
    local stateClips = widget.clipsPerState and widget.clipsPerState[stateName]
    if stateClips then
      for clipName, alpha in pairs(clips) do
        local stockClip = stateClips[clipName]
        if stockClip then
          stateClips[clipName] = function(...)
            stockClip(...)
            for _, row in ipairs(rows) do
              row:completeAnimation()
              row:setAlpha(alpha)
            end
          end
        end
      end
    end
  end

  LUI.OverrideFunction_CallOriginalSecond(widget, "close", function()
    for _, row in ipairs(rows) do
      row:close()
    end
  end)
end

local function addReviveIcons(hud, controller)
  local container = hud.fullscreenContainer
  local indicators = container and DataSources.WorldSpaceIndicators.getModel(controller)
  if not indicators then
    return
  end

  local existing = {}
  local child = container:getFirstChild()
  while child do
    if child.bleedOutClient then
      existing[child.bleedOutClient] = true
    end
    child = child:getNextSibling()
  end

  local bleedingOut = 2 ^ Enum.BleedOutStateFlags.BLEEDOUT_STATE_FLAG_BLEEDING_OUT
  local beingRevived = 2 ^ Enum.BleedOutStateFlags.BLEEDOUT_STATE_FLAG_BEING_REVIVED
  local icons = {}
  for client = STOCK_MAX, MAX_PLAYERS - 1 do
    if not existing[client] then
      local model = Engine.CreateModel(indicators, "bleedOutModel" .. client)
      Engine.SetModelValue(Engine.CreateModel(model, "playerName"), Engine.GetGamertagForClient(controller, client))
      Engine.SetModelValue(Engine.CreateModel(model, "prompt"), "ZMUI_REVIVE")
      Engine.SetModelValue(Engine.CreateModel(model, "arrowAngle"), 0)
      Engine.SetModelValue(Engine.CreateModel(model, "bleedOutPercent"), 0)
      local clock = Engine.CreateModel(model, "clockPercent")
      local flags = Engine.CreateModel(model, "stateFlags")
      Engine.SetModelValue(clock, 0)
      Engine.SetModelValue(flags, 0)

      local icon = LUI.UIElement.new()
      icon.bleedOutClient = client
      icon:setLeftRight(true, false, 0, 0)
      icon:setTopBottom(true, false, 0, 0)
      icon:setModel(model)
      icon:setAlpha(0)
      local reviveIcon = CoD.ZM_Revive.new(hud, controller)
      reviveIcon:setLeftRight(true, false, 0, 0)
      reviveIcon:setTopBottom(true, false, 0, 0)
      reviveIcon:setScale(0.3)
      reviveIcon:setModel(model)
      reviveIcon:processEvent({ name = "update_state", menu = hud })
      icon:addElement(reviveIcon)
      container:addElement(icon)

      local objective = REVIVE_OBJECTIVE + client
      local shownFlags = 0
      icon.update = function()
        local down = Engine.GetObjectiveState(controller, objective) == CoD.OBJECTIVESTATE_ACTIVE
        local wanted = 0
        if down then
          local x, y, z = Engine.GetObjectivePosition(controller, objective)
          if x and y and z then
            icon:setupEntityContainer(objective, x, y, z + 40)
            icon:setEntityContainerClamp(true)
          end
          local progress = tonumber(Engine.GetObjectiveProgress(controller, objective)) or 0
          wanted = bleedingOut
          if progress > 0 then
            wanted = bleedingOut + beingRevived
            Engine.SetModelValue(clock, progress)
          end
        end
        if wanted ~= shownFlags then
          shownFlags = wanted
          icon:setAlpha(down and 1 or 0)
          Engine.SetModelValue(flags, wanted)
        end
      end
      table.insert(icons, icon)
    end
  end

  if #icons > 0 then
    hud:registerEventHandler("more_players_revive_tick", function()
      for _, icon in ipairs(icons) do
        try(icon.update)
      end
    end)
    hud:addElement(LUI.UITimer.new(100, { name = "more_players_revive_tick" }, false, hud))
  end
end

try(require, "ui.uieditor.widgets.HUD.ZM_Score.ZMScr")
if CoD.ZMScr then
  local createScoreList = CoD.ZMScr.new
  CoD.ZMScr.new = function(menu, controller)
    local widget = createScoreList(menu, controller)
    try(addScoreRows, widget, menu, controller)
    if not menu.morePlayersRevive then
      menu.morePlayersRevive = true
      menu:registerEventHandler("more_players_revive", function()
        try(addReviveIcons, menu, controller)
      end)
      menu:addElement(LUI.UITimer.new(1000, { name = "more_players_revive" }, true, menu))
    end
    return widget
  end
end
