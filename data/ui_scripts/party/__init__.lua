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

if not Engine.IsInGame() then
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

local ROW_HEIGHT = 33.6
local STOCK_ROWS = 5

local function fitOptions(instance, top)
  local extra = math.max(0, (BoiiiStartMenuOptionCount or STOCK_ROWS) - STOCK_ROWS)
  if extra == 0 or not instance or not instance.buttonList then
    return
  end
  instance.buttonList:setVerticalCount(STOCK_ROWS + extra)
  instance.buttonList:setTopBottom(true, false, top, top + (STOCK_ROWS + extra) * ROW_HEIGHT)
  if instance.StartMenuConnectionMeterContainer0 then
    local shift = extra * ROW_HEIGHT
    instance.StartMenuConnectionMeterContainer0:setTopBottom(true, false, 206 + shift, 409.37 + shift)
  end
end

local function restoreGameOptions(widgetName, top)
  local widget = CoD[widgetName]
  if not widget or type(widget.new) ~= "function" then
    return
  end
  if widget.boiiiRestoredNew and widget.new == widget.boiiiRestoredNew then
    return
  end

  local originalNew = widget.new
  local restoredNew = function(menu, controller)
    if BoiiiStartMenuGameOptions then
      DataSources.StartMenuGameOptions = BoiiiStartMenuGameOptions
    end
    local instance = originalNew(menu, controller)
    fitOptions(instance, top)
    return instance
  end
  widget.new = restoredNew
  widget.boiiiRestoredNew = restoredNew
end

restoreGameOptions("StartMenu_GameOptions", 5)
restoreGameOptions("StartMenu_GameOptions_ZM", 4.91)
