local modeInfo = LobbyData:UITargetFromId(Engine.GetLobbyUIScreen())
local maxClients = modeInfo.maxClients

-- Disable setting party privacy in menu. Auto set to open + max.
Engine.SetDvar("partyprivacyenabled", 0)
Engine.SetDvar("tu4_partyprivacyuseglobal", 0)
Engine.SetDvar("tu4_partyprivacyluacheck", 0)

-- Fix for invisible bots in custom games
if maxClients >= 1 then
  Engine.SetDvar("party_maxplayers", maxClients)
end

if Engine.GetCurrentMap() == "core_frontend" then
  return
end

-- Removed check for public matches to allow team change in ranked matches
CoD.IsTeamChangeAllowed = function()
  if Engine.GetGametypeSetting("allowInGameTeamChange") == 1 then
    return true
  else
    return false
  end
end

-- Set com_maxclients InGame so players can join via direct connect (default from lobbydata)
Engine.SetDvar("com_maxclients", maxClients)

require("datasources_start_menu_game_options")

pcall(require, "ui.uieditor.widgets.StartMenu.StartMenu_ConnectionMeterContainer")

local ROW_HEIGHT = 33.6
local STOCK_ROWS = 5
local METER_HEIGHT = 203.37

local function meterEnabled(controller)
  local ok, enabled = pcall(ConnectionMeter_isEnabled, nil, controller)
  return (not ok or enabled) and not Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_GAME)
end

local function createSimpleMeter(controller)
  local meter = LUI.UIElement.new()
  local function line(top, bottom, text, color)
    local element = LUI.UIText.new()
    element:setLeftRight(true, true, 0, 0)
    element:setTopBottom(true, false, top, bottom)
    element:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
    element:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
    element:setRGB(color[1], color[2], color[3])
    element:setText(text)
    meter:addElement(element)
    return element
  end
  line(0, 22, "CONNECTION METER", { 1, 0.55, 0.1 })
  local ping = line(26, 46, "PING: -", { 0.9, 0.9, 0.9 })
  local loss = line(48, 68, "PACKET LOSS: -", { 0.9, 0.9, 0.9 })

  local stats = Engine.CreateModel(Engine.GetGlobalModel(), "NetStatsInfo", true)
  ping:subscribeToModel(Engine.CreateModel(stats, "NETSTATS_LATENCY", true), function(model)
    local value = Engine.GetModelValue(model)
    if value then
      ping:setText("PING: " .. value .. "ms")
    end
  end)
  loss:subscribeToModel(Engine.CreateModel(stats, "NETSTATS_PACKETLOSS", true), function(model)
    local value = Engine.GetModelValue(model)
    if value then
      loss:setText("PACKET LOSS: " .. value .. "%")
    end
  end)
  meter:setAlpha(meterEnabled(controller) and 1 or 0)
  return meter
end

local function addConnectionMeter(instance, menu, controller, meterTop)
  if instance.StartMenuConnectionMeterContainer0 then
    return
  end
  local meter = CoD.StartMenu_ConnectionMeterContainer and CoD.StartMenu_ConnectionMeterContainer.new(menu, controller)
    or createSimpleMeter(controller)
  meter:setLeftRight(true, false, 12, 361)
  meter:setTopBottom(true, false, meterTop, meterTop + METER_HEIGHT)
  instance:addElement(meter)
  instance.StartMenuConnectionMeterContainer0 = meter
  LUI.OverrideFunction_CallOriginalSecond(instance, "close", function(element)
    element.StartMenuConnectionMeterContainer0:close()
  end)
end

local function fitOptions(instance, top, meterTop)
  local extra = math.max(0, (BoiiiStartMenuOptionCount or STOCK_ROWS) - STOCK_ROWS)
  if extra == 0 or not instance or not instance.buttonList then
    return
  end
  instance.buttonList:setVerticalCount(STOCK_ROWS + extra)
  instance.buttonList:setTopBottom(true, false, top, top + (STOCK_ROWS + extra) * ROW_HEIGHT)
  if instance.StartMenuConnectionMeterContainer0 then
    local shift = extra * ROW_HEIGHT
    instance.StartMenuConnectionMeterContainer0:setTopBottom(
      true,
      false,
      meterTop + shift,
      meterTop + METER_HEIGHT + shift
    )
  end
end

local function restoreGameOptions(widgetName, top, meterTop, addMeter)
  local widget = CoD[widgetName]
  if not widget or type(widget.new) ~= "function" then
    return
  end
  if widget.boiiiRestoredNew and widget.new == widget.boiiiRestoredNew then
    return
  end

  local originalNew = widget.new
  local restoredNew = function(menu, controller)
    if BoiiiAppendHostOptions then
      BoiiiAppendHostOptions(DataSources.StartMenuGameOptions)
    elseif BoiiiStartMenuGameOptions then
      DataSources.StartMenuGameOptions = BoiiiStartMenuGameOptions
    end
    local instance = originalNew(menu, controller)
    if addMeter then
      addConnectionMeter(instance, menu, controller, meterTop)
    end
    fitOptions(instance, top, meterTop)
    return instance
  end
  widget.new = restoredNew
  widget.boiiiRestoredNew = restoredNew
end

restoreGameOptions("StartMenu_GameOptions", 5, 206)
restoreGameOptions("StartMenu_GameOptions_ZM", 4.91, 245, true)
