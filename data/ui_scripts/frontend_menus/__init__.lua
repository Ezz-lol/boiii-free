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
      Engine.Exec(nil, "LobbyStopDemo")                                                -- Enable map vote at start lobby
    end
    utils.AddLargeButton(controller, buttonTable, CoD.LobbyButtons.MP_START_GAME, 1)   --Launch match button
    utils.AddSpacer(buttonTable, 1)

    utils.AddSpacer(buttonTable)
    utils.AddSmallButton(controller, buttonTable, CoD.LobbyButtons.MP_CUSTOM_SETUP_GAME) --Setup game in public lobby
  elseif menuId == LobbyData.UITargets.UI_MPLOBBYONLINEARENAGAME.id then
    utils.AddSpacer(buttonTable)
    utils.AddSmallButton(controller, buttonTable, CoD.LobbyButtons.SETTING_UP_BOTS) --Bot setting button in public lobby
  end
end

local targetButtons = {
  [LobbyData.UITargets.UI_MAIN.id] = CoD.LobbyMenus.ModeSelect,
  [LobbyData.UITargets.UI_MODESELECT.id] = CoD.LobbyMenus.ModeSelect,
  [LobbyData.UITargets.UI_CPLOBBYLANGAME.id] = CoD.LobbyMenus.CPButtonsLAN,
  [LobbyData.UITargets.UI_CPLOBBYLANCUSTOMGAME.id] = CoD.LobbyMenus.CPButtonsLANCUSTOM,
  [LobbyData.UITargets.UI_CPLOBBYONLINE.id] = CoD.LobbyMenus.CPButtonsOnline,
  [LobbyData.UITargets.UI_CPLOBBYONLINEPUBLICGAME.id] = CoD.LobbyMenus.CPButtonsPublicGame,
  [LobbyData.UITargets.UI_CPLOBBYONLINECUSTOMGAME.id] = CoD.LobbyMenus.CPButtonsCustomGame,
  [LobbyData.UITargets.UI_CP2LOBBYLANGAME.id] = CoD.LobbyMenus.CPZMButtonsLAN,
  [LobbyData.UITargets.UI_CP2LOBBYLANCUSTOMGAME.id] = CoD.LobbyMenus.CPButtonsLANCUSTOM,
  [LobbyData.UITargets.UI_CP2LOBBYONLINE.id] = CoD.LobbyMenus.CPZMButtonsOnline,
  [LobbyData.UITargets.UI_CP2LOBBYONLINEPUBLICGAME.id] = CoD.LobbyMenus.CPZMButtonsPublicGame,
  [LobbyData.UITargets.UI_CP2LOBBYONLINECUSTOMGAME.id] = CoD.LobbyMenus.CPButtonsCustomGame,
  [LobbyData.UITargets.UI_DOALOBBYLANGAME.id] = CoD.LobbyMenus.DOAButtonsLAN,
  [LobbyData.UITargets.UI_DOALOBBYONLINE.id] = CoD.LobbyMenus.DOAButtonsOnline,
  [LobbyData.UITargets.UI_DOALOBBYONLINEPUBLICGAME.id] = CoD.LobbyMenus.DOAButtonsPublicGame,
  [LobbyData.UITargets.UI_MPLOBBYLANGAME.id] = CoD.LobbyMenus.MPButtonsLAN,
  [LobbyData.UITargets.UI_MPLOBBYMAIN.id] = CoD.LobbyMenus.MPButtonsMain,
  [LobbyData.UITargets.UI_MPLOBBYONLINE.id] = CoD.LobbyMenus.MPButtonsOnline,
  [LobbyData.UITargets.UI_MPLOBBYONLINEPUBLICGAME.id] = CoD.LobbyMenus.MPButtonsOnlinePublic,
  [LobbyData.UITargets.UI_MPLOBBYONLINEMODGAME.id] = CoD.LobbyMenus.MPButtonsModGame,
  [LobbyData.UITargets.UI_MPLOBBYONLINECUSTOMGAME.id] = CoD.LobbyMenus.MPButtonsCustomGame,
  [LobbyData.UITargets.UI_MPLOBBYONLINEARENA.id] = CoD.LobbyMenus.MPButtonsArena,
  [LobbyData.UITargets.UI_MPLOBBYONLINEARENAGAME.id] = CoD.LobbyMenus.MPButtonsArenaGame,
  [LobbyData.UITargets.UI_FRLOBBYONLINEGAME.id] = CoD.LobbyMenus.FRButtonsOnlineGame,
  [LobbyData.UITargets.UI_FRLOBBYLANGAME.id] = CoD.LobbyMenus.FRButtonsLANGame,
  [LobbyData.UITargets.UI_ZMLOBBYLANGAME.id] = CoD.LobbyMenus.ZMButtonsLAN,
  [LobbyData.UITargets.UI_ZMLOBBYONLINE.id] = CoD.LobbyMenus.ZMButtonsOnline,
  [LobbyData.UITargets.UI_ZMLOBBYONLINEPUBLICGAME.id] = CoD.LobbyMenus.ZMButtonsPublicGame,
  [LobbyData.UITargets.UI_ZMLOBBYONLINECUSTOMGAME.id] = CoD.LobbyMenus.ZMButtonsCustomGame,
  [LobbyData.UITargets.UI_MPLOBBYONLINETHEATER.id] = CoD.LobbyMenus.ButtonsTheaterGame,
  [LobbyData.UITargets.UI_ZMLOBBYONLINETHEATER.id] = CoD.LobbyMenus.ButtonsTheaterGame
}

CoD.LobbyMenus.AddButtonsForTarget = function(controller, id)
  local buttonFunc = targetButtons[id]
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
  local result = {}
  buttonFunc(controller, result, isLeader)
  addCustomButtons(controller, id, result, isLeader)
  return result
end
