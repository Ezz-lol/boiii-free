if Engine.GetCurrentMap() ~= "core_frontend" or not CoD.LobbyBase or not CoD.LobbyMenus then
  return
end

local modded = type(Engine.IsUsingMods) == "function"
  and Engine.IsUsingMods()
  and (type(Engine.UsingModsUgcName) ~= "function" or Engine.UsingModsUgcName() ~= "usermaps")

pcall(require, "ui.uieditor.menus.GameSettings.GameSettings_Bots")

local function isZombies()
  return Engine.CurrentSessionMode() == Enum.eModes.MODE_ZOMBIES
end

if not modded and DataSources.ChangeMapMaps then
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
  local function setBotDvar(_, item, _, dvar)
    UpdateInfoModels(item)
    if item.value ~= Engine.DvarInt(nil, dvar) then
      Engine.SetDvar(dvar, item.value)
    end
  end
  local features = {
    { "zm_bot_revive", "Revive Players" },
    { "zm_bot_doors", "Open Doors" },
    { "zm_bot_box", "Use Box" },
    { "zm_bot_wallbuys", "Buy Wall Weapons" },
    { "zm_bot_perks", "Buy Perks" },
    { "zm_bot_pap", "Pack-a-Punch" },
    { "zm_bot_windows", "Repair Windows" },
    { "zm_bot_powerups", "Pick Up Power-ups" },
    { "zm_bot_gums", "Use GobbleGums" },
    { "zm_bot_wunderfizz", "Use Der Wunderfizz" },
    { "zm_bot_shield", "Pick Up Shield" },
    { "zm_bot_power", "Turn On Power" },
    { "zm_bot_grenades", "Use Grenades" },
  }
  local zmBotSettings = DataSourceHelpers.ListSetup("BotSettings", function(controller)
    local counts = { { option = "MENU_DISABLED", value = 0, default = true } }
    for count = 1, 17 do
      table.insert(counts, { option = tostring(count), value = count })
    end
    local settings = {
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Bot Count",
        "MENU_FREE_BOT_DESC",
        "ZMBotOptions_Count",
        "zm_bots",
        counts,
        nil,
        setBotDvar
      ),
      CoD.OptionsUtility.CreateDvarSettings(controller, "Bot Behaviour", "", "ZMBotOptions_Mode", "zm_bot_mode", {
        { option = "Follow", value = 0, default = true },
        { option = "Wander", value = 1 },
        { option = "Camp", value = 2 },
      }, nil, setBotDvar),
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Bot Difficulty",
        "",
        "ZMBotOptions_Difficulty",
        "zm_bot_difficulty",
        {
          { option = "Recruit", value = 0 },
          { option = "Regular", value = 1, default = true },
          { option = "Hardened", value = 2 },
          { option = "Veteran", value = 3 },
        },
        nil,
        setBotDvar
      ),
      CoD.OptionsUtility.CreateDvarSettings(controller, "Bot Loadouts", "", "ZMBotOptions_Loadout", "zm_bot_loadout", {
        { option = "Default", value = 0, default = true },
        { option = "Copy Host", value = 1 },
      }, nil, setBotDvar),
    }
    for _, feature in ipairs(features) do
      table.insert(
        settings,
        CoD.OptionsUtility.CreateDvarSettings(controller, feature[2], "", "ZMBotOptions_" .. feature[1], feature[1], {
          { option = "On", value = 1, default = true },
          { option = "Off", value = 0 },
        }, nil, setBotDvar)
      )
    end
    return settings
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
      require("ui.uieditor.widgets.Scrollbars.verticalScrollbar")
      menu.Options.ButtonList:setVerticalCount(13)
      menu.Options.ButtonList:setVerticalScrollbar(CoD.verticalScrollbar)
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

local function openChangeMap(_, _, controller, _, menu)
  SetPerControllerTableProperty(controller, "disableGameSettingsOptions", true)
  CoD.LobbyBase.OpenChangeMap(menu, controller)
end

local function openBotSettings(_, _, controller, _, menu)
  SetPerControllerTableProperty(controller, "disableGameSettingsOptions", true)
  CoD.LobbyBase.OpenBotSettings(menu, controller)
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

local function swapButtons(result, first, second)
  local a, b
  for index, button in ipairs(result) do
    if button.optionDisplay == first then
      a = index
    elseif button.optionDisplay == second then
      b = index
    end
  end
  if a and b then
    local lastA, lastB = result[a].isLastButtonInGroup, result[b].isLastButtonInGroup
    result[a], result[b] = result[b], result[a]
    result[a].isLastButtonInGroup, result[b].isLastButtonInGroup = lastA, lastB
  end
end

local addButtonsForTarget = CoD.LobbyMenus.AddButtonsForTarget
CoD.LobbyMenus.AddButtonsForTarget = function(controller, id)
  local result = addButtonsForTarget(controller, id)
  local targets = LobbyData.UITargets
  if not result or (id ~= targets.UI_ZMLOBBYONLINECUSTOMGAME.id and id ~= targets.UI_ZMLOBBYLANGAME.id) then
    return result
  end
  swapButtons(
    result,
    CoD.LobbyButtons.ZM_CHANGE_RANKED_SETTTINGS.stringRef,
    CoD.LobbyButtons.ZM_SERVER_SETTINGS.stringRef
  )
  for index, button in ipairs(result) do
    if button.optionDisplay == CoD.LobbyButtons.ZM_CHANGE_MAP.stringRef then
      local added = {}
      if modded then
        if CoD.ZMMapHasCharacterSelect and CoD.ZMMapHasCharacterSelect() then
          table.insert(added, newButton("CHOOSE CHARACTER", openCharacterSelect, "btnZMCharacterSelect"))
        end
        if LUI.createMenu.GameSettings_Bots then
          table.insert(added, newButton("MENU_SETUP_BOTS_CAPS", openBotSettings, "btnZMSetupBots"))
        end
      else
        button.action = openChangeMap
        table.insert(added, newButton("MPUI_SETUP_GAME_CAPS", openGameSetup, "btnZMGameSetup"))
      end
      if #added > 0 then
        added[#added].isLastButtonInGroup = button.isLastButtonInGroup
        button.isLastButtonInGroup = false
      end
      for offset, extra in ipairs(added) do
        table.insert(result, index + offset, extra)
      end
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
