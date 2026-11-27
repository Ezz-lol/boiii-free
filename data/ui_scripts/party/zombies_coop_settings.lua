-- This menu saves the next lobby preference. It never resizes a live session.
local settings = {}
local targets = {
  UI_ZMLOBBYONLINE = "LOBBY_MODE_CUSTOM",
  UI_ZMLOBBYONLINEPUBLICGAME = "LOBBY_MODE_PUBLIC",
  UI_ZMLOBBYONLINECUSTOMGAME = "LOBBY_MODE_CUSTOM",
  UI_ZMLOBBYLANGAME = "LOBBY_MODE_CUSTOM",
}

local function eligible(id)
  if
    Engine.GetCurrentMap() ~= "core_frontend"
    or Engine.IsInGame()
    or Engine.CurrentSessionMode() ~= Enum.eModes.MODE_ZOMBIES
    or not Engine.IsLobbyActive(Enum.LobbyType.LOBBY_TYPE_GAME)
    or not Engine.IsLobbyHost(Enum.LobbyType.LOBBY_TYPE_GAME)
    or (type(Engine.LobbyHostLaunchTaskInProgress) == "function" and Engine.LobbyHostLaunchTaskInProgress())
  then
    return false
  end
  if type(game) ~= "table" or type(game.cooplobbyinfo) ~= "function" then
    return false
  end
  local ok, info = pcall(game.cooplobbyinfo)
  if not ok or type(info) ~= "table" or info.enabled ~= true then
    return false
  end
  for name, mode in pairs(targets) do
    local target = LobbyData.UITargets[name]
    if
      type(target) == "table"
      and target.id == id
      and target.mainMode == Enum.LobbyMainMode.LOBBY_MAINMODE_ZM
      and target.lobbyType == Enum.LobbyType.LOBBY_TYPE_GAME
      and Enum.LobbyMode[mode] ~= nil
      and target.lobbyMode == Enum.LobbyMode[mode]
    then
      return true
    end
  end
  return false
end

local function updateCapacity(_, selection, _, dvarName)
  local value = type(selection) == "table" and selection.value or nil
  if
    dvarName ~= "zm_coopPlayerLimit"
    or type(value) ~= "number"
    or value ~= math.floor(value)
    or value < 1
    or value > 8
    or not eligible(Engine.GetLobbyUIScreen())
  then
    return
  end
  if Engine.DvarInt(nil, "zm_coopPlayerLimit") == value then
    return
  end
  Engine.SetDvar("zm_coopPlayerLimit", value)
  if type(UpdateInfoModels) == "function" then
    UpdateInfoModels(selection)
  end
end

local function buttonWrapperInstalled(menus, expected)
  if type(expected) ~= "function" then
    return false
  end
  local current = menus.AddButtonsForTarget
  local sources = menus.BoiiiCoopButtonSources
  -- Accept exact identity through our own outer wrappers only. Unknown/stock
  -- replacements break the chain and must be wrapped afresh.
  for _ = 1, 8 do
    if current == expected then
      return true
    end
    if type(current) ~= "function" or type(sources) ~= "table" then
      return false
    end
    current = sources[current]
  end
  return false
end

local function recordButtonWrapper(menus, wrapper, original)
  if type(menus.BoiiiCoopButtonSources) ~= "table" then
    menus.BoiiiCoopButtonSources = setmetatable({}, { __mode = "k" })
  end
  menus.BoiiiCoopButtonSources[wrapper] = original
end

function settings.install()
  if
    Engine.GetCurrentMap() ~= "core_frontend"
    or type(CoD) ~= "table"
    or not CoD.OptionsUtility
    or type(CoD.OptionsUtility.CreateDvarSettings) ~= "function"
    or not CoD.Menu
    or not LUI
    or not LUI.createMenu
    or not DataSources
    or not DataSourceHelpers
    or type(DataSourceHelpers.ListSetup) ~= "function"
  then
    return false
  end

  DataSources.BoiiiZombiesLobbySettings = DataSourceHelpers.ListSetup("BoiiiZombiesLobbySettings", function(controller)
    if not eligible(Engine.GetLobbyUIScreen()) then
      return {}
    end
    local values = {}
    for count = 1, 8 do
      table.insert(values, { option = tostring(count), value = count, default = count == 4 })
    end
    return {
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Next lobby size",
        "Choose 1 to 8 players for a new Zombies lobby. Default: 4. Leave and recreate the lobby to apply your choice. "
          .. "This keeps the current lobby and any running game unchanged. Expanded sessions require matching experimental clients and supported maps.",
        "BoiiiZombiesLobbySettings_capacity",
        "zm_coopPlayerLimit",
        values,
        nil,
        updateCapacity
      ),
    }
  end)

  LUI.createMenu.BoiiiZombiesLobbySettingsMenu = function(controller)
    if not eligible(Engine.GetLobbyUIScreen()) then
      return nil
    end
    local self = CoD.Menu.NewForUIEditor("BoiiiZombiesLobbySettingsMenu")
    self.soundSet = "ChooseDecal"
    self:setOwner(controller)
    self:setLeftRight(true, true, 0, 0)
    self:setTopBottom(true, true, 0, 0)
    self:playSound("menu_open", controller)
    self.buttonModel =
      Engine.CreateModel(Engine.GetModelForController(controller), "BoiiiZombiesLobbySettingsMenu.buttonPrompts")
    self.anyChildUsesUpdateState = true

    local background = CoD.GameSettings_Background.new(self, controller)
    background:setLeftRight(true, true, 0, 0)
    background:setTopBottom(true, true, 0, 0)
    background.MenuFrame.titleLabel:setText(Engine.Localize("ZOMBIES LOBBY SETTINGS"))
    background.MenuFrame.cac3dTitleIntermediary0.FE3dTitleContainer0.MenuTitle.TextBox1.Label0:setText(
      Engine.Localize("ZOMBIES LOBBY SETTINGS")
    )
    background.GameSettingsSelectedItemInfo.GameModeInfo:setAlpha(0)
    background.GameSettingsSelectedItemInfo.GameModeName:setAlpha(0)
    self:addElement(background)
    self.GameSettingsBackground = background

    local options = CoD.Competitive_SettingsList.new(self, controller)
    options:setLeftRight(true, false, 26, 741)
    options:setTopBottom(true, false, 135, 720)
    options.Title.DescTitle:setText(Engine.Localize("Next lobby"))
    options.ButtonList:setVerticalCount(1)
    options.ButtonList:setDataSource("BoiiiZombiesLobbySettings")
    self:addElement(options)
    self.Options = options
    options.id = "Options"

    self:AddButtonCallbackFunction(self, controller, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE, nil, function()
      GoBack(self, controller)
      return true
    end, function(_, menu)
      CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE, "MENU_BACK")
      return true
    end, false)
    background.MenuFrame:setModel(self.buttonModel, controller)
    self:processEvent({ name = "menu_loaded", controller = controller })
    self:processEvent({ name = "update_state", menu = self })
    if not self:restoreState() then
      options:processEvent({ name = "gain_focus", controller = controller })
    end
    LUI.OverrideFunction_CallOriginalSecond(self, "close", function(element)
      element.GameSettingsBackground:close()
      element.Options:close()
      Engine.UnsubscribeAndFreeModel(
        Engine.GetModel(Engine.GetModelForController(controller), "BoiiiZombiesLobbySettingsMenu.buttonPrompts")
      )
    end)
    return self
  end

  local menus = CoD.LobbyMenus
  if
    type(menus) ~= "table"
    or type(menus.AddButtonsForTarget) ~= "function"
    or buttonWrapperInstalled(menus, menus.BoiiiCoopSettingsWrapper)
  then
    return true
  end
  local original = menus.AddButtonsForTarget
  local function pack(...)
    return { n = select("#", ...), ... }
  end
  local function addButtons(controller, id, ...)
    local results = pack(original(controller, id, ...))
    if type(results[1]) == "table" and eligible(id) then
      local found = false
      for _, button in ipairs(results[1]) do
        if button.customId == "btnCoopLobbySettings" then
          found = true
        end
      end
      if not found then
        table.insert(results[1], {
          optionDisplay = "ZOMBIES LOBBY SETTINGS",
          customId = "btnCoopLobbySettings",
          isLargeButton = false,
          isLastButtonInGroup = false,
          disabled = false,
          action = function(_, _, selectedController, _, menu)
            if eligible(Engine.GetLobbyUIScreen()) then
              OpenPopup(menu, "BoiiiZombiesLobbySettingsMenu", selectedController)
            end
          end,
        })
      end
    end
    return unpack(results, 1, results.n)
  end
  recordButtonWrapper(menus, addButtons, original)
  menus.BoiiiCoopSettingsWrapper = addButtons
  menus.AddButtonsForTarget = addButtons
  return true
end

return settings
