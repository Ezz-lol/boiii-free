if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

local utils = require("utils")

CoD.LobbyButtons.MP_STATS = {
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

CoD.LobbyButtons.MP_CUSTOM_SETUP_GAME = {
  stringRef = "MPUI_SETUP_GAME_CAPS",
  action = OpenSetupGameMP,
  customId = "btnSetupGame",
}

local shouldShowMapVote = false
local addCustomButtons = function(controller, menuId, buttonTable, isLeader)
  if menuId == LobbyData.UITargets.UI_MPLOBBYONLINE.id or menuId == LobbyData.UITargets.UI_ZMLOBBYONLINE.id then
    utils.AddSpacer(buttonTable)
    utils.AddSmallButton(controller, buttonTable, CoD.LobbyButtons.MP_STATS)
  end

  if menuId == LobbyData.UITargets.UI_MPLOBBYONLINE.id or menuId == LobbyData.UITargets.UI_ZMLOBBYONLINE.id or menuId == LobbyData.UITargets.UI_MPLOBBYMAIN.id or menuId == LobbyData.UITargets.UI_MPLOBBYLANGAME.id then
    Engine.Mods_Lists_UpdateUsermaps()
  end

  if menuId == LobbyData.UITargets.UI_MPLOBBYONLINE.id then
    shouldShowMapVote = true
  elseif menuId == LobbyData.UITargets.UI_MPLOBBYONLINEPUBLICGAME.id then
    if shouldShowMapVote == true then
      shouldShowMapVote = false
      --Enable map vote at start lobby
      Engine.Exec(nil, "LobbyStopDemo")
    end
    utils.AddLargeButton(controller, buttonTable, CoD.LobbyButtons.MP_START_GAME, 1) --Launch match button
    utils.AddSpacer(buttonTable, 1)

    utils.AddSpacer(buttonTable)
    utils.AddSmallButton(controller, buttonTable, CoD.LobbyButtons.MP_CUSTOM_SETUP_GAME) --Setup game in public lobby
  elseif menuId == LobbyData.UITargets.UI_MPLOBBYONLINEARENAGAME.id then
    utils.AddSpacer(buttonTable)
    utils.AddSmallButton(controller, buttonTable, CoD.LobbyButtons.SETTING_UP_BOTS) --Bot setting button in public lobby
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
