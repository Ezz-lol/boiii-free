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

local function applyGameOptionsPatch(instance, menu, controller)
  if not instance or not instance.buttonList then
    return false
  end
  if BoiiiStartMenuGameOptions then
    DataSources.StartMenuGameOptions = BoiiiStartMenuGameOptions
  end
  instance.buttonList:setTopBottom(true, false, 5, 271)
  instance.buttonList:setVerticalCount(8)
  if instance.StartMenuConnectionMeterContainer0 then
    instance.StartMenuConnectionMeterContainer0:setTopBottom(true, false, 280, 425)
  end
  return true
end

local function expandGameOptionsList(widgetName)
  local widget = CoD[widgetName]
  if not widget or type(widget.new) ~= "function" then
    return
  end
  if widget.boiiiExpandedNew and widget.new == widget.boiiiExpandedNew then
    return
  end

  local originalNew = widget.new
  local expandedNew = function(menu, controller)
    if BoiiiStartMenuGameOptions then
      DataSources.StartMenuGameOptions = BoiiiStartMenuGameOptions
    end
    local instance = originalNew(menu, controller)
    applyGameOptionsPatch(instance, menu, controller)
    return instance
  end
  widget.new = expandedNew
  widget.boiiiExpandedNew = expandedNew
end

expandGameOptionsList("StartMenu_GameOptions")
expandGameOptionsList("StartMenu_GameOptions_ZM")

local function expandExistingGameOptions(element, depth)
  if not element or depth > 24 then
    return
  end

  local id = tostring(element.id or "")
  if (id == "StartMenu_GameOptions" or id == "StartMenu_GameOptions_ZM") and element.buttonList then
    applyGameOptionsPatch(element, nil, nil)
  end

  if element.getFirstChild then
    local child = element:getFirstChild()
    while child do
      expandExistingGameOptions(child, depth + 1)
      child = child:getNextSibling()
    end
  end
end

if LUI.roots then
  for _, root in pairs(LUI.roots) do
    if type(root) == "table" then
      expandExistingGameOptions(root, 0)
    end
  end
end
