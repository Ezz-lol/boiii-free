if CoD.isFrontend or not Engine.DvarBool(nil, "boiii_xpBar") then
  return
end

-- Rank table row offsets, matching the stats menu
local modeRanks = {
  [Enum.eModes.MODE_MULTIPLAYER] = {
    rankTable = "gamedata/tables/mp/mp_ranktable.csv",
    paragonTable = "gamedata/tables/mp/mp_paragonranktable.csv",
    rankRowOffset = 3,
  },
  [Enum.eModes.MODE_ZOMBIES] = {
    rankTable = "gamedata/tables/zm/zm_ranktable.csv",
    paragonTable = "gamedata/tables/zm/zm_paragonranktable.csv",
    rankRowOffset = 2,
  },
}

local mode = Engine.CurrentSessionMode()
local ranks = modeRanks[mode]
if not ranks then
  return
end

local MASTER_PRESTIGE = 11
local PARAGON_ROW_OFFSET = 2
local MIN_XP_COLUMN = 2
local XP_TO_NEXT_COLUMN = 3

local DEFAULT_WIDTH = 1280
local BAR_HEIGHT = 2
local FILL_MS = 600
local FADE_MS = 150
local POLL_MS = 250
local POPUP_MS = 1800
local POPUP_WIDTH = 200
local POPUP_HEIGHT = 14
local POPUP_GAP = 4
local POPUP_DRIFT = 12
local POPUP_LEFT = 200 -- right of centre, clears kill playercards

-- Same order as the XP Bar Color option
local COLORS = {
  { 1.0, 0.58, 0.2 }, -- Orange
  { 1.0, 0.82, 0.25 }, -- Gold
  { 0.3, 0.65, 1.0 }, -- Blue
  { 0.35, 0.85, 0.35 }, -- Green
  { 0.95, 0.3, 0.3 }, -- Red
  { 0.7, 0.45, 1.0 }, -- Purple
  { 1.0, 1.0, 1.0 }, -- White
}

local SPAWN_EVENTS = { player_spawned = true, prematch_timer_ended = true }

local HIDE_BITS = {
  Enum.UIVisibilityBit.BIT_SCOREBOARD_OPEN,
  Enum.UIVisibilityBit.BIT_UI_ACTIVE,
  Enum.UIVisibilityBit.BIT_HIDE_FOR_FULLSCREEN_MENU,
  Enum.UIVisibilityBit.BIT_CAMERA_ANIM_HIDING_HUD,
  Enum.UIVisibilityBit.BIT_IN_KILLCAM,
  Enum.UIVisibilityBit.BIT_FINAL_KILLCAM,
  Enum.UIVisibilityBit.BIT_ROUND_END_KILLCAM,
  Enum.UIVisibilityBit.BIT_GAME_ENDED,
  Enum.UIVisibilityBit.BIT_SPECTATING_CLIENT,
  Enum.UIVisibilityBit.BIT_SELECTING_LOCATIONAL_KILLSTREAK,
  Enum.UIVisibilityBit.BIT_IN_GUIDED_MISSILE,
  Enum.UIVisibilityBit.BIT_IN_REMOTE_MISSILE,
  Enum.UIVisibilityBit.BIT_IS_DEMO_PLAYING,
}

local function readSettings()
  local color = Engine.DvarInt(nil, "boiii_xpBarColor") + 1
  return {
    enabled = Engine.DvarBool(nil, "boiii_xpBar"),
    top = Engine.DvarInt(nil, "boiii_xpBarPosition") == 1,
    popups = Engine.DvarBool(nil, "boiii_xpBarPopups"),
    color = COLORS[color] and color or 1,
  }
end

local function hudHidden(controller)
  if not Engine.IsVisibilityBitSet(controller, Enum.UIVisibilityBit.BIT_HUD_VISIBLE) then
    return true
  end
  for _, bit in ipairs(HIDE_BITS) do
    if Engine.IsVisibilityBitSet(controller, bit) then
      return true
    end
  end
  return false
end

local function statValue(list, name)
  local stat = list[name]
  return stat and tonumber(stat.StatValue:get()) or 0
end

local function tableNumber(tableName, row, column)
  return tonumber(Engine.TableLookupGetColumnValueForRow(tableName, row, column))
end

local function readRank(controller)
  local stats = Engine.GetPlayerStats(controller, CoD.STATS_LOCATION_NORMAL, mode)
  local list = stats and stats.PlayerStatsList
  if not list then
    return nil
  end

  local rank = {}
  if CoD.PrestigeUtility.GetCurrentPLevel(controller, mode) >= MASTER_PRESTIGE then
    rank.tableName = ranks.paragonTable
    rank.row = statValue(list, "PARAGON_RANK") + PARAGON_ROW_OFFSET
    rank.xp = statValue(list, "PARAGON_RANKXP")
  else
    rank.tableName = ranks.rankTable
    rank.row = statValue(list, "RANK") + ranks.rankRowOffset
    rank.xp = statValue(list, "RANKXP")
  end
  rank.minXp = tableNumber(rank.tableName, rank.row, MIN_XP_COLUMN)
  rank.span = tableNumber(rank.tableName, rank.row, XP_TO_NEXT_COLUMN)
  return rank
end

local function progress(rank)
  if not rank.minXp or not rank.span or rank.span <= 0 then
    return 1
  end
  return math.max(0, math.min(1, (rank.xp - rank.minXp) / rank.span))
end

local function popupPositions(top)
  local away = top and 1 or -1
  local near = away * (BAR_HEIGHT + POPUP_GAP)
  local far = near + away * POPUP_HEIGHT
  local drift = away * POPUP_DRIFT
  local from = { top, not top, math.min(near, far), math.max(near, far) }
  local to = { top, not top, from[3] + drift, from[4] + drift }
  return from, to
end

local function applyStyle(bar, settings)
  local edge = settings.top and { true, false, 0, BAR_HEIGHT } or { false, true, -BAR_HEIGHT, 0 }
  bar.back:setTopBottom(unpack(edge))
  bar.fill:setTopBottom(unpack(edge))

  local color = COLORS[settings.color]
  bar.fill:setRGB(color[1], color[2], color[3])
  bar.popup:setRGB(color[1], color[2], color[3])

  bar.popupFrom, bar.popupTo = popupPositions(settings.top)
  bar.popup:setTopBottom(unpack(bar.popupFrom))
end

local function viewportWidth(bar)
  local width = bar.uiRoot:getWidth()
  return width and width > 0 and width or DEFAULT_WIDTH
end

local function slideFill(bar, width, ms)
  bar.fill:beginAnimation("keyframe", ms, false, true, CoD.TweenType.Linear)
  bar.fill:setLeftRight(true, false, 0, width)
end

local function showPopup(bar, amount)
  bar.popup:setText(string.format("+%d XP", amount))
  bar.popup:setTopBottom(unpack(bar.popupFrom))
  bar.popup:setAlpha(1)
  bar.popup:beginAnimation("keyframe", POPUP_MS, false, true, CoD.TweenType.Linear)
  bar.popup:setTopBottom(unpack(bar.popupTo))
  bar.popup:setAlpha(0)
end

local function refreshVisibility(bar)
  local visible = bar.enabled and bar.spawned and not hudHidden(bar.controller)
  if bar.visible == visible then
    return
  end
  bar.visible = visible
  bar.root:beginAnimation("keyframe", FADE_MS, false, false, CoD.TweenType.Linear)
  bar.root:setAlpha(visible and 1 or 0)
end

local function update(bar)
  local settings = readSettings()
  bar.enabled = settings.enabled
  refreshVisibility(bar)
  if not settings.enabled then
    -- Avoids one large popup for XP earned while disabled
    bar.rank = nil
    return
  end

  local styleKey = tostring(settings.top) .. settings.color
  if bar.styleKey ~= styleKey then
    bar.styleKey = styleKey
    applyStyle(bar, settings)
  end

  local width = viewportWidth(bar)
  local rank = readRank(bar.controller)
  if not rank then
    return
  end

  if not bar.rank or width ~= bar.width then
    bar.rank = rank
    bar.width = width
    bar.pendingWidth = nil
    bar.fill:setLeftRight(true, false, 0, width * progress(rank))
    return
  end

  local previous = bar.rank
  if rank.xp == previous.xp and rank.row == previous.row then
    return
  end
  bar.rank = rank

  local target = width * progress(rank)
  if rank.tableName ~= previous.tableName or rank.row > previous.row then
    bar.pendingWidth = target
    slideFill(bar, width, FILL_MS / 2)
  else
    bar.pendingWidth = nil
    slideFill(bar, target, FILL_MS)
  end

  if settings.popups and rank.tableName == previous.tableName and rank.xp > previous.xp then
    showPopup(bar, rank.xp - previous.xp)
  end
end

local function createBar(root, controller)
  local bar = {
    controller = controller,
    uiRoot = root,
    enabled = false,
    visible = false,
    -- Multiplayer waits for the first spawn
    spawned = mode ~= Enum.eModes.MODE_MULTIPLAYER,
  }

  bar.root = LUI.UIElement.new()
  bar.root:setLeftRight(true, true, 0, 0)
  bar.root:setTopBottom(true, true, 0, 0)
  bar.root:setAlpha(0)
  root:addElement(bar.root)

  bar.back = LUI.UIImage.new()
  bar.back:setLeftRight(true, true, 0, 0)
  bar.back:setRGB(0, 0, 0)
  bar.back:setAlpha(0.45)
  bar.root:addElement(bar.back)

  bar.fill = LUI.UIImage.new()
  bar.fill:setLeftRight(true, false, 0, 0)
  bar.root:addElement(bar.fill)

  bar.popup = LUI.UIText.new()
  bar.popup:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
  bar.popup:setLeftRight(false, false, POPUP_LEFT, POPUP_LEFT + POPUP_WIDTH)
  bar.popup:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
  bar.popup:setAlpha(0)
  bar.root:addElement(bar.popup)

  bar.fill:registerEventHandler("transition_complete_keyframe", function(element, event)
    if bar.pendingWidth and not event.interrupted then
      local width = bar.pendingWidth
      bar.pendingWidth = nil
      element:setLeftRight(true, false, 0, 0)
      slideFill(bar, width, FILL_MS)
    end
  end)

  local controllerModel = Engine.GetModelForController(controller)
  bar.root:subscribeToModel(Engine.CreateModel(controllerModel, "scriptNotify"), function(model)
    if SPAWN_EVENTS[Engine.GetModelValue(model)] then
      bar.spawned = true
      refreshVisibility(bar)
    end
  end, false)

  local function watchBit(bit)
    bar.root:subscribeToModel(Engine.CreateModel(controllerModel, "UIVisibilityBit." .. bit), function()
      refreshVisibility(bar)
    end, false)
  end
  watchBit(Enum.UIVisibilityBit.BIT_HUD_VISIBLE)
  for _, bit in ipairs(HIDE_BITS) do
    watchBit(bit)
  end

  -- On the UI root, as timers don't run inside the hidden bar
  root:addElement(LUI.UITimer.newElementTimer(POLL_MS, false, function()
    update(bar)
  end))

  root.boiiiXpBar = bar
end

local primary = Engine.GetPrimaryController()
for controller = 0, Engine.GetMaxLocalControllers() - 1 do
  local root = LUI.roots["UIRoot" .. controller]
  if root and not root.boiiiXpBar and (controller == primary or Engine.IsControllerBeingUsed(controller)) then
    createBar(root, controller)
  end
end
