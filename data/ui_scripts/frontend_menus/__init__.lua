if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

local enableLobbyMapVote = true             -- toggle map vote in public lobby
local enableLargeServerBrowserButton = true -- toggle large server browser button

local utils = require("utils")
require("datasources_start_menu_tabs")
require("datasources_change_map_categories")
require("datasources_gamesettingsflyout_buttons")

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

CoD.LobbyButtons.STATS = {
  stringRef = "STATS",
  action = function(self, element, controller, param, menu)
    SetPerControllerTableProperty(controller, "disableGameSettingsOptions", true)
    OpenPopup(menu, "BoiiiStatsMenu", controller)
  end,
  customId = "btnMPStats"
}

CoD.LobbyButtons.MP_START_GAME = {
  stringRef = "MENU_START_GAME_CAPS",
  action = function(self, element, controller, param, menu)
    Engine.SetDvar("party_minplayers", 1)
    Engine.Exec(nil, "launchgame")
  end,
  customId = "btnStartGame"
}

CoD.LobbyButtons.SETTING_UP_BOTS = {
  stringRef = "MENU_SETUP_BOTS_CAPS",
  action = function(self, element, controller, param, menu)
    SetPerControllerTableProperty(controller, "disableGameSettingsOptions", true)
    OpenPopup(menu, "GameSettings_Bots", controller)
  end,
  customId = "btnSettingUpBots"
}

CoD.LobbyButtons.GameSettingsFlyoutArenas = {
  stringRef = "MPUI_SETUP_GAME_CAPS",
  action = function(self, element, controller, param, menu)
    SetPerControllerTableProperty(controller, "disableGameSettingsOptions", true)
    OpenPopup(menu, "GameSettingsFlyoutMP", controller)
  end,
  customId = "btnGameSettingsFlyoutMP"
}

CoD.LobbyButtons.GameSettingsFlyoutMP = {
  stringRef = "MPUI_SETUP_GAME_CAPS",
  action = function(self, element, controller, param, menu)
    SetPerControllerTableProperty(controller, "disableGameSettingsOptions", true)
    OpenPopup(menu, "GameSettingsFlyoutMPCustom", controller)
  end,
  customId = "btnGameSettingsFlyoutMPCustom"
}

CoD.LobbyButtons.SERVER_BROWSER = {
  stringRef = "MENU_SERVER_BROWSER_CAPS",
  action = function(self, element, controller, param, menu)
    SetPerControllerTableProperty(controller, "disableGameSettingsOptions", true)
    OpenPopup(menu, "LobbyServerBrowserOnline", controller)
  end,
  customId = "btnDedicated"
}

local shouldShowMapVote = enableLobbyMapVote
local lobbyMapVote = function(lobbyMapVoteIsEnabled)
  if lobbyMapVoteIsEnabled == true then
    Engine.Exec(nil, "LobbyStopDemo")
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

  if menuId == LobbyData.UITargets.UI_MPLOBBYONLINE.id or menuId == LobbyData.UITargets.UI_ZMLOBBYONLINE.id then
    utils.AddSmallButton(controller, buttonTable, CoD.LobbyButtons.STATS)
  end

  if menuId == LobbyData.UITargets.UI_MPLOBBYONLINE.id or menuId == LobbyData.UITargets.UI_ZMLOBBYONLINE.id or menuId == LobbyData.UITargets.UI_MPLOBBYMAIN.id or menuId == LobbyData.UITargets.UI_MPLOBBYLANGAME.id then
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
  local model = nil
  if Engine.IsLobbyActive(Enum.LobbyType.LOBBY_TYPE_GAME) then
    model = Engine.GetModel(DataSources.LobbyRoot.getModel(controller), "gameClient.isHost")
  else
    model = Engine.GetModel(DataSources.LobbyRoot.getModel(controller), "privateClient.isHost")
  end
  local isLeader = nil
  if model ~= nil then
    isLeader = Engine.GetModelValue(model)
  else
    isLeader = 1
  end
  local result = oldAddButtonsForTarget(controller, id)
  addCustomButtons(controller, id, result, isLeader)
  return result
end
