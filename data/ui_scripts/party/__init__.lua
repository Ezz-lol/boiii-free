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

local function expandGameOptionsList(widgetName)
  local widget = CoD[widgetName]
  if not widget or type(widget.new) ~= "function" or widget.boiiiExpandedOptions then
    return
  end

  local originalNew = widget.new
  widget.new = function(menu, controller)
    local instance = originalNew(menu, controller)
    if instance.buttonList then
      instance.buttonList:setTopBottom(true, false, 5, 239)
      instance.buttonList:setVerticalCount(7)
    end
    if instance.StartMenuConnectionMeterContainer0 then
      instance.StartMenuConnectionMeterContainer0:setTopBottom(true, false, 248, 425)
    end
    return instance
  end
  widget.boiiiExpandedOptions = true
end

expandGameOptionsList("StartMenu_GameOptions")
expandGameOptionsList("StartMenu_GameOptions_ZM")
