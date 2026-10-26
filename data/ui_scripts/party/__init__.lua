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

require("connection_meter")

local ROW_HEIGHT = 33.6
local STOCK_ROWS = 5
local METER_HEIGHT = 203.37

local function addConnectionMeter(instance, menu, controller)
  if instance.StartMenuConnectionMeterContainer0 or not CoD.StartMenu_ConnectionMeterContainer then
    return
  end
  local meter = CoD.StartMenu_ConnectionMeterContainer.new(menu, controller)
  meter:setLeftRight(true, false, 12, 361)
  meter:setTopBottom(true, false, 180, 180 + METER_HEIGHT)
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
  if meterTop and instance.StartMenuConnectionMeterContainer0 then
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
      addConnectionMeter(instance, menu, controller)
    end
    fitOptions(instance, top, meterTop)
    return instance
  end
  widget.new = restoredNew
  widget.boiiiRestoredNew = restoredNew
end

restoreGameOptions("StartMenu_GameOptions", 5, 206)
restoreGameOptions("StartMenu_GameOptions_ZM", 4.91, nil, true)
