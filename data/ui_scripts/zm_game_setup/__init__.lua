if Engine.GetCurrentMap() ~= "core_frontend" or not CoD.LobbyBase or not CoD.LobbyMenus then
  return
end

if
  type(Engine.IsUsingMods) == "function"
  and Engine.IsUsingMods()
  and (type(Engine.UsingModsUgcName) ~= "function" or Engine.UsingModsUgcName() ~= "usermaps")
then
  return
end

local function isZombies()
  return Engine.CurrentSessionMode() == Enum.eModes.MODE_ZOMBIES
end

if DataSources.ChangeMapMaps then
  local prepareMaps = DataSources.ChangeMapMaps.prepare
  DataSources.ChangeMapMaps.prepare = function(...)
    if not isZombies() then
      return prepareMaps(...)
    end
    local startswith = LUI.startswith
    LUI.startswith = function(text, prefix)
      return startswith(text, prefix == "mp_" and "zm_" or prefix)
    end
    local ok, err = pcall(prepareMaps, ...)
    LUI.startswith = startswith
    if not ok then
      error(err)
    end
  end
end

if DataSources.BotSettings then
  local stockBotSettings = DataSources.BotSettings
  local zmBotSettings = DataSourceHelpers.ListSetup("BotSettings", function(controller)
    local counts = { { option = "MENU_DISABLED", value = 0, default = true } }
    for count = 1, 17 do
      table.insert(counts, { option = tostring(count), value = count })
    end
    return {
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "BOT COUNT",
        "MENU_FREE_BOT_DESC",
        "ZMBotOptions_Count",
        "zm_bots",
        counts
      ),
      CoD.OptionsUtility.CreateDvarSettings(controller, "BOT BEHAVIOUR", "", "ZMBotOptions_Mode", "zm_bot_mode", {
        { option = "FOLLOW", value = 0, default = true },
        { option = "WANDER", value = 1 },
        { option = "CAMP", value = 2 },
      }),
    }
  end, true)
  DataSources.BotSettings = setmetatable({}, {
    __index = function(_, key)
      return (isZombies() and zmBotSettings or stockBotSettings)[key]
    end,
  })
end

if LUI.createMenu.GameSettings_Bots then
  local createBotsMenu = LUI.createMenu.GameSettings_Bots
  LUI.createMenu.GameSettings_Bots = function(controller)
    local menu = createBotsMenu(controller)
    if isZombies() then
      menu.Options.Title.DescTitle:setText("BOTS")
      menu.GameSettingsBackground.GameSettingsSelectedItemInfo:setAlpha(0)
    end
    return menu
  end
end

local function openGameSetup(_, _, controller, _, menu)
  SetPerControllerTableProperty(controller, "disableGameSettingsOptions", true)
  OpenPopup(menu, "GameSettingsFlyoutMPCustom", controller)
end

local function openCharacterSelect(_, _, controller, _, menu)
  CoD.ZMOpenCharacterSelect(menu, controller)
end

local function newButton(text, action, customId)
  return {
    optionDisplay = text,
    action = action,
    customId = customId,
    isLargeButton = false,
    isLastButtonInGroup = false,
    disabled = false,
    selected = false,
  }
end

local addButtonsForTarget = CoD.LobbyMenus.AddButtonsForTarget
CoD.LobbyMenus.AddButtonsForTarget = function(controller, id)
  local result = addButtonsForTarget(controller, id)
  local targets = LobbyData.UITargets
  if not result or (id ~= targets.UI_ZMLOBBYONLINECUSTOMGAME.id and id ~= targets.UI_ZMLOBBYLANGAME.id) then
    return result
  end
  for index, button in ipairs(result) do
    if button.optionDisplay == CoD.LobbyButtons.ZM_CHANGE_MAP.stringRef then
      local setup = newButton("MPUI_SETUP_GAME_CAPS", openGameSetup, "btnZMGameSetup")
      setup.isLastButtonInGroup = button.isLastButtonInGroup
      result[index] = setup
      return result
    end
  end
  if CoD.ZMMapHasCharacterSelect and CoD.ZMMapHasCharacterSelect() then
    table.insert(result, 1, newButton("CHOOSE CHARACTER", openCharacterSelect, "btnZMCharacterSelect"))
  end
  return result
end

local prepareLobbyButtons = DataSources.LobbyButtons.prepare
DataSources.LobbyButtons.prepare = function(controller, list, filter)
  prepareLobbyButtons(controller, list, filter)
  if list.characterSelectMapSub then
    list:removeSubscription(list.characterSelectMapSub)
  end
  list.characterSelectSupported = CoD.ZMMapHasCharacterSelect and CoD.ZMMapHasCharacterSelect()
  local mapModel = Engine.CreateModel(Engine.GetGlobalModel(), "MapVote.mapVoteMapNext")
  list.characterSelectMapSub = list:subscribeToModel(mapModel, function()
    local supported = CoD.ZMMapHasCharacterSelect and CoD.ZMMapHasCharacterSelect()
    if supported ~= list.characterSelectSupported then
      list.characterSelectSupported = supported
      list:updateDataSource()
    end
  end, false)
end
