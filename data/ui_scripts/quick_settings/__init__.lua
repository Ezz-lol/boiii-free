if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

DataSources.BoiiiQuickSettings = DataSourceHelpers.ListSetup("BoiiiQuickSettings", function(controller)
  local optionsTable = {}

  local updateDvar = function(f1_arg0, f1_arg1, f1_arg2, dvarName, f1_arg4)
    local oldValue = Engine.DvarInt(nil, dvarName)
    local newValue = f1_arg1.value
    UpdateInfoModels(f1_arg1)
    if oldValue == newValue then
      return
    end
    Engine.SetDvar(dvarName, f1_arg1.value)
  end

  -- FOV (65-120 in steps of 5)
  local fovOptions = {}
  local currentFov = 80
  pcall(function()
    currentFov = Engine.DvarInt(nil, "cg_fov_default")
  end)
  for fov = 65, 120, 5 do
    table.insert(fovOptions, {
      option = fov == 80 and "80 (Default)" or tostring(fov),
      value = fov,
      default = fov == currentFov,
    })
  end
  table.insert(
    optionsTable,
    CoD.OptionsUtility.CreateDvarSettings(
      controller,
      "Field of View",
      "Adjust your field of view (65-120).",
      "QuickSettings_fov",
      "cg_fov_default",
      fovOptions,
      nil,
      updateDvar
    )
  )

  -- FOV Scale (zoom FOV multiplier)
  -- NOTE: CreateDvarSettings uses DvarInt internally to match selection.
  -- DvarInt("cg_fovScale") returns 1 for 1.0, so only 1.0 auto-selects.
  -- We default to 1.0 and use float values directly.
  local fovScaleOptions = {}
  local fovScaleValues = { 0.5, 0.6, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0, 1.05, 1.1, 1.15, 1.2, 1.5, 2.0 }
  for _, scale in ipairs(fovScaleValues) do
    table.insert(fovScaleOptions, {
      option = scale == 1.0 and "1.0 (Default)" or tostring(scale),
      value = scale,
      default = scale == 1.0,
    })
  end
  table.insert(
    optionsTable,
    CoD.OptionsUtility.CreateDvarSettings(
      controller,
      "FOV Scale",
      "Multiplier applied to ADS and zoom FOV. 1.0 = default.",
      "QuickSettings_fovscale",
      "cg_fovScale",
      fovScaleOptions,
      nil,
      function(f1_arg0, f1_arg1, f1_arg2, dvarName, f1_arg4)
        UpdateInfoModels(f1_arg1)
        Engine.SetDvar(dvarName, f1_arg1.value)
      end
    )
  )

  -- Max FPS
  local fpsOptions = {}
  local currentFps = 0
  pcall(function()
    currentFps = Engine.DvarInt(nil, "com_maxfps")
  end)
  local fpsValues = { 0, 30, 60, 90, 120, 144, 165, 240, 300, 500 }
  for _, fps in ipairs(fpsValues) do
    table.insert(fpsOptions, {
      option = fps == 0 and "Unlimited" or tostring(fps),
      value = fps,
      default = fps == currentFps,
    })
  end
  table.insert(
    optionsTable,
    CoD.OptionsUtility.CreateDvarSettings(
      controller,
      "Max FPS",
      "Cap your maximum framerate. 0 = unlimited.",
      "QuickSettings_maxfps",
      "com_maxfps",
      fpsOptions,
      nil,
      updateDvar
    )
  )

  -- VSync toggle
  table.insert(
    optionsTable,
    CoD.OptionsUtility.CreateDvarSettings(
      controller,
      "VSync",
      "Sync framerate to monitor refresh rate.",
      "QuickSettings_vsync",
      "r_vsync",
      {
        {
          option = "MENU_DISABLED",
          value = 0,
          default = true,
        },
        {
          option = "MENU_ENABLED",
          value = 1,
        },
      },
      nil,
      updateDvar
    )
  )

  -- Draw FPS counter toggle
  table.insert(
    optionsTable,
    CoD.OptionsUtility.CreateDvarSettings(
      controller,
      "Show FPS Counter",
      "Display frames per second on screen.",
      "QuickSettings_drawfps",
      "com_drawFPS_PC",
      {
        {
          option = "MENU_DISABLED",
          value = 0,
          default = true,
        },
        {
          option = "MENU_ENABLED",
          value = 1,
        },
      },
      nil,
      updateDvar
    )
  )

  -- ── Game Settings ──

  -- Bot difficulty
  table.insert(
    optionsTable,
    CoD.OptionsUtility.CreateDvarSettings(
      controller,
      "Bot Difficulty",
      "Set AI bot difficulty level.",
      "QuickSettings_botdifficulty",
      "bot_difficulty",
      {
        {
          option = "Recruit",
          value = 0,
          default = true,
        },
        {
          option = "Regular",
          value = 1,
        },
        {
          option = "Hardened",
          value = 2,
        },
        {
          option = "Veteran",
          value = 3,
        },
      },
      nil,
      updateDvar
    )
  )

  -- Max clients for custom games (MP only)
  if Engine.CurrentSessionMode() == Enum.eModes.MODE_MULTIPLAYER then
    local maxClientOptions = {}
    local currentMax = 18
    pcall(function()
      currentMax = Engine.DvarInt(nil, "com_maxclients")
    end)
    local clientValues = { 1, 2, 4, 6, 8, 10, 12, 14, 16, 18 }
    for _, val in ipairs(clientValues) do
      table.insert(maxClientOptions, {
        option = tostring(val),
        value = val,
        default = val == currentMax,
      })
    end
    table.insert(
      optionsTable,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Max Players",
        "Maximum number of players in custom games.",
        "QuickSettings_maxclients",
        "com_maxclients",
        maxClientOptions,
        nil,
        updateDvar
      )
    )
  end

  -- Min players to start (solo play toggle)
  table.insert(
    optionsTable,
    CoD.OptionsUtility.CreateDvarSettings(
      controller,
      "Min Players to Start",
      "Minimum players required to start a match.",
      "QuickSettings_minplayers",
      "party_minplayers",
      {
        {
          option = "1 (Solo)",
          value = 1,
          default = true,
        },
        {
          option = "2",
          value = 2,
        },
        {
          option = "4",
          value = 4,
        },
        {
          option = "6",
          value = 6,
        },
      },
      nil,
      updateDvar
    )
  )

  -- Game speed (timescale)
  table.insert(
    optionsTable,
    CoD.OptionsUtility.CreateDvarSettings(
      controller,
      "Game Speed",
      "Adjust game speed. 1 = normal.",
      "QuickSettings_timescale",
      "timescale",
      {
        {
          option = "0.25x (Quarter)",
          value = 0.25,
        },
        {
          option = "0.5x (Half)",
          value = 0.5,
        },
        {
          option = "1x (Normal)",
          value = 1,
          default = true,
        },
        {
          option = "1.5x (Fast)",
          value = 1.5,
        },
        {
          option = "2x (Double)",
          value = 2,
        },
        {
          option = "4x (Quad)",
          value = 4,
        },
      },
      nil,
      function(f1_arg0, f1_arg1, f1_arg2, dvarName, f1_arg4)
        UpdateInfoModels(f1_arg1)
        Engine.SetDvar(dvarName, f1_arg1.value)
      end
    )
  )

  return optionsTable
end)

-- ── Quick Settings Menu ─────────────────────────────────────────────────────

LUI.createMenu.BoiiiQuickSettingsMenu = function(controller)
  local self = CoD.Menu.NewForUIEditor("BoiiiQuickSettingsMenu")
  if PreLoadFunc then
    PreLoadFunc(self, controller)
  end
  self.soundSet = "ChooseDecal"
  self:setOwner(controller)
  self:setLeftRight(true, true, 0, 0)
  self:setTopBottom(true, true, 0, 0)
  self:playSound("menu_open", controller)
  self.buttonModel =
    Engine.CreateModel(Engine.GetModelForController(controller), "BoiiiQuickSettingsMenu.buttonPrompts")
  self.anyChildUsesUpdateState = true

  local GameSettingsBackground = CoD.GameSettings_Background.new(self, controller)
  GameSettingsBackground:setLeftRight(true, true, 0, 0)
  GameSettingsBackground:setTopBottom(true, true, 0, 0)
  GameSettingsBackground.MenuFrame.titleLabel:setText(Engine.Localize("QUICK SETTINGS"))
  GameSettingsBackground.MenuFrame.cac3dTitleIntermediary0.FE3dTitleContainer0.MenuTitle.TextBox1.Label0:setText(
    Engine.Localize("QUICK SETTINGS")
  )
  GameSettingsBackground.GameSettingsSelectedItemInfo.GameModeInfo:setAlpha(0)
  GameSettingsBackground.GameSettingsSelectedItemInfo.GameModeName:setAlpha(0)
  self:addElement(GameSettingsBackground)
  self.GameSettingsBackground = GameSettingsBackground

  local Options = CoD.Competitive_SettingsList.new(self, controller)
  Options:setLeftRight(true, false, 26, 741)
  Options:setTopBottom(true, false, 135, 720)
  Options.Title.DescTitle:setText(Engine.Localize("Frequently Used Settings"))
  Options.ButtonList:setVerticalCount(14)
  Options.ButtonList:setDataSource("BoiiiQuickSettings")
  self:addElement(Options)
  self.Options = Options

  self:AddButtonCallbackFunction(
    self,
    controller,
    Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE,
    nil,
    function(element, menu, controller, model)
      GoBack(self, controller)
      SetPerControllerTableProperty(controller, "disableGameSettingsOptions", nil)
      return true
    end,
    function(element, menu, controller)
      CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE, "MENU_BACK")
      return true
    end,
    false
  )

  GameSettingsBackground.MenuFrame:setModel(self.buttonModel, controller)
  Options.id = "Options"

  self:processEvent({ name = "menu_loaded", controller = controller })
  self:processEvent({ name = "update_state", menu = self })
  if not self:restoreState() then
    self.Options:processEvent({ name = "gain_focus", controller = controller })
  end

  LUI.OverrideFunction_CallOriginalSecond(self, "close", function(element)
    element.GameSettingsBackground:close()
    element.Options:close()
    Engine.UnsubscribeAndFreeModel(
      Engine.GetModel(Engine.GetModelForController(controller), "BoiiiQuickSettingsMenu.buttonPrompts")
    )
  end)

  if PostLoadFunc then
    PostLoadFunc(self, controller)
  end

  return self
end
