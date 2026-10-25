if Engine.GetCurrentMap() ~= "core_frontend" then
  return
end

require("ui.uieditor.widgets.Lobby.Common.FE_TabBar")
require("ui.uieditor.widgets.Scrollbars.verticalScrollbar")

local function updateDvar(controller, optionModel, optionsList, dvarName, extraParam)
  local oldValue = Engine.DvarInt(nil, dvarName)
  local newValue = optionModel.value
  UpdateInfoModels(optionModel)
  if oldValue == newValue then
    return
  end
  Engine.SetDvar(dvarName, optionModel.value)
end

local function toggle(controller, dvar, name, description)
  return CoD.OptionsUtility.CreateDvarSettings(controller, name, description, "QuickSettings_" .. dvar, dvar, {
    { option = "MENU_DISABLED", value = 0, default = true },
    { option = "MENU_ENABLED", value = 1 },
  }, nil, updateDvar)
end

DataSources.BoiiiQuickSettings = DataSourceHelpers.ListSetup("BoiiiQuickSettings", function(controller)
  return {
    toggle(controller, "boiii_skipIntro", "Skip Intro", "Skip the intro video on startup."),
    toggle(
      controller,
      "boiii_skipForestCinematic",
      "Skip Forest Video",
      "Skip only the forest video before joining a Zombies game. Map videos still play."
    ),
    toggle(
      controller,
      "boiii_skipCinematics",
      "Skip All Cinematics",
      "Skip every cinematic and video, including map videos."
    ),
    toggle(
      controller,
      "boiii_ultrawide",
      "Ultrawide Support",
      "Use the full width of ultrawide screens. Requires a restart."
    ),
    toggle(
      controller,
      "boiii_allowCheats",
      "Allow Cheats",
      "Allow cheat commands like noclip and god mode. Only this setting can turn cheats on, maps and scripts can't."
    ),
  }
end)

DataSources.BoiiiQuickSettingsVideo = DataSourceHelpers.ListSetup("BoiiiQuickSettingsVideo", function(controller)
  local optionsTable = {}

  -- A value set outside this menu isn't one of the fixed steps. List it too,
  -- or the menu falls back to the first option and overwrites the dvar.
  local function addCurrentValue(values, current)
    if type(current) ~= "number" then
      return false
    end
    for _, value in ipairs(values) do
      if value == current then
        return false
      end
    end
    table.insert(values, current)
    table.sort(values)
    return true
  end

  -- FOV (65-120 in steps of 5)
  local fovOptions = {}
  local currentFov = 80
  pcall(function()
    currentFov = Engine.DvarInt(nil, "cg_fov_default")
  end)
  local fovValues = {}
  for fov = 65, 120, 5 do
    table.insert(fovValues, fov)
  end
  local fovCustom = addCurrentValue(fovValues, currentFov)
  for _, fov in ipairs(fovValues) do
    local label = tostring(fov)
    if fov == 80 then
      label = "80 (Default)"
    elseif fovCustom and fov == currentFov then
      label = fov .. " (Custom)"
    end
    table.insert(fovOptions, {
      option = label,
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
      function(controller, optionModel, optionsList, dvarName, extraParam)
        UpdateInfoModels(optionModel)
        Engine.SetDvar(dvarName, optionModel.value)
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
  local fpsCustom = addCurrentValue(fpsValues, currentFps)
  for _, fps in ipairs(fpsValues) do
    local label = tostring(fps)
    if fps == 0 then
      label = "Unlimited"
    elseif fpsCustom and fps == currentFps then
      label = fps .. " (Custom)"
    end
    table.insert(fpsOptions, {
      option = label,
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

  table.insert(optionsTable, toggle(controller, "r_vsync", "VSync", "Sync framerate to monitor refresh rate."))
  table.insert(
    optionsTable,
    toggle(controller, "com_drawFPS_PC", "Show FPS Counter", "Display frames per second on screen.")
  )


  return optionsTable
end)

DataSources.BoiiiQuickSettingsGame = DataSourceHelpers.ListSetup("BoiiiQuickSettingsGame", function(controller)
  local optionsTable = {}

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
      function(controller, optionModel, optionsList, dvarName, extraParam)
        UpdateInfoModels(optionModel)
        Engine.SetDvar(dvarName, optionModel.value)
      end
    )
  )

  return optionsTable
end)

DataSources.BoiiiQuickSettingsConsole = DataSourceHelpers.ListSetup("BoiiiQuickSettingsConsole", function(controller)
  return {
    toggle(controller, "boiii_fullLogs", "Full Console Logs", "Show every log message in the console."),
    toggle(
      controller,
      "boiii_logScriptErrors",
      "Log Script Errors",
      "Log every script error, including non-fatal ones."
    ),
  }
end)

DataSources.BoiiiQuickSettingsXpBar = DataSourceHelpers.ListSetup("BoiiiQuickSettingsXpBar", function(controller)
  local function setting(name, description, dvar, options)
    return CoD.OptionsUtility.CreateDvarSettings(
      controller,
      name,
      description,
      "QuickSettings_" .. dvar,
      dvar,
      options,
      nil,
      updateDvar
    )
  end

  return {
    setting("XP Bar", "Show an XP progress bar in Multiplayer and Zombies.", "boiii_xpBar", {
      { option = "MENU_DISABLED", value = 0, default = true },
      { option = "MENU_ENABLED", value = 1 },
    }),
    setting("Position", "Show the XP bar at the bottom or the top of the screen.", "boiii_xpBarPosition", {
      { option = "Bottom", value = 0, default = true },
      { option = "Top", value = 1 },
    }),
    setting("XP Popups", "Show the XP you earn next to the XP bar.", "boiii_xpBarPopups", {
      { option = "MENU_DISABLED", value = 0 },
      { option = "MENU_ENABLED", value = 1, default = true },
    }),
    -- Same order as COLORS in the xp_bar script
    setting("Color", "Color of the XP bar and its popups.", "boiii_xpBarColor", {
      { option = "Orange", value = 0, default = true },
      { option = "Gold", value = 1 },
      { option = "Blue", value = 2 },
      { option = "Green", value = 3 },
      { option = "Red", value = 4 },
      { option = "Purple", value = 5 },
      { option = "White", value = 6 },
    }),
  }
end)

DataSources.BoiiiQuickSettingsTabs = DataSourceHelpers.ListSetup("BoiiiQuickSettingsTabs", function(controller)
  local function tab(name, dataSourceName, title)
    return {
      models = { tabName = name, tabIcon = "" },
      properties = { tabId = dataSourceName, dataSourceName = dataSourceName, title = title },
    }
  end

  return {
    { models = { tabIcon = CoD.buttonStrings.shoulderl }, properties = { m_mouseDisabled = true } },
    tab("GENERAL", "BoiiiQuickSettings", "General"),
    tab("VIDEO", "BoiiiQuickSettingsVideo", "Video"),
    tab("GAME", "BoiiiQuickSettingsGame", "Game"),
    tab("XP BAR", "BoiiiQuickSettingsXpBar", "XP Bar"),
    tab("CONSOLE", "BoiiiQuickSettingsConsole", "Console"),
    { models = { tabIcon = CoD.buttonStrings.shoulderr }, properties = { m_mouseDisabled = true } },
  }
end, true)

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
  local menuTitle =
    GameSettingsBackground.MenuFrame.cac3dTitleIntermediary0.FE3dTitleContainer0.MenuTitle.TextBox1.Label0
  GameSettingsBackground.MenuFrame.titleLabel:setText(Engine.Localize("BOIII SETTINGS"))
  GameSettingsBackground.MenuFrame.titleLabel:setRGB(1, 0.55, 0.1)
  menuTitle:setText(Engine.Localize("BOIII SETTINGS"))
  menuTitle:setRGB(1, 0.55, 0.1)
  GameSettingsBackground.GameSettingsSelectedItemInfo.GameModeInfo:setAlpha(0)
  GameSettingsBackground.GameSettingsSelectedItemInfo.GameModeName:setAlpha(0)
  self:addElement(GameSettingsBackground)
  self.GameSettingsBackground = GameSettingsBackground

  local Options = CoD.Competitive_SettingsList.new(self, controller)
  Options:setLeftRight(true, false, 26, 741)
  Options:setTopBottom(true, false, 135, 720)
  Options.Title.DescTitle:setText(Engine.Localize("General"))
  Options.Title.DescTitle:setRGB(1, 0.55, 0.1)
  Options.ButtonList:setVerticalCount(11)
  Options.ButtonList:setDataSource("BoiiiQuickSettings")
  Options.ButtonList:setVerticalScrollbar(CoD.verticalScrollbar)
  Options.ButtonList.verticalScrollbar:setLeftRight(true, false, 549, 559)
  self:addElement(Options)
  self.Options = Options

  local TabList = CoD.FE_TabBar.new(self, controller)
  TabList:setLeftRight(true, false, 0, 2496)
  TabList:setTopBottom(true, false, 84, 123)
  TabList.Tabs.grid:setHorizontalCount(7)
  TabList.Tabs.grid:setDataSource("BoiiiQuickSettingsTabs")
  self:addElement(TabList)
  self.TabList = TabList

  self:registerEventHandler("list_active_changed", function(element, event)
    if element.dataSourceName and element.title then
      Options.Title.DescTitle:setText(Engine.Localize(element.title))
      Options.ButtonList:setDataSource(element.dataSourceName)
    end
  end)

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
    element.TabList:close()
    Engine.UnsubscribeAndFreeModel(
      Engine.GetModel(Engine.GetModelForController(controller), "BoiiiQuickSettingsMenu.buttonPrompts")
    )
  end)

  if PostLoadFunc then
    PostLoadFunc(self, controller)
  end

  return self
end
