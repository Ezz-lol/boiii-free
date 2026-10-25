if Engine.GetCurrentMap() == "core_frontend" then
  return
end

local isPrivateGame = true
pcall(function()
  if Dvar.cl_connected_to_dedi and Dvar.cl_connected_to_dedi:get() then
    isPrivateGame = false
  end
end)

if isPrivateGame then
  local tweakNames = {
    g_speed = "Movement Speed",
    bg_gravity = "Gravity",
    player_sustainAmmo = "Infinite Ammo",
    timescale = "Game Speed",
    cg_drawGun = "Weapon Model",
    r_fog = "Fog",
    cg_fov_default = "Field of View",
    cg_fovScale = "FOV Scale",
  }

  local function showTweakToast(dvarName, value)
    pcall(function()
      if CoD.OverlayUtility and CoD.OverlayUtility.ShowToast then
        local label = tweakNames[dvarName] or dvarName
        CoD.OverlayUtility.ShowToast(
          "DefaultState",
          "GAME TWEAKS",
          label .. ": " .. tostring(value),
          "t7_icon_info_overlays_bkg"
        )
      end
    end)
  end

  local lastTweakValues = {}
  for dvarName, _ in pairs(tweakNames) do
    pcall(function()
      lastTweakValues[dvarName] = tostring(Engine.DvarString(nil, dvarName))
    end)
  end

  local function showTweakToastIfChanged(dvarName, value)
    local normalizedValue = tostring(value)
    local previousValue = lastTweakValues[dvarName]
    lastTweakValues[dvarName] = normalizedValue
    if previousValue ~= nil and previousValue ~= normalizedValue then
      showTweakToast(dvarName, value)
    end
  end

  local function updateDvar(controller, optionModel, optionsList, dvarName, extraParam)
    UpdateInfoModels(optionModel)
    local val = optionModel.value
    pcall(function()
      Engine.Exec(optionsList, dvarName .. " " .. tostring(val))
    end)
    showTweakToastIfChanged(dvarName, val)
  end

  local tweaksMenu = nil
  Engine.SetDvar("boiii_tweakGod", 0)
  Engine.SetDvar("boiii_tweakNoclip", 0)

  local function toggleCheat(command)
    return function(controller, optionModel, optionsList, dvarName, extraParam)
      UpdateInfoModels(optionModel)
      if Engine.DvarInt(nil, dvarName) ~= optionModel.value then
        Engine.SetDvar(dvarName, optionModel.value)
        Engine.Exec(optionsList, command)
      end
    end
  end

  local function onOffOptions(dvarName)
    local enabled = Engine.DvarInt(nil, dvarName) == 1
    return {
      { option = "MENU_DISABLED", value = 0, default = not enabled },
      { option = "MENU_ENABLED", value = 1, default = enabled },
    }
  end

  DataSources.BoiiiGameTweaks = DataSourceHelpers.ListSetup("BoiiiGameTweaks", function(controller)
    local t = {}

    table.insert(
      t,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Allow Cheats",
        "Allow cheat commands like noclip and god mode.",
        "GameTweaks_allowcheats",
        "boiii_allowCheats",
        onOffOptions("boiii_allowCheats"),
        nil,
        function(controller, optionModel, optionsList, dvarName, extraParam)
          UpdateInfoModels(optionModel)
          if Engine.DvarInt(nil, dvarName) ~= optionModel.value then
            Engine.SetDvar(dvarName, optionModel.value)
            if tweaksMenu then
              tweaksMenu.Options:updateDataSource()
            end
          end
        end
      )
    )

    if Engine.DvarBool(nil, "boiii_allowCheats") then
      table.insert(
        t,
        CoD.OptionsUtility.CreateDvarSettings(
          controller,
          "God Mode",
          "Take no damage.",
          "GameTweaks_god",
          "boiii_tweakGod",
          onOffOptions("boiii_tweakGod"),
          nil,
          toggleCheat("god")
        )
      )
      table.insert(
        t,
        CoD.OptionsUtility.CreateDvarSettings(
          controller,
          "Noclip",
          "Fly through walls.",
          "GameTweaks_noclip",
          "boiii_tweakNoclip",
          onOffOptions("boiii_tweakNoclip"),
          nil,
          toggleCheat("noclip")
        )
      )
    end

    local speedOptions = {}
    local currentSpeed = 190
    pcall(function()
      currentSpeed = Engine.DvarInt(nil, "g_speed")
    end)
    for _, val in ipairs({ 100, 150, 190, 250, 300, 400, 500, 800 }) do
      table.insert(speedOptions, {
        option = val == 190 and "190 (Default)" or tostring(val),
        value = val,
        default = val == currentSpeed,
      })
    end
    table.insert(
      t,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Movement Speed",
        "Player movement speed.",
        "GameTweaks_gspeed",
        "g_speed",
        speedOptions,
        nil,
        updateDvar
      )
    )

    local gravityOptions = {}
    local currentGravity = 800
    pcall(function()
      currentGravity = Engine.DvarInt(nil, "bg_gravity")
    end)
    for _, val in ipairs({ 50, 100, 200, 400, 800, 1200, 1600, 2000 }) do
      table.insert(gravityOptions, {
        option = val == 800 and "800 (Default)" or val <= 100 and val .. " (Moon)" or tostring(val),
        value = val,
        default = val == currentGravity,
      })
    end
    table.insert(
      t,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Gravity",
        "Lower = floaty, higher = heavy.",
        "GameTweaks_gravity",
        "bg_gravity",
        gravityOptions,
        nil,
        updateDvar
      )
    )

    table.insert(
      t,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Infinite Ammo",
        "Never run out of ammo.",
        "GameTweaks_infiniteammo",
        "player_sustainAmmo",
        {
          { option = "MENU_DISABLED", value = 0, default = true },
          { option = "MENU_ENABLED", value = 1 },
        },
        nil,
        updateDvar
      )
    )

    table.insert(
      t,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Game Speed",
        "Speed up or slow down the game.",
        "GameTweaks_timescale",
        "timescale",
        {
          { option = "0.25x (Slow Mo)", value = 0.25 },
          { option = "0.5x (Half Speed)", value = 0.5 },
          { option = "1x (Normal)", value = 1, default = true },
          { option = "1.5x (Fast)", value = 1.5 },
          { option = "2x (Double)", value = 2 },
          { option = "4x (Chaos)", value = 4 },
        },
        nil,
        updateDvar
      )
    )

    table.insert(
      t,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Hide Weapon Model",
        "Hide the gun model on screen.",
        "GameTweaks_drawgun",
        "cg_drawGun",
        {
          { option = "Show (Default)", value = 1, default = true },
          { option = "Hide", value = 0 },
        },
        nil,
        updateDvar
      )
    )

    table.insert(
      t,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Disable Fog",
        "Remove fog effects.",
        "GameTweaks_fog",
        "r_fog",
        {
          { option = "Fog On (Default)", value = 1, default = true },
          { option = "Fog Off", value = 0 },
        },
        nil,
        updateDvar
      )
    )

    -- FOV
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
      t,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "Field of View",
        "Adjust your field of view (65-120).",
        "GameTweaks_fov",
        "cg_fov_default",
        fovOptions,
        nil,
        updateDvar
      )
    )

    -- FOV Scale
    local fovScaleOptions = {}
    for _, s in ipairs({ 0.5, 0.6, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0, 1.05, 1.1, 1.15, 1.2, 1.5, 2.0 }) do
      table.insert(fovScaleOptions, {
        option = s == 1.0 and "1.0 (Default)" or tostring(s),
        value = s,
        default = s == 1.0,
      })
    end
    table.insert(
      t,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        "FOV Scale",
        "Multiplier for ADS/zoom FOV. 1.0 = default.",
        "GameTweaks_fovscale",
        "cg_fovScale",
        fovScaleOptions,
        nil,
        function(controller, optionModel, optionsList, dvarName, extraParam)
          UpdateInfoModels(optionModel)
          Engine.SetDvar(dvarName, optionModel.value)
          showTweakToastIfChanged(dvarName, optionModel.value)
        end
      )
    )

    return t
  end)

  require("ui.uieditor.widgets.Scrollbars.verticalScrollbar")
  require("ui.uieditor.widgets.StartMenu.StartMenu_Options_Slider")
  require("ui.uieditor.widgets.BackgroundFrames.GenericMenuFrame")

  LUI.createMenu.BoiiiGameTweaksMenu = function(controller)
    local self = CoD.Menu.NewForUIEditor("BoiiiGameTweaksMenu")
    self.soundSet = "ChooseDecal"
    self:setOwner(controller)
    self:setLeftRight(true, true, 0, 0)
    self:setTopBottom(true, true, 0, 0)
    self:playSound("menu_open", controller)
    self.buttonModel = Engine.CreateModel(Engine.GetModelForController(controller), "BoiiiGameTweaksMenu.buttonPrompts")
    self.anyChildUsesUpdateState = true

    local background = LUI.UIImage.new()
    background:setLeftRight(true, true, 0, 0)
    background:setTopBottom(true, true, 0, 0)
    background:setRGB(0.06, 0.06, 0.06)
    self:addElement(background)

    local backgroundImage = LUI.UIImage.new()
    backgroundImage:setLeftRight(true, true, 0, 0)
    backgroundImage:setTopBottom(true, true, 0, 0)
    backgroundImage:setImage(RegisterImage("uie_fe_cp_background"))
    self:addElement(backgroundImage)

    local orange = { 1, 0.55, 0.1 }
    local function text(left, right, top, bottom, color)
      local element = LUI.UIText.new()
      element:setLeftRight(true, false, left, right)
      element:setTopBottom(true, false, top, bottom)
      element:setTTF("fonts/RefrigeratorDeluxe-Regular.ttf")
      element:setAlignment(Enum.LUIAlignment.LUI_ALIGNMENT_LEFT)
      if color then
        element:setRGB(color[1], color[2], color[3])
      end
      self:addElement(element)
      return element
    end

    local panel = LUI.UIImage.new()
    panel:setLeftRight(true, false, 600, 1110)
    panel:setTopBottom(true, false, 166, 252)
    panel:setRGB(0, 0, 0)
    panel:setAlpha(0.45)
    self:addElement(panel)

    local sectionTitle = text(64, 560, 132, 158, orange)
    sectionTitle:setText(Engine.Localize("Private Match Tweaks"))
    local highlight = text(616, 1094, 178, 204, orange)
    local desc = text(616, 1094, 212, 232)

    local list = LUI.UIList.new(self, controller, 2, 0, nil, false, false, 0, 0, false, true)
    list:makeFocusable()
    list:setLeftRight(true, false, -30, 640)
    list:setTopBottom(true, false, 90, 658)
    list:setScale(0.72)
    list:setWidgetType(CoD.StartMenu_Options_Slider)
    list:setVerticalCount(10)
    list:setDataSource("BoiiiGameTweaks")
    list:setVerticalScrollbar(CoD.verticalScrollbar)
    list.verticalScrollbar:setLeftRight(true, false, 551, 561)
    self:addElement(list)
    self.Options = list

    local frame = CoD.GenericMenuFrame.new(self, controller)
    frame:setLeftRight(true, true, 0, 0)
    frame:setTopBottom(true, true, 0, 0)
    frame.titleLabel:setText(Engine.Localize("GAME TWEAKS"))
    frame.titleLabel:setRGB(1, 0.55, 0.1)
    frame.cac3dTitleIntermediary0.FE3dTitleContainer0.MenuTitle.TextBox1.Label0:setText(Engine.Localize("GAME TWEAKS"))
    frame.cac3dTitleIntermediary0.FE3dTitleContainer0.MenuTitle.TextBox1.Label0:setRGB(1, 0.55, 0.1)
    self:addElement(frame)
    self.MenuFrame = frame

    desc:linkToElementModel(list, "desc", true, function(model)
      local value = Engine.GetModelValue(model)
      if value then
        desc:setText(Engine.Localize(value))
      end
    end)
    highlight:linkToElementModel(list, "name", true, function(model)
      local value = Engine.GetModelValue(model)
      if value then
        highlight:setText(Engine.Localize(value))
      end
    end)

    self:AddButtonCallbackFunction(
      self,
      controller,
      Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE,
      nil,
      function(element, menu, controller, model)
        GoBack(self, controller)
        return true
      end,
      function(element, menu, controller)
        CoD.Menu.SetButtonLabel(menu, Enum.LUIButton.LUI_KEY_XBB_PSCIRCLE, "MENU_BACK")
        return true
      end,
      false
    )

    frame:setModel(self.buttonModel, controller)
    list.id = "Options"
    tweaksMenu = self

    self:processEvent({ name = "menu_loaded", controller = controller })
    self:processEvent({ name = "update_state", menu = self })
    if not self:restoreState() then
      list:processEvent({ name = "gain_focus", controller = controller })
    end

    LUI.OverrideFunction_CallOriginalSecond(self, "close", function(element)
      tweaksMenu = nil
      element.MenuFrame:close()
      element.Options:close()
      Engine.UnsubscribeAndFreeModel(
        Engine.GetModel(Engine.GetModelForController(controller), "BoiiiGameTweaksMenu.buttonPrompts")
      )
    end)

    return self
  end
end
