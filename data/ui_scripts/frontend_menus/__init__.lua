if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

if not CoD.LobbyButtons then
  return
end

local utils = require("utils")

CoD.LobbyButtons.WORKSHOP = {
  stringRef = "WORKSHOP",
  action = function(self, element, controller, param, menu)
    OpenPopup(menu, "BoiiiWorkshopMenu", controller)
  end,
  customId = "btnWorkshop",
}

local function addWorkshopButton(controller, menuId, buttonTable)
  if menuId ~= LobbyData.UITargets.UI_MODESELECT.id or not buttonTable then
    return
  end
  local modsIndex = utils.GetButtonIndex(buttonTable, CoD.LobbyButtons.MODS_LOAD)
  utils.AddSmallButton(controller, buttonTable, CoD.LobbyButtons.WORKSHOP, modsIndex and modsIndex + 1)
end

local statsButton = {
  stringRef = "STATS",
  action = function(self, element, controller, param, menu)
    SetPerControllerTableProperty(controller, "disableGameSettingsOptions", true)
    OpenPopup(menu, "BoiiiStatsMenu", controller)
  end,
  customId = "btnMPStats",
}

local quickSettingsButton = {
  stringRef = "^8" .. Engine.Localize("BOIII SETTINGS"),
  action = function(self, element, controller, param, menu)
    SetPerControllerTableProperty(controller, "disableGameSettingsOptions", true)
    OpenPopup(menu, "BoiiiQuickSettingsMenu", controller)
  end,
  customId = "btnQuickSettings",
}

local function friendsPartyLabel()
  if Engine.DvarBool(nil, "friends_open") then
    return "^2PARTY IS OPEN"
  end
  return "^1PARTY IS CLOSED"
end

local friendsPartyButton = {
  stringRef = friendsPartyLabel(),
  action = function(self, element, controller, param, menu)
    Engine.ExecNow(controller, "friends_open")
    Engine.SetModelValue(Engine.GetModel(element:getModel(), "displayText"), friendsPartyLabel())
  end,
  customId = "btnFriendsParty",
}

local function isLobbyLeader(controller)
  local model = nil
  if Engine.IsLobbyActive(Enum.LobbyType.LOBBY_TYPE_GAME) then
    model = Engine.GetModel(DataSources.LobbyRoot.getModel(controller), "gameClient.isHost")
  else
    model = Engine.GetModel(DataSources.LobbyRoot.getModel(controller), "privateClient.isHost")
  end
  if model == nil then
    return 1
  end
  return Engine.GetModelValue(model)
end

local function isMainLobby(menuId)
  local targets = LobbyData.UITargets
  return menuId == targets.UI_MPLOBBYMAIN.id
    or menuId == targets.UI_MPLOBBYONLINE.id
    or menuId == targets.UI_ZMLOBBYONLINE.id
    or menuId == targets.UI_ZMLOBBYLANGAME.id
    or (targets.UI_CPLOBBYONLINE and menuId == targets.UI_CPLOBBYONLINE.id)
    or (targets.UI_CPLOBBYLANGAME and menuId == targets.UI_CPLOBBYLANGAME.id)
end

local function hasButton(buttonTable, button)
  for _, item in ipairs(buttonTable) do
    if item.customId == button.customId or item.optionDisplay == button.stringRef then
      return true
    end
  end
  return false
end

local function appendButton(controller, buttonTable, button)
  if not hasButton(buttonTable, button) then
    utils.AddSmallButton(controller, buttonTable, button)
  end
end

local function addModButtons(controller, menuId, buttonTable)
  if isMainLobby(menuId) then
    appendButton(controller, buttonTable, quickSettingsButton)
  end

  local isLeader = isLobbyLeader(controller)
  if
    (isLeader == 1 or isLeader == true)
    and Engine.GetLobbyNetworkMode() ~= Enum.LobbyNetworkMode.LOBBY_NETWORKMODE_LAN
  then
    for _, anchor in ipairs({
      CoD.LobbyButtons.ZM_CHANGE_MAP,
      CoD.LobbyButtons.MP_CUSTOM_SETUP_GAME,
      CoD.LobbyButtons.CP_SELECT_MISSION,
    }) do
      if utils.GetButtonIndex(buttonTable, anchor) then
        friendsPartyButton.stringRef = friendsPartyLabel()
        appendButton(controller, buttonTable, friendsPartyButton)
        break
      end
    end
  end
end

if
  type(Engine.IsUsingMods) == "function"
  and Engine.IsUsingMods()
  and (type(Engine.UsingModsUgcName) ~= "function" or Engine.UsingModsUgcName() ~= "usermaps")
then
  local oldAddButtonsForTarget = CoD.LobbyMenus.AddButtonsForTarget
  CoD.LobbyMenus.AddButtonsForTarget = function(controller, id)
    local result = oldAddButtonsForTarget(controller, id)
    if result then
      addWorkshopButton(controller, id, result)
      addModButtons(controller, id, result)
    end
    return result
  end
  return
end

local enableLobbyMapVote = true -- toggle map vote in public lobby
local enableLargeServerBrowserButton = true -- toggle large server browser button

require("datasources_start_menu_tabs")
require("datasources_change_map_categories")

CoD.LobbyButtons.STATS = statsButton
CoD.LobbyButtons.QUICK_SETTINGS = quickSettingsButton
CoD.LobbyButtons.FRIENDS_PARTY = friendsPartyButton
require("datasources_gamesettingsflyout_buttons")

CoD.LobbyButtons.PLAY_LOCAL.stringRef = "^1" .. Engine.Localize("MENU_PLAY_LOCAL_CAPS")
CoD.LobbyButtons.PLAY_ONLINE.stringRef = "^2" .. Engine.Localize("XBOXLIVE_PLAY_ONLINE_CAPS")

CoD.LobbyButtons.MP_PUBLIC_MATCH = {
  stringRef = "MENU_PLAY_CAPS",
  action = NavigateToLobby_SelectionList,
  param = "MPLobbyOnline",
  customId = "btnPublicMatch",
}

CoD.LobbyButtons.MP_FIND_MATCH = {
  stringRef = "MPUI_BASICTRAINING_CAPS",
  action = OpenFindMatch,
  customId = "btnFindMatch",
}

CoD.LobbyButtons.STEAM_STORE = {
  stringRef = "MENU_STORE_CAPS",
  action = function(self, element, controller, param, menu)
    local storeRoot = Engine.CreateModel(Engine.GetModelForController(controller), "StoreRoot")
    Engine.SetModelValue(Engine.CreateModel(storeRoot, "actionSource", true), "StoreButton")
    Engine.SetModelValue(Engine.CreateModel(storeRoot, "storeSource", true), LobbyData:GetCurrentMenuTarget().name)
    OpenOverlay(menu, "Store", controller)
  end,
  customId = "btnSteamStore",
}

CoD.LobbyButtons.MP_START_GAME = {
  stringRef = "MENU_START_GAME_CAPS",
  action = function(self, element, controller, param, menu)
    Engine.SetDvar("party_minplayers", 1)
    Engine.Exec(nil, "launchgame")
  end,
  customId = "btnStartGame",
}

CoD.LobbyButtons.SETTING_UP_BOTS = {
  stringRef = "MENU_SETUP_BOTS_CAPS",
  action = function(self, element, controller, param, menu)
    SetPerControllerTableProperty(controller, "disableGameSettingsOptions", true)
    OpenPopup(menu, "GameSettings_Bots", controller)
  end,
  customId = "btnSettingUpBots",
}

CoD.LobbyButtons.GameSettingsFlyoutArenas = {
  stringRef = "MPUI_SETUP_GAME_CAPS",
  action = function(self, element, controller, param, menu)
    SetPerControllerTableProperty(controller, "disableGameSettingsOptions", true)
    OpenPopup(menu, "GameSettingsFlyoutMP", controller)
  end,
  customId = "btnGameSettingsFlyoutMP",
}

CoD.LobbyButtons.GameSettingsFlyoutMP = {
  stringRef = "MPUI_SETUP_GAME_CAPS",
  action = function(self, element, controller, param, menu)
    SetPerControllerTableProperty(controller, "disableGameSettingsOptions", true)
    OpenPopup(menu, "GameSettingsFlyoutMPCustom", controller)
  end,
  customId = "btnGameSettingsFlyoutMPCustom",
}

CoD.LobbyButtons.SERVER_BROWSER = {
  stringRef = "MENU_SERVER_BROWSER_CAPS",
  action = function(self, element, controller, param, menu)
    SetPerControllerTableProperty(controller, "disableGameSettingsOptions", true)
    OpenPopup(menu, "LobbyServerBrowserOnline", controller)
  end,
  customId = "btnDedicated",
}

local shouldShowMapVote = enableLobbyMapVote
local lobbyMapVote = function(lobbyMapVoteIsEnabled)
  if lobbyMapVoteIsEnabled == true then
    Engine.Exec(nil, "LobbyStopDemo")
  end
end

local function addFriendsPartyButton(controller, menuId, buttonTable, isLeader)
  if
    (isLeader ~= 1 and isLeader ~= true)
    or Engine.GetLobbyNetworkMode() == Enum.LobbyNetworkMode.LOBBY_NETWORKMODE_LAN
  then
    return
  end
  for _, anchor in ipairs({
    CoD.LobbyButtons.ZM_CHANGE_MAP,
    CoD.LobbyButtons.MP_CUSTOM_SETUP_GAME,
    CoD.LobbyButtons.CP_SELECT_MISSION,
  }) do
    local index = utils.GetButtonIndex(buttonTable, anchor)
    if index then
      CoD.LobbyButtons.FRIENDS_PARTY.stringRef = friendsPartyLabel()
      utils.AddSmallButton(controller, buttonTable, CoD.LobbyButtons.FRIENDS_PARTY, index + 1)
      buttonTable[index + 1].isLastButtonInGroup = buttonTable[index].isLastButtonInGroup
      buttonTable[index].isLastButtonInGroup = false
      return
    end
  end
end

local addCustomButtons = function(controller, menuId, buttonTable, isLeader)
  if menuId == LobbyData.UITargets.UI_MPLOBBYMAIN.id then
    utils.RemoveSpaces(buttonTable)
    local theaterIndex = utils.GetButtonIndex(buttonTable, CoD.LobbyButtons.THEATER_MP)
    if theaterIndex ~= nil then
      utils.AddSpacer(buttonTable, theaterIndex - 1)
    end
  end

  if
    menuId == LobbyData.UITargets.UI_MPLOBBYMAIN.id
    or menuId == LobbyData.UITargets.UI_MPLOBBYONLINE.id
    or menuId == LobbyData.UITargets.UI_ZMLOBBYONLINE.id
    or menuId == LobbyData.UITargets.UI_ZMLOBBYLANGAME.id
    or (LobbyData.UITargets.UI_CPLOBBYONLINE and menuId == LobbyData.UITargets.UI_CPLOBBYONLINE.id)
    or (LobbyData.UITargets.UI_CPLOBBYLANGAME and menuId == LobbyData.UITargets.UI_CPLOBBYLANGAME.id)
  then
    utils.AddSmallButton(controller, buttonTable, CoD.LobbyButtons.STATS)
    utils.AddSmallButton(controller, buttonTable, CoD.LobbyButtons.QUICK_SETTINGS)
  end

  addFriendsPartyButton(controller, menuId, buttonTable, isLeader)

  if menuId == LobbyData.UITargets.UI_MPLOBBYONLINECUSTOMGAME.id then
    utils.AddSpacer(buttonTable)
    utils.AddSmallButton(controller, buttonTable, CoD.LobbyButtons.BLACK_MARKET)
  end

  if menuId == LobbyData.UITargets.UI_ZMLOBBYLANGAME.id then
    for _, button in ipairs({
      CoD.LobbyButtons.ZM_BUBBLEGUM_BUFFS,
      CoD.LobbyButtons.ZM_MEGACHEW_FACTORY,
      CoD.LobbyButtons.ZM_GOBBLEGUM_RECIPES,
    }) do
      if utils.GetButtonIndex(buttonTable, button) == nil then
        utils.AddSmallButton(controller, buttonTable, button)
      end
    end
  end

  if
    menuId == LobbyData.UITargets.UI_MPLOBBYONLINE.id
    or menuId == LobbyData.UITargets.UI_ZMLOBBYONLINE.id
    or menuId == LobbyData.UITargets.UI_MPLOBBYMAIN.id
    or menuId == LobbyData.UITargets.UI_MPLOBBYLANGAME.id
  then
    Engine.Mods_Lists_UpdateUsermaps()
  end

  if menuId == LobbyData.UITargets.UI_MPLOBBYONLINE.id then
    shouldShowMapVote = enableLobbyMapVote
    if enableLargeServerBrowserButton then
      utils.AddLargeButton(controller, buttonTable, CoD.LobbyButtons.SERVER_BROWSER, 1)
    end
  elseif menuId == LobbyData.UITargets.UI_MPLOBBYONLINEPUBLICGAME.id then
    utils.RemoveButton(buttonTable, CoD.LobbyButtons.MP_PUBLIC_LOBBY_LEADERBOARD)

    utils.AddLargeButton(controller, buttonTable, CoD.LobbyButtons.MP_START_GAME, 1)
    utils.AddSmallButton(controller, buttonTable, CoD.LobbyButtons.GameSettingsFlyoutMP, 2)
    utils.AddSpacer(buttonTable, utils.GetButtonIndex(buttonTable, CoD.LobbyButtons.GameSettingsFlyoutMP))

    lobbyMapVote(shouldShowMapVote)
    shouldShowMapVote = false
  elseif menuId == LobbyData.UITargets.UI_MPLOBBYONLINEARENAGAME.id then
    utils.AddLargeButton(controller, buttonTable, CoD.LobbyButtons.MP_START_GAME, 1)
    utils.AddSmallButton(controller, buttonTable, CoD.LobbyButtons.GameSettingsFlyoutArenas, 2)

    utils.AddSpacer(buttonTable, utils.GetButtonIndex(buttonTable, CoD.LobbyButtons.GameSettingsFlyoutArenas))
  end

  if menuId == LobbyData.UITargets.UI_ZMLOBBYONLINE.id then
    utils.RemoveButton(buttonTable, CoD.LobbyButtons.THEATER_ZM)
    utils.AddLargeButton(controller, buttonTable, CoD.LobbyButtons.THEATER_ZM)

    utils.RemoveSpaces(buttonTable)
    utils.AddSpacer(buttonTable, utils.GetButtonIndex(buttonTable, CoD.LobbyButtons.SERVER_BROWSER))
    local bgbIndex = utils.GetButtonIndex(buttonTable, CoD.LobbyButtons.ZM_BUBBLEGUM_BUFFS)
    if bgbIndex ~= nil then
      utils.AddSpacer(buttonTable, bgbIndex - 1)
    end
    utils.AddSpacer(buttonTable, utils.GetButtonIndex(buttonTable, CoD.LobbyButtons.STATS))
  end
end

local oldAddButtonsForTarget = CoD.LobbyMenus.AddButtonsForTarget
CoD.LobbyMenus.AddButtonsForTarget = function(controller, id)
  local isLeader = isLobbyLeader(controller)
  local result = oldAddButtonsForTarget(controller, id)
  addWorkshopButton(controller, id, result)
  addCustomButtons(controller, id, result, isLeader)
  return result
end
