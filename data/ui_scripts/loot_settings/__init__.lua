if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

local function dvarInt(name, fallback)
  local value = fallback
  pcall(function()
    value = Engine.DvarInt(nil, name)
  end)
  return value
end

local function choices(dvarName, fallback, list)
  local current = dvarInt(dvarName, fallback)
  local options = {}
  for _, entry in ipairs(list) do
    table.insert(options, {
      option = entry[1],
      value = entry[2],
      default = entry[2] == current,
    })
  end
  return options
end

local function setDvar(controller, optionModel, optionsList, dvarName, extraParam)
  UpdateInfoModels(optionModel)
  Engine.SetDvar(dvarName, optionModel.value)
end

DataSources.BoiiiLootSettings = DataSourceHelpers.ListSetup("BoiiiLootSettings", function(controller)
  local optionsTable = {}

  local function add(title, description, id, dvarName, options, update)
    table.insert(
      optionsTable,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        title,
        description,
        "BoiiiLootSettings_" .. id,
        dvarName,
        options,
        nil,
        update or setDvar
      )
    )
  end

  local onOff = { { "MENU_DISABLED", 0 }, { "MENU_ENABLED", 1 } }

  add(
    "Server Rewards",
    "Let servers you play on give you Cryptokeys and items, e.g. end-of-match rewards. Disabled = ignore them all.",
    "server_rewards",
    "cg_loot_server_rewards",
    choices("cg_loot_server_rewards", 1, onOff)
  )

  add(
    "Server Keys per Match",
    "The most Cryptokeys a server can give you in one match. Anything above this is ignored.",
    "server_max_keys",
    "cg_loot_server_max_keys",
    choices("cg_loot_server_max_keys", 100, {
      { "10", 10 },
      { "25", 25 },
      { "50", 50 },
      { "100 (Default)", 100 },
      { "250", 250 },
      { "1000", 1000 },
    })
  )

  add(
    "Server Items per Match",
    "The most Black Market items a server can give you in one match. Anything above this is ignored.",
    "server_max_items",
    "cg_loot_server_max_items",
    choices("cg_loot_server_max_items", 10, {
      { "0", 0 },
      { "1", 1 },
      { "3", 3 },
      { "5", 5 },
      { "10 (Default)", 10 },
      { "25", 25 },
    })
  )

  add(
    "Replace Black Market Button",
    "Show this Black Market in place of the original (non-working) Black Market button. Disabled = show both.",
    "hide_blackmarket",
    "cg_loot_hide_blackmarket",
    choices("cg_loot_hide_blackmarket", 1, onOff)
  )

  return optionsTable
end)

LUI.createMenu.BoiiiLootSettingsMenu = function(controller)
  local self = CoD.Menu.NewForUIEditor("BoiiiLootSettingsMenu")
  if PreLoadFunc then
    PreLoadFunc(self, controller)
  end
  self.soundSet = "ChooseDecal"
  self:setOwner(controller)
  self:setLeftRight(true, true, 0, 0)
  self:setTopBottom(true, true, 0, 0)
  self:playSound("menu_open", controller)
  self.buttonModel = Engine.CreateModel(Engine.GetModelForController(controller), "BoiiiLootSettingsMenu.buttonPrompts")
  self.anyChildUsesUpdateState = true

  local background = CoD.GameSettings_Background.new(self, controller)
  background:setLeftRight(true, true, 0, 0)
  background:setTopBottom(true, true, 0, 0)
  background.MenuFrame.titleLabel:setText(Engine.Localize("BLACK MARKET SETTINGS"))
  background.MenuFrame.cac3dTitleIntermediary0.FE3dTitleContainer0.MenuTitle.TextBox1.Label0:setText(
    Engine.Localize("BLACK MARKET SETTINGS")
  )
  background.GameSettingsSelectedItemInfo.GameModeInfo:setAlpha(0)
  background.GameSettingsSelectedItemInfo.GameModeName:setAlpha(0)
  self:addElement(background)
  self.GameSettingsBackground = background

  local options = CoD.Competitive_SettingsList.new(self, controller)
  options:setLeftRight(true, false, 26, 741)
  options:setTopBottom(true, false, 135, 720)
  options.Title.DescTitle:setText(Engine.Localize("Black Market Drops"))
  options.ButtonList:setVerticalCount(10)
  options.ButtonList:setDataSource("BoiiiLootSettings")
  self:addElement(options)
  self.Options = options

  self:AddButtonCallbackFunction(
    self,
    controller,
    Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE,
    nil,
    function(element, menu, actionController, model)
      GoBack(self, actionController)
      return true
    end,
    function(element, menu, actionController)
      CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE, "MENU_BACK")
      return true
    end,
    false
  )

  background.MenuFrame:setModel(self.buttonModel, controller)
  options.id = "Options"

  self:processEvent({ name = "menu_loaded", controller = controller })
  self:processEvent({ name = "update_state", menu = self })
  if not self:restoreState() then
    self.Options:processEvent({ name = "gain_focus", controller = controller })
  end

  LUI.OverrideFunction_CallOriginalSecond(self, "close", function(element)
    element.GameSettingsBackground:close()
    element.Options:close()
    Engine.UnsubscribeAndFreeModel(
      Engine.GetModel(Engine.GetModelForController(controller), "BoiiiLootSettingsMenu.buttonPrompts")
    )
  end)

  if PostLoadFunc then
    PostLoadFunc(self, controller)
  end

  return self
end
