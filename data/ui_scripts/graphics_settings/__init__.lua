if Engine.IsUsingMods() and Engine.UsingModsUgcName() ~= "" and Engine.UsingModsUgcName() ~= "usermaps" then
  return
end

DataSources.OptionPCGraphicsCard = DataSourceHelpers.ListSetup("PC.OptionPCGraphicsCard", function()
  return {
    { models = { value = 0, valueDisplay = "Windows Default" } },
    { models = { value = 1, valueDisplay = "Power Saving" } },
    { models = { value = 2, valueDisplay = "High Performance" } },
  }
end, true)

local pendingGpuPreference = nil
local pendingLod = {}

local applyGraphicsOptions = ApplyPCGraphicsOptions
ApplyPCGraphicsOptions = function(...)
  applyGraphicsOptions(...)
  if pendingGpuPreference ~= nil then
    local preference = pendingGpuPreference
    pendingGpuPreference = nil
    game.setgpupreference(preference)
  end
  for dvar, value in pairs(pendingLod) do
    Engine.SetDvar(dvar, value)
  end
  pendingLod = {}
end

local revertGraphicsOptions = CoD.PCUtil.RevertUnsavedGraphicsOptions
CoD.PCUtil.RevertUnsavedGraphicsOptions = function(...)
  pendingGpuPreference = nil
  pendingLod = {}
  return revertGraphicsOptions(...)
end

DataSources.OptionGraphicsVideo = DataSourceHelpers.ListSetup("PC.OptionGraphicsVideo", function(controller)
  local videoSettings = {}
  table.insert(videoSettings, {
    models = {
      label = "PLATFORM_DISPLAY_MODE",
      description = "PLATFORM_DISPLAY_MODE_DESC",
      profileVarName = "r_fullscreen",
      datasource = "OptionPCGraphicsDisplayMode",
      widgetType = "dropdown",
    },
    properties = CoD.PCUtil.DependantDropdownProperties,
  })
  table.insert(videoSettings, {
    models = {
      label = "PLATFORM_MONITOR",
      description = "PLATFORM_MONITOR_DESC",
      profileVarName = "r_monitor",
      datasource = "OptionPCGraphicsMonitor",
      widgetType = "dropdown",
      disabledFunction = function()
        return Engine.GetHardwareProfileValueAsString("r_fullscreen") == "0"
      end,
    },
    properties = CoD.PCUtil.DependantDropdownProperties,
  })
  table.insert(videoSettings, {
    models = {
      label = "Graphics Card",
      description = "Which graphics card runs the game. Pick High Performance if the game lags on a laptop. Needs a game restart to apply.",
      profileVarName = "gpu_preference",
      profileType = "function",
      getFunction = function()
        return pendingGpuPreference or game.getgpupreference()
      end,
      setFunction = function(_, value)
        pendingGpuPreference = value
      end,
      datasource = "OptionPCGraphicsCard",
      widgetType = "dropdown",
    },
    properties = CoD.PCUtil.OptionsGenericDropdownProperties,
  })
  table.insert(videoSettings, {
    models = {
      label = "PLATFORM_SCREEN_RESOLUTION",
      description = "PLATFORM_SCREEN_RESOLUTION_DESC",
      profileVarName = "r_mode",
      datasource = "OptionPCGraphicsScreenResolution",
      widgetType = "dropdown",
      disabledFunction = function()
        return Engine.GetHardwareProfileValueAsString("r_fullscreen") == "2"
      end,
    },
    properties = CoD.PCUtil.DependantDropdownProperties,
  })
  table.insert(videoSettings, {
    models = {
      label = "PLATFORM_REFRESH_RATE",
      description = "PLATFORM_REFRESH_RATE_DESC",
      profileVarName = "r_refreshRate",
      datasource = "OptionPCGraphicsRefreshRate",
      widgetType = "dropdown",
      disabledFunction = function()
        return Engine.GetHardwareProfileValueAsString("r_fullscreen") ~= "1"
      end,
    },
    properties = CoD.PCUtil.OptionsGenericDropdownProperties,
  })
  table.insert(videoSettings, {
    models = {
      label = "PLATFORM_SCENE_RESOLUTION_RATIO",
      description = "PLATFORM_SCENE_RESOLUTION_RATIO_DESC",
      profileVarName = "r_sceneResolutionMultiplier",
      datasource = "OptionPCGraphicsSceneResolution",
      widgetType = "dropdown",
    },
    properties = CoD.PCUtil.DependantDropdownProperties,
  })
  table.insert(videoSettings, {
    models = {
      label = "PLATFORM_DISPLAY_GAMMA",
      description = "PLATFORM_DISPLAY_GAMMA_DESC",
      profileVarName = "r_videoMode",
      datasource = "OptionPCGraphicsDisplayGamma",
      widgetType = "dropdown",
    },
    properties = CoD.PCUtil.OptionsGenericDropdownProperties,
  })
  table.insert(videoSettings, {
    models = {
      label = "PLATFORM_BRIGHTNESS",
      description = "PLATFORM_BRIGHTNESS_DESC",
      profileVarName = "r_sceneBrightness",
      profileType = "user",
      lowValue = -1,
      highValue = 1,
      widgetType = "slider",
    },
    properties = CoD.PCUtil.OptionsGenericSliderProperties,
  })
  table.insert(videoSettings, {
    models = {
      label = "PLATFORM_FOV",
      description = "PLATFORM_FOV_DESC",
      profileVarName = "cg_fov_default",
      lowValue = 65,
      highValue = 120,
      useIntegerDisplay = 1,
      widgetType = "slider",
    },
    properties = CoD.PCUtil.OptionsGenericSliderProperties,
  })
  table.insert(videoSettings, {
    models = {
      widgetType = "spacer",
      height = 32,
    },
  })
  table.insert(videoSettings, {
    models = {
      label = "PLATFORM_MAX_FPS",
      description = "PLATFORM_MAX_FPS_DESC",
      profileVarName = "com_maxfps",
      lowValue = 0,
      highValue = 500,
      useIntegerDisplay = 1,
      widgetType = "slider",
    },
    properties = CoD.PCUtil.OptionsGenericSliderProperties,
  })
  table.insert(videoSettings, {
    models = {
      label = "PLATFORM_SYNC_EVERY_FRAME",
      description = "PLATFORM_VSYNC_DESC",
      profileVarName = "r_vsync",
      widgetType = "checkbox",
    },
    properties = CoD.PCUtil.OptionsGenericCheckboxProperties,
  })
  table.insert(videoSettings, {
    models = {
      label = "PLATFORM_DRAW_FPS",
      description = "PLATFORM_DRAW_FPS_DESC",
      profileVarName = "com_drawFPS_PC",
      widgetType = "checkbox",
    },
    properties = CoD.PCUtil.OptionsGenericCheckboxProperties,
  })
  return videoSettings
end, true)
DataSources.OptionGraphicsVideo.getWidgetTypeForItem = function(list, dataItemModel, row)
  if dataItemModel then
    local widgetType = Engine.GetModelValue(Engine.GetModel(dataItemModel, "widgetType"))
    if widgetType == "dropdown" then
      return CoD.OptionDropdown
    elseif widgetType == "checkbox" then
      return CoD.StartMenu_Options_CheckBoxOption
    elseif widgetType == "slider" then
      return CoD.StartMenu_Options_SliderBar
    elseif widgetType == "spacer" then
      return CoD.VerticalListSpacer
    end
  end
  return nil
end

require("ui.uieditor.widgets.PC.StartMenu.GraphicsOptions.StartMenu_Options_PC_Graphics_Video")

DataSources.OptionGraphicsDetail = DataSourceHelpers.ListSetup("PC.OptionGraphicsDetail", function()
  local items = {}
  for _, setting in ipairs(BoiiiLodSettings) do
    local datasource = "BoiiiLod_" .. setting.dvar
    DataSources[datasource] = DataSourceHelpers.ListSetup("PC." .. datasource, function()
      local values = {}
      for _, option in ipairs(setting.options) do
        table.insert(values, { models = { value = option.value, valueDisplay = option.option } })
      end
      return values
    end, true)
    table.insert(items, {
      models = {
        label = setting.name,
        description = setting.description,
        profileVarName = setting.dvar,
        profileType = "function",
        getFunction = function()
          return pendingLod[setting.dvar] or Engine.DvarInt(nil, setting.dvar)
        end,
        setFunction = function(_, value)
          pendingLod[setting.dvar] = value
        end,
        datasource = datasource,
        widgetType = "dropdown",
      },
      properties = CoD.PCUtil.OptionsGenericDropdownProperties,
    })
  end
  return items
end, true)
DataSources.OptionGraphicsDetail.getWidgetTypeForItem = DataSources.OptionGraphicsVideo.getWidgetTypeForItem

CoD.StartMenu_Options_PC_Graphics_Detail = InheritFrom(LUI.UIElement)
CoD.StartMenu_Options_PC_Graphics_Detail.new = function(menu, controller)
  local self = CoD.StartMenu_Options_PC_Graphics_Video.new(menu, controller)
  self.id = "StartMenu_Options_PC_Graphics_Detail"
  self.graphicsList:setDataSource("OptionGraphicsDetail")
  return self
end

DataSources.OptionGraphicsCategories = DataSourceHelpers.ListSetup("PC.OptionGraphicsCategories", function()
  local function tab(name, widget)
    return { models = { tabName = name, tabWidget = widget } }
  end
  return {
    { models = { tabIcon = CoD.buttonStrings.shoulderl }, properties = { m_mouseDisabled = true } },
    tab("PLATFORM_VIDEO_CAPS", "CoD.StartMenu_Options_PC_Graphics_Video"),
    tab("PLATFORM_ADVANCED_CAPS", "CoD.StartMenu_Options_PC_Graphics_Advanced"),
    tab("DETAIL", "CoD.StartMenu_Options_PC_Graphics_Detail"),
    tab("MENU_GAME_CAPS", "CoD.StartMenu_Options_PC_Graphics_Game"),
    { models = { tabIcon = CoD.buttonStrings.shoulderr }, properties = { m_mouseDisabled = true } },
  }
end, true)
